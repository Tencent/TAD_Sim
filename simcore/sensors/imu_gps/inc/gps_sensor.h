// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: qq@tencent.com (Emperor Penguin)
//
#pragma once

#include "location.pb.h"
#include "sensor_core.h"

#include "chrono/core/ChFrameMoving.h"

namespace gpssensor {
class CGPSSensor : public sensor::CSensorBase {
 private:
  // postion of gps
  void calPositionOfGPSSensor(osi3::GPS &gps);

  // velocity of gps
  void calVelocityOfGPSSensor(osi3::GPS &gps);

  // track of gps, from ENU to NED
  void calTrackOfGPSSensor(osi3::GPS &gps);

  // state of gps
  void setStateOfGPSSensor(osi3::GPS &gps);

  // timestamp of gps
  void setTimeOfGPSSensor(const int64_t &timTimestampMill, osi3::GPS &gps);

 public:
  CGPSSensor();

  // calculate gps sensor data
  void calGPSSensorData(const sim_msg::Location &location, const int64_t &t_ms);

  // get gps sensor topic
  std::string getGPSSensorTopic();

  // get gps sensor data
  osi3::GPS getGPSData();

  // init gps sensor
  bool initGPSSensor(const sim_msg::Location &egoStartLocation, const std::string &buffer,
                     const std::string &groupname);

 private:
  const std::string _GPS_TOPIC = "GPS_SIM";
  bool _sensorValid = true;
  osi3::GPS mGPS;

  ::chrono::ChVector<double> mMapOrigin;
  ::chrono::ChFrameMoving<double> mEgoMoving;
  ::chrono::ChQuaternion<double> NED_ENU_Q;
};
}  // namespace gpssensor
