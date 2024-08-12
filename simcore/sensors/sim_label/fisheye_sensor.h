// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>
#include "camera_sensor.h"
#include "scene_sensor_group.pb.h"

class FisheyeSensor : public CameraSensor {
 public:
  explicit FisheyeSensor(int id);

  void setDistortion(double k1, double k2, double k3, double k4);

  virtual Eigen::Vector2d world2pixel(const Eigen::Vector3d &wp) const;

 protected:
  double k4 = 0;
};
extern std::map<int, std::shared_ptr<FisheyeSensor>> fisheyes;

bool LoadFisheye(const sim_msg::Sensor &sensor, const std::string &device);
