/**
 * @file rsu.h
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <vector>
#include "../sim_algorithm/sim_obu_congestion.h"
#include "base.h"
#include "msg.h"
#include "sensor_objects_def.h"

class Rsu : public Base {
 public:
  void Init(tx_sim::InitHelper& helper);
  void Reset(tx_sim::ResetHelper& helper);
  void Step(tx_sim::StepHelper& helper);

 private:
  std::vector<std::int64_t> FilterRsu(const sim_msg::Location& loc);
  void PopMessage2Msg(SimRsuSensors& sim);
  void FilterMessage2Sim(const sim_msg::Traffic& traffic, const sim_msg::Location& loc, const double time,
                         SimRsuSensors& sim);

 private:
  std::string groupname;
  std::map<std::pair<std::int64_t, MsgName>, double> rsu_timestamp_last;
  std::map<std::pair<std::int64_t, MsgName>, std::uint64_t> rsu_frame_count;
};
