// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval/general/eval_milleage.h"
#include "eval/meta/eval_meta.h"
#include "eval_factory.h"

namespace eval {
class EvalFactoryNoCheck final : public EvalFactory {
 public:
  EvalFactoryNoCheck();
  virtual ~EvalFactoryNoCheck();
};
}  // namespace eval
