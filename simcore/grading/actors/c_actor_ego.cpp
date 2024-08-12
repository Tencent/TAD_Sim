// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_ego.h"
#include "common/coord_trans.h"
#include "control.pb.h"
#include "location.pb.h"
#include "manager/actor_manager.h"
#include "manager/msg_manager.h"
#include "utils/detection.h"
#include "utils/eval_types.h"
#include "utils/proto_helper.h"
#include "vehicle_geometry.pb.h"

namespace std {
template <>
struct hash<hadmap::OBJECT_TYPE> {
  size_t operator()(const hadmap::OBJECT_TYPE &obj_type) const {
    // 提供一个简单的哈希函数，直接将枚举值转换为size_t类型
    return static_cast<size_t>(obj_type);
  }
};
}  // namespace std

namespace eval {
/*==========================*/
CEgoActor::CEgoActor() {
  for (auto i = 0; i < 4; ++i) m_wheel_pos.push_back(Eigen::Vector3d());
}

CEgoActor::~CEgoActor() {}

// get if actor on solid boundry
bool CEgoActor::IsOnSolidBoundry(LaneBoundry &boundry_on) {
  if (m_on_solid_boundry) boundry_on.CopyInfoFrom(m_solid_boundry);
  return m_on_solid_boundry;
}

bool CEgoActor::IsOnSolidSolidBoundry(LaneBoundry &boundry_on) {
  if (m_on_solid_solid_boundry) boundry_on.CopyInfoFrom(m_solid_solid_boundry);
  return m_on_solid_solid_boundry;
}

// get if actor on dotted boundry
bool CEgoActor::IsOnDottedBoundry(LaneBoundry &boundry_on) {
  if (m_on_dotted_boundry) boundry_on.CopyInfoFrom(m_dotted_boundry);
  return m_on_dotted_boundry;
}
// get if actor on stop line
bool CEgoActor::IsOnStopLine(StopLine &stop_line_on) {
  if (m_on_stop_line) stop_line_on.CopyInfoFrom(m_stopline);
  return m_on_stop_line;
}
// get if actor on crosswalk
bool CEgoActor::IsOnCrosswalk(Crosswalk &crosswalk_on) {
  if (m_on_crosswalk) crosswalk_on.CopyInfoFrom(m_crosswalk);
  return m_on_crosswalk;
}

// get if lane num increasing
bool CEgoActor::IsLaneNumIncreasing(double radius) {
  if (radius <= 0.0) return m_laneNum_Inc;
  return m_laneNum_Inc && m_ref_line_max_distance > 0.0 && m_ref_line_max_distance <= radius;
}

// get if lane num decreasing
bool CEgoActor::IsLaneNumDecreasing(double radius) {
  if (radius <= 0.0) return m_laneNum_Dec;
  return m_laneNum_Dec && m_ref_line_max_distance > 0.0 && m_ref_line_max_distance <= radius;
}

// check if number of lanes in front of ego is decreasing
void CEgoActor::CheckLaneNumChange() {
  // default state
  m_laneNum_Dec = false;
  m_laneNum_Inc = false;

  // get map info
  ActorMapInfoPtr map_info = m_map_agent.GetMapInfo();

  // check
  if (map_info.get() == nullptr) {
    return;
  }

  // check lane number
  if (!map_info->m_on_road) {
    return;
  }

  // check
  m_laneNum_Inc = map_info->m_active_lane.m_lane_num_increasing;
  m_laneNum_Dec = map_info->m_active_lane.m_lane_num_decreasing;
}

// Calculate vehicle speed distance vector
void CEgoActor::CalSpeedDistanceYaw() {
  if (m_previous_frame_location == nullptr) {
    m_previous_frame_location = std::make_shared<CLocation>(GetLocation());
    m_speed_yaw = -1;
  } else {
    double cur_x = GetLocation().GetPosition().GetX();
    double cur_y = GetLocation().GetPosition().GetY();
    double cur_z = GetLocation().GetPosition().GetZ();
    double pre_x = m_previous_frame_location->GetPosition().GetX();
    double pre_y = m_previous_frame_location->GetPosition().GetY();
    double pre_z = m_previous_frame_location->GetPosition().GetZ();
    if (cur_x == pre_x && cur_y == pre_y && cur_z == pre_z) {
      return;
    }
    CPosition speed_vec = CEvalMath::Sub(GetLocation().GetPosition(), m_previous_frame_location->GetPosition());
    *m_previous_frame_location = GetLocation();

    m_speed_yaw = std::atan2(speed_vec.GetY(), speed_vec.GetX());
    EvalMapObjectPtr obj = std::make_shared<EvalMapObject>();
    VLOG_2 << "[find indicate sign:] " << (GetIndicateSign(obj) ? "true\n" : "false\n");
  }
}

// calculate actor lateral offset from reference line
void CEgoActor::CalRefLineLateralOffset() {
  m_ref_line_lateral_offset = 0.0;

  ActorMapInfoPtr map_info = m_map_agent.GetMapInfo();

  if (map_info && map_info->m_on_road) {
    const eval::EvalPoints &sample_points = map_info->m_active_lane.m_ref_line.sample_points;
    int index = CEvalMath::GetNearestPointIndex(sample_points, GetLocation());
    if (0 <= index && index < sample_points.size()) {
      m_ref_line_lateral_offset = CEvalMath::GetLateralDist(sample_points.at(index), GetLocation());
    } else {
      LOG_ERROR << "eval | ego is too farway from ref line.\n";
    }
  }
}

// cal state of ego wheels with stop line
void CEgoActor::CalStateWihtStopLine() {
  m_state_with_stopline = EgoFarFromStopLine;
  const RectCorners &wheels_corners_enu = GetWheelPosENU();
  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road) {
    EvalMapObjectPtr &&stop_objects = _map_mgr->GetMapObjects(hadmap::OBJECT_TYPE_Stop, GetLocation().GetPosition());
    if (stop_objects) {
      double road_len = map_info->m_active_road.m_road_length;
      hadmap::txObjects m_stopline_objs = stop_objects->m_map_objects;
      const hadmap::txLineCurve *pLine =
          dynamic_cast<const hadmap::txLineCurve *>(map_info->m_active_road.m_road->getGeometry());
      double ego_f_s = 0, ego_f_l = 0, ego_f_yaw = 0, ego_r_s = 0, ego_r_l = 0, ego_r_yaw = 0;
      const RectCorners &corners_enu = GetWheelPosENU();

      pLine->xy2sl((corners_enu[0][0] + corners_enu[1][0]) / 2, (corners_enu[0][1] + corners_enu[1][1]) / 2, ego_f_s,
                   ego_f_l, ego_f_yaw);
      pLine->xy2sl((corners_enu[2][0] + corners_enu[3][0]) / 2, (corners_enu[2][1] + corners_enu[3][1]) / 2, ego_r_s,
                   ego_r_l, ego_r_yaw);

      for (auto i = 0; i < m_stopline_objs.size(); ++i) {
        auto stopline = m_stopline_objs.at(i);
        if (stopline->getRoadId() != GetLaneID().tx_road_id) continue;
        double s = 0, t = 0;
        stopline->getST(s, t);
        if (abs(s / road_len) > 0.5) {
          // local side
          m_state_with_stopline = EgoFrontOfLocalSideStopLine;
          if (ego_f_s >= s && ego_r_s <= s) {
            m_state_with_stopline = EgoOnLocalSideStopLine;
          }
        } else {
          // oppsite side
          m_state_with_stopline = EgoBehindOppositeSideStopLine;
          if (ego_f_s <= s && ego_r_s >= s) {
            m_state_with_stopline = EgoOnOppositeSideStopLine;
          }
        }
      }
      // CheckDistanceFromStopLine();
    }

  } else {
    m_state_with_stopline = EgoInJunction;
  }
}

// calculate actor lateral yaw from reference line
void CEgoActor::CalRefLineLateralYaw() {
  m_ref_line_lateral_yaw = 0.0;

  ActorMapInfoPtr map_info = m_map_agent.GetMapInfo();

  if (map_info && map_info->m_on_road) {
    const eval::EvalPoints &sample_points = map_info->m_active_lane.m_ref_line.sample_points;
    int index = CEvalMath::GetNearestPointIndex(sample_points, GetLocation());
    if (0 <= index && index < sample_points.size()) {
      m_ref_line_lateral_yaw = CEvalMath::GetLateralYaw(sample_points.at(index), GetLocation());
    } else {
      LOG_ERROR << "eval | ego is too farway from ref line.\n";
    }
  }
}

// calculate max distance from ego to the furthest point of reference line
void CEgoActor::CalRefLineMaxDistance() {
  m_ref_line_max_distance = 0.0;

  ActorMapInfoPtr map_info = m_map_agent.GetMapInfo();

  if (map_info && map_info->m_on_road) {
    const eval::EvalPoints &sample_points = map_info->m_active_lane.m_ref_line.sample_points;
    double max_dist = -1.0, tmp_dist = 0.0;
    // the farthest point in the forward direction is usually at the back
    for (int i = static_cast<int>(sample_points.size()) - 1; i >= 0; --i) {
      double tmp_dist = CEvalMath::AbsoluteDistance2D(sample_points.at(i).GetPosition(), GetLocation().GetPosition());
      if (tmp_dist > max_dist) max_dist = tmp_dist;
      if (tmp_dist < 0.5) break;  // break while too close. points behind will no longer be in the forward direction.
    }
    if (max_dist >= 0.0) {
      m_ref_line_max_distance = max_dist;
    } else {
      LOG_ERROR << "ego or ref line illegal.\n";
    }
  }
}

// test if actor on solid/dotted line
void CEgoActor::CheckActorOnSolidOrDottedLine() {
  // default
  m_on_dotted_boundry = false;
  m_on_solid_boundry = false;
  m_on_solid_solid_boundry = false;

  bool on_line = false;
  auto map_info = m_map_agent.GetMapInfo();

  if (map_info && map_info->m_on_road) {
    // copy boundry pointer
    std::vector<LaneBoundry *> boundries = {&(map_info->m_active_lane.m_left_boundry),
                                            &(map_info->m_active_lane.m_right_boundry)};

    // calculate AABB of actor
    RectCorners &&ego_corners = TransCorners2BaseCoord();
    CPosition bl, tr;
    CollisionDetection::CalAABB(ego_corners, bl, tr);

    for (auto i = 0; i < boundries.size(); ++i) {
      const eval::LaneBoundry &boundry = *boundries.at(i);
      VLOG_2 << "boundry_pkid::" << boundry.boundry_pkid << ", is left boudry:" << boundry.left_boundary
             << ", lane mark type:" << boundry.lane_mark_type << "\n";

      for (auto line_point : boundry.sample_points) {
        double &&abs_dist = CEvalMath::AbsoluteDistance2D(line_point.GetPosition(), GetLocation().GetPosition());
        if (abs_dist >= GetBaseLength()) {
          continue;
        }
        if (!CollisionDetection::IsInsideAABB(line_point.GetPosition(), bl, tr)) continue;
        if (CollisionDetection::IsInsideRect(line_point.GetPosition(), ego_corners)) {
          on_line = true;
          break;
        }
      }

      if (on_line) {
        if (boundry.lane_mark_type & hadmap::LANE_MARK_Solid) {
          m_on_solid_boundry = true;
          m_solid_boundry.CopyInfoFrom(boundry);
        }
        if (boundry.lane_mark_type & hadmap::LANE_MARK_SolidSolid) {
          m_on_solid_solid_boundry = true;
          m_solid_solid_boundry.CopyInfoFrom(boundry);
        }
        if (boundry.lane_mark_type & hadmap::LANE_MARK_Broken) {
          m_on_dotted_boundry = true;
          m_dotted_boundry.CopyInfoFrom(boundry);
        }
        break;
      }
    }
  }
}

// test if actor on one specific stop line
void CEgoActor::CheckActorOnStopLine() {
  // default

  m_on_stop_line = false;
  m_stop_line_s_dist = -1.0;
  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road && _map_mgr) {
    EvalMapObjectPtr &&stop_objects =
        _map_mgr->GetMapObjects(hadmap::OBJECT_TYPE_Stop, GetLocation().GetPosition(), 35.0);
    if (stop_objects) {
      double road_len = map_info->m_active_road.m_road_length;
      hadmap::txObjects m_stopline_objs = stop_objects->m_map_objects;
      const hadmap::txLineCurve *pLine =
          dynamic_cast<const hadmap::txLineCurve *>(map_info->m_active_road.m_road->getGeometry());
      double ego_f_s = 0, ego_f_l = 0, ego_f_yaw = 0, ego_r_s = 0, ego_r_l = 0, ego_r_yaw = 0;

      CPosition ego_milldle_front = TransMiddleFront2BaseCoord();
      CPosition ego_milldle_rear = TransMiddleRear2BaseCoord();

      _map_mgr->ENUToWGS84(ego_milldle_front);
      _map_mgr->ENUToWGS84(ego_milldle_rear);

      pLine->xy2sl(ego_milldle_front.GetX(), ego_milldle_front.GetY(), ego_f_s, ego_f_l, ego_f_yaw);
      pLine->xy2sl(ego_milldle_rear.GetX(), ego_milldle_rear.GetY(), ego_r_s, ego_r_l, ego_r_yaw);
      for (auto i = 0; i < m_stopline_objs.size(); ++i) {
        auto stopline = m_stopline_objs.at(i);
        if (stopline->getRoadId() != GetLaneID().tx_road_id) continue;
        double s = 0, t = 0, y = 0;
        // stopline->getST(s, t);
        hadmap::txPoint sl_point = stopline->getPos();
        pLine->xy2sl(sl_point.x, sl_point.y, s, t, y);

        double stopline_l = 0, stopline_h = 0, stopline_w = 0;
        stopline->getLWH(stopline_l, stopline_w, stopline_h);
        if (ego_f_s <= s + stopline_w / 2) m_stop_line_s_dist = (s + stopline_w / 2 - ego_f_s);
        VLOG_2 << "ego_f_s: " << ego_f_s << ", ego_r_s: " << ego_r_s << ", s: " << s << ", stopline_l: " << stopline_l
               << ", stopline_w: " << stopline_w << ", stopline_h: " << stopline_h
               << ", m_stop_line_s_dist: " << m_stop_line_s_dist << "\n";
        if (ego_f_s >= s + stopline_w / 2 && ego_r_s <= s + stopline_w / 2) {
          m_on_stop_line = true;
        }
      }
    }
  }
  VLOG_2 << "[CheckActorOnStopLine] call end!  and the ans is : [" << (m_on_stop_line ? "true]\n" : "false]\n");
}

// test if actor on one specific crosswolk
void CEgoActor::CheckActorOnCrosswalk() {
  // default
  m_on_crosswalk = false;

  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road && _map_mgr) {
    EvalMapObjectPtr crosswalk_objects =
        _map_mgr->GetMapObjects(hadmap::OBJECT_TYPE_CrossWalk, GetLocation().GetPosition());
    if (crosswalk_objects) {
      const hadmap::txLineCurve *pLine =
          dynamic_cast<const hadmap::txLineCurve *>(map_info->m_active_road.m_road->getGeometry());
      double ego_f_s = 0.0, ego_f_l = 0.0, ego_f_yaw = 0.0, ego_r_s = 0.0, ego_r_l = 0.0, ego_r_yaw = 0.0;
      CPosition ego_milldle_front = TransMiddleFront2BaseCoord();
      CPosition ego_milldle_rear = TransMiddleRear2BaseCoord();
      _map_mgr->ENUToWGS84(ego_milldle_front);
      _map_mgr->ENUToWGS84(ego_milldle_rear);
      pLine->xy2sl(ego_milldle_front.GetX(), ego_milldle_front.GetY(), ego_f_s, ego_f_l, ego_f_yaw);
      pLine->xy2sl(ego_milldle_rear.GetX(), ego_milldle_rear.GetY(), ego_r_s, ego_r_l, ego_r_yaw);

      // VLOG_2 << "crosswalk_objects->m_map_objects.size(): " << crosswalk_objects->m_map_objects.size() << "\n";
      for (auto i = 0; i < crosswalk_objects->m_map_objects.size(); ++i) {
        const hadmap::txObjectPtr map_obj = crosswalk_objects->m_map_objects.at(i);
        if (map_obj->getRoadId() != GetLaneID().tx_road_id) continue;
        double s = 0.0, t = 0.0, y = 0.0;
        hadmap::txPoint sl_point = map_obj->getPos();
        pLine->xy2sl(sl_point.x, sl_point.y, s, t, y);
        double crosswalk_l = 0.0, crosswalk_h = 0.0, crosswalk_w = 0.0;
        map_obj->getLWH(crosswalk_l, crosswalk_w, crosswalk_h);
        VLOG_2 << "ego_f_s: " << ego_f_s << ", ego_r_s: " << ego_r_s << ", s: " << s << ", crosswalk_l: " << crosswalk_l
               << ", crosswalk_w: " << crosswalk_w << ", crosswalk_h: " << crosswalk_h << "\n";

        if (ego_f_s >= s && ego_r_s <= s + crosswalk_w) {
          m_on_crosswalk = true;
          m_crosswalk.object_pkid = map_obj->getId();
          m_crosswalk.sample_points = crosswalk_objects->m_samples.at(i);
          VLOG_2 << "m_crosswalk.object_pkid: " << m_crosswalk.object_pkid << "\n";
        }
        if (m_on_crosswalk) break;
      }
    }
  }
}

EvalMapObjectPtr CEgoActor::GetSpecificTypesOfObjects(const hadmap::OBJECT_TYPE &obj_type, double radius) {
  auto map_info = m_map_agent.GetMapInfo();
  EvalMapObjectPtr objects;
  if (map_info && map_info->m_on_road && _map_mgr) {
    objects = _map_mgr->GetMapObjects(obj_type, GetLocation().GetPosition(), radius);
  }
  VLOG_2 << "[ GetSpecificTypesOfObjects ] find object: [ " << (objects == nullptr ? -1 : objects->m_map_objects.size())
         << " ]\n";
  return objects;
}

bool CEgoActor::CurLaneIsRightmostLane() {
  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road) {
    return map_info->m_active_lane.m_right_lane == nullptr;
  }
  return false;
}

bool CEgoActor::CurLaneIsLeftmostLane() {
  auto map_info = m_map_agent.GetMapInfo();

  if (map_info && map_info->m_on_road) {
    return map_info->m_active_lane.m_left_lane == nullptr;
  }
  return false;
}

bool CEgoActor::CheckWheelWithinTheLaneBoundry() {
  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road) {
    auto IsBoundryInWheelPos = [](const eval::LaneBoundry &boundry, const RectCorners &wheel_pos) {
      if (boundry.sample_points.empty()) return true;  // return true if sample_points empty
      for (int j = 0; j < boundry.sample_points.size(); ++j) {
        const CLocation &loc = boundry.sample_points.at(j);
        if (CollisionDetection::IsInsideRect(loc.GetPosition(), wheel_pos)) {
          return true;
        }
      }
      return false;
    };

    VLOG_3 << "m_wheel_pos[0]: (" << m_wheel_pos[0][0] << ", " << m_wheel_pos[0][1] << ", " << m_wheel_pos[0][2]
           << "); m_wheel_pos[1]: (" << m_wheel_pos[1][0] << ", " << m_wheel_pos[1][1] << ", " << m_wheel_pos[1][2]
           << "); m_wheel_pos[2]: (" << m_wheel_pos[2][0] << ", " << m_wheel_pos[2][1] << ", " << m_wheel_pos[2][2]
           << "); m_wheel_pos[3]: (" << m_wheel_pos[3][0] << ", " << m_wheel_pos[3][1] << ", " << m_wheel_pos[3][2]
           << ")\n";

    bool is_on_cur_lane_left_bdr = IsBoundryInWheelPos(map_info->m_active_lane.m_left_boundry, m_wheel_pos);
    bool is_on_cur_lane_right_bdr = IsBoundryInWheelPos(map_info->m_active_lane.m_right_boundry, m_wheel_pos);
    VLOG_2 << "check wheels is_on_cur_lane_left_bdr: " << is_on_cur_lane_left_bdr
           << ", is_on_cur_lane_right_bdr: " << is_on_cur_lane_right_bdr << "\n";
    if (!is_on_cur_lane_left_bdr && !is_on_cur_lane_right_bdr) {  // return early to save computation
      return true;
    }

    bool is_on_left_lane_left_bdr = IsBoundryInWheelPos(map_info->m_active_lane.m_left_lane_left_boundry, m_wheel_pos);
    bool is_on_right_lane_right_bdr =
        IsBoundryInWheelPos(map_info->m_active_lane.m_right_lane_right_boundry, m_wheel_pos);
    VLOG_2 << "is_on_left_lane_left_bdr: " << is_on_left_lane_left_bdr
           << ", is_on_right_lane_right_bdr: " << is_on_right_lane_right_bdr << "\n";
    if (is_on_cur_lane_left_bdr && is_on_left_lane_left_bdr || is_on_cur_lane_right_bdr && is_on_right_lane_right_bdr) {
      return false;
    }
  } else {  // false if map_info null or m_on_road false
    return false;
  }
  return true;
}

bool CEgoActor::CheckCornerWithinTheLaneBoundry() {
  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road) {
    auto IsBoundryInCorner = [](const eval::LaneBoundry &boundry, const CPosition &bl, const CPosition &tr) {
      if (boundry.sample_points.empty()) return true;  // return true if sample_points empty
      for (int j = 0; j < boundry.sample_points.size(); ++j) {
        const CLocation &loc = boundry.sample_points.at(j);
        if (CollisionDetection::IsInsideAABB(loc.GetPosition(), bl, tr)) {
          return true;
        }
      }
      return false;
    };

    CPosition bl, tr;
    CollisionDetection::CalAABB(TransCorners2BaseCoord(), bl, tr);
    VLOG_3 << "bl: (" << bl.GetX() << ", " << bl.GetY() << ", " << bl.GetZ() << "); tr: (" << tr.GetX() << ", "
           << tr.GetY() << ", " << tr.GetZ() << ")\n";

    bool is_on_cur_lane_left_bdr = IsBoundryInCorner(map_info->m_active_lane.m_left_boundry, bl, tr);
    bool is_on_cur_lane_right_bdr = IsBoundryInCorner(map_info->m_active_lane.m_right_boundry, bl, tr);
    VLOG_2 << "check corners is_on_cur_lane_left_bdr: " << is_on_cur_lane_left_bdr
           << ", is_on_cur_lane_right_bdr: " << is_on_cur_lane_right_bdr << "\n";
    if (!is_on_cur_lane_left_bdr && !is_on_cur_lane_right_bdr) {  // return early to save computation
      return true;
    }

    bool is_on_left_lane_left_bdr = IsBoundryInCorner(map_info->m_active_lane.m_left_lane_left_boundry, bl, tr);
    bool is_on_right_lane_right_bdr = IsBoundryInCorner(map_info->m_active_lane.m_right_lane_right_boundry, bl, tr);
    VLOG_2 << "is_on_left_lane_left_bdr: " << is_on_left_lane_left_bdr
           << ", is_on_right_lane_right_bdr: " << is_on_right_lane_right_bdr << "\n";
    if (is_on_cur_lane_left_bdr && is_on_left_lane_left_bdr || is_on_cur_lane_right_bdr && is_on_right_lane_right_bdr) {
      return false;
    }
  } else {  // false if map_info null or m_on_road false
    return false;
  }
  return true;
}

// 0 --> error 1 --> front 2 ---> rear
uint8_t CEgoActor::CheckFrontOrRear(const eval::CLocation &loc) {
  CLocation obj_loc(loc);
  if (obj_loc.GetPosition().GetCoordType() == hadmap::COORD_WGS84) {
    CPosition tmp(obj_loc.GetPosition());
    _map_mgr->WGS84ToENU(tmp);
    obj_loc.MutablePosition()->SetValues(tmp.GetX(), tmp.GetY(), tmp.GetZ());
  }

  eval::CLocation new_obj_loc = CEvalMath::CalLocationXB(this->GetLocation(), obj_loc);

  return (new_obj_loc.GetPosition().GetX() - GetShape().GetLength() / 2 <= 0) ? 2 : 1;
}

eval::CLocation CEgoActor::GetObjectCLocation(const hadmap::txObjectPtr &obj) {
  hadmap::txPoint obj_point = obj->getPos();

  CPosition obj_pos_enu = CPosition(obj_point.x, obj_point.y, obj_point.z, Coord_WGS84);

  _map_mgr->WGS84ToENU(obj_pos_enu);

  double r = 0, p = 0, y = 0;
  obj->getRPY(r, p, y);
  return eval::CLocation(obj_pos_enu.GetX(), obj_pos_enu.GetY(), obj_pos_enu.GetZ(), r, p, y);
}

bool CEgoActor::GetSign(EvalMapObjectPtr obj, const std::set<hadmap::OBJECT_SUB_TYPE> &sub_type_set,
                        const hadmap::OBJECT_TYPE &obj_type, double radius, uint8_t front_or_rear) {
  auto objects = GetSpecificTypesOfObjects(obj_type, radius);
  bool res = false;
  // hadmap::OBJECT_TYPE m_object_type;
  // hadmap::txObjects m_map_objects;
  // std::vector<eval::EvalPoints> m_samples;
  if (obj == nullptr) {
    VLOG_2 << "[ GetSign ] obj is nullptr";
    return false;
  }
  if (objects) {
    obj->Clear();
    obj->m_object_type = obj_type;

    hadmap::txObjects signs = objects->m_map_objects;
    VLOG_2 << "the signs.size() : " << signs.size() << "\n";

    for (int i = 0; i < signs.size(); i++) {
      const auto sign = signs.at(i);

      eval::CLocation sign_location = GetObjectCLocation(sign);

      if (CheckFrontOrRear(sign_location) == front_or_rear &&
          sub_type_set.find(sign->getObjectSubType()) != sub_type_set.end() &&
          (sign->getOdData().roadid == GetLaneID().tx_road_id || GetMapInfo()->m_in_junction)) {
        res = true;
        (obj->m_map_objects).push_back(sign);
        (obj->m_samples).push_back(eval::EvalPoints());
      }
    }
  }
  return res;
}

bool CEgoActor::GetSpeedLimitSign(EvalMapObjectPtr obj, double radius) {
  std::set<hadmap::OBJECT_SUB_TYPE> speed_limit_signs_set = {hadmap::SIGN_BAN_SPPED_120,
                                                             hadmap::SIGN_BAN_SPPED_100,
                                                             hadmap::SIGN_BAN_SPPED_80,
                                                             hadmap::SIGN_BAN_SPPED_70,
                                                             hadmap::SIGN_BAN_SPPED_60,
                                                             hadmap::SIGN_BAN_SPPED_50,
                                                             hadmap::SIGN_BAN_SPPED_40,
                                                             hadmap::SIGN_BAN_SPPED_30,
                                                             hadmap::SIGN_BAN_SPPED_20,
                                                             hadmap::SIGN_BAN_SPPED_05,
                                                             hadmap::SIGN_INDOCATION_LOWEST_SPEED_40,
                                                             hadmap::SIGN_INDOCATION_LOWEST_SPEED_50,
                                                             hadmap::SIGN_INDOCATION_LOWEST_SPEED_60};
  return GetSign(obj, speed_limit_signs_set, hadmap::OBJECT_TYPE_TrafficSign, radius);
}

bool CEgoActor::GetIndicateSign(EvalMapObjectPtr obj, double radius) {
  std::set<hadmap::OBJECT_SUB_TYPE> indicate_signs_set = {hadmap::SIGN_INDOCATION_STRAIGHT,
                                                          hadmap::SIGN_INDOCATION_LOWEST_SPEED_60,
                                                          hadmap::SIGN_INDOCATION_LOWEST_SPEED_40,
                                                          hadmap::SIGN_INDOCATION_ALONG_RIGHT,
                                                          hadmap::SIGN_INDOCATION_PEDESTRIAN_CROSSING,
                                                          hadmap::SIGN_INDOCATION_TURN_RIGHT,
                                                          hadmap::SIGN_INDOCATION_ROUNDABOUT,
                                                          hadmap::SIGN_INDOCATION_TURN_LEFT,
                                                          hadmap::SIGN_INDOCATION_STRAIGHT_RIGHT,
                                                          hadmap::SIGN_INDOCATION_STRAIGHT_LEFT,
                                                          hadmap::SIGN_INDOCATION_WALK,
                                                          hadmap::SIGN_INDOCATION_NO_MOTOR,
                                                          hadmap::SIGN_INDOCATION_MOTOR,
                                                          hadmap::SIGN_INDOCATION_ALONG_LEFT,
                                                          hadmap::SIGN_INDOCATION_PASS_STAIGHT_001,
                                                          hadmap::SIGN_INDOCATION_PASS_STAIGHT_002,
                                                          hadmap::SIGN_INDOCATION_WHISTLE,
                                                          hadmap::SIGN_INDOCATION_LEFT_AND_RIGHT,
                                                          hadmap::SIGN_INDOCATION_PARKING};
  return GetSign(obj, indicate_signs_set, hadmap::OBJECT_TYPE_TrafficSign, radius);
}

bool CEgoActor::GetWarningSign(EvalMapObjectPtr obj, double radius) {
  std::set<hadmap::OBJECT_SUB_TYPE> warning_signs_set = {hadmap::SIGN_WARNING_SLOWDOWN,
                                                         hadmap::SIGN_WARNING_TURNLEFT,
                                                         hadmap::SIGN_WARNING_TURNRIGHT,
                                                         hadmap::SIGN_WARNING_T_RIGHT,
                                                         hadmap::SIGN_WARNING_T_DOWN,
                                                         hadmap::SIGN_WARNING_CROSS,
                                                         hadmap::SIGN_WARNING_CHILD,
                                                         hadmap::SIGN_WARNING_UP,
                                                         hadmap::SIGN_WARNING_BUILDING,
                                                         hadmap::SIGN_WARNING_NARROW_LEFT,
                                                         hadmap::SIGN_WARNING_NARROW_BOTH,
                                                         hadmap::SIGN_WARNING_RAILWAY,
                                                         hadmap::SIGN_WARNING_T_LEFT,
                                                         hadmap::SIGN_WARNING_STEEP_LEFT,
                                                         hadmap::SIGN_WARNING_STEEP_RIGHT,
                                                         hadmap::SIGN_WARNING_VILLAGE,
                                                         hadmap::SIGN_WARNING_DIKE_LEFT,
                                                         hadmap::SIGN_WARNING_DIKE_RIGHT,
                                                         hadmap::SIGN_WARAING_T_CROSSED,
                                                         hadmap::SIGN_WARAING_FERRY,
                                                         hadmap::SIGN_WARAING_FALL_ROCK,
                                                         hadmap::SIGN_WARAING_REVERSE_CURVE_LEFT,
                                                         hadmap::SIGN_WARAING_REVERSE_CURVE_RIGHT,
                                                         hadmap::SIGN_WARAING_WATER_PAVEMENT,
                                                         hadmap::SIGN_WARNING_T_BOTH,
                                                         hadmap::SIGN_WARNING_JOIN_LEFT,
                                                         hadmap::SIGN_WARNING_JOIN_RIGHT,
                                                         hadmap::SIGN_WARNING_Y_LEFT,
                                                         hadmap::SIGN_WARNING_CIRCLE_CROSS,
                                                         hadmap::SIGN_WARNING_Y_RIGHT,
                                                         hadmap::SIGN_WARNING_CURVE_AHEAD,
                                                         hadmap::SIGN_WARNING_LONG_DESCENT,
                                                         hadmap::SIGN_WARNING_ROUGH_ROAD,
                                                         hadmap::SIGN_WARNING_SNOW,
                                                         hadmap::SIGN_WARNING_DISABLE,
                                                         hadmap::SIGN_WARNING_ANIMALS,
                                                         hadmap::SIGN_WARNING_ACCIDENT,
                                                         hadmap::SIGN_WARNING_TIDALBU_LANE,
                                                         hadmap::SIGN_WARNING_BAD_WEATHER,
                                                         hadmap::SIGN_WARNING_LOWLYING,
                                                         hadmap::SIGN_WARNING_HIGHLYING,
                                                         hadmap::SIGN_WARNING_DOWNHILL,
                                                         hadmap::SIGN_WARNING_QUEUESLIKELY,
                                                         hadmap::SIGN_WARNING_CROSS_PLANE,
                                                         hadmap::SIGN_WARNING_TUNNEL,
                                                         hadmap::SIGN_WARNING_TUNNEL_LIGHT,
                                                         hadmap::SIGN_WARNING_HUMPBACK_BRIDGE,
                                                         hadmap::SIGN_WARNING_NARROW_RIGHT,
                                                         hadmap::SIGN_WARNING_NON_MOTOR,
                                                         hadmap::SIGN_WARNING_SLIPPERY,
                                                         hadmap::SIGN_WARNING_TRIFFICLIGHT,
                                                         hadmap::SIGN_WARNING_DETOUR_RIGHT,
                                                         hadmap::SIGN_WARNING_NARROW_BRIDGE,
                                                         hadmap::SIGN_WARNING_KEEP_DISTANCE,
                                                         hadmap::SIGN_WARNING_MERGE_LEFT,
                                                         hadmap::SIGN_WARNING_MERGE_RIGHT,
                                                         hadmap::SIGN_WARNING_CROSSWIND,
                                                         hadmap::SIGN_WARNING_ICY_ROAD,
                                                         hadmap::SIGN_WARNING_ROCKFALL,
                                                         hadmap::SIGN_WARNING_CAUTION,
                                                         hadmap::SIGN_WARNING_FOGGY,
                                                         hadmap::SIGN_WARNING_LIVESTOCK,
                                                         hadmap::SIGN_WARNING_DETOUR_LEFT,
                                                         hadmap::SIGN_WARNING_DETOUR_BOTH,
                                                         hadmap::SIGN_WARNING_BOTHWAY,
                                                         hadmap::SIGN_WARNING_PED};
  return GetSign(obj, warning_signs_set, hadmap::OBJECT_TYPE_TrafficSign, radius);
}

bool CEgoActor::GetProhibitionSign(EvalMapObjectPtr obj, double radius) {
  std::set<hadmap::OBJECT_SUB_TYPE> prohibition_signs_set = {hadmap::SIGN_BAN_STRAIGHT,
                                                             hadmap::SIGN_BAN_VEHICLE,
                                                             hadmap::SIGN_BAN_SPPED_120,
                                                             hadmap::SIGN_BAN_SPPED_100,
                                                             hadmap::SIGN_BAN_SPPED_80,
                                                             hadmap::SIGN_BAN_SPPED_70,
                                                             hadmap::SIGN_BAN_SPPED_60,
                                                             hadmap::SIGN_BAN_SPPED_50,
                                                             hadmap::SIGN_BAN_SPPED_40,
                                                             hadmap::SIGN_BAN_SPPED_30,
                                                             hadmap::SIGN_BAN_STOP_YIELD,
                                                             hadmap::SIGN_BAN_HEIGHT_5,
                                                             hadmap::SIGN_BAN_SPPED_20,
                                                             hadmap::SIGN_BAN_SPPED_05,
                                                             hadmap::SIGN_BAN_DIVERINTO,
                                                             hadmap::SIGN_BAN_MOTOR_BIKE,
                                                             hadmap::SIGN_BAN_WEIGHT_50,
                                                             hadmap::SIGN_BAN_WEIGHT_20,
                                                             hadmap::SIGN_BAN_HONKING,
                                                             hadmap::SIGN_BAN_TRUCK,
                                                             hadmap::SIGN_BAN_WEIGHT_30,
                                                             hadmap::SIGN_BAN_WEIGHT_10,
                                                             hadmap::SIGN_BAN_TEMP_PARKING,
                                                             hadmap::SIGN_BAN_AXLE_WEIGHT_14,
                                                             hadmap::SIGN_BAN_AXLE_WEIGHT_13,
                                                             hadmap::SIGN_BAN_WEIGHT_40,
                                                             hadmap::SIGN_BAN_SLOW,
                                                             hadmap::SIGN_BAN_TURN_LEFT,
                                                             hadmap::SIGN_BAN_DANGEROUS_GOODS,
                                                             hadmap::SIGN_BAN_TRACTORS,
                                                             hadmap::SIGN_BAN_TRICYCLE,
                                                             hadmap::SIGN_BAN_MINIBUS,
                                                             hadmap::SIGN_BAN_STRAIGHT_AND_LEFT,
                                                             hadmap::SIGN_BAN_VEHICLE_BY_HUMAN,
                                                             hadmap::SIGN_BAN_TRACYCLE01_BY_HUMAN,
                                                             hadmap::SIGN_BAN_TRACYCLE02_BY_HUMAN,
                                                             hadmap::SIGN_BAN_TURN_RIGHT,
                                                             hadmap::SIGN_BAN_LEFT_AND_RIGHT,
                                                             hadmap::SIGN_BAN_STRAIGHT_AND_RIGHT,
                                                             hadmap::SIGN_BAN_GO,
                                                             hadmap::SIGN_BAN_GIVE_WAY,
                                                             hadmap::SIGN_BAN_BUS_TURN_RIGHT,
                                                             hadmap::SIGN_BAN_TRUCK_TURN_RIGHT,
                                                             hadmap::SIGN_BAN_BYCICLE_DOWN,
                                                             hadmap::SIGN_BAN_BYCICLE_UP,
                                                             hadmap::SIGN_BAN_NO_OVERTAKING,
                                                             hadmap::SIGN_BAN_BUS_TURN_LEFT,
                                                             hadmap::SIGN_BAN_OVERTAKING,
                                                             hadmap::SIGN_BAN_ANIMALS,
                                                             hadmap::SIGN_BAN_BUS,
                                                             hadmap::SIGN_BAN_ELECTRO_TRICYCLE,
                                                             hadmap::SIGN_BAN_NO_MOTOR,
                                                             hadmap::SIGN_BAN_TRUCK_TURN_LEFT,
                                                             hadmap::SIGN_BAN_TRAILER,
                                                             hadmap::SIGN_BAN_HUMAN,
                                                             hadmap::SIGN_BAN_THE_TWO_TYPES,
                                                             hadmap::SIGN_BAN_HEIGHT_3_5,
                                                             hadmap::SIGN_BAN_HEIGHT_3,
                                                             hadmap::SIGN_BAN_AXLE_WEIGHT_10,
                                                             hadmap::SIGN_BAN_CUSTOMS_MARK,
                                                             hadmap::SIGN_BAN_STOP,
                                                             hadmap::SIGN_BAN_LONG_PARKING,
                                                             hadmap::SIGN_BAN_REMOVE_LIMIT_40,
                                                             hadmap::SIGN_BAN_UTURN};
  return GetSign(obj, prohibition_signs_set, hadmap::OBJECT_TYPE_TrafficSign, radius);
}

bool CEgoActor::GetStopLineAndCrossWalk(EvalMapObjectPtr obj, double radius) {
  std::set<hadmap::OBJECT_SUB_TYPE> stopline_crosswalk_sign = {hadmap::CrossWalk_001, hadmap::Stop_Line};
  return GetSign(obj, stopline_crosswalk_sign, hadmap::OBJECT_TYPE_CrossWalk, radius) ||
         GetSign(obj, stopline_crosswalk_sign, hadmap::OBJECT_TYPE_Stop, radius);
}

void CEgoActor::CheckDistanceFromStopLine() { CheckActorOnStopLine(); }

bool CEgoActor::IsMotorway() {
  auto map_info = m_map_agent.GetMapInfo();
  if (map_info && map_info->m_on_road) {
    VLOG_2 << "[IsMotorway] get road type : [" << map_info->m_active_road.m_road->getRoadType() << "]" << " road id :["
           << GetMapInfo()->m_active_road.m_road->getId() << "]\n";
    return map_info->m_active_road.m_on_motorway;
  }
  return false;
}

bool CEgoActor::IsReverse() {
  CEuler speed_euler(0, 0, m_speed_yaw);
  double yawDiff = CEvalMath::YawDiff(GetLocation().GetEuler(), speed_euler);
  VLOG_2 << "ego_body_yaw: " << GetLocation().GetEuler().GetYaw() << " | ego_speed_yaw: " << speed_euler.GetYaw()
         << " | yawDiff: " << yawDiff << "\n";
  return yawDiff < -M_PI_2 || yawDiff > M_PI_2;
}

bool CEgoActor::TheObstacleIsBehindVehicle(hadmap::objectpkid obj_id, double radius) {
  std::unordered_set<hadmap::OBJECT_TYPE> obstacles_set = {
      hadmap::OBJECT_TYPE_Obstacle, hadmap::OBJECT_TYPE_ChargingPile, hadmap::OBJECT_TYPE_Tree,
      hadmap::OBJECT_TYPE_Building, hadmap::OBJECT_TYPE_BusStation};

  for (auto obj_type : obstacles_set) {
    auto objects = GetSpecificTypesOfObjects(obj_type, radius);
    if (objects) {
      hadmap::txObjects signs = objects->m_map_objects;
      // const std::vector<eval::EvalPoints> &signs_samples = objects->m_samples;
      for (int i = 0; i < signs.size(); i++) {
        auto sign = signs.at(i);
        VLOG_2 << "the object [" << sign->getId() << "] \n";
        if (sign->getId() == obj_id) {
          if (CheckFrontOrRear(GetObjectCLocation(sign)) == 2) {
            VLOG_2 << "the object [" << obj_id << "] is behind vehicle\n";
            return true;
          } else {
            VLOG_2 << "the object [" << obj_id << "] is not behind vehicle\n";
            return false;
          }
        }
      }
    }
  }
  VLOG_2 << "not find the object [" << obj_id << "]\n";
  return false;
}

bool CEgoActor::TheObstacleIsBehindVehicle(const CLocation &loc) { return CheckFrontOrRear(loc) == 2; }

bool CEgoActor::TheObstacleIsBehindVehicle(hadmap::txObjectPtr obj) {
  return CheckFrontOrRear(GetObjectCLocation(obj)) == 2;
}

bool CEgoActor::GetFrontRoadMarkings(std::vector<EvalMapObjectPtr> &obj, double radius) {
  auto map_info = m_map_agent.GetMapInfo();
  EvalMapObjectPtr tmp_obj;

  auto CheckAngle = [this](hadmap::txObjectPtr obj) -> bool {
    auto obj_loc = GetObjectCLocation(obj);
    auto obj_pos = obj_loc.GetPosition();
    const auto &posVec = CEvalMath::Sub(obj_pos, GetLocation().GetPosition());
    double yaw = std::atan2(posVec.GetY(), posVec.GetX());
    CEuler euler(0, 0, yaw);

    double yawDiff = CEvalMath::YawDiff(GetLocation().GetEuler(), euler);

    // 5 angle
    return yawDiff > -M_PI / 36 && yawDiff < M_PI / 36;
  };

  std::vector<EvalMapObjectPtr>().swap(obj);
  if (map_info && map_info->m_on_road && _map_mgr) {
    std::set<hadmap::OBJECT_TYPE> road_marking_set = {
        hadmap::OBJECT_TYPE_CrossWalk,
        hadmap::OBJECT_TYPE_Arrow,
        hadmap::OBJECT_TYPE_Stop,
    };

    for (auto obj_type : road_marking_set) {
      tmp_obj = _map_mgr->GetMapObjects(obj_type, GetLocation().GetPosition(), radius);

      double min_distance = radius;
      hadmap::txObjectPtr target_obj = nullptr;
      EvalMapObjectPtr obj_ptr = std::make_shared<EvalMapObject>();

      if (tmp_obj != nullptr) {
        // check object type
        for (auto it = tmp_obj->m_map_objects.begin(); it != tmp_obj->m_map_objects.end(); ++it) {
          // check is same lane
          if (CheckFrontOrRear(GetObjectCLocation(*it)) == 1 &&
              (*it)->getRoadId() == GetLaneID().tx_road_id) {  // front
            double obj_distance = CheckDistanceWithObject(GetObjectCLocation(*it).GetPosition());
            hadmap::OBJECT_TYPE obj_type = tmp_obj->m_object_type;
            // if it's Arrow, need checking angle
            if (min_distance > obj_distance && (obj_type != hadmap::OBJECT_TYPE_Arrow || CheckAngle(*it))) {
              VLOG_2 << "the front obj id " << (*it)->getId();
              min_distance = obj_distance;
              target_obj = *it;
            }
          }
        }
        if (target_obj != nullptr) {
          VLOG_2 << "the min distance is " << min_distance << "  and the obj id is : " << target_obj->getId() << "\n";
          obj_ptr->m_object_type = obj_type;
          obj_ptr->m_map_objects.push_back(target_obj);
          obj_ptr->m_samples.push_back(eval::EvalPoints());
          obj.push_back(obj_ptr);
        }
      }
    }
  }
  return obj.size() > 0;
}

double CEgoActor::CheckDistanceWithObject(CPosition pos) {
  if (pos.GetCoordType() == eval::Coord_WGS84) {
    _map_mgr->WGS84ToENU(pos);
  }
  return CEvalMath::Distance2D(pos, GetLocation().GetPosition());
}

hadmap::txObjectPtr CEgoActor::getClosestParkingSpace(double radius /*100*/) {
  EvalMapObjectPtr parking_space_objs = GetSpecificTypesOfObjects(hadmap::OBJECT_TYPE_ParkingSpace, radius);

  double min_distance = radius;
  hadmap::txObjectPtr res = nullptr;
  if (parking_space_objs && parking_space_objs->m_map_objects.size() > 0) {
    for (hadmap::txObjectPtr obj_ptr : parking_space_objs->m_map_objects) {
      double obj_distance = CheckDistanceWithObject(GetObjectCLocation(obj_ptr).GetPosition());
      if (min_distance >= obj_distance) {
        min_distance = obj_distance;
        res = obj_ptr;
      }
    }
    if (res) {
      VLOG_2 << "[CheckDistanceWithObject] find parking space typeid:[" << res->getObjectType()
             << "] and the distance is :[" << min_distance << "]\n";
    }
  } else {
    VLOG_2 << "[CheckDistanceWithObject] not find parking space in radius: [" << radius << "]\n";
  }

  return res;
}
/*==========================*/
CEgoAssemble::CEgoAssemble() {
  m_front = nullptr;
  m_trailers.reserve(3);
  m_trailers.clear();
}

CEgoAssemble::~CEgoAssemble() {}

void CEgoAssemble::SetFront(CEgoActorPtr front) { m_front = front; }

void CEgoAssemble::AddTrailer(CEgoActorPtr trailer) {
  if (trailer) m_trailers.push_back(trailer);
}
// if has trailers
bool CEgoAssemble::HasTrailer() { return m_trailers.size() > 0 && m_trailers.at(0) != nullptr; }

// get vehicle body command
VehicleBodyControl CEgoAssemble::GetVehicleBodyCmd() {
  if (m_front) return m_front->GetVehicleBodyCmd();
  return VehicleBodyControl();
}

// get map info
ActorMapInfoPtr CEgoAssemble::GetMapInfo() {
  ActorMapInfoPtr map_info;
  if (m_front) map_info = m_front->GetMapInfo();
  return map_info;
}

// get if actor on solid boundry
bool CEgoAssemble::IsOnSolidBoundry(LaneBoundry &boundry_on) {
  bool on_solid_boundry = false;

  if (m_front) {
    on_solid_boundry = m_front->IsOnSolidBoundry(boundry_on);
    if (HasTrailer()) {
      auto trailer = GetFirstTrailer();
      on_solid_boundry = on_solid_boundry || trailer->IsOnSolidBoundry(boundry_on);
    }
  }
  return on_solid_boundry;
}

// get if actor on solid boundry
bool CEgoAssemble::IsOnSolidSolidBoundry(LaneBoundry &boundry_on) {
  bool on_solidSolid_boundry = false;
  if (m_front) {
    on_solidSolid_boundry = m_front->IsOnSolidSolidBoundry(boundry_on);
    if (HasTrailer()) {
      auto trailer = GetFirstTrailer();
      on_solidSolid_boundry = on_solidSolid_boundry || trailer->IsOnSolidSolidBoundry(boundry_on);
    }
  }
  return on_solidSolid_boundry;
}

// get if actor on dotted boundry
bool CEgoAssemble::IsOnDottedBoundry(LaneBoundry &boundry_on) {
  bool on_dotted_boundry = false;

  if (m_front) {
    on_dotted_boundry = m_front->IsOnDottedBoundry(boundry_on);
    if (HasTrailer()) {
      auto trailer = GetFirstTrailer();
      on_dotted_boundry = on_dotted_boundry || trailer->IsOnDottedBoundry(boundry_on);
    }
  }
  return on_dotted_boundry;
}

// get if actor on stop line
bool CEgoAssemble::IsOnStopLine(StopLine &stop_line_on) {
  bool on_stop_line = false;

  if (m_front) {
    on_stop_line = m_front->IsOnStopLine(stop_line_on);
    if (HasTrailer()) {
      auto trailer = GetFirstTrailer();
      on_stop_line = on_stop_line || trailer->IsOnStopLine(stop_line_on);
    }
  }
  return on_stop_line;
}

// get actor lateral offset from reference line
double CEgoAssemble::GetRefLineLateralOffset() {
  double lateral_offset = 0.0;
  if (m_front) lateral_offset = m_front->GetRefLineLateralOffset();
  return lateral_offset;
}

// get actor lateral yaw from reference line
double CEgoAssemble::GetRefLineLateralYaw() {
  double lateral_yaw = 0.0;
  if (m_front) lateral_yaw = m_front->GetRefLineLateralYaw();
  return lateral_yaw;
}

// if has ref line
bool CEgoAssemble::HasRefLine() {
  bool has_ref = false;
  if (m_front) {
    auto map = m_front->GetMapInfo();
    has_ref = map && map->m_on_road && (map->m_active_lane.m_ref_line.sample_points.size() > 0);
  }
  return has_ref;
}

/*==========================*/
void CEgoActorBuilder::Build(const EvalMsg &msg, ActorReposity &actor_repo) {
  if (msg.GetTopic() == topic::LOCATION || msg.GetTopic() == topic::LOCATION_TRAILER) {
    CEgoActorBuilder::BuildFrom(msg, actor_repo);
  }
}

CJerk CEgoActorBuilder::sPreAcc;

void CEgoActorBuilder::BuildFrom(const EvalMsg &msg, ActorReposity &actor_map) {
  // get ego msg
  sim_msg::VehicleState v_state;
  EvalMsg e_msg = CMsgManager::GetInstance()->Get("VEHICLE_STATE");

  sim_msg::Control_V2 v_control_v2;
  EvalMsg c_msg = CMsgManager::GetInstance()->Get(topic::CONTROL_V2);

  // get map ptr
  auto map_ptr = CMapManager::GetInstance();
  auto actorMgr_ptr = CActorManager::GetInstance();
  const auto &actorMgrParams = actorMgr_ptr->getParameters();

  if (map_ptr && actorMgr_ptr && msg.GetPayload().size() > 0) {
    // build traffic
    sim_msg::Location loc;
    loc.ParseFromString(msg.GetPayload());

    CEgoActorPtr ego_ptr = nullptr;
    const sim_msg::VehicleGeometoryList &ego_geometry = actorMgrParams.egoGeometry;

    if (msg.GetTopic() == topic::LOCATION) {
      ActorAgentPtr ego_agent = actor_map[Actor_Ego_Front]->at(0);
      ego_ptr = dynamic_cast<CEgoActorPtr>(ego_agent->GetActorPtr());
      if (ego_ptr == nullptr) throw "ego front actor is nullptr.\n";
      ego_ptr->SetType(Actor_Ego_Front);
      ego_ptr->MutableShape()->SetValues(ego_size::ego.length, ego_size::ego.width, ego_size::ego.height);
      ego_agent->SetState(Actor_Valid);

      auto veh_state_ptr = ego_ptr->MutableVehicleState();

      // priority: VEHICLE_STATE > CONTROL_V2. The same variables will be overwritten
      VLOG_1 << "grading| CONTROL_V2.size() = " << c_msg.GetPayload().size() << "\n";
      if (c_msg.GetPayload().size() > 0 && v_control_v2.ParseFromString(c_msg.GetPayload())) {
        // VLOG_2 << "grading| control_v2 = " << v_control_v2.ShortDebugString() << "\n";
        veh_state_ptr->powertrain_state.gead_mode = (VehicleState::GearMode)v_control_v2.gear_cmd();
        veh_state_ptr->chassis_state.SteeringWheelAngle = v_control_v2.control_cmd().request_steer_wheel_angle();
        *(ego_ptr->MutableVehicleBodyCmd()) = FromControl(v_control_v2);
      }
      VLOG_1 << "grading| VEHICLE_STATE.size() = " << e_msg.GetPayload().size() << "\n";
      if (e_msg.GetPayload().size() > 0 && v_state.ParseFromString(e_msg.GetPayload())) {
        // VLOG_2 << "grading| VEHICLE_STATE = " << v_state.ShortDebugString() << "\n";
        veh_state_ptr->powertrain_state.gead_mode = (VehicleState::GearMode)v_state.powertrain_state().gead_mode();
        veh_state_ptr->chassis_state.SteeringWheelAngle = v_state.chassis_state().steeringwheelangle();
        *(ego_ptr->MutableVehicleBodyCmd()) = FromVehicleState(v_state);
      }

      if (ego_geometry.has_front()) {
        const sim_msg::VehicleGeometory &front_geometry = ego_geometry.front();
        ego_ptr->MutableShape()->SetValues(front_geometry.vehicle_geometory().length(),
                                           front_geometry.vehicle_geometory().width(),
                                           front_geometry.vehicle_geometory().height());
        VLOG_2 << "ego size length, width, height:" << ego_ptr->GetShape().GetLength() << ", "
               << ego_ptr->GetShape().GetWidth() << ", " << ego_ptr->GetShape().GetHeight() << "\n";
      }
    } else if (msg.GetTopic() == topic::LOCATION_TRAILER) {
      ActorAgentPtr ego_agent = actor_map[Actor_Ego_Trailer]->at(0);
      ego_ptr = dynamic_cast<CEgoActorPtr>(ego_agent->GetActorPtr());
      if (ego_ptr == nullptr) throw "ego trailer actor is nullptr.\n";
      ego_ptr->SetType(Actor_Ego_Trailer);
      ego_ptr->MutableShape()->SetValues(ego_size::trailer.length, ego_size::trailer.width, ego_size::trailer.height);
      ego_agent->SetState(Actor_Valid);

      if (ego_geometry.trailer_size() > 0) {
        const sim_msg::VehicleGeometory &trailer_geometry = ego_geometry.trailer().at(0);
        ego_ptr->MutableShape()->SetValues(trailer_geometry.vehicle_geometory().length(),
                                           trailer_geometry.vehicle_geometory().width(),
                                           trailer_geometry.vehicle_geometory().height());
        VLOG_2 << "ego trailer size length, width, height:" << ego_ptr->GetShape().GetLength() << ", "
               << ego_ptr->GetShape().GetWidth() << ", " << ego_ptr->GetShape().GetHeight() << "\n";
      }
    }

    if (ego_ptr == nullptr) {
      throw "ego actor is nullptr.\n";
      return;
    }

    ego_ptr->MutableSimTime()->FromSecond(loc.t());
    auto pos_ptr = ego_ptr->MutableLocation()->MutablePosition();
    auto raw_pos_ptr = ego_ptr->MutableRawLocation()->MutablePosition();
    auto euler_ptr = ego_ptr->MutableLocation()->MutableEuler();
    auto speed_ptr = ego_ptr->MutableSpeed();
    auto acc_ptr = ego_ptr->MutableAcc();
    auto angular_ptr = ego_ptr->MutableAngularV();
    auto jerk_ptr = ego_ptr->MutableJerk();

    pos_ptr->SetValues(loc.position().x(), loc.position().y(), loc.position().z(), Coord_WGS84);
    raw_pos_ptr->SetValues(loc.position().x(), loc.position().y(), loc.position().z(), Coord_WGS84);
    map_ptr->WGS84ToENU(*pos_ptr);
    euler_ptr->SetValues(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());
    speed_ptr->SetValues(loc.velocity().x(), loc.velocity().y(), loc.velocity().z());
    acc_ptr->SetValues(loc.acceleration().x(), loc.acceleration().y(), loc.acceleration().z());
    angular_ptr->SetValues(loc.angular().x(), loc.angular().y(), loc.angular().z());

    auto deltaAcc = CEvalMath::Sub(*acc_ptr, sPreAcc);
    sPreAcc.SetValues(loc.acceleration().x(), loc.acceleration().y(), loc.acceleration().z());
    double deltaStep = getModuleStepTime();
    jerk_ptr->SetValues(deltaAcc.GetX() / deltaStep, deltaAcc.GetY() / deltaStep, deltaAcc.GetZ() / deltaStep);

    auto TransToBBX = [](CLocation *loc, const sim_msg::VehicleGeometory &veh_geometry) {
      const sim_msg::Vec3 &bbx = veh_geometry.vehicle_coord().bounding_box_center();
      Eigen::Vector3d bbx_offset = {bbx.x(), bbx.y(), bbx.z()};
      Eigen::Vector3d &&bbx_trans = loc->GetRotMatrix() * bbx_offset;
      CPosition ori = loc->GetPosition();
      loc->MutablePosition()->SetValues(ori.GetX() + bbx_trans[0], ori.GetY() + bbx_trans[1],
                                        ori.GetZ() + bbx_trans[2]);
    };

    auto TransWheelToENU = [](CEgoActorPtr ego_ptr, const sim_msg::VehicleGeometory &veh_geometry) {
      double front_offset = veh_geometry.vehicle_geometory().front_axle_to_front();
      double rear_offset = veh_geometry.vehicle_geometory().rear_axle_to_rear();

      RectCorners &&corners = ego_ptr->GetShape().GetRectCorners();

      Eigen::Vector3d fl_offset = corners[0] - Eigen::Vector3d{front_offset, 0, 0};
      Eigen::Vector3d &&fl_trans =
          ego_ptr->GetLocation().GetRotMatrix() * fl_offset + ego_ptr->GetLocation().GetPosition().GetPoint();
      Eigen::Vector3d fr_offset = corners[1] - Eigen::Vector3d{front_offset, 0, 0};
      Eigen::Vector3d &&fr_trans =
          ego_ptr->GetLocation().GetRotMatrix() * fr_offset + ego_ptr->GetLocation().GetPosition().GetPoint();

      Eigen::Vector3d rr_offset = corners[2] + Eigen::Vector3d{rear_offset, 0, 0};
      Eigen::Vector3d &&rr_trans =
          ego_ptr->GetLocation().GetRotMatrix() * rr_offset + ego_ptr->GetLocation().GetPosition().GetPoint();
      Eigen::Vector3d rl_offset = corners[3] + Eigen::Vector3d{rear_offset, 0, 0};
      Eigen::Vector3d &&rl_trans =
          ego_ptr->GetLocation().GetRotMatrix() * rl_offset + ego_ptr->GetLocation().GetPosition().GetPoint();

      auto &wheel_pos = *(ego_ptr->MutableWheelPosENU());
      wheel_pos[0] = fl_trans;
      wheel_pos[1] = fr_trans;
      wheel_pos[2] = rr_trans;
      wheel_pos[3] = rl_trans;
    };

    if (msg.GetTopic() == topic::LOCATION) {
      if (ego_geometry.has_front()) {
        const sim_msg::VehicleGeometory &front_geometry = ego_geometry.front();
        TransToBBX(ego_ptr->MutableLocation(), front_geometry);
        TransWheelToENU(ego_ptr, front_geometry);
        VLOG_2 << "do TransWheelToENU" << "\n";
      }
    }

    if (msg.GetTopic() == topic::LOCATION_TRAILER) {
      if (ego_geometry.trailer_size() > 0) {
        const sim_msg::VehicleGeometory &trailer_geometry = ego_geometry.trailer().at(0);
        TransToBBX(ego_ptr->MutableLocation(), trailer_geometry);
      }
    }

    // update map info
    MapQueryTaskCfg queryTask(*pos_ptr);
    ego_ptr->UpateMapInfo(queryTask);
    if (ego_ptr->GetMapInfo()) ego_ptr->SetLaneID(ego_ptr->GetMapInfo()->m_lane_id);
    ego_ptr->CheckActorOnSolidOrDottedLine();
    ego_ptr->CheckActorOnStopLine();
    ego_ptr->CheckActorOnCrosswalk();
    ego_ptr->CalRefLineLateralOffset();
    ego_ptr->CalRefLineLateralYaw();
    ego_ptr->CalRefLineMaxDistance();
    ego_ptr->CheckLaneNumChange();
    ego_ptr->CalStateWihtStopLine();
    ego_ptr->CalSpeedDistanceYaw();
  }
}
}  // namespace eval
