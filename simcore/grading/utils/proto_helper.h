// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "control.pb.h"
#include "control_v2.pb.h"
#include "location.pb.h"
#include "trajectory.pb.h"
#include "utils/eval_math.h"
#include "vehState.pb.h"

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>

#include <fstream>
#include <iostream>
#include <string>

namespace eval {
// from trajectory message to eval points vector
EvalPoints FromTrajectory(const sim_msg::Trajectory &traj);

// from control message to eval VehicleFeedback
VehicleBodyControl FromControl(const sim_msg::Control &ctrl);

// from control v2 message to eval VehicleFeedback
VehicleBodyControl FromControl(const sim_msg::Control_V2 &ctrl_v2);

// from vehicle state message to eval VehicleFeedback
VehicleBodyControl FromVehicleState(const sim_msg::VehicleState &vehicle_state);

// trans to ENU coord
void Traj2ENU(sim_msg::Trajectory &traj);

// trans to ENU coord
void Prediction2ENU(sim_msg::Prediction &pred);

// get nearest point index from trajectory by dist
int GetNearestByDist(const sim_msg::Trajectory &traj_enu, const CPosition &pivot_enu, double dist_thresh = 3.0);

// get nearest point index from trajectory by time
int GetNearestByTime(const sim_msg::Trajectory &traj_enu, double t_s, double t_thresh = 0.5);

// get nearest point index from trajectory by time
int GetNearestByTime(const sim_msg::Prediction &pred_enu, double t_s, double t_thresh = 0.101);

// from trajectory point to dynamic actor
CDynamicActor TrajPoint2DynActor(const sim_msg::TrajectoryPoint &point_enu);

// protobuf message to json string
bool protoToJson(const google::protobuf::Message &message, std::string &json_str);

// json string to protobuf message
bool jsonToProto(const std::string &json_str, google::protobuf::Message &message);

// load content from file
std::string loadContentFromFile(const std::string &filePath);
}  // namespace eval
