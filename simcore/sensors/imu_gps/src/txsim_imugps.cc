/**
 * @file txsim_imugps.cc
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "txsim_imugps.h"
#include <cmath>
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "json/json.h"
#include "settings.pb.h"
#include "vehState.pb.h"

#define SPLIT_LINE "======================================="

// #define DEBUG_IMU_2_TRAFFIC 1
// #define DEBUG_GPS_2_TRAFFIC 1

namespace imugps {
CCIMU_GPS::CCIMU_GPS() {
  m_enable_sensor = true;
  mMode = Mode::IMUGPS_MODE;
}

CCIMU_GPS::~CCIMU_GPS() {}

void CCIMU_GPS::Init(tx_sim::InitHelper& helper) {
  LOG(INFO) << SPLIT_LINE;

  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kVehicleState);

  std::string mode = helper.GetParameter("mode");
  mode = Mode::IMUGPS_MODE;

  if (mode == "imu") {
    mMode = Mode::IMU_MODE;
    helper.Publish(topic::IMU_SIM);
    LOG(INFO) << "mode is " << mMode << ", imu only.\n";
  } else if (mode == "gps") {
    mMode = Mode::GPS_MODE;
    helper.Publish(topic::GPS_SIM);
    LOG(INFO) << "mode is " << mMode << ", gps only.\n";
  } else {
    mMode = Mode::IMUGPS_MODE;
    LOG(INFO) << "mode is " << mMode << ", imu gps both.\n";
    helper.Publish(topic::IMU_SIM);
    helper.Publish(topic::GPS_SIM);
  }

#if (defined(DEBUG_IMU_2_TRAFFIC) || defined(DEBUG_GPS_2_TRAFFIC))
  helper.Publish("TRAFFIC");
#endif
}

void CCIMU_GPS::Reset(tx_sim::ResetHelper& helper) {
  // init values
  mFirstFrame = true;

  // get config from scenario
  try {
    m_sensor = std::make_shared<CIMUGPSInterface>();
    m_start_loc.ParseFromString(helper.ego_start_location());
    m_enable_sensor = m_sensor->init(helper.scene_pb(), helper.group_name(), helper.ego_start_location(), m_delta_t);
    LOG(INFO) << "imu/gps sensor reset done.\n";
  } catch (const std::exception& e) {
    LOG(ERROR) << "imu_gps | error, " << e.what() << "\n";
    m_enable_sensor = false;
  }

  LOG(INFO) << "imu/gps sensor m_enable_sensor:" << m_enable_sensor << "\n";
}

void CCIMU_GPS::Step(tx_sim::StepHelper& helper) {
  int64_t t_ms = static_cast<int64_t>(helper.timestamp());

  if (m_enable_sensor) {
    // calculate and publish imu/gps sensor data
    std::string loc_payload, chassis_payload;
    helper.GetSubscribedMessage(tx_sim::topic::kLocation, loc_payload);
    helper.GetSubscribedMessage(tx_sim::topic::kVehicleState, chassis_payload);

    // frist frame
    if (chassis_payload.size() == 0 && mFirstFrame) {
      sim_msg::VehicleState vehicleState;

      auto wheelSpeed = vehicleState.mutable_chassis_state()->mutable_wheel_speed();
      double speed = std::hypot(m_start_loc.velocity().x(), m_start_loc.velocity().y());
      wheelSpeed->set_frontleft(speed);
      wheelSpeed->set_frontright(speed);
      wheelSpeed->set_rearleft(speed);
      wheelSpeed->set_rearright(speed);
      vehicleState.SerializeToString(&chassis_payload);
      mFirstFrame = false;
    }

    std::string imu_payload, gps_payload;

    // calculate and publish imu sensor data
    if (mMode == Mode::IMUGPS_MODE || mMode == Mode::IMU_MODE) {
      m_sensor->calIMUData(loc_payload, chassis_payload, t_ms);
      m_sensor->getIMUPayload(imu_payload);
      helper.PublishMessage(topic::IMU_SIM, imu_payload);
    }

    if (mMode == Mode::IMUGPS_MODE || mMode == Mode::GPS_MODE) {
      m_sensor->calGPSData(loc_payload, t_ms);
      m_sensor->getGPSPayload(gps_payload);
      helper.PublishMessage(topic::GPS_SIM, gps_payload);
    }

    // for debug, must close when release, @dhu
#if (defined(DEBUG_IMU_2_TRAFFIC) || defined(DEBUG_GPS_2_TRAFFIC))
    sim_msg::Traffic traffic;
    // debug for imu/gps, must close when deploy
#  ifdef DEBUG_IMU_2_TRAFFIC
    osi3::imu imu;
    imu.ParseFromString(imu_payload);
    debugIMU2Traffic(traffic, imu);
#  endif

#  ifdef DEBUG_GPS_2_TRAFFIC
    osi3::GPS gps;
    gps.ParseFromString(gps_payload);
    debugGPS2Traffic(traffic, gps);
#  endif
    std::string traffic_payload;
    traffic.SerializeToString(&traffic_payload);
    helper.PublishMessage(tx_sim::topic::kTraffic, traffic_payload);
#endif
  } else {
    LOG_EVERY_N(WARNING, 100) << "imu/gps sensor not enabled.\n";
  }
}

void CCIMU_GPS::Stop(tx_sim::StopHelper& helper) {
  // std::cout << name_ << " module stopped." << std::endl;
  m_sensor.reset();
}

void CCIMU_GPS::debugIMU2Traffic(sim_msg::Traffic& traffic, const osi3::imu& imu) {
  sim_msg::Car* ptrCar = traffic.add_cars();
  double v = sqrt(pow(imu.imu_rigid().velocity().x(), 2.0) + pow(imu.imu_rigid().velocity().y(), 2.0));

  ptrCar->set_id(123456);
  ptrCar->set_length(4.6);
  ptrCar->set_width(1.8);
  ptrCar->set_v(v);
  ptrCar->set_theta(imu.imu_rigid().eulerangle().yaw());
  ptrCar->set_heading(imu.imu_rigid().eulerangle().yaw());
  ptrCar->set_x(imu.imu_rigid().gnss().longti());
  ptrCar->set_y(imu.imu_rigid().gnss().lat());
  ptrCar->set_z(imu.imu_rigid().gnss().alt());
  ptrCar->set_type(1);
}

void CCIMU_GPS::debugGPS2Traffic(sim_msg::Traffic& traffic, const osi3::GPS& gps) {
  sim_msg::Car* ptrCar1 = traffic.add_cars();
  double v1 = gps.vel_hrz() / 3.6;
  double yaw = -gps.track() / 180.0 * 3.1415926 + 3.1415926 / 2.0;

  ptrCar1->set_id(1234567);
  ptrCar1->set_length(4.6);
  ptrCar1->set_width(1.8);
  ptrCar1->set_v(v1);
  ptrCar1->set_theta(yaw);
  ptrCar1->set_heading(yaw);
  ptrCar1->set_x(gps.longitude());
  ptrCar1->set_y(gps.latitude());
  ptrCar1->set_z(gps.height());
  ptrCar1->set_type(1);
}

}  // namespace imugps
TXSIM_MODULE(imugps::CCIMU_GPS)
