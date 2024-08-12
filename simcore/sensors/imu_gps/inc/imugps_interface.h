// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <memory>
#include <string>

#ifdef _WIN32
#  define MODULE_API __declspec(dllexport)
#else
#  define MODULE_API
#endif

namespace gpssensor {
class CGPSSensor;
using CGPSSensorPtr = std::shared_ptr<CGPSSensor>;
}  // namespace gpssensor

namespace imusensor {
class CIMUSensor;
using CIMUSensorPtr = std::shared_ptr<CIMUSensor>;
}  // namespace imusensor

namespace imugps {
class CIMUGPSInterface final {
 public:
  CIMUGPSInterface();
  ~CIMUGPSInterface();
  CIMUGPSInterface(const CIMUGPSInterface &) = delete;
  CIMUGPSInterface &operator=(const CIMUGPSInterface &) = delete;
  CIMUGPSInterface(CIMUGPSInterface &&) = delete;
  CIMUGPSInterface &operator=(CIMUGPSInterface &&) = delete;

 public:
  bool init(const std::string &buffer, const std::string &groupname, const std::string &start_location,
            double dt = 0.01);
  void calIMUData(const std::string &loc_payload, const std::string &chassis_payload, int64_t t_ms);
  void calGPSData(const std::string &loc_payload, int64_t t_ms);
  void stop();

  void getIMUPayload(std::string &payload);
  void getGPSPayload(std::string &payload);

 private:
  imusensor::CIMUSensorPtr m_imu;
  gpssensor::CGPSSensorPtr m_gps;

  std::string m_sensor_xml;
  double m_dt;  // unit s
  bool mIMUValid, mGPSValid;
};
}  // namespace imugps
