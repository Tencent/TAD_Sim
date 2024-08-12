// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "感知类-评测指标组注册工厂", perception type eval kpi factory.
 */
class EvalFactoryPerception final : public EvalFactory {
 public:
  EvalFactoryPerception();
  virtual ~EvalFactoryPerception();
};
}  // namespace eval
