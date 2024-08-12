// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class DynamicSteerWriter {
 public:
  void fillDynamicSteer(tx_car::car &m_car);
};
