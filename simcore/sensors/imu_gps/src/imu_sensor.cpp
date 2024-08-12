/**
 * @file imu_sensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "imu_sensor.h"
#include "common/coord_trans.h"

#include "sensor_cfg.h"
#include "sensor_core.h"

namespace imusensor {
CIMUSensor::CIMUSensor() {
  _sensorValid = false;
  _dt = 0.01;
}

std::string CIMUSensor::getIMUSensorTopic() { return _IMU_TOPIC; }

// get imu sensor cfg from xml file
bool CIMUSensor::initIMUSensor(const sim_msg::Location &startLocation, const std::string &buffer,
                               const std::string &groupname, const double &dt) {
  sensor::CSensorCfg _imuCfgWrap;
  _dt = dt;

  // set map origin
  const auto &wgs84_pos = startLocation.position();

  mMapOrigin[0] = wgs84_pos.x();
  mMapOrigin[1] = wgs84_pos.y();
  mMapOrigin[2] = wgs84_pos.z();

  _sensorValid = _imuCfgWrap.parseIMUConfig(buffer, groupname, _imuCfg);

  if (_sensorValid) {
    // get imu orientation
    chrono::ChQuaternion<double> imuQ(_imuCfg.orientation().qw(), _imuCfg.orientation().qx(),
                                      _imuCfg.orientation().qy(), _imuCfg.orientation().qz());
    mImuQ_Local.Set(imuQ);
    mImuQ_Local_Inv = mImuQ_Local.GetInverse();

    // get imu offset
    mImu_Local_Offset = {_imuCfg.position().x(), _imuCfg.position().y(), _imuCfg.position().z()};

    // imu local moving
    mImuLocal.SetPos(mImu_Local_Offset);
    mImuLocal.SetRot(mImuQ_Local);

    mImuLocal.SetPos_dt(chrono::ChVector<double>());
    mImuLocal.SetWvel_loc(chrono::ChVector<double>());

    mImuLocal.SetPos_dtdt(chrono::ChVector<double>());
    mImuLocal.SetWacc_loc(chrono::ChVector<double>());

    // ego q
    mEgoQ_Previous.Q_from_Euler123(
        chrono::ChVector<double>(startLocation.rpy().x(), startLocation.rpy().y(), startLocation.rpy().z()));
  }

  return _sensorValid;
}

osi3::imu_IMUCfg CIMUSensor::getIMUCfg() { return _imuCfg; }

osi3::imu CIMUSensor::getIMUData() { return _imu; }

osi3::imu_IMURigidENUGNSS CIMUSensor::imuDataIntegrete(osi3::imu_IMUSensorData &imuData,
                                                       const sim_msg::Location &location) {
  osi3::imu_IMURigidENUGNSS imuRigid;

  return imuRigid;
}

// calculate imu senor data, ground truth data, imu rigid data
void CIMUSensor::calIMUData(const sim_msg::Location &location, const int64_t &t_ms) {
  if (_sensorValid) {
    calIMUSensorData(location, _imuData);

#ifdef _INTEGRATE_IMU_RIGID_
    _imu.mutable_imu_rigid()->CopyFrom(CIMUSensor::imuDataIntegrete(_imuData, location));
#else
    // calIMUGroundTruth(location,vehOut,_imuTruth);
    calIMUInGlobal(location, _imuRigid);
    _imu.mutable_imu_rigid()->CopyFrom(_imuRigid);
#endif

    _imu.mutable_imu_data()->CopyFrom(_imuData);

    setTimeOfIMUSensor(t_ms, _imu);
  }
}

// timestamp of gps
void CIMUSensor::setTimeOfIMUSensor(const int64_t &t_ms, osi3::imu &imu) {
  int64_t secs = t_ms / 1000;
  int64_t nanosecs = (t_ms % 1000) * 1000000;

  imu.mutable_timestamp()->set_seconds(secs);
  imu.mutable_timestamp()->set_nanos(nanosecs);
}

// get imu sensor data by location and vehicle state
void CIMUSensor::calIMUSensorData(const sim_msg::Location &location, osi3::imu_IMUSensorData &imuSensor) {
  // get ego location
  const auto &wgs84_pos = location.position();
  ::chrono::ChVector<> position = {wgs84_pos.x(), wgs84_pos.y(), wgs84_pos.z()};
  coord_trans_api::lonlat2enu(position[0], position[1], position[2], mMapOrigin[0], mMapOrigin[1], mMapOrigin[2]);

  // get ego velocity and omega
  const auto &velocityENU = location.velocity();
  const auto &angularVelocity = location.angular();
  ::chrono::ChVector<double> velocity = {velocityENU.x(), velocityENU.y(), velocityENU.z()};
  ::chrono::ChVector<double> omega = {angularVelocity.x(), angularVelocity.y(), angularVelocity.z()};

  // get ego orientation
  const auto &rpy = location.rpy();
  ::chrono::ChVector<double> eulerRPY = {rpy.x(), rpy.y(), rpy.z()};
  ::chrono::ChQuaternion<double> egoQ;
  egoQ.Q_from_Euler123(eulerRPY);

  // get ego acceleration in ENU coordinate
  const auto &acc = location.acceleration();
  ::chrono::ChVector<double> accEgoBody = {acc.x(), acc.y(), acc.z()};
  ::chrono::ChVector<double> accEgoENU = egoQ.Rotate(accEgoBody);

  // update ego moving
  mEgoMoving.SetPos(position);
  mEgoMoving.SetRot(egoQ);

  mEgoMoving.SetPos_dt(velocity);
  mEgoMoving.SetWvel_loc(omega);

  mEgoMoving.SetPos_dtdt(accEgoENU);
  mEgoMoving.SetWacc_loc(chrono::ChVector<double>());

  // calculate imu data
  CIMUSensor::accelChassis2IMU(location, imuSensor);
  CIMUSensor::angularVChassis2IMU(location, imuSensor);
}

// imu data in global or debug
void CIMUSensor::calIMUInGlobal(const sim_msg::Location &location, osi3::imu_IMURigidENUGNSS &imuRigid) {
  // get imu moving in ENU
  chrono::ChFrameMoving<double> imuENUMoving;
  mEgoMoving.TransformLocalToParent(mImuLocal, imuENUMoving);

  // set imu rigid
  chrono::ChQuaternion<double> imuQ = imuENUMoving.GetRot();
  chrono::ChVector<double> imuRPY = imuQ.Q_to_Euler123();
  chrono::ChVector<double> position = imuENUMoving.GetPos();
  chrono::ChVector<double> velocity = imuENUMoving.GetPos_dt();

  imuRigid.mutable_posenu()->set_x(position[0]);
  imuRigid.mutable_posenu()->set_y(position[1]);
  imuRigid.mutable_posenu()->set_z(position[2]);

  coord_trans_api::enu2lonlat(position[0], position[1], position[2], mMapOrigin[0], mMapOrigin[1], mMapOrigin[2]);

  imuRigid.mutable_eulerangle()->set_roll(imuRPY[0]);
  imuRigid.mutable_eulerangle()->set_pitch(imuRPY[1]);
  imuRigid.mutable_eulerangle()->set_yaw(imuRPY[2]);

  imuRigid.mutable_quatrot()->set_qw(imuQ.e0());
  imuRigid.mutable_quatrot()->set_qx(imuQ.e1());
  imuRigid.mutable_quatrot()->set_qy(imuQ.e2());
  imuRigid.mutable_quatrot()->set_qz(imuQ.e3());

  imuRigid.mutable_gnss()->set_longti(position[0]);
  imuRigid.mutable_gnss()->set_lat(position[1]);
  imuRigid.mutable_gnss()->set_alt(position[2]);

  imuRigid.mutable_velocity()->set_x(velocity[0]);
  imuRigid.mutable_velocity()->set_y(velocity[1]);
  imuRigid.mutable_velocity()->set_z(velocity[2]);
}

// accel from chassis to imu
void CIMUSensor::accelChassis2IMU(const sim_msg::Location &location, osi3::imu_IMUSensorData &imuSensor) {
  // get IMU offset in Ego coordinate
  chrono::ChVector<double> imuAccInENU = mEgoMoving.PointAccelerationLocalToParent(mImu_Local_Offset);

  // add gravity to z direction in ENU coordinate
  imuAccInENU[2] = imuAccInENU[2] + sensor::CSensorMath::_gravity;

  // acc in imu coordinate
  chrono::ChVector<double> imuAcc = (mImuQ_Local_Inv * mEgoMoving.GetRot().GetInverse()).Rotate(imuAccInENU);

  osi3::Vector3d imuAcc_Normal;

  // calculate imu acc
  imuAcc_Normal.set_x(imuAcc[0] / sensor::CSensorMath::_gravity);
  imuAcc_Normal.set_y(imuAcc[1] / sensor::CSensorMath::_gravity);
  imuAcc_Normal.set_z(imuAcc[2] / sensor::CSensorMath::_gravity);

  // set imu accel
  imuSensor.mutable_accel()->CopyFrom(imuAcc_Normal);
}

// angular_v from chassis to imu
void CIMUSensor::angularVChassis2IMU(const sim_msg::Location &location, osi3::imu_IMUSensorData &imuSensor) {
  // calculate last imu coordiante rot
  chrono::ChQuaternion<double> lastImuQ = mEgoQ_Previous * mImuQ_Local;
  chrono::ChQuaternion<double> currentImuQ = mEgoMoving.GetRot() * mImuQ_Local;

  // omega
  chrono::ChQuaternion<double> delta_Q = (currentImuQ - lastImuQ) * 2.0 / _dt;
  chrono::ChQuaternion<double> wq = lastImuQ.GetConjugate() * delta_Q;

  chrono::ChVector<double> omega = chrono::ChVector<double>(wq.e1(), wq.e2(), wq.e3()) / lastImuQ.Length2();

  // set angular v
  auto angular_v = imuSensor.mutable_angular_v();
  angular_v->set_x(omega[0] * _RAD_2_DEG);
  angular_v->set_y(omega[1] * _RAD_2_DEG);
  angular_v->set_z(omega[2] * _RAD_2_DEG);

  // update ego Q
  mEgoQ_Previous = mEgoMoving.GetRot();
}

// wheelspd from chassis to imu
void CIMUSensor::wheelspdChassis(const double &rl, const double &rr) {
  auto wheelSpdPtr = _imu.mutable_imu_data()->mutable_wheelspd();

  wheelSpdPtr->set_wheelspdrl(rl * _MS_2_KMH);
  wheelSpdPtr->set_wheelspdrr(rr * _MS_2_KMH);
}
}  // namespace imusensor
