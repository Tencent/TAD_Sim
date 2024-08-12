// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "utils/proto_helper.h"
#include "actors/c_actor_dynamic.h"
#include "manager/map_manager.h"

#include <stdexcept>

namespace eval {
EvalPoints FromTrajectory(const sim_msg::Trajectory &traj) {
  EvalPoints traj_pts;
  traj_pts.reserve(traj.point_size());

  auto map_mgr = CMapManager::GetInstance();
  CPosition pos;
  CEuler euler;

  for (auto i = 0; i < traj.point_size() && map_mgr; ++i) {
    pos.SetValues(traj.point().at(i).x(), traj.point().at(i).y(), traj.point().at(i).z(), Coord_WGS84);
    euler.SetValues(0.0, 0.0, traj.point().at(i).theta());
    map_mgr->WGS84ToENU(pos);
    traj_pts.emplace_back(pos, euler);
  }

  return traj_pts;
}

void Traj2ENU(sim_msg::Trajectory &traj) {
  auto map_mgr = CMapManager::GetInstance();

  for (auto i = 0; i < traj.point_size() && map_mgr; ++i) {
    CPosition pos(traj.point().at(i).x(), traj.point().at(i).y(), traj.point().at(i).z(), Coord_WGS84);
    map_mgr->WGS84ToENU(pos);
    traj.mutable_point()->at(i).set_x(pos.GetX());
    traj.mutable_point()->at(i).set_y(pos.GetY());
    traj.mutable_point()->at(i).set_z(pos.GetZ());
  }
}

// trans to ENU coord
void Prediction2ENU(sim_msg::Prediction &pred) {
  auto map_mgr = CMapManager::GetInstance();
  if (map_mgr.get() == nullptr) {
    LOG_ERROR << "map manager pointer is nullptr.\n";
    throw std::runtime_error("map manager pointer is nullptr");
    return;
  }

  for (auto i = 0; i < pred.point_size() && map_mgr; ++i) {
    CPosition pos(pred.point().at(i).x(), pred.point().at(i).y(), pred.point().at(i).z(), Coord_WGS84);
    map_mgr->WGS84ToENU(pos);
    pred.mutable_point()->at(i).set_x(pos.GetX());
    pred.mutable_point()->at(i).set_y(pos.GetY());
    pred.mutable_point()->at(i).set_z(pos.GetZ());
  }
}

VehicleBodyControl FromControl(const sim_msg::Control &ctrl) {
  VehicleBodyControl veh_fb;

  veh_fb.m_harzard = static_cast<OnOff>(ctrl.body_cmd().warn_light());
  veh_fb.m_high_beam = static_cast<OnOff>(ctrl.body_cmd().high_beam());
  veh_fb.m_horn = static_cast<OnOff>(ctrl.body_cmd().horn());
  veh_fb.m_low_beam = static_cast<OnOff>(ctrl.body_cmd().low_beam());
  veh_fb.m_turn_left_lamp =
      static_cast<OnOff>(ctrl.body_cmd().turn_light() == sim_msg::Control_TurnLight::Control_TurnLight_TURN_LEFT);
  veh_fb.m_turn_right_lamp =
      static_cast<OnOff>(ctrl.body_cmd().turn_light() == sim_msg::Control_TurnLight::Control_TurnLight_TURN_RIGHT);
  veh_fb.m_position_lamp = static_cast<OnOff>(ctrl.body_cmd().position_lamp());

  return veh_fb;
}

VehicleBodyControl FromControl(const sim_msg::Control_V2 &ctrl_v2) {
  VehicleBodyControl veh_fb;

  veh_fb.m_horn = static_cast<OnOff>(ctrl_v2.body_cmd().horn());

  veh_fb.m_turn_left_lamp = static_cast<OnOff>(ctrl_v2.body_cmd().left_turn());
  veh_fb.m_turn_right_lamp = static_cast<OnOff>(ctrl_v2.body_cmd().right_turn());

  veh_fb.m_low_beam = static_cast<OnOff>(ctrl_v2.body_cmd().low_beam());
  veh_fb.m_position_lamp = static_cast<OnOff>(ctrl_v2.body_cmd().position_lamp());
  veh_fb.m_high_beam = static_cast<OnOff>(ctrl_v2.body_cmd().high_beam());
  veh_fb.m_harzard = static_cast<OnOff>(ctrl_v2.body_cmd().hazard_light());

  return veh_fb;
}

VehicleBodyControl FromVehicleState(const sim_msg::VehicleState &vehicle_state) {
  VehicleBodyControl veh_fb;
  veh_fb.m_turn_left_lamp = static_cast<OnOff>(vehicle_state.body_state().leftturnsiglampsts());
  veh_fb.m_turn_right_lamp = static_cast<OnOff>(vehicle_state.body_state().rightturnsiglampsts());
  return veh_fb;
}

// get nearest point index from trajectory by dist0
int GetNearestByDist(sim_msg::Trajectory &traj, const CPosition &pivot_enu, double dist_thresh) {
  int index = -1;
  double min_dist = 1e12;
  CPosition pt;

  for (int i = 0; i < traj.point_size(); ++i) {
    const sim_msg::TrajectoryPoint &point = traj.point().at(i);
    pt.SetValues(point.x(), point.y(), point.z());
    double dist = CEvalMath::Distance2D(pt, pivot_enu);
    if (dist <= min_dist && dist <= dist_thresh) {
      min_dist = dist;
      index = i;
    }
  }

  return index;
}

// get nearest point index from trajectory by time
int GetNearestByTime(const sim_msg::Trajectory &traj, double t_s, double t_thresh) {
  int index = -1;
  double min_t_diff = 1e12;

  for (int i = 0; i < traj.point_size(); ++i) {
    const sim_msg::TrajectoryPoint &point = traj.point().at(i);
    double t_diff = std::abs(point.t() - t_s);
    if (t_diff <= min_t_diff && t_diff <= t_thresh) {
      min_t_diff = t_diff;
      index = i;
    }
  }

  return index;
}

int GetNearestByTime(const sim_msg::Prediction &pred_enu, double t_s, double t_thresh) {
  int index = -1;
  double min_t_diff = 1e12;

  for (int i = 0; i < pred_enu.point_size(); ++i) {
    const sim_msg::TrajectoryPoint &point = pred_enu.point().at(i);
    double t_diff = std::abs(point.t() - t_s);
    if (t_diff <= min_t_diff && t_diff <= t_thresh) {
      min_t_diff = t_diff;
      index = i;
    }
  }

  return index;
}

// from trajectory point to dynamic actor
CDynamicActor TrajPoint2DynActor(const sim_msg::TrajectoryPoint &point_enu) {
  CDynamicActor fellow;

  auto pos = fellow.MutableLocation()->MutablePosition();
  auto euler = fellow.MutableLocation()->MutableEuler();
  auto speed = fellow.MutableSpeed();
  auto acc = fellow.MutableAcc();

  pos->SetValues(point_enu.x(), point_enu.y(), point_enu.z());
  euler->SetValues(0.0, 0.0, point_enu.theta());
  speed->SetValues(point_enu.v() * std::cos(point_enu.theta()), point_enu.v() * std::sin(point_enu.theta()), 0.0);
  acc->SetValues(point_enu.a(), 0.0, 0.0);

  return fellow;
}

bool protoToJson(const google::protobuf::Message &message, std::string &json_str) {
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = true;
  options.always_print_primitive_fields = true;
  options.preserve_proto_field_names = true;

  json_str.clear();
  return google::protobuf::util::MessageToJsonString(message, &json_str, options).ok();
}

bool jsonToProto(const std::string &json_str, google::protobuf::Message &message) {
  return google::protobuf::util::JsonStringToMessage(json_str, &message).ok();
}

// load content from file
std::string loadContentFromFile(const std::string &filePath) {
  std::ifstream file(filePath);

  if (file.is_open()) {
    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return file_contents;
  } else {
    std::cerr << "Error: could not open file" << std::endl;
  }

  return std::string();
}
}  // namespace eval
