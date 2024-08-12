// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_application.h"

class TADSim_Planning_Application : public txApplication {
 public:
  TADSim_Planning_Application() = default;
  ~TADSim_Planning_Application() = default;

 public:
  virtual int Run(int argc, char* argv[]) noexcept override;
  virtual void PrintParamValues() noexcept override;
};
using TADSim_Planning_ApplicationPtr = std::shared_ptr<TADSim_Planning_Application>;
