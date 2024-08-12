// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "控制类-评测指标组注册工厂", control type eval kpi factory.
 */
class EvalFactoryControl final : public EvalFactory {
 public:
  EvalFactoryControl();
  virtual ~EvalFactoryControl();
};

}  // namespace eval
