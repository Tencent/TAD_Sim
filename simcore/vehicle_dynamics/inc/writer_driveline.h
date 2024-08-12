// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "car.pb.h"
#include "inc/car_common.h"

class DrivelineParamWriter {
 public:
  DrivelineParamWriter();
  void fillDriveline(tx_car::car &m_car);
};
