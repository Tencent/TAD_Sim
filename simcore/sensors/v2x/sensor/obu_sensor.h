/**
 * @file ObuSensor.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>
#include "ini.h"
#include "v2x_asn1_2020.pb.h"

struct ObuConfig {
  int FrequencyBSM = 10;
  double DistanceOpen = 800;
  double DistanceCity = 300;
  int Band = 2;  // 5905-5915，5915-5925，5905-5925
  double Mbps = 10;
  double SystemDelay = 10;
  double CommuDelay = 20;
  bool TriggerImmediately = true;
  double PosAccuracy = 0.5;
  bool NoTeam = false;
  bool DisableRSU = false;
  std::string PreBSM;
};

class ObuSensor {
 public:
  ObuSensor();
  virtual ~ObuSensor();

  ObuConfig config;

  virtual void setPosition(const Eigen::Vector3d &p);

  virtual bool Init();

 protected:
  Eigen::Vector3d T;
};
