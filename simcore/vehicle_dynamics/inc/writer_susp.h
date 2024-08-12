// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class SuspWriter {
 public:
  void fillSusp(tx_car::car &m_car);
};
