// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_simulation_loop.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class BillboardLoop : public Base::txSimulationLoop {
 public:
  BillboardLoop() TX_DEFAULT;
  virtual ~BillboardLoop() TX_DEFAULT;

  virtual void Init(tx_sim::InitHelper& helper) TX_OVERRIDE;
  virtual void Reset(tx_sim::ResetHelper& helper) TX_OVERRIDE;
  virtual void Step(tx_sim::StepHelper& helper) TX_OVERRIDE;
  virtual void Stop(tx_sim::StopHelper& helper) TX_OVERRIDE;

 protected:
  Base::txSimulationTemplatePtr m_SimPtr = nullptr;
};
using BillboardLoopPtr = std::shared_ptr<BillboardLoop>;
TX_NAMESPACE_CLOSE(TrafficFlow)
