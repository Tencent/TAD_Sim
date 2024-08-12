// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_cloud_stand_alone_loop.h"
#include "tx_cloud_stand_alone_template.h"
#include "tx_header.h"
#include "tx_logger.h"
#include "tx_tc_gflags.h"

TX_NAMESPACE_OPEN(TrafficFlow)

void CloudStandAloneLoop::Init(tx_sim::InitHelper& helper) {
  LOG(INFO) << "call CloudStandAloneLoop::Init.";
  m_SimMode = SimulationMode::eCloudStandAlone;

  m_SimPtr = std::make_shared<CloudStandAloneTemplate>();

  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Init(helper);
  } else {
    LOG(ERROR) << "UnSupport Simulation Mode.";
  }
}

void CloudStandAloneLoop::Reset(tx_sim::ResetHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Reset(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void CloudStandAloneLoop::Step(tx_sim::StepHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Step(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void CloudStandAloneLoop::Stop(tx_sim::StopHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Stop(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

TXSIM_MODULE(TrafficFlow::CloudStandAloneLoop)
