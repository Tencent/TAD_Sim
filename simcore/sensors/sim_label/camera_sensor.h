/**
 * @file CameraSensor.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>
#include "fov_filter_sensor.h"
#include "scene_sensor_group.pb.h"

class CameraSensor : public FovFilterSensor {
 public:
  explicit CameraSensor(int id);

  void setRotation(double roll, double pitch, double yaw);
  void setIntrinsic(const Eigen::Matrix3d &imat);
  void setDistortion(double k1, double k2, double k3, double p1, double p2);
  void setResolution(int w, int h);
  void setFPS(double fps);

  double FPS() const { return fps; }
  int W() const { return w; }
  int H() const { return h; }

  virtual bool Init();
  virtual bool inFov(const Eigen::Vector3d &wp) const;
  virtual Eigen::Vector2d world2pixel(const Eigen::Vector3d &wp) const;

 protected:
  Eigen::Matrix3d K;
  int w = 0, h = 0;
  double k1 = 0, k2 = 0, k3 = 0, p1 = 0, p2 = 0;
  double fps = 10;
};
extern std::map<int, std::shared_ptr<CameraSensor>> cameras;
extern std::map<int, std::shared_ptr<CameraSensor>> semantics;

bool LoadCamera(const sim_msg::Sensor &sensor, const std::string &device);
