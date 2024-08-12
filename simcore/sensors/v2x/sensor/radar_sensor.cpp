/**
 * @file RadarSensor.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "radar_sensor.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <sstream>

/**
 * @brief Construct a new Sensor object
 *
 * @param _id : sensor id
 */
RadarSensor::RadarSensor(std::int64_t _id) : id(_id), radar_para{0} {
  R = Eigen::Matrix3d::Identity();
  Rl = Eigen::Matrix3d::Identity();
  Ri = Eigen::Matrix3d::Identity();
  Rli = Eigen::Matrix3d::Identity();
}

/**
 * @brief Destroy the Sensor:: Sensor object
 *
 */
RadarSensor::~RadarSensor() {}

/**
 * @brief Set sensor position
 *
 * @param p position of installation
 */
void RadarSensor::setPosition(const Eigen::Vector3d &p) { Tl = -p; }

/**
 * @brief Set sensor rotation
 *
 * @param roll roll angle of installation
 * @param pitch pitch angle of installation
 * @param yaw yaw angle of installation
 */
void RadarSensor::setRotation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Rli = q.toRotationMatrix();
  Rl = q.toRotationMatrix().inverse();
}

/**
 * @brief Set radar parameters
 *
 * @param para parameters of radar
 */
void RadarSensor::setRadar(const radar_bit::radar_parameter &para) { radar_para = para; }

/**
 * @brief Set car position
 *
 * @param p position of car
 */
void RadarSensor::setCarPosition(const Eigen::Vector3d &p) { T = -p; }

/**
 * @brief Set car rotation
 *
 * @param roll roll angle of car
 * @param pitch pitch angle of car
 * @param yaw yaw angle of car
 */
void RadarSensor::setCarRoatation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Ri = q.toRotationMatrix();
  R = Ri.inverse();
}

/**
 * @brief Calculate the local direction vector and radar normal vector
 *
 * @param nor direction vector of object
 * @return dot the local direction vector and radar normal vector
 */
double RadarSensor::faceto(const Eigen::Vector3d &nor) {
  Eigen::Vector3d n = Rli * Ri * Eigen::Vector3d(1, 0, 0);
  return -n.dot(nor.normalized());
}

/**
 * @brief calculate the point is in fov
 *
 * @param wp world position of point
 * @return true if it is in fov
 * @return false
 */
bool RadarSensor::inFov(const Eigen::Vector3d &wp) const {
  Eigen::Vector3d c = Vector(wp);
  if (c.norm() >= maxDistance) {
    return false;
  }
  double h = -std::atan2(c.y(), c.x());
  double v = std::atan2(c.z(), std::sqrt(c.x() * c.x() + c.y() * c.y()));
  return h > radar_para.hfov * 0.5 && h < -radar_para.hfov * 0.5 && v > radar_para.vfov * 0.5 &&
         v < -radar_para.vfov * 0.5;
}

/**
 * @brief
 *
 * @param wps world position of points
 * @return true if there is in fov
 * @return false if there is not in fov
 */
bool RadarSensor::inFov(const std::vector<Eigen::Vector3d> &wps) const {
  double minH = DBL_MAX, minV = DBL_MAX, maxH = -DBL_MAX, maxV = -DBL_MAX;
  for (const auto &wp : wps) {
    Eigen::Vector3d c = Vector(wp);
    if (c.norm() >= maxDistance) {
      continue;
    }
    double h = -std::atan2(c.y(), c.x());
    double v = std::atan2(c.z(), std::sqrt(c.x() * c.x() + c.y() * c.y()));
    minH = std::min(minH, h);
    minV = std::min(minV, v);
    maxH = std::max(maxH, h);
    maxV = std::max(maxV, v);
  }
  if (minH > maxH || minV > maxV) {
    return false;
  }

  return !(minH > radar_para.hfov * 0.5 || maxH < -radar_para.hfov * 0.5 || minV > radar_para.vfov * 0.5 ||
           maxV < -radar_para.vfov * 0.5);
}
/**
 * @brief Calculate the local vector
 *
 * @param wp world position of point
 * @return local vector
 */
Eigen::Vector3d RadarSensor::Vector(const Eigen::Vector3d &wp) const { return Rl * (R * (wp + T) + Tl); }

/**
 * @brief Calculate the local vector in car coordinate
 *
 * @param wp point in world coordinate
 * @return local vector in car coordinate
 */
Eigen::Vector3d RadarSensor::VectorWithoutSelfRot(const Eigen::Vector3d &wp) const { return R * (wp + T) + Tl; }

/**
 * @brief Rotate the euler angle into local coordinate
 *
 * @param roll
 * @param pitch
 * @param yaw
 */
void RadarSensor::Rotator(double &roll, double &pitch, double &yaw) const {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Eigen::Matrix3d m = Rl * R * (q.toRotationMatrix());
  Eigen::Vector3d a = m * Eigen::Vector3d(1, 0, 0);
  Eigen::Vector3d b = m * Eigen::Vector3d(0, 1, 0);
  yaw = std::atan2(a.y(), a.x());
  a = Eigen::AngleAxisd(-yaw, Eigen::Vector3d::UnitZ()) * a;
  b = Eigen::AngleAxisd(-yaw, Eigen::Vector3d::UnitZ()) * b;
  pitch = std::atan2(-a.z(), a.x());
  b = Eigen::AngleAxisd(-pitch, Eigen::Vector3d::UnitY()) * b;
  roll = std::atan2(b.z(), b.y());
}

/**
 * @brief Rotate the euler angle into local coordinate
 *
 * @param np direction vector in world coordinate
 * @return direction vector in local coordinate
 */
Eigen::Vector3d RadarSensor::Rotator(const Eigen::Vector3d &np) const { return Rl * R * np; }

/**
 * @brief Rotate the euler angle into car coordinate
 *
 * @param np direction vector in world coordinate
 * @return direction vector in car coordinate
 */
Eigen::Vector3d RadarSensor::RotatorWithoutSelfRot(const Eigen::Vector3d &np) const { return R * np; }

/**
 * @brief local coordinate to world coordinate
 *
 * @param local vector in local coordinate
 * @return vector in world coordinate
 */
Eigen::Vector3d RadarSensor::LocalVector(const Eigen::Vector3d &local) const { return Ri * (Rli * local - Tl) - T; }

/**
 * @brief rotate the vector into world coordinate
 *
 * @param np direction in local coordinate
 * @return direction vector in world coordinate
 */
Eigen::Vector3d RadarSensor::LocalRotator(const Eigen::Vector3d &np) const {
  Eigen::Quaterniond q = Eigen::AngleAxisd(np.z(), Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(np.y(), Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(np.x(), Eigen::Vector3d::UnitX());
  Eigen::Matrix3d m = Rli * Ri * q.matrix();
  Eigen::Vector3d rpy = m.eulerAngles(2, 1, 0);
  std::swap(rpy.x(), rpy.z());
  return rpy;
}
