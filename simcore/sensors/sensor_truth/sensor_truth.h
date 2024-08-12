/**
 * @file SensorTruth.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <set>
#include "age.h"
#include "fov_filter_sensor.h"
#include "noise.h"
#include "truth_base.h"
#include "osi_datarecording.pb.h"
#include "traffic.pb.h"
#include "types/map_defs.h"
#include "union.pb.h"

class SensorTruth : public TruthBase {
 public:
  SensorTruth() = default;

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 protected:
 private:
  std::map<int, Age[4]> ages;
  Noise noise;

  void handle_ego(const FovFilterSensor &fov, const sim_msg::Union &union_location, osi3::SensorData *sensordata,
                  double time_stamp);
  void handle_traffic(const FovFilterSensor &fov, const sim_msg::Traffic &traffic, osi3::SensorData *sensordata,
                      double time_stamp);
};
