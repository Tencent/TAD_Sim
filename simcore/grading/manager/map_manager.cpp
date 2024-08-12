// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>

#include "actors/c_actor_static.h"
#include "common/coord_trans.h"
#include "db_codes.h"
#include "location.pb.h"
#include "manager/map_manager.h"
#include "mapengine/hadmap_codes.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

namespace eval {
/*==========================*/
ActorMapInfoPtr MapQueryAgent::GetMapInfo() {
  ActorMapInfoPtr map_ready;
  // swap queried map info
  {
    std::lock_guard<std::mutex> guard(m_query_lock);
    if (m_query) map_ready = m_query;
  }
  return map_ready;
}

void MapQueryAgent::UpdateMapInfo(const MapQueryTaskCfg &task) {
  std::lock_guard<std::mutex> guard(m_update_lock);
  m_update_task = task;
}

void MapQueryAgent::MapUpdataThreadFunc() {
  VLOG_0 << "map update thread start.\n";

  MapQueryTaskCfg task;

  // do update
  while (!m_stop_task) {
    // retrive task
    {
      std::lock_guard<std::mutex> guard(m_update_lock);
      task = m_update_task;
    }
    // get hadmap instance
    auto map_mgr = CMapManager::GetInstance();

    // update
    if (map_mgr && CEvalMath::Distance2D(task.m_pivot_enu, m_last_enu) >= m_update_thresh) {
      ActorMapInfoPtr map_info = std::make_shared<ActorMapInfo>();

      // query map info
      if (map_info && map_mgr->QueryMapInfo(map_info, task)) {
        // update latest enu position
        m_last_enu = task.m_pivot_enu;

        // copy output
        {
          std::lock_guard<std::mutex> guard(m_query_lock);
          m_query = map_info;
        }
      }
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  VLOG_0 << "map update thread quit.\n";
}

/*==========================*/
MapManager::MapManager(double radius) {
  _state = MAPState::MAP_NOT_OK;
  _map_handle = nullptr;
  _map_type = hadmap::MAP_DATA_TYPE::SQLITE;
  _radius = radius;
  _sample_step = 0.1;
}

bool MapManager::QueryMapInfo(ActorMapInfoPtr &map_info, const MapQueryTaskCfg &task) {
  try {
    // default
    map_info->m_on_road = false;
    map_info->m_in_junction = false;
    map_info->m_lane_id.Clear();
    map_info->m_active_lane.m_on_narrow_lane = false;
    map_info->m_active_lane.m_on_entry_expressway = false;
    map_info->m_active_lane.m_on_exit_expressway = false;
    map_info->m_active_lane.m_no_center_lane = false;
    map_info->m_active_lane.m_on_lane_acceleration = false;
    map_info->m_active_lane.m_on_lane_deceleration = false;
    map_info->m_active_lane.m_on_lane_shoulder = false;
    map_info->m_active_lane.m_on_lane_emergency = false;
    map_info->m_active_lane.m_lane_num_decreasing = false;
    map_info->m_active_lane.m_lane_num_increasing = false;

    map_info->m_active_road.m_is_unbidirectional = false;
    map_info->m_active_road.m_on_parking_area = false;
    map_info->m_active_road.m_on_private_road = false;
    map_info->m_active_road.m_on_motorway = false;
    map_info->m_active_road.m_on_ramp = false;
    map_info->m_active_road.m_on_urban = false;
    map_info->m_active_road.m_on_narrow_road = false;
    map_info->m_active_road.m_on_sharp_curve = false;
    map_info->m_active_road.m_on_steep_slope = false;
    map_info->m_active_road.m_on_bridge = false;
    map_info->m_active_road.m_on_tunnel = false;
    map_info->m_active_road.m_on_control_access = false;
    map_info->m_active_road.m_has_parking_sign = false;
    map_info->m_active_road.m_has_parking_marking = false;
    map_info->m_active_road.m_road_length = 0.0;

    if (map_info && _map_handle) {
      // debug log
      VLOG_2 << "update map info started.\n";
      VLOG_2 << "pivot enu:[" << task.m_pivot_enu.GetX() << ", " << task.m_pivot_enu.GetY() << ", "
             << task.m_pivot_enu.GetZ() << "].\n";

      // lane link and lane
      hadmap::txLanePtr lane_ptr;
      hadmap::txLaneLinkPtr link_ptr;

      // query lane from hadmap
      if (task.m_use_query_laneid) {
        // get lane from hadmap by id
        const LaneID &cur_lane_id = task.m_query_lane_id;
        lane_ptr = GetLane(cur_lane_id);

        // get lanelink from hadmap by id
        link_ptr = GetLaneLink(map_info->m_lane_id.tx_lanelink_id);
      } else {
        // get lane from hadmap by position
        CPosition pivot_wgs84 = task.m_pivot_enu;
        ENUToWGS84(pivot_wgs84);

        // search lane and lane link from hadmap according to given point
        lane_ptr = CalLaneByPosition(pivot_wgs84.GetX(), pivot_wgs84.GetY(), pivot_wgs84.GetZ());
        link_ptr = CalLaneLinkByPosition(pivot_wgs84.GetX(), pivot_wgs84.GetY(), pivot_wgs84.GetZ());
      }

      // calculate map info
      if (!lane_ptr) {
        LOG_ERROR << "fail to get lane ptr.\n";
      } else {
        // get left and right lane
        hadmap::getLeftLane(_map_handle, lane_ptr, map_info->m_active_lane.m_left_lane);
        hadmap::getRightLane(_map_handle, lane_ptr, map_info->m_active_lane.m_right_lane);

        // get pre and next lanes
        hadmap::getPrevLanes(_map_handle, lane_ptr, map_info->m_active_lane.m_pre_lanes);
        hadmap::getNextLanes(_map_handle, lane_ptr, map_info->m_active_lane.m_next_lanes);

        // get pre and next lane links
        hadmap::getPrevLaneLinks(_map_handle, lane_ptr->getTxLaneId(), map_info->m_active_lane.m_pre_lanelinks);
        hadmap::getNextLaneLinks(_map_handle, lane_ptr->getTxLaneId(), map_info->m_active_lane.m_next_lanelinks);

        // get road
        hadmap::getRoad(_map_handle, lane_ptr->getRoadId(), true, map_info->m_active_road.m_road);

        // check if lane number change
        auto getSections = [&](hadmap::txLanePtr lane_ptr) -> hadmap::txSections {
          hadmap::txSectionId section_id;
          section_id.roadId = lane_ptr->getRoadId();
          section_id.sectionId = lane_ptr->getSectionId();
          hadmap::txSections curSections;
          hadmap::getSections(_map_handle, section_id, curSections);
          return curSections;
        };

        // auto curSection = lane_ptr->getSection();
        auto curSections = getSections(lane_ptr);
        if (map_info->m_active_lane.m_next_lanes.size() > 0) {
          auto nextLane = map_info->m_active_lane.m_next_lanes.at(0);
          if (nextLane.get() != nullptr) {
            auto nextSections = getSections(nextLane);
            if (curSections.size() > 0 && nextSections.size() > 0) {
              auto nextLaneNum = nextSections.front()->getLanes().size();
              auto curLaneNum = curSections.front()->getLanes().size();

              map_info->m_active_lane.m_lane_num_decreasing = nextLaneNum < curLaneNum;
              map_info->m_active_lane.m_lane_num_increasing = nextLaneNum > curLaneNum;
            }
            // auto nextSection = nextLane->getSection();
            // if (nextSection.get() != nullptr && curSection.get() != nullptr) {
            //   auto nextLaneNum = nextSection->getLanes().size();
            //   auto curLaneNum = curSection->getLanes().size();

            //  map_info->m_active_lane.m_lane_num_decreasing = nextLaneNum < curLaneNum;
            //  map_info->m_active_lane.m_lane_num_increasing = nextLaneNum > curLaneNum;
            //}
          }
        }

        // check on narrow lane
        map_info->m_active_lane.m_on_narrow_lane = CalLaneWidth(lane_ptr->getTxLaneId()) <= 3.0;  // m

        // check on entry expressway
        map_info->m_active_lane.m_on_entry_expressway = lane_ptr->getLaneType() == hadmap::LANE_TYPE::LANE_TYPE_ENTRY;

        // check on exit expressway
        map_info->m_active_lane.m_on_exit_expressway = lane_ptr->getLaneType() == hadmap::LANE_TYPE::LANE_TYPE_EXIT;

        // check whether no_center_lane
        map_info->m_active_lane.m_no_center_lane = false;

        // check on lane acceleration
        map_info->m_active_lane.m_on_lane_acceleration = lane_ptr->getLaneType() == hadmap::LANE_TYPE::LANE_TYPE_ONRAMP;

        // check on lane deceleration
        map_info->m_active_lane.m_on_lane_deceleration =
            lane_ptr->getLaneType() == hadmap::LANE_TYPE::LANE_TYPE_OFFRAMP;

        // check on lane shoulder
        map_info->m_active_lane.m_on_lane_shoulder = (lane_ptr->getLaneType() == hadmap::LANE_TYPE_Shoulder);

        // check on lane emergency
        map_info->m_active_lane.m_on_lane_emergency = (lane_ptr->getLaneType() == hadmap::LANE_TYPE_Emergency);

        // check on lane emergency
        map_info->m_active_lane.m_on_connecting_ramp = (lane_ptr->getLaneType() == hadmap::LANE_TYPE_CONNECTINGRAMP);

        // ckeck on parking area road
        map_info->m_active_road.m_on_parking_area =
            map_info->m_active_road.m_road->getRoadType() == hadmap::ROAD_TYPE_PA;

        // ckeck on private road
        map_info->m_active_road.m_on_private_road =
            (map_info->m_active_road.m_road->getRoadType() == hadmap::ROAD_TYPE_PrivateRoad);

        // check on motorway road
        map_info->m_active_road.m_on_motorway =
            (map_info->m_active_road.m_road->getRoadType() == hadmap::ROAD_TYPE_ODR_Motorway);

        // check on ramp road
        map_info->m_active_road.m_on_ramp =
            (map_info->m_active_road.m_road->getRoadType() == hadmap::ROAD_TYPE_GenerapRamp);

        // check on urban
        map_info->m_active_road.m_on_urban = map_info->m_active_road.m_road->isUrban();

        // check on narrow road
        map_info->m_active_road.m_on_narrow_road = GetCondOnNarrowRoad(lane_ptr);

        // check on sharp curve
        map_info->m_active_road.m_on_sharp_curve = GetCondOnSharpCurve(map_info->m_active_road.m_road);

        // check on steep slope
        map_info->m_active_road.m_on_steep_slope = GetCondOnSlopeGrade(map_info->m_active_road.m_road);

        // check on bridge
        map_info->m_active_road.m_on_bridge = GetCondHasPedestrianBridge(task.m_pivot_enu, 20.0);

        // check on tunnel
        map_info->m_active_road.m_on_tunnel = GetCondHasTunnel(task.m_pivot_enu, 20.0);

        // check on control_access
        map_info->m_active_road.m_on_control_access =
            (map_info->m_active_road.m_road->getRoadType() == hadmap::ROAD_TYPE_ControlAccess);

        // check whether unbidirectional
        map_info->m_active_road.m_is_unbidirectional = GetCondUnbidirectional(lane_ptr->getTxLaneId());

        // check whether has_parking_sign
        map_info->m_active_road.m_has_parking_sign = GetCondHasParkingSign(task.m_pivot_enu);

        // check whether has_parking_marking
        map_info->m_active_road.m_has_parking_marking = GetCondHasParkingMarking(task.m_pivot_enu);

        // get road length
        auto road_ptr = GetRoad(lane_ptr->getTxLaneId());
        if (road_ptr) map_info->m_active_road.m_road_length = road_ptr->getLength();

        // sample lane boundries
        CalLaneBoundries(lane_ptr, map_info, task);
        CalLaneSingleBoundry(map_info->m_active_lane.m_left_lane, map_info->m_active_lane.m_left_lane_left_boundry,
                             task, true);
        CalLaneSingleBoundry(map_info->m_active_lane.m_right_lane, map_info->m_active_lane.m_right_lane_right_boundry,
                             task, false);

        // sample lane reference line
        CalRefLine(lane_ptr, map_info, task);

        // set infos
        hadmap::txLaneId lane_id = lane_ptr->getTxLaneId();
        map_info->m_lane_id = {lane_id.roadId, lane_id.sectionId, lane_id.laneId, INVALID_MAP_ID};
        map_info->m_active_lane.m_lane = lane_ptr;
        map_info->m_on_road = true;
      }

      if (!link_ptr) {
        LOG_ERROR << "fail to get lane link ptr.\n";
        // return false;
      } else {
        // sample points
        ConvertToEvalPoints(link_ptr->getGeometry(), map_info->m_active_lanelink.m_sample_points, task.m_pivot_enu);

        // get pre and next lane
        map_info->m_active_lanelink.m_pre_lane = GetLane(link_ptr->fromTxLaneId());
        map_info->m_active_lanelink.m_next_lane = GetLane(link_ptr->toTxLaneId());

        // set infos
        map_info->m_in_junction = true;
        map_info->m_lane_id.tx_lanelink_id = link_ptr->getId();
        map_info->m_active_lanelink.m_lanelink = link_ptr;
      }

      VLOG_2 << "update map info quit.\n";

      return true;
    }
  } catch (const std::exception &e) {
    LOG_ERROR << "fail to query map info from hadmap, " << e.what() << "\n";
  }

  return false;
}

void MapManager::WGS84ToENU(CPosition &pt) {
  if (pt.GetCoordType() == Coord_WGS84) {
    double x = pt.GetX(), y = pt.GetY(), z = pt.GetZ();
    coord_trans_api::lonlat2enu(x, y, z, _map_origin.x, _map_origin.y, _map_origin.z);
    pt.SetValues(x, y, z, Coord_ENU);
  }
}

void MapManager::ENUToWGS84(CPosition &pt) {
  if (pt.GetCoordType() == Coord_ENU) {
    double x = pt.GetX(), y = pt.GetY(), z = pt.GetZ();
    coord_trans_api::enu2lonlat(x, y, z, _map_origin.x, _map_origin.y, _map_origin.z);
    pt.SetValues(x, y, z, Coord_WGS84);
  }
}

bool MapManager::Connect(const std::string &map_path, const std::string &start_loc_payload) {
  // release hadmap
  Release();

  // map origin
  sim_msg::Location loc;
  loc.ParseFromString(start_loc_payload);
  _map_origin.x = loc.position().x();
  _map_origin.y = loc.position().y();
  _map_origin.z = loc.position().z();

  VLOG_0 << std::fixed << std::setprecision(9) << "map origin/start location:" << _map_origin.x << ", " << _map_origin.y
         << ", " << _map_origin.z << "\n";
  VLOG_0 << "default radius is " << _radius << "\n";

  // check hadmap file path
  {
    auto extension_pos = map_path.find_last_of('.');
    std::string map_extension("");

    if (extension_pos != map_path.npos) {
      map_extension = map_path.substr(extension_pos + 1, map_path.size() - extension_pos - 1);
    }

    if (map_extension == "sqlite" || map_extension == "SQLITE") {
      VLOG_0 << "map type is sqlite.\n";
      _map_type = hadmap::MAP_DATA_TYPE::SQLITE;
    } else if (map_extension == "xodr" || map_extension == "XODR") {
      VLOG_0 << "map type is xodr.\n";
      _map_type = hadmap::MAP_DATA_TYPE::OPENDRIVE;
    } else if (map_extension == "xml" || map_extension == "XML") {
      VLOG_0 << "map type is xml.\n";
      _map_type = hadmap::MAP_DATA_TYPE::OPENDRIVE;
    } else {
      LOG_ERROR << "hadmap type:" << map_extension << ", use .sqlite or .xodr.\n";
      _state = MAPState::MAP_NOT_OK;
      return false;
    }
  }

  // connect hadmap
  try {
    if (hadmap::hadmapConnect(map_path.c_str(), _map_type, &_map_handle) == TX_HADMAP_HANDLE_OK) {
      _state = MAPState::MAP_OK;
      VLOG_0 << "hadmap connectted. " << map_path << "\n";
    } else {
      LOG_ERROR << "hadmap connect failed. " << map_path << "\n";
      _state = MAPState::MAP_NOT_OK;
      Release();
      return false;
    }
  } catch (const std::exception &e) {
    LOG_ERROR << "fail to hadmap connect. exception: " << e.what() << "\n";
    _state = MAPState::MAP_NOT_OK;
    Release();
    return false;
  }

  // get all map objects
  TransAllObjects();
  return true;
}

void MapManager::Release() {
  // release hadmap pointer
  if (_map_handle != nullptr) {
    m_map_objects.clear();
    m_map_stat_actors.clear();
    hadmap::hadmapClose(&_map_handle);
    _map_handle = nullptr;
  }

  VLOG_0 << "hadmap released.\n";
}

hadmap::txLanePtr MapManager::CalLaneByPosition(double longti, double lati, double alti) {
  hadmap::txLanePtr lane_ptr;
  hadmap::txPoint cur_loc = {longti, lati, alti};

  if (_map_handle) {
    auto code = hadmap::getLane(_map_handle, cur_loc, lane_ptr, 2.5);
    if (TX_HADMAP_DATA_OK != code) {
      lane_ptr.reset();
      VLOG_0 << "actor geodetic position:[" << longti << ", " << lati << ", " << alti << "].\n";
      VLOG_0 << "fail to get closest lane with exit code:" << code << "\n";
    } else {
      VLOG_2 << std::fixed << std::setprecision(9) << "actor geodetic position:[" << longti << ", " << lati << ", "
             << alti << "].\n";
      VLOG_2 << "lane_ptr's road_id: " << lane_ptr->getRoadId() << ", section_id: " << lane_ptr->getSectionId()
             << ", lane_id: " << lane_ptr->getTxLaneId() << ".\n";
    }
  }

  return lane_ptr;
}

hadmap::txLaneLinkPtr MapManager::CalLaneLinkByPosition(double longti, double lati, double alti) {
  hadmap::txLaneLinkPtr lane_link;
  hadmap::txPoint cur_loc = {longti, lati, alti};

  if (_map_handle) {
    auto code = hadmap::getLaneLink(_map_handle, cur_loc, lane_link);
    if (TX_HADMAP_DATA_OK != code) {
      lane_link.reset();
      LOG_ERROR << "actor position:[" << longti << ", " << lati << ", " << alti << "].\n";
      LOG_ERROR << "fail to get closest lane link with exit code:" << code << "\n";
    }
  }

  return lane_link;
}

bool MapManager::CalLaneBoundries(hadmap::txLanePtr &lane_ptr, ActorMapInfoPtr &map_query,
                                  const MapQueryTaskCfg &task) {
  // lambda, used to sample hadmap curve
  auto sample_boundry = [this, &task](hadmap::txLaneBoundaryPtr &bdr, LaneBoundry &bdr_out) {
    if (bdr) {
      const hadmap::txCurve *curve_wgs84 = bdr->getGeometry();

      // for these boundaries, radius is 40m
      if (task.m_use_sample) this->ConvertToEvalPoints(curve_wgs84, bdr_out.sample_points, task.m_pivot_enu, 40.0);

      bdr_out.boundry_pkid = bdr->getId();
      bdr_out.lane_mark_type = bdr->getLaneMark();
    } else {
      LOG_ERROR << "boundry ptr is null.\n";
    }
  };

  if (lane_ptr.get() && map_query) {
    // get lane left and right boundry
    hadmap::txLaneBoundaryPtr bdr_left_ptr;
    hadmap::txLaneBoundaryPtr bdr_right_ptr;

    hadmap::getBoundary(_map_handle, lane_ptr->getLeftBoundaryId(), bdr_left_ptr);
    hadmap::getBoundary(_map_handle, lane_ptr->getRightBoundaryId(), bdr_right_ptr);

    // sample lane boundries
    sample_boundry(bdr_left_ptr, map_query->m_active_lane.m_left_boundry);
    sample_boundry(bdr_right_ptr, map_query->m_active_lane.m_right_boundry);
  } else {
    LOG_ERROR << "close lane ptr is null.\n";
  }

  return true;
}

bool MapManager::CalLaneSingleBoundry(hadmap::txLanePtr &lane_ptr, LaneBoundry &lane_boundary,
                                      const MapQueryTaskCfg &task, const bool is_left_bdr) {
  // lambda, used to sample hadmap curve
  auto sample_boundry = [this, &task](hadmap::txLaneBoundaryPtr &bdr, LaneBoundry &bdr_out) {
    if (bdr) {
      const hadmap::txCurve *curve_wgs84 = bdr->getGeometry();

      if (task.m_use_sample) this->ConvertToEvalPoints(curve_wgs84, bdr_out.sample_points, task.m_pivot_enu);

      bdr_out.boundry_pkid = bdr->getId();
      bdr_out.lane_mark_type = bdr->getLaneMark();
    } else {
      LOG_ERROR << "boundry ptr is null.\n";
    }
  };

  if (lane_ptr.get()) {
    // get lane left and right boundry
    hadmap::txLaneBoundaryPtr bdr_ptr;
    if (is_left_bdr) {
      hadmap::getBoundary(_map_handle, lane_ptr->getLeftBoundaryId(), bdr_ptr);
      sample_boundry(bdr_ptr, lane_boundary);
    } else {
      hadmap::getBoundary(_map_handle, lane_ptr->getRightBoundaryId(), bdr_ptr);
      sample_boundry(bdr_ptr, lane_boundary);
    }
  } else {
    LOG_ERROR << "close lane ptr is null.\n";
  }
  return true;
}

void MapManager::CalRefLine(hadmap::txLanePtr &lane_ptr, ActorMapInfoPtr &map_query, const MapQueryTaskCfg &task) {
  if (lane_ptr) {
    const hadmap::txCurve *curve = (hadmap::txCurve *)lane_ptr->getGeometry();
    if (task.m_use_sample)
      ConvertToEvalPoints(curve, map_query->m_active_lane.m_ref_line.sample_points, task.m_pivot_enu, 30.0);
  } else {
    LOG_ERROR << "lane ptr is nullptr.\n";
  }
}

EvalMapObjectPtr MapManager::GetMapObjects(const hadmap::OBJECT_TYPE &obj_type, const CPosition &pivot_enu,
                                           double radius) {
  EvalMapObjectPtr map_objs_ret;

  if (m_map_objects.find(obj_type) != m_map_objects.end()) {
    EvalMapObjectPtr &map_objects = m_map_objects[obj_type];
    for (auto idx = 0; idx < map_objects->m_map_objects.size(); ++idx) {
      // filter invalid gps position
      hadmap::txPoint &&map_obj_pos = map_objects->m_map_objects.at(idx)->getPos();
      CPosition obj_pos_enu(map_obj_pos.x, map_obj_pos.y, map_obj_pos.z, Coord_WGS84);
      if (std::abs(map_obj_pos.x) >= 1e-6 && std::abs(map_obj_pos.y) >= 1e-6) {
        // to enu
        WGS84ToENU(obj_pos_enu);

        // filter by radius
        if (CEvalMath::Sub(obj_pos_enu, pivot_enu).GetNormal2D() < radius) {
          if (map_objs_ret.get() == nullptr) map_objs_ret = std::make_shared<EvalMapObject>();
          map_objs_ret->m_object_type = obj_type;
          map_objs_ret->m_samples.push_back(map_objects->m_samples.at(idx));
          map_objs_ret->m_map_objects.push_back(map_objects->m_map_objects.at(idx));
        }
      }
    }
  }

  return map_objs_ret;
}

EvalMapObjectPtr MapManager::GetMapObjects(const hadmap::OBJECT_TYPE &obj_type,
                                           const std::vector<hadmap::OBJECT_SUB_TYPE> &obj_sub_types,
                                           const CPosition &pivot_enu, double radius) {
  EvalMapObjectPtr map_objs_ret;

  if (m_map_objects.find(obj_type) != m_map_objects.end()) {
    EvalMapObjectPtr &map_objects = m_map_objects[obj_type];
    for (auto idx = 0; idx < map_objects->m_map_objects.size(); ++idx) {
      // get map object
      auto map_object = map_objects->m_map_objects.at(idx);

      // filter invalid gps position
      hadmap::txPoint &&map_obj_pos = map_object->getPos();
      CPosition obj_pos_enu(map_obj_pos.x, map_obj_pos.y, map_obj_pos.z, Coord_WGS84);
      if (std::abs(map_obj_pos.x) >= 1e-6 && std::abs(map_obj_pos.y) >= 1e-6) {
        // to enu
        WGS84ToENU(obj_pos_enu);

        // check subtypes
        auto match_subtype = std::find(obj_sub_types.begin(), obj_sub_types.end(), map_object->getObjectSubType());
        if (match_subtype == obj_sub_types.end()) {
          continue;
        }

        // filter by radius and sub_type
        if (CEvalMath::Sub(obj_pos_enu, pivot_enu).GetNormal2D() < radius) {
          if (map_objs_ret.get() == nullptr) map_objs_ret = std::make_shared<EvalMapObject>();
          map_objs_ret->m_object_type = obj_type;
          map_objs_ret->m_samples.push_back(map_objects->m_samples.at(idx));
          map_objs_ret->m_map_objects.push_back(map_objects->m_map_objects.at(idx));
        }
      }
    }
  }

  return map_objs_ret;
}

EvalMapStatActorPtr MapManager::GetMapStatActors(const hadmap::OBJECT_TYPE &obj_type, const CPosition &pivot_enu,
                                                 double radius) {
  EvalMapStatActorPtr map_objs_ret;
  if (m_map_stat_actors.find(obj_type) != m_map_stat_actors.end()) {
    EvalMapStatActorPtr &map_objects = m_map_stat_actors[obj_type];
    VLOG_3 << "[MapManager] OBJECT_TYPE: " << obj_type << " ; m_map_actors size:" << map_objects->m_map_actors.size()
           << " .\n";
    for (auto idx = 0; idx < map_objects->m_map_actors.size(); ++idx) {
      // filter invalid gps position
      CPosition map_obj_pos = map_objects->m_map_actors.at(idx)->GetLocation().GetPosition();
      VLOG_3 << "[GetMapStatActors] obj type: " << obj_type << ", id: " << map_objects->m_map_actors.at(idx)->GetID()
             << ", obj_pos: (" << map_obj_pos.GetX() << ", " << map_obj_pos.GetY() << ", " << map_obj_pos.GetZ()
             << "),  pivot_enu: (" << pivot_enu.GetX() << ", " << pivot_enu.GetY() << ", " << pivot_enu.GetZ()
             << "), distance: " << CEvalMath::Sub(map_obj_pos, pivot_enu).GetNormal2D();

      // filter by radius
      if (CEvalMath::Sub(map_obj_pos, pivot_enu).GetNormal2D() < radius) {
        VLOG_2 << "[GetMapStatActors] map obj distance meets need. obj type: " << obj_type
               << ", id: " << map_objects->m_map_actors.at(idx)->GetID() << ", obj_pos: (" << map_obj_pos.GetX() << ", "
               << map_obj_pos.GetY() << ", " << map_obj_pos.GetZ() << "),  pivot_enu: (" << pivot_enu.GetX() << ", "
               << pivot_enu.GetY() << ", " << pivot_enu.GetZ()
               << "), distance: " << CEvalMath::Sub(map_obj_pos, pivot_enu).GetNormal2D() << " .\n";
        if (map_objs_ret.get() == nullptr) map_objs_ret = std::make_shared<EvalMapStatActor>();
        map_objs_ret->m_object_type = obj_type;
        map_objs_ret->m_samples.push_back(map_objects->m_samples.at(idx));
        map_objs_ret->m_map_actors.push_back(map_objects->m_map_actors.at(idx));
      }
    }
  }
  return map_objs_ret;
}

void MapManager::ConvertToEvalPoints(const hadmap::txCurve *curve_wgs84, EvalPoints &sample_points,
                                     const CPosition &pivot_enu, double provided_radius, bool enable_pivot_filter,
                                     bool sample_yaw) {
  sample_points.clear();

  if (curve_wgs84) {
    // sample curve
    hadmap::PointVec map_pts_egs84;
    curve_wgs84->sample(_sample_step, map_pts_egs84);
    auto curve_coord_type = curve_wgs84->getCoordType();
    eval::CoordType sample_coord_type = (curve_coord_type == hadmap::CoordType::COORD_WGS84)
                                            ? eval::CoordType::Coord_WGS84
                                            : eval::CoordType::Coord_ENU;

    // pivot enu
    double length = 0.0;

    for (auto i = 0; i < map_pts_egs84.size(); ++i) {
      const hadmap::txPoint &pt_wgs84 = map_pts_egs84.at(i);

      // create map point and convert to ENU
      CLocation map_point(pt_wgs84.x, pt_wgs84.y, pt_wgs84.z, 0.0, 0.0, 0.0, sample_coord_type);
      WGS84ToENU(*map_point.MutablePosition());
      length += _sample_step;

      // calculate yaw of map point and save
      bool within_radius = false;
      if (provided_radius >= 0.0) {
        within_radius = CEvalMath::AbsoluteDistance2D(map_point.GetPosition(), pivot_enu) <= provided_radius;
      } else {
        within_radius = CEvalMath::AbsoluteDistance2D(map_point.GetPosition(), pivot_enu) <= _radius;
      }
      bool save_sample = enable_pivot_filter ? within_radius : true;
      if (save_sample) {
        // get and set yaw
        if (sample_yaw) map_point.MutableEuler()->SetValues(0.0, 0.0, curve_wgs84->getYaw(length) * const_deg_2_rad);
        // save point
        sample_points.emplace_back(map_point.GetPosition(), map_point.GetEuler());
      }
    }
  } else {
    LOG_ERROR << "curve is nullptr, can not convert hadmap curve to eval points.\n";
  }
}

hadmap::txLanePtr MapManager::GetLane(const eval::LaneID &lane_id) {
  hadmap::txLanePtr lane_ptr;
  if (_map_handle) {
    hadmap::txLaneId laneid;
    laneid.laneId = lane_id.tx_lane_id;
    laneid.sectionId = lane_id.tx_section_id;
    laneid.roadId = lane_id.tx_road_id;
    hadmap::getLane(_map_handle, laneid, lane_ptr);
  }

  return lane_ptr;
}

hadmap::txLanePtr MapManager::GetLane(const hadmap::txLaneId &lane_id) {
  hadmap::txLanePtr lane_ptr;
  if (_map_handle) hadmap::getLane(_map_handle, lane_id, lane_ptr);
  return lane_ptr;
}

hadmap::txSectionPtr MapManager::GetSection(const eval::LaneID &lane_id) {
  hadmap::txLanePtr lane_ptr = GetLane(lane_id);
  hadmap::txSectionPtr section_ptr;
  if (lane_ptr) {
    section_ptr = lane_ptr->getSection();
  }
  return section_ptr;
}

hadmap::txSectionPtr MapManager::GetSection(const hadmap::txLaneId &lane_id) {
  hadmap::txLanePtr lane_ptr;
  hadmap::txSectionPtr section_ptr;
  if (_map_handle) {
    hadmap::getLane(_map_handle, lane_id, lane_ptr);
    if (lane_ptr) section_ptr = lane_ptr->getSection();
  }
  return section_ptr;
}

hadmap::txRoadPtr MapManager::GetRoad(const eval::LaneID &lane_id) {
  CElapsedTime e1("GetRoadByLaneID");
  hadmap::txLanePtr lane_ptr = GetLane(lane_id);
  hadmap::txRoadPtr road_ptr;
  if (_map_handle && lane_ptr) {
    hadmap::getRoad(_map_handle, lane_ptr->getRoadId(), true, road_ptr);
  }
  return road_ptr;
}

hadmap::txRoadPtr MapManager::GetRoad(const hadmap::txLaneId &lane_id) {
  CElapsedTime e1("GetRoadBytxLaneId");
  hadmap::txLanePtr lane_ptr = GetLane(lane_id);
  hadmap::txRoadPtr road_ptr;
  if (_map_handle && lane_ptr) {
    hadmap::getRoad(_map_handle, lane_ptr->getRoadId(), true, road_ptr);
  }
  return road_ptr;
}

hadmap::txLaneLinkPtr MapManager::GetLaneLink(const hadmap::lanelinkpkid &lanelink_pkid) {
  hadmap::txLaneLinkPtr link_ptr;
  if (_map_handle) {
    hadmap::getLaneLink(_map_handle, lanelink_pkid, link_ptr);
  }
  return link_ptr;
}

hadmap::txLanes MapManager::GetLanesUnderSection(const hadmap::txLaneId &lane_id) {
  hadmap::txLanes lanes;
  if (_map_handle) {
    hadmap::getLanes(_map_handle, hadmap::txLaneId(lane_id.roadId, lane_id.sectionId, LANE_PKID_INVALID), lanes);
  }
  return lanes;
}

// get the leftmost lane boundary of actor driving road
hadmap::txLaneBoundaryPtr MapManager::GetLeftmostLaneBoundary(const hadmap::txLaneId &lane_id) {
  hadmap::txLaneBoundaryPtr leftmost_lane_boundary_ptr;

  // assertion
  if (_map_handle != nullptr) return leftmost_lane_boundary_ptr;

  for (auto lane_ptr : GetLanesUnderSection(lane_id)) {
    if (lane_ptr && (lane_ptr->getId() == -1)) {
      // 20220501(xingboliu): mapsdk error,
      // getLeftBoundary alway return leftmost_lane_boundary_ptr is empty
      // but getLeftBoundaryId is fine
      // so hadmap::getBoundary to get leftmost_lane_boundary_ptr
      // but return not empty lane mark set None
      // leftmost_lane_boundary_ptr = lane_ptr->getLeftBoundary();
      hadmap::getBoundary(_map_handle, lane_ptr->getLeftBoundaryId(), leftmost_lane_boundary_ptr);
      break;
    }
  }
  return leftmost_lane_boundary_ptr;
}

// calculate the number of driving ways in the direction of the actor
int MapManager::CalDrivingLaneNum(const hadmap::txLaneId &lane_id) {
  // assertion
  if (_map_handle == nullptr) return 0;

  int driving_lane_num = 0;
  // Iterate all lanes in the ego driving direction and
  // judge whether the type is LANE_TYPE_Driving and counting
  for (auto lane_ptr : GetLanesUnderSection(lane_id)) {
    if (lane_ptr && lane_ptr->getLaneType() == hadmap::LANE_TYPE_Driving) {
      ++driving_lane_num;
    }
  }
  return driving_lane_num;
}

// calculate lane width
double MapManager::CalLaneWidth(const hadmap::txLaneId &lane_id) {
  // assertion
  if (_map_handle == nullptr) return 0.0;

  double lane_width = 0.0;
  hadmap::txLanePtr lane_ptr = GetLane(lane_id);
  // get left boundary and right boundary
  hadmap::txLaneBoundaryPtr left_boundary_ptr;
  hadmap::txLaneBoundaryPtr right_boundary_ptr;
  hadmap::getBoundary(_map_handle, lane_ptr->getLeftBoundaryId(), left_boundary_ptr);
  hadmap::getBoundary(_map_handle, lane_ptr->getRightBoundaryId(), right_boundary_ptr);

  if (left_boundary_ptr && right_boundary_ptr) {
    // get left boundary point
    const hadmap::txCurve *left_curve_ptr = left_boundary_ptr->getGeometry();
    hadmap::txPoint left_point = left_curve_ptr->getPoint(left_curve_ptr->getLength());
    // VLOG_1 << "left_point.x " << left_point.x * 10E6 << "\n";

    // get right boundary point
    const hadmap::txCurve *right_curve_ptr = right_boundary_ptr->getGeometry();
    hadmap::txPoint right_point = right_curve_ptr->getPoint(right_curve_ptr->getLength());
    // VLOG_1 << "right_point.x " << right_point.x * 10E6 << "\n";

    // calculate lane width
    // lane_width_1 = sqrt(pow(left_point.x - right_point.x, 2) + pow(left_point.y - right_point.y, 2));
    // lane_width_1 = lane_width_1 * 1.0e5;
    CPosition left_loc = CPosition(left_point.x, left_point.y, left_point.z, Coord_WGS84);
    WGS84ToENU(left_loc);
    CPosition right_loc = CPosition(right_point.x, right_point.y, right_point.z, Coord_WGS84);
    WGS84ToENU(right_loc);
    lane_width = CEvalMath::AbsoluteDistance2D(left_loc, right_loc);
    // VLOG_1 << "lane_width:  " << lane_width << "\n";
  }
  return lane_width;
}

// whether it an opposite direction
bool MapManager::IsOppositeDirection(const CLocation &loc_1, const CLocation &loc_2) {
  double yaw_diff = CEvalMath::YawDiff(loc_1.GetEuler(), loc_2.GetEuler());
  return (abs(yaw_diff) <= M_PI && std::abs(yaw_diff) >= 0.667 * M_PI);  // >= 2.093
}

// whether two points in range
bool MapManager::IsInRange(const CLocation &loc_1, const CLocation &loc_2, const double radius) {
  double dist = CEvalMath::AbsoluteDistance2D(loc_1.GetPosition(), loc_2.GetPosition());
  if (dist <= radius) {
    return true;
  }
  return false;
}

double MapManager::GetSpeedLimitFromMap(const hadmap::txLanePtr &lane_ptr) {
  // CElapsedTime e1("GetSpeedLimitFromMap");
  double speed_limit_from_map = -1.0;
  if (lane_ptr) {
    // get lane speed limit from hadmap
    speed_limit_from_map = lane_ptr->getSpeedLimit() / 3.6;
  }
  return speed_limit_from_map > 1.0 ? speed_limit_from_map : -1.0;
}

void MapManager::TransAllObjects() {
  hadmap::txObjects all_objects;
  std::vector<hadmap::txLaneId> lane_ids;
  std::vector<hadmap::OBJECT_TYPE> obj_types;
  auto ret_code = hadmap::getObjects(_map_handle, lane_ids, obj_types, all_objects);
  if (ret_code == TX_HADMAP_DATA_OK) {
    LOG_ERROR << "[MapManager] all_objects: " << all_objects.size() << "\n";
    for (auto obj_iter = all_objects.begin(); obj_iter != all_objects.end(); obj_iter++) {
      try {
        hadmap::txObjectPtr object_ptr = *obj_iter;
        int32_t obj_type = object_ptr->getObjectType();
        eval::EvalPoints sample_points;
        sample_points.reserve(const_MSPS);
        hadmap::txPoint &&map_obj_pos = object_ptr->getPos();
        if (std::abs(map_obj_pos.x) >= 1e-6 && std::abs(map_obj_pos.y) >= 1e-6) {
          if (m_map_objects.find(obj_type) == m_map_objects.end())
            m_map_objects[obj_type] = std::make_shared<EvalMapObject>();

          const hadmap::txCurve *curve_ptr = object_ptr->getGeom()->getGeometry();

          if (curve_ptr) ConvertToEvalPoints(curve_ptr, sample_points, {0.0, 0.0, 0.0}, -1.0, false, false);
          m_map_objects[obj_type]->m_samples.push_back(sample_points);
          m_map_objects[obj_type]->m_map_objects.push_back(object_ptr);
          m_map_objects[obj_type]->m_object_type = static_cast<hadmap::OBJECT_TYPE>(obj_type);

          // trans map object to stat actor
          if (m_map_stat_actors.find(obj_type) == m_map_stat_actors.end())
            m_map_stat_actors[obj_type] = std::make_shared<EvalMapStatActor>();

          std::shared_ptr<CStaticActor> actor_obj = std::make_shared<CStaticActor>();

          actor_obj->SetID(object_ptr->getId());
          auto pos_ptr = actor_obj->MutableLocation()->MutablePosition();
          auto rpy_ptr = actor_obj->MutableLocation()->MutableEuler();

          pos_ptr->SetValues(map_obj_pos.x, map_obj_pos.y, map_obj_pos.z, CoordType::Coord_WGS84);
          WGS84ToENU(*pos_ptr);
          double roll = 0, pitch = 0, yaw = 0;
          object_ptr->getRPY(roll, pitch, yaw);
          rpy_ptr->SetValues(0.0, 0.0, yaw * M_PI / 180);

          double length = 0, width = 0, height = 0;
          object_ptr->getLWH(length, width, height);
          actor_obj->MutableShape()->SetValues(length, width, height);

          m_map_stat_actors[obj_type]->m_samples.push_back(sample_points);
          m_map_stat_actors[obj_type]->m_map_actors.push_back(actor_obj);
          m_map_stat_actors[obj_type]->m_object_type = static_cast<hadmap::OBJECT_TYPE>(obj_type);
          VLOG_0 << "[MapManager] obj_type:  " << obj_type << "; id:  " << object_ptr->getId() << "; lwh:  " << length
                 << "," << width << "," << height << "; rpy:  " << roll << "," << pitch << "," << yaw << std::fixed
                 << std::setprecision(9) << "; pos:  " << map_obj_pos.x << "," << map_obj_pos.y << "," << map_obj_pos.z
                 << "\n";
        }
      } catch (const std::exception &e) {
        LOG_ERROR << "fail to calculate hadmap objects geometory. " << e.what() << "\n";
      }
    }
  } else {
    VLOG_2 << "fail to query map objects,  hadmap return code is " << ret_code << "\n";
  }
}

// return 0 1 2 4 8 -> 0~15 15~30 30~60 60~90
uint8_t MapManager::GetSlopeGrade(const hadmap::txRoadPtr &road_ptr) {
  auto slope_vec = road_ptr->getSlope();
  double sumSlope = 0;
  for (const auto &slope : slope_vec) {
    sumSlope += std::abs(slope.m_slope);
  }
  double slope_avg = sumSlope / slope_vec.size();
  double angle = atan(slope_avg) * 180 / M_PI;
  // std::cout << "angle: [" << angle << "] ";
  if (angle < 30.0) {
    return 1 << int(angle / 15.0);
  }
  return 1 << int(angle / 30.0) + 1;
}

bool MapManager::GetCondOnSlopeGrade(const hadmap::txRoadPtr &road_ptr) { return GetSlopeGrade(road_ptr) >= 4; }

double MapManager::GetSharpCurve(const hadmap::txRoadPtr &road_ptr) {
  auto curve_vec = road_ptr->getCurvature();
  double curve_sum = 0;
  // std::cout << std::endl << "----------" << std::endl;
  for (const auto &curve : curve_vec) {
    curve_sum += curve.m_curvature;
    // std::cout << curve.m_curvature << std::endl;
  }
  // std::cout << "----------" << std::endl;
  // std::cout << curve_sum / curve_vec.size() << "  " << curve_vec.size() << " ";
  // return (curve_sum / curve_vec.size()) > 0.02;
  return curve_sum / curve_vec.size();
}

bool MapManager::GetCondOnSharpCurve(const hadmap::txRoadPtr &road_ptr) { return GetSharpCurve(road_ptr) >= 0.02; }

bool MapManager::GetCondUnbidirectional(const hadmap::txLaneId &lane_id) {
  // CElapsedTime e1("GetCondUnbidirectional");
  const hadmap::txLanes lanes = GetLanesUnderSection(lane_id);
  VLOG_1 << "this is first lane id " << lanes.front()->getId() << "\n";
  if (!lanes.empty() && lanes.front()->getId() > 0) {
    VLOG_1 << "isBidirectional \n";
    return true;
  }
  return false;
}

//
// whether the road is narrow road
// when the total ground width of two-way one lane is <= 6m
// \par References:
// [1] http://www.gsglbz.com/articles/dzlzqb.html
//
bool MapManager::GetCondOnNarrowRoad(const hadmap::txLanePtr &lane_ptr) {
  // CElapsedTime e1("GetCondOnNarrowRoad");
  if (lane_ptr) {
    hadmap::txRoadPtr road_ptr = GetRoad(lane_ptr->getTxLaneId());
    if (road_ptr) {
      VLOG_1 << "this is road_ptr \n";
      bool is_unbidirectional = GetCondUnbidirectional(lane_ptr->getTxLaneId());
      if (is_unbidirectional) {
        VLOG_1 << "isBidirectional \n";
        return lane_ptr->getLaneWidth() <= 3.0;
      }
    }
  }
  return false;
}

bool MapManager::GetCondHasParkingSign(const CPosition &pivot_enu, double radius) {
  hadmap::OBJECT_TYPE obj_type = hadmap::OBJECT_TYPE::OBJECT_TYPE_TrafficSign;
  std::vector<hadmap::OBJECT_SUB_TYPE> obj_sub_types{hadmap::SIGN_BAN_LONG_PARKING, hadmap::SIGN_INDOCATION_PARKING};

  EvalMapObjectPtr parking_sign_objs = GetMapObjects(obj_type, obj_sub_types, pivot_enu, radius);

  if (parking_sign_objs.get() == nullptr) {
    return false;
  }

  return parking_sign_objs->m_map_objects.size() >= 1;
}

bool MapManager::GetCondHasParkingMarking(const CPosition &pivot_enu, double radius) {
  bool result = false;

  // deal OBJECT_TYPE_RoadSign
  hadmap::OBJECT_TYPE obj_type_road_sign = hadmap::OBJECT_TYPE::OBJECT_TYPE_RoadSign;
  std::vector<hadmap::OBJECT_SUB_TYPE> obj_sub_types_road_sign{hadmap::RoadSign_Disable_Parking};
  EvalMapObjectPtr parking_road_sign_objs =
      GetMapObjects(obj_type_road_sign, obj_sub_types_road_sign, pivot_enu, radius);

  if (parking_road_sign_objs.get() == nullptr) {
    result = false;
  } else {
    result = parking_road_sign_objs->m_map_objects.size() >= 1;
  }

  if (result) {
    return result;
  }

  // deal OBJECT_TYPE_ParkingSpace
  hadmap::OBJECT_TYPE obj_type_parking_space = hadmap::OBJECT_TYPE::OBJECT_TYPE_ParkingSpace;
  std::vector<hadmap::OBJECT_SUB_TYPE> obj_sub_types_parking_space{
      hadmap::Parking_Space_001, hadmap::Parking_Space_002, hadmap::Parking_Space_003,
      hadmap::Parking_Space_004, hadmap::Parking_Space_005, hadmap::Parking_Space_006};

  EvalMapObjectPtr parking_parking_space_objs =
      GetMapObjects(obj_type_parking_space, obj_sub_types_parking_space, pivot_enu, radius);

  if (parking_parking_space_objs.get() == nullptr) {
    return false;
  }
  return parking_parking_space_objs->m_map_objects.size() >= 1;
}

bool MapManager::GetCondHasTunnel(const CPosition &pivot_enu, double radius) {
  hadmap::OBJECT_TYPE obj_type = hadmap::OBJECT_TYPE::OBJECT_TYPE_Tunnel;
  std::vector<hadmap::OBJECT_SUB_TYPE> obj_sub_types{hadmap::Tunnel_001};

  EvalMapObjectPtr objs = GetMapObjects(obj_type, obj_sub_types, pivot_enu, radius);

  if (objs.get() == nullptr) {
    return false;
  }

  return objs->m_map_objects.size() >= 1;
}

bool MapManager::GetCondHasPedestrianBridge(const CPosition &pivot_enu, double radius) {
  hadmap::OBJECT_TYPE obj_type = hadmap::OBJECT_TYPE::OBJECT_TYPE_PedestrianBridge;
  std::vector<hadmap::OBJECT_SUB_TYPE> obj_sub_types{hadmap::PedestrianBridge};

  EvalMapObjectPtr objs = GetMapObjects(obj_type, obj_sub_types, pivot_enu, radius);

  if (objs.get() == nullptr) {
    return false;
  }

  return objs->m_map_objects.size() >= 1;
}

}  // namespace eval
