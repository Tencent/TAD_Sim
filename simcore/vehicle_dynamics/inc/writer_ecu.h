// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class EcuParaWriter {
 public:
  EcuParaWriter();
  void fillPowEcu(tx_car::car& m_car);
  void fillHCU(tx_car::car& m_car);
};
