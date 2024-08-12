/**
 * @file CameraTruth.h
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
#include "osi_util.h"
#include "visable_calculate.h"
#include "object_size.h"
#include "camera_sensor.h"
#include "traffic.pb.h"
#include "union.pb.h"

class CameraTruth : public OsiSensorData {
 public:
  void Init(tx_sim::InitHelper& helper);
  void Step(tx_sim::StepHelper& helper);
  void Stop(tx_sim::StopHelper& helper);

 private:
  double maxDist = 100;
  double completeness = 0.5;
  double minArea = 20;

  points VbufferInit(const ObjectBBox& bbox, double heading, const Eigen::Vector3d& pos, const CameraSensor& cam);
  void handle_traffic(const CameraSensor& cam, const sim_msg::Traffic& traffic, const Eigen::Vector3d& locPosition,
                      std::vector<VisableBuffer>& vbs);
  void handle_ego(const CameraSensor& cam, const sim_msg::Union& union_location, const Eigen::Vector3d& locPosition,
                  std::vector<VisableBuffer>& vbs);
};
