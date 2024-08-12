/**
 * @file LidarSensor.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "lidar_sensor.h"
#include <float.h>
#include <Eigen/Geometry>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>

/// lidar sensor by id
LidarSensor::LidarSensor(int _id) : FovFilterSensor(_id) {}

/**
 * @brief initialize lidar sensor
 *
 * @return true on success
 * @return false
 */
bool LidarSensor::Init() {
  if (rayNum < 1) return false;
  if (leftFOV > rightFOV) return false;
  if (verAngle.empty()) {
    verAngle.resize(rayNum);
    verAngle[0] = downFOV;
    for (int i = 1; i < rayNum; ++i) {
      verAngle[i] = downFOV + i * (upFOV - downFOV) / (rayNum - 1);
    }
  } else {
    if (verAngle.size() != rayNum) return false;
    downFOV = DBL_MAX;
    upFOV = -DBL_MAX;
    for (auto a : verAngle) {
      downFOV = std::min(a, downFOV);
      upFOV = std::max(a, upFOV);
    }
    std::sort(verAngle.begin(), verAngle.end());
  }
  downFOV = std::min(0.0, std::max(-1.5707963267948966192313216916398, downFOV));
  upFOV = std::max(0.0, std::min(1.5707963267948966192313216916398, upFOV));
  // leftFOV = std::min(0.0,std::max(-3.1415926535897932384626433832795, leftFOV));
  // rightFOV = std::max(0.0,std::min(3.1415926535897932384626433832795, rightFOV));

  verMap.resize(std::ceil((upFOV - downFOV) / resVerial));
  if (verMap.empty()) return false;
  int cur = 0;
  for (size_t i = 0; i < verMap.size(); ++i) {
    double a = downFOV + i * resVerial;
    while (cur < rayNum - 1 && a > verAngle[cur + 1]) cur++;
    verMap[i] = cur;
  }

  return FovFilterSensor::Init();
}

/**
 * @brief convert world coordinate to pixel coordinate
 *
 * @param wp : world coordinate
 * @param uv : pixel coordinate
 * @return true on success
 * @return false on failure
 */
bool LidarSensor::world2pixel(const Eigen::Vector3d &wp, Eigen::Vector2d &uv) const {
  // get local point
  Eigen::Vector3d c = FovVector(wp);
  if (c.norm() >= dRange) {
    return false;
  }
  double h = -std::atan2(c.y(), c.x());
  double v = std::atan2(c.z(), std::sqrt(c.x() * c.x() + c.y() * c.y()));
  if (h >= leftFOV && h <= rightFOV && v >= downFOV && v <= upFOV) {
    uv.x() = (h - leftFOV) / resHorizonal;
    uv.y() = (v - downFOV) / resVerial;
    if (uv.y() >= 0 && uv.y() < verMap.size()) {
      uv.y() = verMap.at(std::floor(uv.y()));
      return true;
    }
  }

  return false;
}
