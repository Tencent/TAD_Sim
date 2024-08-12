// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "txcar_base.h"

#include <cmath>

#include "eigen3/Eigen/Dense"

#ifndef M_PI
#  define M_PI (3.14159265358979323846)
#endif

namespace tx_car {
struct TxCarEuler {
  double roll;
  double pitch;
  double yaw;
};

// FLU: front-left-up, FRD: front-right-down
class TxCarMath {
 public:
  static Eigen::Quaterniond toQuaternion(const TxCarEuler &euler);
  static TxCarEuler toEuler(const Eigen::Matrix3d &rot);
  static TxCarEuler toEuler(const Eigen::Quaterniond &q);
  static void saturation(Eigen::Vector3d &vec, double lowLimit = -1e-6, double upLimit = 1e-6);
  static void saturation(double &var, double lowLimit = -1e-6, double upLimit = 1e-6);

 public:
  // from FRD to FLU
  Eigen::Vector3d FRD_To_FLU(const Eigen::Vector3d &frd_vec);

  // from FLU to FRD
  Eigen::Vector3d FLU_To_FRD(const Eigen::Vector3d &flu_vec);

  // from FRD to FLU
  TxCarEuler FRD_To_FLU(const TxCarEuler &frd_euler);

  // from FLU to FRD
  TxCarEuler FLU_To_FRD(const TxCarEuler &flu_euler);

 private:
  // FLU: front-left-up, FRD: front-right-down
  const TxCarEuler Euler_Roll_180 = {M_PI, 0.0, 0.0};
  const Eigen::Matrix3d R_Roll_180 = TxCarMath::toQuaternion(Euler_Roll_180).toRotationMatrix();
  const Eigen::Matrix3d R_FLU_FRD = R_Roll_180;
  const Eigen::Matrix3d R_FRD_FLU = R_Roll_180;
};
}  // namespace tx_car
