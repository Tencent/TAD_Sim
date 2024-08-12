// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "通常类-评测指标组注册工厂", general type eval kpi factory.
 */
class EvalFactoryGeneral final : public EvalFactory {
 public:
  EvalFactoryGeneral();
  virtual ~EvalFactoryGeneral();
};
}  // namespace eval
