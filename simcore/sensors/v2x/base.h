/**
 * @file base.h
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once

#include "txsim_module.h"

class Base : public tx_sim::SimModule {
 public:
  void Init(tx_sim::InitHelper& helper) {}
  void Reset(tx_sim::ResetHelper& helper) {}
  void Step(tx_sim::StepHelper& helper) {}
  void Stop(tx_sim::StopHelper& helper) {}

 protected:
  tx_sim::Vector3d map_ori;
  friend class SimV2X;
};
