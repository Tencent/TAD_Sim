// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>
#include "fov_filter_sensor.h"
#include "scene_sensor_group.pb.h"

class LidarSensor : public FovFilterSensor {
 public:
  explicit LidarSensor(int id);
  virtual bool Init();
  bool world2pixel(const Eigen::Vector3d &wp, Eigen::Vector2d &uv) const;

 private:
  double fps = 10;
  int rayNum = 128;
  double resHorizonal = 0.001745329251994329576923690768489;  // 0.01du

  std::vector<double> verAngle;

  //
  double resVerial = 3.4906585039886591538473815369772e-4;  // 0.02du
  std::vector<int> verMap;

  friend bool LoadLidar(const sim_msg::Sensor &, const std::string &);
};
extern std::map<int, std::shared_ptr<LidarSensor>> lidars;

bool LoadLidar(const sim_msg::Sensor &sensor, const std::string &device);
