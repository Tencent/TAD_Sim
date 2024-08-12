// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txcar_math.h"
#include <cmath>
#include "inc/car_log.h"

namespace tx_car {
Eigen::Quaterniond TxCarMath::toQuaternion(const TxCarEuler &euler) {
  Eigen::Quaterniond q;
  q = Eigen::AngleAxisd(euler.yaw, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(euler.pitch, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(euler.roll, Eigen::Vector3d::UnitX());
  // q.normalize();
  return q;
}
TxCarEuler TxCarMath::toEuler(const Eigen::Matrix3d &rot) {
  Eigen::Quaterniond q(rot);
  return toEuler(q);
}
TxCarEuler TxCarMath::toEuler(const Eigen::Quaterniond &quat) {
  struct Q {
    double w;
    double x;
    double y;
    double z;
  };

  TxCarEuler angles;
  const Eigen::Vector4d &q_xyzw = quat.coeffs();
  Q q{q_xyzw[3], q_xyzw[0], q_xyzw[1], q_xyzw[2]};

  // roll (x-axis rotation)
  double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
  double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
  angles.roll = std::atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  double sinp = std::sqrt(1.0 + 2 * (q.w * q.y - q.x * q.z));
  double cosp = std::sqrt(1.0 - 2 * (q.w * q.y - q.x * q.z));
  angles.pitch = -M_PI / 2.0 + 2 * atan2(sinp, cosp);

  // yaw (z-axis rotation)
  double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
  double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
  angles.yaw = std::atan2(siny_cosp, cosy_cosp);

  return angles;
}

void TxCarMath::saturation(Eigen::Vector3d &vec, double lowLimit, double upLimit) {
  for (auto i = 0; i < 3; ++i) {
    TxCarMath::saturation(vec[i], lowLimit, upLimit);
  }
}
void TxCarMath::saturation(double &var, double lowLimit, double upLimit) {
  if (lowLimit < upLimit) {
    for (auto i = 0; i < 3; ++i) {
      if (lowLimit < var && var < upLimit) {
        var = 0.0;
      }
    }
  } else {
    LOG_ERROR << "lowLimit should be smaller than upLimit.\n";
  }
}

// from FRD to FLU
Eigen::Vector3d TxCarMath::FRD_To_FLU(const Eigen::Vector3d &frd_vec) {
  Eigen::Vector3d flu_vec;
  flu_vec = R_FLU_FRD * frd_vec;
  return flu_vec;
}

// from FLU to FRD
Eigen::Vector3d TxCarMath::FLU_To_FRD(const Eigen::Vector3d &flu_vec) {
  Eigen::Vector3d frd_vec;
  frd_vec = R_FRD_FLU * flu_vec;
  return frd_vec;
}

// from FRD to FLU
TxCarEuler TxCarMath::FRD_To_FLU(const TxCarEuler &frd_euler) {
  TxCarEuler flu_euler;

  Eigen::Matrix3d R_FRD = toQuaternion(frd_euler).toRotationMatrix();
  Eigen::Matrix3d R_FLU = R_FLU_FRD * R_FRD;
  flu_euler = toEuler(R_FLU);

  return flu_euler;
}

// from FLU to FRD
TxCarEuler TxCarMath::FLU_To_FRD(const TxCarEuler &flu_euler) {
  TxCarEuler frd_euler;

  Eigen::Matrix3d R_FLU = toQuaternion(flu_euler).toRotationMatrix();
  Eigen::Matrix3d R_FRD = R_FRD_FLU * R_FLU;
  frd_euler = toEuler(R_FRD);

  return frd_euler;
}
}  // namespace tx_car
