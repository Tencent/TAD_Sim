// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "factory/eval_factory.h"

namespace eval {
/**
 * @brief "外部类-评测指标组注册工厂", external type eval kpi factory.
 */
class EvalFactoryExternal final : public EvalFactory {
 public:
  EvalFactoryExternal();
  virtual ~EvalFactoryExternal();
};
}  // namespace eval
