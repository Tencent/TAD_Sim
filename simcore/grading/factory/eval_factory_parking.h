// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "泊车类-评测指标组注册工厂", parking type eval kpi factory.
 */
class EvalFactoryParking final : public EvalFactory {
 public:
  EvalFactoryParking();
  virtual ~EvalFactoryParking();
};
}  // namespace eval
