// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <string>
#include <vector>
#include "environment.pb.h"
#include "txsim_module.h"

class envpb final : public tx_sim::SimModule {
 public:
  envpb();
  virtual ~envpb();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  std::vector<std::pair<double, sim_msg::EnvironmentalConditions>> envdata;
  std::string interpMethod;
  size_t cur_idx = 0;
};
