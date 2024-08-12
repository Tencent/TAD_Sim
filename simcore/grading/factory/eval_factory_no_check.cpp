// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval/general/eval_milleage.h"
#include "eval/meta/eval_meta.h"
#include "eval_factory.h"

#include "eval_factory_no_check.h"

namespace eval {
// global factory variable
// DefinieEvalFactory(EvalFactoryNoCheck);

EvalFactoryNoCheck::EvalFactoryNoCheck() {
  Register(EvalMileage::_kpi_name, EvalMileage::Build);
  Register(EvalMeta::_kpi_name, EvalMeta::Build);
}
EvalFactoryNoCheck::~EvalFactoryNoCheck() {}
}  // namespace eval
