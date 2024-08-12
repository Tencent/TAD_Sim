// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "carsim_agent.h"
#include "carsim_core.h"
#include "txsim_module.h"

#include <memory>

namespace tx_carsim {
using CarSimAgentPtr = std::shared_ptr<CarSimAgent>;

class CarSimInterface final : public tx_sim::SimModule {
 public:
  CarSimInterface();
  virtual ~CarSimInterface();

  void Init(tx_sim::InitHelper& helper) override;
  void Reset(tx_sim::ResetHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;
  void Stop(tx_sim::StopHelper& helper) override;

 private:
  std::string mName;
  std::string mPayload;

  CarSimAgentPtr mAgent;
  tx_carsim::SCarsimInit mParam;

  // simulation time
  double t;
};
}  // namespace tx_carsim
