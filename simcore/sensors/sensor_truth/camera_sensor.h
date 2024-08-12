/**
 * @file CameraSensor.h
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
  bool inFov(const Eigen::Vector3d &wp) const;
  bool inFov(const Eigen::Vector3d &wp, double distance) const;
  Eigen::Vector2d world2pixel(const Eigen::Vector3d &wp) const;

 private:
  Eigen::Matrix3d K;
  int w = 0, h = 0;
  double k1 = 0, k2 = 0, k3 = 0, p1 = 0, p2 = 0;
  double fps = 10;
};
extern std::map<int, std::shared_ptr<CameraSensor>> cameras;

bool LoadCamera(const sim_msg::Sensor &sensor, const std::string &device);
