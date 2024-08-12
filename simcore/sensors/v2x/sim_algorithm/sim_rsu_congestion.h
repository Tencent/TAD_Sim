// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "sim_base.h"

class SimRsuCongestion : public SimRsuBase {
 public:
  void RunSim() override;
  void RsuSimExcute(const SimSensorObjects& sim, SimAlgorithmRet& sim_ret) override;
};
