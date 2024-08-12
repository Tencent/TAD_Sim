/**
 * @file FovFilterSensor.h
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

class FovFilterSensor {
 public:
  explicit FovFilterSensor(int id);
  virtual ~FovFilterSensor();

  int ID() const { return id; }
  double getRange() const { return dRange; }

  // install slot
  virtual void setPosition(const Eigen::Vector3d& p);
  virtual void setRotation(double roll, double pitch, double yaw);
  virtual void getPosition(Eigen::Vector3d& p);
  virtual void getRotation(Eigen::Vector3d& rpy);
  // ego car
  virtual void setCarPosition(const Eigen::Vector3d& p);
  virtual void setCarRoatation(double roll, double pitch, double yaw);

  virtual void setFov(double v, double h, double r);

  virtual bool Init();
  virtual bool inFov(const Eigen::Vector3d& wp) const;
  virtual bool inFov(const std::vector<Eigen::Vector3d>& wp) const;

  virtual double faceto(const Eigen::Vector3d& nor);
  virtual Eigen::Vector3d FovVector(const Eigen::Vector3d& wp) const;
  virtual Eigen::Vector3d FovVectorWithoutSelfRot(const Eigen::Vector3d& wp) const;
  virtual void FovRotator(double& roll, double& pitch, double& yaw) const;
  virtual Eigen::Vector3d FovRotator(const Eigen::Vector3d& np) const;
  virtual Eigen::Vector3d FovRotatorWithoutSelfRot(const Eigen::Vector3d& np) const;
  virtual Eigen::Vector3d WorldVectorWithoutSelfRot(const Eigen::Vector3d& local) const;

  double downFOV = 0;
  double upFOV = 0;
  double leftFOV = 0;
  double rightFOV = 0;
  double dRange = 0;

 protected:
  int id = 0;
  Eigen::Matrix3d Rl;
  Eigen::Matrix3d Rli;
  Eigen::Vector3d Tl;
  Eigen::Matrix3d R;
  Eigen::Matrix3d Ri;
  Eigen::Vector3d T;
  double roll = 0, pitch = 0, yaw = 0;
};
