/**
 * @file FisheyeSensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "fisheye_sensor.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>

/**
 * @brief Construct a new Camera Sensor object
 *
 * @param _id : sensor id
 */
FisheyeSensor::FisheyeSensor(int _id) : CameraSensor(_id) {}

/**
 * @brief Set the distortion of camera
 *
 * @param _k1 k1 distortion coefficient
 * @param _k2 k2 distortion coefficient
 * @param _k3 k3 distortion coefficient
 * @param _k4 k4 distortion coefficient
 */
void FisheyeSensor::setDistortion(double _k1, double _k2, double _k3, double _k4) {
  k1 = _k1;
  k2 = _k2;
  k3 = _k3;
  k4 = _k4;
}

/**
 * @brief Convert a point in world coordinate system into pixel coordinates
 * https://docs.opencv.org/4.9.0/db/d58/group__calib3d__fisheye.html
 *
 * @param wp point in world coordinate system
 * @return Eigen::Vector2d pixel coordinates
 */
Eigen::Vector2d FisheyeSensor::world2pixel(const Eigen::Vector3d &wp) const {
  Eigen::Vector3d c = Rl * (R * (wp + T) + Tl);
  if (c.z() <= 0) {
    return Eigen::Vector2d(-1, -1);
  }
  double x = c.x() / c.z();
  double y = c.y() / c.z();
  double r = std::sqrt(x * x + y * y);
  double theta = std::atan(r);
  double theta_2 = theta * theta;
  double theta_4 = theta_2 * theta_2;
  double theta_6 = theta_2 * theta_4;
  double theta_8 = theta_2 * theta_6;
  double theta_d = theta * (1 + k1 * theta_2 + k2 * theta_4 + k3 * theta_6 + k4 * theta_8);
  double scale = theta_d / r;
  x *= scale;
  y *= scale;
  Eigen::Vector3d uv = K * Eigen::Vector3d(x, y, 1);
  return Eigen::Vector2d(uv.x() / uv.z(), uv.y() / uv.z());
}

/// @brief all fisheye
std::map<int, std::shared_ptr<FisheyeSensor>> fisheyes;

/**
 * @brief Load camera extrinsic parameters sensor msg
 *
 * @param sensor sensor message
 * @param device id of device
 * @return true on success
 * @return false on faild
 */
bool LoadFisheye(const sim_msg::Sensor &sensor, const std::string &device) {
  if (!device.empty() && device != sensor.extrinsic().device()) {
    return true;
  }
  const double a2r = EIGEN_PI / 180.;
  auto fish = std::make_shared<FisheyeSensor>(sensor.extrinsic().id());
  // Load camera extrinsic parameters
  fish->setRotation(a2r * sensor.extrinsic().rotationx(), a2r * sensor.extrinsic().rotationy(),
                    a2r * sensor.extrinsic().rotationz());
  fish->setPosition(Eigen::Vector3d(sensor.extrinsic().locationx() * 0.01, sensor.extrinsic().locationy() * 0.01,
                                    sensor.extrinsic().locationz() * 0.01));
  int w = 0, h = 0;
  // Load camera width and height
  GetPropValue(sensor.intrinsic().params(), "ResVertial", h);
  GetPropValue(sensor.intrinsic().params(), "Res_Horizontal", w);
  GetPropValue(sensor.intrinsic().params(), "ResHorizonal", w);
  GetPropValue(sensor.intrinsic().params(), "Res_Vertical", h);
  fish->setResolution(w, h);

  Eigen::Matrix3d insmat = Eigen::Matrix3d::Identity();
  std::string tmpbufer;

  // introinsic matrix
  std::string instring;
  GetPropValue(sensor.intrinsic().params(), "IntrinsicMat", instring);
  GetPropValue(sensor.intrinsic().params(), "Intrinsic_Matrix", instring);

  std::replace(instring.begin(), instring.end(), ',', ' ');
  auto iss = std::istringstream(instring);
  for (int i = 0; i < 9; i++) {
    iss >> insmat(i / 3, i % 3);
  }
  fish->setIntrinsic(insmat);

  // get distortion parameters
  GetPropValue(sensor.intrinsic().params(), "Distortion_Parameters", tmpbufer);
  GetPropValue(sensor.intrinsic().params(), "Distortion", tmpbufer);

  // read distortion parameters
  if (!tmpbufer.empty()) {
    std::replace(tmpbufer.begin(), tmpbufer.end(), ',', ' ');
    auto issstr = std::istringstream(tmpbufer);
    double dispara[4] = {0};
    for (int i = 0; i < 4; i++) {
      issstr >> dispara[i];
    }
    fish->setDistortion(dispara[0], dispara[1], dispara[2], dispara[3]);
  }

  double fps = 0;
  // get camera fps
  GetPropValue(sensor.intrinsic().params(), "Frequency", fps);
  fish->setFPS(fps);

  // initialize camera sensor
  if (fish->Init()) {
    fisheyes[fish->ID()] = fish;
    return true;
  }
  return false;
}
