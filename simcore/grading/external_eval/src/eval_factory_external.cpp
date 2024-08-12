// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory_external.h"

// 1、新指标在这里添加头文件
#include "eval_ex_kpi_example.h"

#include "factory/eval_factory_interface.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryExternal);

EvalFactoryExternal::EvalFactoryExternal() {
  std::cout << "EvalFactoryExternal constructed.\n";
  // 2、新指标在这里新增一行Register来注册
  Register(EvalExKpiExample::_kpi_name, EvalExKpiExample::Build);
}
EvalFactoryExternal::~EvalFactoryExternal() {}
}  // namespace eval
