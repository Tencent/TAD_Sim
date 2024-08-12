// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval_factory.h"

namespace eval {
/**
 * @brief "定制类-评测指标组注册工厂", custom type eval kpi factory.
 */
class EvalFactoryCustom final : public EvalFactory {
 public:
  EvalFactoryCustom();
  virtual ~EvalFactoryCustom();
};
}  // namespace eval
