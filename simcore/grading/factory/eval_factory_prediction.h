// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "预测类-评测指标组注册工厂", prediction type eval kpi factory.
 */
class EvalFactoryPrediction final : public EvalFactory {
 public:
  EvalFactoryPrediction();
  virtual ~EvalFactoryPrediction();
};
}  // namespace eval
