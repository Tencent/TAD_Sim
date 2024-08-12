// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txsim_logger_wrapper.h"

namespace pblog {
// override tx_sim::SimModule Init()
void CTxSimLogger::Init(tx_sim::InitHelper &helper) { _pb_logger.Init(helper); }

// override tx_sim::SimModule Reset()
void CTxSimLogger::Reset(tx_sim::ResetHelper &helper) { _pb_logger.Reset(helper); }

// override tx_sim::SimModule Step()
void CTxSimLogger::Step(tx_sim::StepHelper &helper) { _pb_logger.Step(helper); }

// override tx_sim::SimModule Stop()
void CTxSimLogger::Stop(tx_sim::StopHelper &helper) { _pb_logger.Stop(helper); }
}  // namespace pblog

// must including this macro definition to access to simulation system
TXSIM_MODULE(pblog::CTxSimLogger)
