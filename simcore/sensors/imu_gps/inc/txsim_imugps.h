// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "imugps_interface.h"
#include "txsim_module.h"

#include "location.pb.h"
#include "osi_gps.pb.h"
#include "osi_imu.pb.h"
#include "traffic.pb.h"

namespace imugps {
enum Mode { IMUGPS_MODE = 0, IMU_MODE = 1, GPS_MODE = 2 };

namespace topic {
const char IMU_SIM[] = "IMU_SIM";
const char GPS_SIM[] = "GPS_SIM";
}  // namespace topic
class CCIMU_GPS final : public tx_sim::SimModule {
 public:
  CCIMU_GPS();
  virtual ~CCIMU_GPS();

  void Init(tx_sim::InitHelper& helper) override;
  void Reset(tx_sim::ResetHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;
  void Stop(tx_sim::StopHelper& helper) override;

 public:
  void debugIMU2Traffic(sim_msg::Traffic& traffic, const osi3::imu& imu);
  void debugGPS2Traffic(sim_msg::Traffic& traffic, const osi3::GPS& gps);

 private:
  std::string name_;
  std::shared_ptr<CIMUGPSInterface> m_sensor;
  bool m_enable_sensor;
  sim_msg::Location m_start_loc;
  bool mFirstFrame;
  const double m_delta_t = 0.01;
  Mode mMode;
};
}  // namespace imugps
