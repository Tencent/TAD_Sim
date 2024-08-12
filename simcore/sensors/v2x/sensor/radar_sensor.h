/**
 * @file RadarSensor.h
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
#include "rar_init_echo.h"

class RadarSensor {
 public:
  explicit RadarSensor(std::int64_t _id);
  ~RadarSensor();

  void setPosition(const Eigen::Vector3d &p);
  void setRotation(double roll, double pitch, double yaw);
  void setRadar(const radar_bit::radar_parameter &para);

  const radar_bit::radar_parameter &Para() const { return radar_para; }
  double &Delay() { return delay; }
  const double &Delay() const { return delay; }
  double &Frequency() { return frequency; }
  const double &Frequency() const { return frequency; }
  double &MaxDistance() { return maxDistance; }
  const double &MaxDistance() const { return maxDistance; }
  double &Timestamp() { return timestamp; }
  const double &Timestamp() const { return timestamp; }

  void setCarPosition(const Eigen::Vector3d &p);
  void setCarRoatation(double roll, double pitch, double yaw);
  double faceto(const Eigen::Vector3d &nor);
  bool inFov(const Eigen::Vector3d &wps) const;
  bool inFov(const std::vector<Eigen::Vector3d> &wps) const;
  Eigen::Vector3d Vector(const Eigen::Vector3d &wp) const;
  Eigen::Vector3d VectorWithoutSelfRot(const Eigen::Vector3d &wp) const;
  void Rotator(double &roll, double &pitch, double &yaw) const;
  Eigen::Vector3d Rotator(const Eigen::Vector3d &np) const;
  Eigen::Vector3d RotatorWithoutSelfRot(const Eigen::Vector3d &np) const;

  // inverse
  Eigen::Vector3d LocalVector(const Eigen::Vector3d &local) const;
  Eigen::Vector3d LocalRotator(const Eigen::Vector3d &rpy) const;

 private:
  int id;
  radar_bit::radar_parameter radar_para;
  double delay = 0;  // ms
  double frequency = 0;
  double maxDistance = 0;
  double timestamp = 0;
  Eigen::Matrix3d Rl;
  Eigen::Matrix3d Rli;
  Eigen::Vector3d Tl;
  Eigen::Matrix3d R;
  Eigen::Matrix3d Ri;
  Eigen::Vector3d T;
};
