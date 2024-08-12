// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class VehBodyWriter {
 public:
  void fillSprungMass(tx_car::car &m_car);
  void fillVehbody(tx_car::car &m_car);
};
