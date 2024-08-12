/**
 * @file sensor.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>
#include "rar_init_echo.h"

#define FAULT_INVALID -99999.99
struct CustomFault {
  double distance = FAULT_INVALID;
  double speed = FAULT_INVALID;
  double angle = FAULT_INVALID;
  double rcs = FAULT_INVALID;
  double ar = FAULT_INVALID;  // not used
};

class Sensor {
 public:
  explicit Sensor(std::int64_t id);
  ~Sensor();

  void setPosition(const Eigen::Vector3d &p);
  void setRotation(double roll, double pitch, double yaw);
  void setRadar(const radar_bit::radar_parameter &para);

  std::int64_t ID() const { return id; }
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
  bool inFov(const std::vector<Eigen::Vector3d> &wps) const;
  Eigen::Vector3d Vector(const Eigen::Vector3d &wp) const;
  Eigen::Vector3d VectorWithoutSelfRot(const Eigen::Vector3d &wp) const;
  void Rotator(double &roll, double &pitch, double &yaw) const;
  Eigen::Vector3d Rotator(const Eigen::Vector3d &np) const;
  Eigen::Vector3d RotatorWithoutSelfRot(const Eigen::Vector3d &np) const;

  // inverse
  Eigen::Vector3d LocalVector(const Eigen::Vector3d &local) const;
  Eigen::Vector3d LocalRotator(const Eigen::Vector3d &rpy) const;
  const std::map<std::int64_t, std::map<std::int64_t, CustomFault>> &getCustomFault() const { return customFault; }
  std::map<std::int64_t, std::map<std::int64_t, CustomFault>> &getCustomFault() { return customFault; }

 private:
  std::int64_t id;
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

  std::map<std::int64_t, std::map<std::int64_t, CustomFault>> customFault;
};
extern std::map<std::int64_t, std::shared_ptr<Sensor>> radarSensors;
extern std::int64_t ego_id;
int getWeather(double timestamp);
bool LoadRadarSensors(const std::string &buffer, const std::string &groupname);
