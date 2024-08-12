/**
 * @file FovFilterSensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "fov_filter_sensor.h"
#include <float.h>
#include <Eigen/Geometry>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>

/**
 * @brief Construct a new Fov Filter Sensor:: Fov Filter Sensor object
 *
 * @param _id sensor id
 */
FovFilterSensor::FovFilterSensor(int _id) : id(_id) {
  R = Eigen::Matrix3d::Identity();
  Rl = Eigen::Matrix3d::Identity();
  Ri = Eigen::Matrix3d::Identity();
  Rli = Eigen::Matrix3d::Identity();
  Tl = Eigen::Vector3d::Zero();
  T = Eigen::Vector3d::Zero();
}

/**
 * @brief Destroy the Fov Filter Sensor:: Fov Filter Sensor object
 *
 */
FovFilterSensor::~FovFilterSensor() {}

/**
 * @brief set install position
 *
 * @param p position
 */
void FovFilterSensor::setPosition(const Eigen::Vector3d &p) { Tl = -p; }

/**
 * @brief set install rotation
 *
 * @param _roll roll of sensor
 * @param _pitch pitch of sensor
 * @param _yaw yaw of sensor
 */
void FovFilterSensor::setRotation(double _roll, double _pitch, double _yaw) {
  roll = _roll;
  pitch = _pitch;
  yaw = _yaw;
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Rli = q.toRotationMatrix();
  Rl = q.toRotationMatrix().inverse();
}
/**
 * @brief get position
 *
 * @param p return the position
 */
void FovFilterSensor::getPosition(Eigen::Vector3d &p) { p = -Tl; }
/**
 * @brief Get Rotation
 *
 * @param rpy return the rotation
 */
void FovFilterSensor::getRotation(Eigen::Vector3d &rpy) {
  rpy.x() = roll;
  rpy.y() = pitch;
  rpy.z() = yaw;
  // rpy = Rli.eulerAngles(2, 1, 0);
  // std::swap(rpy.x(), rpy.z());
}

/**
 * @brief initialization
 *
 * @return true on success
 * @return false on failure
 */
bool FovFilterSensor::Init() {
  if (leftFOV > rightFOV || downFOV > upFOV) return false;
  return true;
}

/**
 * @brief set car position
 *
 * @param p car position
 */
void FovFilterSensor::setCarPosition(const Eigen::Vector3d &p) { T = -p; }

/**
 * @brief set car rotation
 *
 * @param roll root of car
 * @param pitch pitch of car
 * @param yaw yaw of car
 */
void FovFilterSensor::setCarRoatation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Ri = q.toRotationMatrix();
  R = Ri.inverse();
}

/**
 * @brief set Fov
 *
 * @param v vertical fov
 * @param h horizontal fov
 * @param r range
 */
void FovFilterSensor::setFov(double v, double h, double r) {
  leftFOV = -h * 0.5;
  rightFOV = h * 0.5;
  downFOV = -v * 0.5;
  upFOV = v * 0.5;
  dRange = r;
  std::cout << "fov[" << id << "]:" << dRange << " " << leftFOV << " " << rightFOV << " " << downFOV << " " << upFOV
            << std::endl;
}

/**
 * @brief check whether a point is in the FOV
 *
 * @param wp point in world coordinate
 * @return true if it's inside
 * @return false if it's outside
 */
bool FovFilterSensor::inFov(const Eigen::Vector3d &wp) const {
  Eigen::Vector3d c = FovVector(wp);
  if (c.norm() >= dRange) {
    return false;
  }
  double h = -std::atan2(c.y(), c.x());
  double v = std::atan2(c.z(), std::sqrt(c.x() * c.x() + c.y() * c.y()));
  return h > leftFOV && h < rightFOV && v > downFOV && v < upFOV;
}

/**
 * @brief check whether points are in the FOV
 *
 * @param wps points in world coordinate
 * @return true if there is any point inside
 * @return false if there is no point inside
 */
bool FovFilterSensor::inFov(const std::vector<Eigen::Vector3d> &wps) const {
  bool isin = false;
  for (const auto &wp : wps) {
    isin |= inFov(wp);
  }
  return isin;
}
/**
 * @brief calcute a direction vector dot that faces to this sensor
 *
 * @param nor direction vector
 * @return dot of sensor normal and the direction vector
 */
double FovFilterSensor::faceto(const Eigen::Vector3d &nor) {
  Eigen::Vector3d n = Rli * Ri * Eigen::Vector3d(1, 0, 0);
  return -n.dot(nor.normalized());
}

/**
 * @brief calculate the local position of the sensor
 *
 * @param wp vector in world coordinate
 * @return local vector of sensor
 */
Eigen::Vector3d FovFilterSensor::FovVector(const Eigen::Vector3d &wp) const { return Rl * (R * (wp + T) + Tl); }

/**
 * @brief calculate the local position without self rotation
 *
 * @param wp vector in world coordinate
 * @return local vector of sensor without self rotation
 */
Eigen::Vector3d FovFilterSensor::FovVectorWithoutSelfRot(const Eigen::Vector3d &wp) const { return R * (wp + T) + Tl; }

void FovFilterSensor::FovRotator(double &roll, double &pitch, double &yaw) const {
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
 * @brief calculate the local rotation of the sensor
 *
 * @param np direction vector in world coordinate
 * @return local rotated direction vector
 */
Eigen::Vector3d FovFilterSensor::FovRotator(const Eigen::Vector3d &np) const { return Rl * R * np; }

/**
 * @brief calculate the local rotation of the sensor without self rotation
 *
 * @param np direction vector in world coordinate
 * @return local rotated direction vector withoud self rotation
 */
Eigen::Vector3d FovFilterSensor::FovRotatorWithoutSelfRot(const Eigen::Vector3d &np) const { return R * np; }

/**
 * @brief calculate the local position without self rotation
 *
 * @param local position in world coordinate
 * @return local position of sensor without self rotation
 */
Eigen::Vector3d FovFilterSensor::WorldVectorWithoutSelfRot(const Eigen::Vector3d &local) const {
  return Ri * (local - Tl) - T;
}

/// all fov sensor
std::map<int, std::shared_ptr<FovFilterSensor>> fovfilters;
/**
 * @brief load fov sensor from sensor message
 *
 * @param sensor message
 * @param device id of device
 * @return true on success
 * @return false on failure
 */
bool LoadFovFilter(const sim_msg::Sensor &sensor, const std::string &device) {
  if (!device.empty() && device != sensor.extrinsic().device()) {
    return true;
  }
  const double a2r = EIGEN_PI / 180.;
  auto fov = std::make_shared<FovFilterSensor>(sensor.extrinsic().id());
  fov->setRotation(a2r * sensor.extrinsic().rotationx(), a2r * sensor.extrinsic().rotationy(),
                   a2r * sensor.extrinsic().rotationz());
  fov->setPosition(Eigen::Vector3d(sensor.extrinsic().locationx() * 0.01, sensor.extrinsic().locationy() * 0.01,
                                   sensor.extrinsic().locationz() * 0.01));

  double hfov = 0, vfov = 0, radius = 0;

  // read param
  GetPropValue(sensor.intrinsic().params(), "FovVertial", vfov);  // FovVertical
  GetPropValue(sensor.intrinsic().params(), "FovHorizonal", hfov);
  GetPropValue(sensor.intrinsic().params(), "Radius", radius);
  GetPropValue(sensor.intrinsic().params(), "hfov", hfov);
  GetPropValue(sensor.intrinsic().params(), "vfov", vfov);
  GetPropValue(sensor.intrinsic().params(), "drange", radius);

  fov->setFov(vfov * a2r, hfov * a2r, radius);
  if (fov->Init()) {
    fovfilters[fov->ID()] = fov;
    return true;
  }

  return false;
}
/**
 * @brief add default fov sensor
 *
 * @param id id
 * @param v vertical fov
 * @param h horizontal fov
 * @param r range
 * @return true on success
 * @return false on failure
 */
bool AddDefaultFov(int id, double v, double h, double r) {
  auto sensor = std::make_shared<FovFilterSensor>(id);
  sensor->setFov(v, h, r);
  fovfilters[sensor->ID()] = sensor;
  return true;
}
