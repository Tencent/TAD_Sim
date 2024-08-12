// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class SteerMappedWriter {
 public:
  void fillMappedSteer(tx_car::car &m_car);
};
