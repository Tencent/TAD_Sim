// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "txsim_module.h"

namespace adapter {
class OSIAdapter final : public tx_sim::SimModule {
 private:
  double _t_ms;

 protected:
  void Transform(tx_sim::StepHelper &helper);

 public:
  OSIAdapter();
  virtual ~OSIAdapter();

  void Init(tx_sim::InitHelper &helper);
  void Reset(tx_sim::ResetHelper &helper);
  void Step(tx_sim::StepHelper &helper);
  void Stop(tx_sim::StopHelper &helper);
};
}  // namespace adapter

TXSIM_MODULE(adapter::OSIAdapter)
