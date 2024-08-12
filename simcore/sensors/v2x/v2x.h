/**
 * @file v2x.h
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "base.h"
#include "txsim_module.h"

class SimV2X final : public tx_sim::SimModule {
 public:
  SimV2X();
  virtual ~SimV2X();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  std::string tadsimPath;
  std::vector<std::shared_ptr<Base>> modules;
};
