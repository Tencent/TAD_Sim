// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "规划类-评测指标组注册工厂", planning type eval kpi factory.
 */
class EvalFactoryPlanning final : public EvalFactory {
 public:
  EvalFactoryPlanning();
  virtual ~EvalFactoryPlanning();
};
}  // namespace eval
