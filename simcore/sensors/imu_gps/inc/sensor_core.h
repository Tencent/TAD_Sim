// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "Eigen/Dense"
#include "glog/logging.h"

#include "basic.pb.h"
#include "osi_gps.pb.h"
#include "osi_imu.pb.h"

#define _DEG_2_RAD (1.0 / 180.0 * 3.1415926)
#define _RAD_2_DEG (1.0 / 3.1415926 * 180.0)
#define _MS_2_KMH (3.6)

#define _DEFAULT_GAUSSIAN_NOISE_ \
  { 0.0, 0.1 }
#define _DEFAULT_SENSOR_NOISE_ _DEFAULT_GAUSSIAN_NOISE_

namespace sensor {
struct SGAUSSIANNoise {
  double _mean;
  double _std_error;
};

typedef SGAUSSIANNoise SSensorNoise;

class CSensorBase {
 public:
  CSensorBase();
  virtual ~CSensorBase();

 protected:
  std::string _name;
};

enum QuatTransOrder { Order_ZYX, Order_ZXY, Order_XYZ };

class CSensorMath {
 public:
  static double _gravity;
  static Eigen::Vector3d _gravityVec;

 public:
  // get rotation quaternion of angular velocity by deltatime
  static Eigen::Quaterniond deltaRotate(const Eigen::Vector3d &cur_angular_v, const double &dt);
};
}  // namespace sensor
