/**
 * @file UltrasonicSensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "ultrasonic_sensor.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/geometry.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>

/**
 * @brief Construct a new Ultrasonic Sensor:: Ultrasonic Sensor object
 *
 * @param _id sensor id
 */
UltrasonicSensor::UltrasonicSensor(int _id) : FovFilterSensor(_id) {}

/**
 * @brief Set the Rotation of ultrasonic
 *
 * @param roll rool angle
 * @param pitch pitch angle
 * @param yaw yaw angle
 */
void UltrasonicSensor::setRotation(double _roll, double _pitch, double _yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(_yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(_pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(_roll, Eigen::Vector3d::UnitX()) *
                         Eigen::AngleAxisd(-EIGEN_PI * 0.5, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(-EIGEN_PI * 0.5, Eigen::Vector3d::UnitX());
  Rli = q.toRotationMatrix();
  Rl = q.toRotationMatrix().inverse();
}

/**
 * @brief Initialization for ultrasonic
 *
 * @return true on success
 * @return false on failure
 */
bool UltrasonicSensor::Init() {
  if (!FovFilterSensor::Init()) return false;
  K = Eigen::Matrix3d::Identity();
  K(0, 2) = 0;
  K(1, 2) = 0;
  K(0, 0) = 0.5 / std::tan(rightFOV);
  K(1, 1) = 0.5 / std::tan(upFOV);
  Ki = K.inverse();
  return true;
}

/**
 * @brief calculate the distance between the sensor and the target
 *
 * @param loc local position of the target
 * @param roll roll angle of the target
 * @param pitch pitch angle of the target
 * @param yaw yaw angle of the target
 * @param len length of the target
 * @param wid width of the target
 * @param hei height of the target
 * @return distance between the sensor and the target
 */
double UltrasonicSensor::distance(const Eigen::Vector3d &loc, double roll, double pitch, double yaw, double len,
                                  double wid, double hei) const {
  // Check if sensor is inside field of view
  if (!inFov(loc, std::max(std::max(len, wid), hei))) {
    return 0;
  }
  // Create a quaternion from rotation angles
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());

  // Scale dimensions by half
  len *= 0.5;
  wid *= 0.5;
  hei *= 0.5;

  // Define vertices of cube
  Eigen::Vector3d vert[8];
  vert[0] = Eigen::Vector3d(wid, hei, -hei);
  vert[1] = Eigen::Vector3d(-wid, hei, -hei);
  vert[2] = Eigen::Vector3d(-wid, -hei, -hei);
  vert[3] = Eigen::Vector3d(wid, -hei, -hei);
  vert[4] = Eigen::Vector3d(wid, hei, hei);
  vert[5] = Eigen::Vector3d(-wid, hei, hei);
  vert[6] = Eigen::Vector3d(-wid, -hei, hei);
  vert[7] = Eigen::Vector3d(wid, -hei, hei);

  // Iterate through each vertex
  for (int i = 0; i < 8; i++) {
    // Calculate camera ray direction vector
    auto c = FovVector(loc + q * vert[i]);
    // Convert to UV coordinates
    double x = c.x() / c.z();
    double y = c.y() / c.z();
    // Transform into normalized device coordinate space
    Eigen::Vector3d uv = K * Eigen::Vector3d(x, y, 1);
    uv.x() /= uv.z();
    uv.y() /= uv.z();
    uv.z() = c.z();
    // Update vertex position
    vert[i] = uv;
  }
  // Define faces of cube
  int face[6][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}};

  // Initialize variables used during intersection testing
  typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> point;
  typedef boost::geometry::model::multi_point<point> points;
  typedef boost::geometry::model::polygon<point> polygon;
  typedef boost::geometry::model::multi_polygon<polygon> mpolygon;

  // Set up current FOV polygon
  polygon thisfov{{{-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {-0.5, 0.5}}};

  // Find nearest surface within range
  Eigen::Vector3d nestuv;
  double nestz = dRange * 2;
  for (int i = 0; i < 6; i++) {
    // Setup temporary polygon for intersecting rays
    polygon thatps;
    // Add all corners of face to temporary polygon
    for (int j = 0; j < 4; j++) {
      auto &p = vert[face[i][j]];
      thatps.outer().push_back(point(p.x(), p.y()));
    }
    // Perform intersection test between current FOV and temp polygons
    mpolygon output;
    boost::geometry::intersection(thisfov, thatps, output);
    // Test each intersection candidate
    for (const auto &ply : output) {
      for (const auto &p : ply.outer()) {
        Eigen::Vector3d vp(p.get<0>(), p.get<1>(), 0);
        // Compute distance along ray to intersection point
        double z =
            inverseBilinearInterpolation2D(vp, vert[face[i][0]], vert[face[i][1]], vert[face[i][2]], vert[face[i][3]]);
        // If closer than previous closest hit, update nestuv and nestz
        if (z > 0 && z < nestz) {
          nestz = z;
          nestuv = vp;
        }
      }
    }
  }
  // Return result based on whether or not a valid hit was found
  if (nestz < 0) {
    return 0.001;
  }
  if (nestz < dRange) {
    Eigen::Vector3d c = Ki * Eigen::Vector3d(nestuv.x(), nestuv.y(), 1);
    if (c.z() > 0) {
      c /= c.z();
      return c.norm() * nestuv.z();
    }
  }
  return 0.0;
}

/**
 * @brief point is in the FOV of the sensor
 *
 * @param wp point in world space
 * @param dis distance threshold to expand range
 * @return true on success
 * @return false on failure
 */
bool UltrasonicSensor::inFov(const Eigen::Vector3d &wp, double dis) const {
  // local coordinates
  Eigen::Vector3d c = FovVector(wp);
  if (c.z() <= 0) {
    return false;
  }
  if (c.norm() >= (dRange + dis)) {
    return false;
  }
  double h = std::atan2(c.x(), c.z());
  double v = -std::atan2(c.y(), std::sqrt(c.x() * c.x() + c.z() * c.z()));
  return h > leftFOV && h < rightFOV && v > downFOV && v < upFOV;
}

/**
 * @brief inverse Bilinear Interpolation for 2D points
 *
 * @param pt point coordinates in and out
 * @param p1 left bottom point
 * @param p2 left top point
 * @param p3 right bottom point
 * @param p4 right top point
 * @return z of the point
 */
double UltrasonicSensor::inverseBilinearInterpolation2D(Eigen::Vector3d &pt, const Eigen::Vector3d &p1,
                                                        const Eigen::Vector3d &p2, const Eigen::Vector3d &p3,
                                                        const Eigen::Vector3d &p4) const {
  double u = 0, v = 0;
  double a =
      (p1.x() - p2.x() + p3.x() - p4.x()) * (p4.y() - p1.y()) - (p1.y() - p2.y() + p3.y() - p4.y()) * (p4.x() - p1.x());
  double b = (p2.x() - p1.x()) * (p4.y() - p1.y()) - (p2.y() - p1.y()) * (p4.x() - p1.x()) +
             (pt.x() - p1.x()) * (p1.y() - p2.y() + p3.y() - p4.y()) -
             (pt.y() - p1.y()) * (p1.x() - p2.x() + p3.x() - p4.x());
  double c = (pt.x() - p1.x()) * (p2.y() - p1.y()) - (pt.y() - p1.y()) * (p2.x() - p1.x());
  double delta = b * b - 4 * a * c;
  if (delta < 0) {
    return -1;
  }
  double sqrtDelta = std::sqrt(delta);

  /** calc v */
  v = (-b + sqrtDelta) / (2 * a);
  if (v < 0 || v > 1) {
    v = (-b - sqrtDelta) / (2 * a);
  }
  /** calc u */
  u = ((pt.x() - p1.x()) - (p4.x() - p1.x()) * v) / ((p2.x() - p1.x()) + (p1.x() - p2.x() + p3.x() - p4.x()) * v);

  /** calc value of pt */
  double val = p1.z() + (p2.z() - p1.z()) * u + (p4.z() - p1.z()) * v + (p1.z() - p2.z() + p3.z() - p4.z()) * u * v;
  pt.z() = val;
  return val;
}

/// @brief all ultrasonics
std::map<int, std::shared_ptr<UltrasonicSensor>> ultrasonics;

/**
 * @brief load all ultrasonics
 *
 * @param sensor sensor message
 * @param device id of the device
 * @return true on success
 * @return false on failure
 */
bool LoadUltrasonic(const sim_msg::Sensor &sensor, const std::string &device) {
  if (!device.empty() && device != sensor.extrinsic().device()) {
    return true;
  }
  const double a2r = EIGEN_PI / 180.;
  auto fov = std::make_shared<UltrasonicSensor>(sensor.extrinsic().id());
  // read position
  fov->setPosition(Eigen::Vector3d(sensor.extrinsic().locationx() * 0.01, sensor.extrinsic().locationy() * 0.01,
                                   sensor.extrinsic().locationz() * 0.01));
  // read rotation
  fov->setRotation(a2r * sensor.extrinsic().rotationx(), a2r * sensor.extrinsic().rotationy(),
                   a2r * sensor.extrinsic().rotationz());
  double hfov = 0, vfov = 0, radius = 0;

  // read fov
  GetPropValue(sensor.intrinsic().params(), "FovHorizonal", hfov);
  GetPropValue(sensor.intrinsic().params(), "FOV_Horizontal", hfov);
  GetPropValue(sensor.intrinsic().params(), "Distance", radius);
  GetPropValue(sensor.intrinsic().params(), "FovVertial", vfov);
  GetPropValue(sensor.intrinsic().params(), "FOV_Vertical", vfov);

  fov->setFov(a2r * std::min(179.999999, vfov), a2r * std::min(179.999999, hfov), radius);

  // init
  if (fov->Init()) {
    ultrasonics[fov->ID()] = fov;
    return true;
  }

  return false;
}
