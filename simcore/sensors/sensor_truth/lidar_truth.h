/**
 * @file LidarTruth.h
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
#include "lidar_sensor.h"
#include "object_size.h"
#include "osi_util.h"
#include "truth_base.h"
#include "visable_calculate.h"
#include "traffic.pb.h"
#include "union.pb.h"

class LidarTruth : public OsiSensorData {
 public:
  void Init(tx_sim::InitHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;

 private:
  double minHitNum = 5;
  double completeness = 0.3;

  points VbufferInit(const ObjectBBox& bbox, double heading, const Eigen::Vector3d& pos, const LidarSensor& lid);
  void handle_traffic(const LidarSensor& lid, const sim_msg::Union& union_location, const sim_msg::Traffic& traffic,
                      const Eigen::Vector3d& locPosition, std::vector<VisableBuffer>& vbs);
};
