// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <fstream>
#include <map>
#include <set>
#include "age.h"
#include "fov_filter_sensor.h"
#include "hmap.h"
#include "truth_base.h"
#include "traffic.pb.h"


class TrafficFilter : public TruthBase {
 public:
  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 public:
 private:
  std::string configDir;
  tx_sim::Vector3d map_ori;
  std::map<int, Age[3]> ages;
  std::set<int> fovids;
  double limitZ = 0;

  hadmap::txLaneId egoLaneid;

  void handle_traffic(const FovFilterSensor &fov, const sim_msg::Traffic &traffic, const Eigen::Vector3d &locPosition,
                      sim_msg::Traffic &out_traffic);

  bool parse_phase(const hadmap::txLaneId &tganeid, const sim_msg::Traffic &traffic,
                   sim_msg::TrafficLight phase_light[]);
};
