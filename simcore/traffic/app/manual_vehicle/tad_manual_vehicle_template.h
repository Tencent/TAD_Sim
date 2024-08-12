// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_manual_vehicle_system.h"
#include "worldsim_sim_template.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_ManualVehicleSimLoop : public WorldsimSimLoop {
  using ParentClass = WorldsimSimLoop;

 public:
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE;
  virtual TAD_ManualVehicleSystemPtr ManualTrafficSystemPtr() TX_NOEXCEPT { return m_tad_manual_vehicle_system_ptr; }

 protected:
  Base::txString strManualVehicle_;
  TAD_ManualVehicleSystemPtr m_tad_manual_vehicle_system_ptr = nullptr;
};

using TAD_ManualVehicleSimLoopPtr = std::shared_ptr<TAD_ManualVehicleSimLoop>;

TX_NAMESPACE_CLOSE(TrafficFlow)
