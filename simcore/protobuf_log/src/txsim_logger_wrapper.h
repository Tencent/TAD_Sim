// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @file txSimLoggerWrapper.h
 * @author DongYuanHu (dongyuanhu@tencent.com)
 * @brief protobuf logger interface
 * @version 0.1
 * @date 2023-07-05
 *
 *
 */
#pragma once

#include "txsim_logger.h"

namespace pblog {
/**
 * @brief Logger wrapper for txSim
 */
class CTxSimLogger final : public tx_sim::SimModule {
 private:
  pblog::txSimLogger _pb_logger;

 public:
  CTxSimLogger() {}
  virtual ~CTxSimLogger() {}

 public:
  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;
};
}  // namespace pblog
