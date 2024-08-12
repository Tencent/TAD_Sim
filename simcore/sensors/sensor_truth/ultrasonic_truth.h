/**
 * @file UltrasonicTruth.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <fstream>
#include <map>
#include "age.h"
#include "object_size.h"
#include "osi_util.h"
#include "ultrasonic_sensor.h"
#include "visable_calculate.h"
#include "osi_datarecording.pb.h"
#include "traffic.pb.h"
#include "union.pb.h"

class UltrasonicTruth : public OsiSensorData {
 public:
  void Init(tx_sim::InitHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;

 private:
  const void* handle_traffic(const UltrasonicSensor& ult, const sim_msg::Union& union_location,
                             const sim_msg::Traffic& traffic, const Eigen::Vector3d& locPosition, int& type,
                             double& mindis);
};
