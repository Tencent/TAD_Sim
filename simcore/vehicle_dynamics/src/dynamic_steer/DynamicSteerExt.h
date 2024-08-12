// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "DynamicSteer.h"
#include "inc/car_common.h"

namespace tx_car {
namespace steer {
class MODULE_API DynamicSteerExt : public DynamicSteer {
 public:
  DynamicSteerExt();
  virtual ~DynamicSteerExt();

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

  /* set values of mapped steer */
  void initMappedSteer();

  /* set values of dynamic steer */
  void initDynamicSteer();

 private:
  tx_car::TxCarInfo m_err_info;
};
}  // namespace steer
}  // namespace tx_car
