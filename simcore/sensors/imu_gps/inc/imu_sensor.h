// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "eigen3/Eigen/Dense"
#include "sensor_core.h"

#include "location.pb.h"

#include "chrono/core/ChFrameMoving.h"

// #define _INTEGRATE_IMU_RIGID_
// #define _RK4_INTEGRETE_
// #define _CAL_EULER_DIFF_
// #define _INTEGRATE_VEHILCE_EULER_

namespace imusensor {
class CIMUSensor : public sensor::CSensorBase {
 public:
  CIMUSensor();

  std::string getIMUSensorTopic();

  // get imu sensor cfg from json file, dt, unit seconds
  bool initIMUSensor(const sim_msg::Location &startLocation, const std::string &buffer, const std::string &groupname,
                     const double &dt = 0.01);

  // calculate imu senor data, ground truth data, imu rigid data
  void calIMUData(const sim_msg::Location &location, const int64_t &t);

  // wheelspd from chassis to imu
  void wheelspdChassis(const double &rl, const double &rr);

  osi3::imu_IMUCfg getIMUCfg();

  osi3::imu getIMUData();

 private:
  // timestamp of imu
  void setTimeOfIMUSensor(const int64_t &t_ms, osi3::imu &imu);

  // get imu sensor data by location and vehicle state
  void calIMUSensorData(const sim_msg::Location &location, osi3::imu_IMUSensorData &imuSensor);

  // accel from chassis to imu
  void accelChassis2IMU(const sim_msg::Location &location, osi3::imu_IMUSensorData &imuSensor);

  // angular_v from chassis to imu
  void angularVChassis2IMU(const sim_msg::Location &location, osi3::imu_IMUSensorData &imuSensor);

  // imu rigid data in global for debug
  void calIMUInGlobal(const sim_msg::Location &location, osi3::imu_IMURigidENUGNSS &imuRigid);

 public:
  // test function for imu sensor, from imu senosr data to imu rigid
  osi3::imu_IMURigidENUGNSS _imuRigidCal;
  osi3::imu_IMURigidENUGNSS imuDataIntegrete(osi3::imu_IMUSensorData &imuData, const sim_msg::Location &location);

 private:
  ::chrono::ChVector<double> mMapOrigin;
  ::chrono::ChFrameMoving<double> mEgoMoving;
  chrono::ChQuaternion<double> mEgoQ_Previous;

  chrono::ChQuaternion<double> mImuQ_Local, mImuQ_Local_Inv;
  chrono::ChVector<double> mImu_Local_Offset;
  chrono::ChFrameMoving<double> mImuLocal;

 private:
  osi3::imu_IMUCfg _imuCfg;
  osi3::imu_IMUGroundTruth _imuTruth;
  osi3::imu_IMUSensorData _imuData;
  osi3::imu_IMURigidENUGNSS _imuRigid;
  osi3::imu _imu;

  const std::string _IMU_TOPIC = "IMU_SIM";
  bool _sensorValid;
  double _dt;  // unit s
};
}  // namespace imusensor
