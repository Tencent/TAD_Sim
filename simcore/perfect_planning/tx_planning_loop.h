// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_simulation_loop.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_PlanningLoop : public Base::txSimulationLoop {
 public:
  TAD_PlanningLoop() TX_DEFAULT;
  virtual ~TAD_PlanningLoop() TX_DEFAULT;

  virtual void Init(tx_sim::InitHelper& helper) TX_OVERRIDE;
  virtual void Reset(tx_sim::ResetHelper& helper) TX_OVERRIDE;
  virtual void Step(tx_sim::StepHelper& helper) TX_OVERRIDE;
  virtual void Stop(tx_sim::StopHelper& helper) TX_OVERRIDE;

 protected:
  Base::txSimulationTemplatePtr m_SimPtr = nullptr;
};
using TAD_PlanningLoopPtr = std::shared_ptr<TAD_PlanningLoop>;
TX_NAMESPACE_CLOSE(TrafficFlow)
