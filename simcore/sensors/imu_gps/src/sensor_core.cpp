/**
 * @file sensor_core.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "sensor_core.h"

#include "basic.pb.h"
#include "osi_gps.pb.h"
#include "osi_imu.pb.h"

namespace sensor {
// gravity acceleration
double CSensorMath::_gravity = 9.8003266714;

Eigen::Vector3d CSensorMath::_gravityVec = {0.0, 0.0, CSensorMath::_gravity};

CSensorBase::CSensorBase() { _name = "imu_gps"; }

CSensorBase::~CSensorBase() {}

// get rotation quaternion of angular velocity by deltatime
Eigen::Quaterniond CSensorMath::deltaRotate(const Eigen::Vector3d &cur_angular_v, const double &dt) {
  Eigen::Vector3d w_hat2(cur_angular_v[0], cur_angular_v[1], cur_angular_v[2]);

  // delta rotation
  Eigen::Vector3d ha = w_hat2 * 0.5 * dt;  // vector of half angle

  double l = ha.norm();  // magnitude

  Eigen::Quaterniond quat;

  if (l > 1e-12) {
    ha *= sin(l) / l;
    Eigen::Quaterniond q(cos(l), ha.x(), ha.y(), ha.z());
    q.normalize();
    quat = q;
  } else {
    Eigen::Quaterniond q(1.0, ha.x(), ha.y(), ha.z());
    q.normalize();
    quat = q;
  }

  return quat;
}
}  // namespace sensor
