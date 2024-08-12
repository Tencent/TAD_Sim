// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_loop.h"
#include "tx_application.h"
#include "tx_header.h"
#include "tx_logger.h"
#include "tx_billboard_flags.h"
#include "tx_billboard_template.h"

TX_NAMESPACE_OPEN(TrafficFlow)
void BillboardLoop::Init(tx_sim::InitHelper& helper) {
  LOG(INFO) << TX_VARS_NAME(sim_mode, helper.GetParameter("sim_mode"));
  m_SimMode = txSimulationLoop::SimulationMode::eBillboard;
  m_SimPtr = std::make_shared<BillboardTemplate>();
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Init(helper);
  } else {
    LOG(ERROR) << "UnSupport Simulation Mode.";
  }
}

void BillboardLoop::Reset(tx_sim::ResetHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Reset(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void BillboardLoop::Step(tx_sim::StepHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Step(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void BillboardLoop::Stop(tx_sim::StopHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Stop(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

TXSIM_MODULE(TrafficFlow::BillboardLoop)
