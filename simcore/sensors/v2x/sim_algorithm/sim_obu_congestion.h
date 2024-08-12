// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "sim_base.h"
#include "traffic.pb.h"

class SimObuCongestion : public SimObuBase {
 public:
  void RunSim() override;
  void ObuSimExcute(const SimObus& sim, SimAlgorithmRet& sim_ret) override;
};
