// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "Chassis.h"
#include "inc/car_common.h"

namespace tx_car {
namespace chassis {
class MODULE_API ChassisExt : public Chassis {
 public:
  ChassisExt();
  virtual ~ChassisExt();

 public:
  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  /* load parameter from json file and init model */
  bool initMDL();

  /* load parameter from json file */
  bool loadParam(const std::string &par_path);

  /* init model by loaded json file */
  bool initParam();
  bool initParamSusp();
  bool initParamBody();
  bool initParamWheelTire();

 private:
  tx_car::TxCarInfo m_err_info;
};
}  // namespace chassis
}  // namespace tx_car
