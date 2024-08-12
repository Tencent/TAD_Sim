/**
 * @file FovFilterSensor.h
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
#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include "scene_sensor_group.pb.h"

class FovFilterSensor {
 public:
  explicit FovFilterSensor(int id);
  virtual ~FovFilterSensor();

  int ID() const { return id; }
  double getRange() const { return dRange; }

  // install slot
  virtual void setPosition(const Eigen::Vector3d &p);
  virtual void setRotation(double roll, double pitch, double yaw);
  virtual void getPosition(Eigen::Vector3d &p);
  virtual void getRotation(Eigen::Vector3d &rpy);
  // ego car
  virtual void setCarPosition(const Eigen::Vector3d &p);
  virtual void setCarRoatation(double roll, double pitch, double yaw);

  virtual void setFov(double v, double h, double r);

  virtual bool Init();
  virtual bool inFov(const Eigen::Vector3d &wp) const;
  virtual bool inFov(const std::vector<Eigen::Vector3d> &wp) const;

  virtual double faceto(const Eigen::Vector3d &nor);
  virtual Eigen::Vector3d FovVector(const Eigen::Vector3d &wp) const;
  virtual Eigen::Vector3d FovVectorWithoutSelfRot(const Eigen::Vector3d &wp) const;
  virtual void FovRotator(double &roll, double &pitch, double &yaw) const;
  virtual Eigen::Vector3d FovRotator(const Eigen::Vector3d &np) const;
  virtual Eigen::Vector3d FovRotatorWithoutSelfRot(const Eigen::Vector3d &np) const;
  virtual Eigen::Vector3d WorldVectorWithoutSelfRot(const Eigen::Vector3d &local) const;

 protected:
  int id = 0;
  double downFOV = 0;
  double upFOV = 0;
  double leftFOV = 0;
  double rightFOV = 0;
  double dRange = 0;
  Eigen::Matrix3d Rl;
  Eigen::Matrix3d Rli;
  Eigen::Vector3d Tl;
  Eigen::Matrix3d R;
  Eigen::Matrix3d Ri;
  Eigen::Vector3d T;
  double roll = 0, pitch = 0, yaw = 0;

  friend bool LoadFovFilter(const std::string &, const std::string &, const std::string &);
  friend bool AddDefaultFov(int, double, double, double);
};
extern std::map<int, std::shared_ptr<FovFilterSensor>> fovfilters;

bool LoadFovFilter(const sim_msg::Sensor &sensor, const std::string &device);
bool AddDefaultFov(int id, double v, double h, double r);

template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, double &value) {
  if (params.find(para) != params.end()) {
    value = std::atof(params.find(para)->second.c_str());
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, float &value) {
  if (params.find(para) != params.end()) {
    value = std::atof(params.find(para)->second.c_str());
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, int &value) {
  if (params.find(para) != params.end()) {
    value = std::atoi(params.find(para)->second.c_str());
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, std::string &value) {
  if (params.find(para) != params.end()) {
    value = params.find(para)->second;
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, bool &value) {
  if (params.find(para) != params.end()) {
    value = true;
    auto v = params.find(para)->second;
    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
    if (v == ("false") || v == ("0") || v == ("close") || v == ("disable")) value = false;
  }
}
