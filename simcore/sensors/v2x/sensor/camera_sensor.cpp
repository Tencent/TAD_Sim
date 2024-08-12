/**
 * @file CameraSensor.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "camera_sensor.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <iostream>

/**
 * @brief Construct a new Camera Sensor object
 *
 * @param _id : sensor id
 */
CameraSensor::CameraSensor(int _id) : FovFilterSensor(_id) { K = Eigen::Matrix3d::Identity(); }

/**
 * @brief Set the rotation of camera
 *
 * @param roll roll angle in radians
 * @param pitch pitch angle in radians
 * @param yaw yaw angle in radians
 */
void CameraSensor::setRotation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) * Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX()) * Eigen::AngleAxisd(-EIGEN_PI * 0.5, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(-EIGEN_PI * 0.5, Eigen::Vector3d::UnitX());
  Rli = q.toRotationMatrix();
  Rl = q.toRotationMatrix().inverse();
}

/**
 * @brief Set the intrinsic matrix of camera
 *
 * @param imat : intrinsic matrix
 */
void CameraSensor::setIntrinsic(const Eigen::Matrix3d &imat) { K = imat; }

/**
 * @brief Set the distortion of camera
 *
 * @param _k1 k1 distortion coefficient
 * @param _k2 k2 distortion coefficient
 * @param _k3 k3 distortion coefficient
 * @param _p1 p1 distortion coefficient
 * @param _p2 p2 distortion coefficient
 */
void CameraSensor::setDistortion(double _k1, double _k2, double _k3, double _p1, double _p2) {
  k1 = _k1;
  k2 = _k2;
  k3 = _k3;
  p1 = _p1;
  p2 = _p2;
}

/**
 * @brief Set the resolution of camera
 *
 * @param _w width of image
 * @param _h height of image
 */
void CameraSensor::setResolution(int _w, int _h) {
  w = _w;
  h = _h;
}

/**
 * @brief Set the FPS of camera
 *
 * @param _fps The FPS of camera
 */
void CameraSensor::setFPS(double _fps) { fps = _fps; }

/**
 * @brief Initialization, calculate fov
 *
 * @return true
 * @return false
 */
bool CameraSensor::Init() {
  std::vector<Eigen::Vector2d> bounds;
  for (int i = 0; i < 11; i++) {
    double y = h * i * 0.1;
    for (int j = 0; j < 11; j++) {
      // Calculate bounding box points and add them to an array
      double x = w * j * 0.1;
      bounds.push_back(Eigen::Vector2d(x, y));
    }
  }
  double maxxfov = 0, maxyfov = 0;
  for (const auto &bd : bounds) {
    double tX = bd.x();
    double tY = bd.y();
    double y = (tY - K(1, 2)) / K(1, 1);
    double x = (tX - K(0, 2) - K(0, 1) * y) / K(0, 0);
    double x0 = x;
    double y0 = y;
    for (int j = 0; j < 10; j++) {
      double r2 = x * x + y * y;
      double distRadialA = 1.f / (1.f + k1 * r2 + k2 * r2 * r2 + k3 * r2 * r2 * r2);
      double deltaX = 2.f * p1 * x * y + p2 * (r2 + 2.f * x * x);
      double deltaY = p1 * (r2 + 2.f * y * y) + 2.f * p2 * x * y;
      x = (x0 - deltaX) * distRadialA;
      y = (y0 - deltaY) * distRadialA;
    }

    maxxfov = std::max(maxxfov, (double)std::abs(std::atan(x)));
    maxyfov = std::max(maxyfov, (double)std::abs(std::atan(y)));
  }
  setFov(maxyfov * 2, maxxfov * 2, 1000);

  return FovFilterSensor::Init();
}
/**
 * @brief Check whether point on image plane
 *
 * @param wp point in world coordinate system
 * @return true if it's inside FOV
 * @return false
 */
bool CameraSensor::inFov(const Eigen::Vector3d &wp) const {
  Eigen::Vector2d uv = world2pixel(wp);
  return uv.x() > 0 && uv.x() < w && uv.y() > 0 && uv.y() < h;
}

/**
 * @brief Convert a point in world coordinate system into pixel coordinates
 *
 * @param wp point in world coordinate system
 * @return Eigen::Vector2d pixel coordinates
 */
Eigen::Vector2d CameraSensor::world2pixel(const Eigen::Vector3d &wp) const {
  Eigen::Vector3d c = Rl * (R * (wp + T) + Tl);
  if (c.z() <= 0) {
    return Eigen::Vector2d(-1, -1);
  }
  double hz = std::atan2(c.x(), c.z());
  double vt = -std::atan2(c.y(), c.z());
  if (hz < leftFOV || hz > rightFOV || vt < downFOV || vt > upFOV) {
    return Eigen::Vector2d(-1, -1);
  }
  double x = c.x() / c.z();
  double y = c.y() / c.z();
  Eigen::Vector3d uv = K * Eigen::Vector3d(x, y, 1);
  if (k1 != 0) {
    double r2 = x * x + y * y;
    double r4 = r2 * r2;
    double r6 = r2 * r4;
    double dx = x * (1 + k1 * r2 + k2 * r4 + k3 * r6) + 2 * p1 * x * y + p2 * (r2 + 2 * x * x);
    double dy = y * (1 + k1 * r2 + k2 * r4 + k3 * r6) + 2 * p2 * x * y + p1 * (r2 + 2 * y * y);
    uv = K * Eigen::Vector3d(dx, dy, 1);
  }
  return Eigen::Vector2d(uv.x() / uv.z(), uv.y() / uv.z());
}
