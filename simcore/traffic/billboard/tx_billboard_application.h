// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_application.h"

class Billboard_Application : public txApplication {
 public:
  Billboard_Application() = default;
  ~Billboard_Application() = default;

 public:
  virtual int Run(int argc, char* argv[]) noexcept override;
  virtual void PrintParamValues() noexcept override;
};
using Billboard_ApplicationPtr = std::shared_ptr<Billboard_Application>;
