/**
 * @file UltrasonicSensor.h
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
#include "fov_filter_sensor.h"
#include "scene_sensor_group.pb.h"

class UltrasonicSensor : public FovFilterSensor {
 public:
  explicit UltrasonicSensor(int id);

  // get nearest distance
  double distance(const Eigen::Vector3d &loc, double roll, double pitch, double yaw, double len, double wid,
                  double hei) const;

  virtual void setRotation(double roll, double pitch, double yaw);

  virtual bool Init();

 private:
  Eigen::Matrix3d K;
  Eigen::Matrix3d Ki;
  bool inFov(const Eigen::Vector3d &wp, double dis = 0) const;
  double inverseBilinearInterpolation2D(Eigen::Vector3d &pt, const Eigen::Vector3d &p1, const Eigen::Vector3d &p2,
                                        const Eigen::Vector3d &p3, const Eigen::Vector3d &p4) const;
};
extern std::map<int, std::shared_ptr<UltrasonicSensor>> ultrasonics;

bool LoadUltrasonic(const sim_msg::Sensor &sensor, const std::string &device);
