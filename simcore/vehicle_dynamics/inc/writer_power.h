// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class PowerParamWriter {
 public:
  PowerParamWriter();
  void fillEngine(tx_car::car &m_car);
  void fillFrontMotor(tx_car::car &m_car);
  void fillRearMotor(tx_car::car &m_car);
  void fillBattery(tx_car::car &m_car);
  void fillHybrid(tx_car::car &m_car);
};
