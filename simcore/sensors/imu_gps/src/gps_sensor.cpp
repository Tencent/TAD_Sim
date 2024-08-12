/**
 * @file gps_sensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "gps_sensor.h"

#include "sensor_cfg.h"
#include "sensor_core.h"

#include "common/coord_trans.h"

namespace gpssensor {
CGPSSensor::CGPSSensor() {}

// get gps sensor topic
std::string CGPSSensor::getGPSSensorTopic() { return _GPS_TOPIC; }

// init gps sensor
bool CGPSSensor::initGPSSensor(const sim_msg::Location& egoStartLocation, const std::string& buffer,
                               const std::string& groupname) {
  // ENU expressed in NED coordinate
  NED_ENU_Q.Q_from_Euler123(chrono::ChVector<double>(M_PI, 0.0, M_PI / 2));

  // set map origin
  const auto& wgs84_pos = egoStartLocation.position();

  mMapOrigin[0] = wgs84_pos.x();
  mMapOrigin[1] = wgs84_pos.y();
  mMapOrigin[2] = wgs84_pos.z();

  // update sensor configuration
  sensor::CSensorCfg _gpsCfg;
  _sensorValid = false;

  _sensorValid = _gpsCfg.parseGPSConfig(buffer, groupname, mGPS);

  return _sensorValid;
}

void CGPSSensor::calGPSSensorData(const sim_msg::Location& location, const int64_t& t_ms) {
  if (_sensorValid) {
    // get ego orientation
    const auto& rpy = location.rpy();
    ::chrono::ChQuaternion<double> egoQ;
    ::chrono::ChVector<double> eulerRPY = {rpy.x(), rpy.y(), rpy.z()};
    egoQ.Q_from_Euler123(eulerRPY);

    // get ego velocity and omega
    const auto& velocityENU = location.velocity();
    ::chrono::ChVector<double> velocity = {velocityENU.x(), velocityENU.y(), velocityENU.z()};
    const auto& angularVelocity = location.angular();
    ::chrono::ChVector<double> omega = {angularVelocity.x(), angularVelocity.y(), angularVelocity.z()};

    // get ego location
    const auto& wgs84_pos = location.position();
    ::chrono::ChVector<> position = {wgs84_pos.x(), wgs84_pos.y(), wgs84_pos.z()};
    coord_trans_api::lonlat2enu(position[0], position[1], position[2], mMapOrigin[0], mMapOrigin[1], mMapOrigin[2]);

    // update ego moving
    mEgoMoving.SetPos(position);
    mEgoMoving.SetRot(egoQ);

    mEgoMoving.SetPos_dt(velocity);
    mEgoMoving.SetWvel_loc(omega);

    mEgoMoving.SetWacc_loc(chrono::ChVector<double>());
    mEgoMoving.SetPos_dtdt(chrono::ChVector<double>());

    // update time of gps
    setTimeOfGPSSensor(t_ms, mGPS);

    // update position of gps
    calPositionOfGPSSensor(mGPS);
    calVelocityOfGPSSensor(mGPS);
    calTrackOfGPSSensor(mGPS);
    setStateOfGPSSensor(mGPS);

    // update map origin
    mMapOrigin[0] = wgs84_pos.x();
    mMapOrigin[1] = wgs84_pos.y();
    mMapOrigin[2] = wgs84_pos.z();
  }
}

// get gps sensor data
osi3::GPS CGPSSensor::getGPSData() { return mGPS; }

// timestamp of gps
void CGPSSensor::setTimeOfGPSSensor(const int64_t& t_ms, osi3::GPS& gps) {
  int64_t secs = t_ms / 1000;
  int64_t nanosecs = (t_ms % 1000) * 1000000;

  gps.mutable_timestamp()->set_seconds(secs);
  gps.mutable_timestamp()->set_nanos(nanosecs);
}

// postion of gps
void CGPSSensor::calPositionOfGPSSensor(osi3::GPS& gps) {
  // get gps offset
  const auto& gpsMountPos = mGPS.mount_pos().position();
  ::chrono::ChVector<double> gpsOffset = {gpsMountPos.x(), gpsMountPos.y(), gpsMountPos.z()};

  // calculate gps position
  ::chrono::ChVector<double> gpsPosition = mEgoMoving.GetRot().Rotate(gpsOffset);

  // from enu to wgs84
  coord_trans_api::enu2lonlat(gpsPosition[0], gpsPosition[1], gpsPosition[2], mMapOrigin[0], mMapOrigin[1],
                              mMapOrigin[2]);

  // set gps and stddev
  gps.set_longitude(gpsPosition[0]);
  gps.set_latitude(gpsPosition[1]);
  gps.set_height(gpsPosition[2]);

  gps.set_lonsdtdev(0.000000001);
  gps.set_latsdtdev(0.000000001);
  gps.set_hgtsdtdev(0.000000001);
}

// velocity of gps
void CGPSSensor::calVelocityOfGPSSensor(osi3::GPS& gps) {
  // get gps offset
  const auto& gpsMountPos = mGPS.mount_pos().position();
  ::chrono::ChVector<double> gpsOffset = {gpsMountPos.x(), gpsMountPos.y(), gpsMountPos.z()};

  // calculate velocity in ENU coordinate
  ::chrono::ChVector<double> gpsENUVelocity = mEgoMoving.PointSpeedLocalToParent(gpsOffset);
  ::chrono::ChVector<double> gpsNEDVelocity = (mEgoMoving.GetRot()).Rotate(gpsENUVelocity);

  double vx = gpsNEDVelocity[0] * _MS_2_KMH;
  double vy = gpsNEDVelocity[1] * _MS_2_KMH;
  double vz = gpsNEDVelocity[2] * _MS_2_KMH;

  gps.set_vel_hrz(std::hypot(vx, vy));
  gps.set_vel_vrt(vz);
}

// track of gps, from ENU to NED
void CGPSSensor::calTrackOfGPSSensor(osi3::GPS& gps) {
  // calculate ego roation in NED coordinate
  ::chrono::ChQuaternion<double> egoRotNED = NED_ENU_Q * mEgoMoving.GetRot();
  ::chrono::ChVector<double> egoRPYNED = egoRotNED.Q_to_Euler123();
  gps.set_track(egoRPYNED[2] * _RAD_2_DEG);
}

// state of gps
void CGPSSensor::setStateOfGPSSensor(osi3::GPS& gps) {
  // 0 is normal
  gps.set_solst(0);

  // pose type set to 50
  gps.set_postype(50);

  // undulation, @dhu
  gps.set_undulation(0.0);
}
}  // namespace gpssensor
