// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "法规类-评测指标组注册工厂", regulation type eval kpi factory.
 */
class EvalFactoryRegulations final : public EvalFactory {
 public:
  EvalFactoryRegulations();
  virtual ~EvalFactoryRegulations();
};
}  // namespace eval
