// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class WheelTireParamWriter {
 public:
  WheelTireParamWriter();
  void fillWheelTire(tx_car::car &m_car);
  void fillBrake(tx_car::car &m_car);
};
