/**
 * @file imugps_interface.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "imugps_interface.h"
#include "gps_sensor.h"
#include "imu_sensor.h"

#include "vehState.pb.h"

namespace imugps {
CIMUGPSInterface::CIMUGPSInterface() {
  m_sensor_xml = "";
  m_dt = 0.01;
}
CIMUGPSInterface::~CIMUGPSInterface() { stop(); }

bool CIMUGPSInterface::init(const std::string &buffer, const std::string &groupname, const std::string &start_location,
                            double dt) {
  // default values
  mIMUValid = false;
  mGPSValid = false;

  m_dt = dt;

  // start location
  sim_msg::Location start_loc;
  start_loc.ParseFromString(start_location);

  // create sensors
  m_imu = std::make_shared<imusensor::CIMUSensor>();
  m_gps = std::make_shared<gpssensor::CGPSSensor>();

  // stop if sensor error
  if (m_imu.get() == nullptr || m_gps.get() == nullptr) {
    LOG(ERROR) << "faile to make imu or gps sensor.\n";
    return false;
  }

  try {
    // get config from xml file
    mIMUValid = m_imu->initIMUSensor(start_loc, buffer, groupname, m_dt);
    mGPSValid = m_gps->initGPSSensor(start_loc, buffer, groupname);

    if (!mIMUValid) LOG(WARNING) << "imu sensor not enabled.\n";
    if (!mGPSValid) LOG(WARNING) << "gps sensor not enabled.\n";
  } catch (const char *msg) {
    LOG(ERROR) << "vd | imu gps init error, " << msg << "\n";
    return mIMUValid || mGPSValid;
  }

  return mIMUValid || mGPSValid;
}
void CIMUGPSInterface::calIMUData(const std::string &loc_payload, const std::string &chassis_payload, int64_t t_ms) {
  if (mIMUValid) {
    if (loc_payload.size() > 0) {
      sim_msg::Location loc;
      loc.ParseFromString(loc_payload);
      m_imu->calIMUData(loc, t_ms);
    }

    if (chassis_payload.size() > 0) {
      sim_msg::VehicleState chassis;
      chassis.ParseFromString(chassis_payload);
      m_imu->wheelspdChassis(chassis.chassis_state().wheel_speed().rearleft(),
                             chassis.chassis_state().wheel_speed().rearright());
    }
  }
}
void CIMUGPSInterface::calGPSData(const std::string &loc_payload, int64_t t_ms) {
  if (mGPSValid) {
    if (loc_payload.size() > 0) {
      sim_msg::Location loc;
      loc.ParseFromString(loc_payload);
      m_gps->calGPSSensorData(loc, t_ms);
    }
  }
}
void CIMUGPSInterface::stop() {
  m_imu.reset();
  m_gps.reset();
}

void CIMUGPSInterface::getIMUPayload(std::string &payload) {
  payload.clear();

  if (mIMUValid) {
    m_imu->getIMUData().SerializeToString(&payload);
  }
}
void CIMUGPSInterface::getGPSPayload(std::string &payload) {
  payload.clear();

  if (mGPSValid) {
    m_gps->getGPSData().SerializeToString(&payload);
  }
}
}  // namespace imugps
