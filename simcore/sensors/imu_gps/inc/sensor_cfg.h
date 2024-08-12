// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "sensor_core.h"

#include <fstream>

namespace sensor {
class CSensorCfg : public CSensorBase {
 private:
  void stringSplit(const std::string strIn, const std::string &spliter, std::vector<std::string> &splitedStrs);

 public:
  // set default imu sensor cfg
  CSensorCfg();

  // parse imu config from json dict
  bool parseIMUConfig(const std::string &buffer, const std::string &groupname, osi3::imu_IMUCfg &imuCfg);

  // parse gps config from json dict
  bool parseGPSConfig(const std::string &buffer, const std::string &groupname, osi3::GPS &gps);
};
}  // namespace sensor
