// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/custom/eval_disengage_times.h"
#include "eval/custom/eval_driving_comfort.h"
#include "eval/custom/eval_driving_safety.h"
#include "eval/custom/eval_drving_efficiency.h"
#include "eval/custom/eval_l4interrupt_check.h"
#include "eval/custom/eval_member_comfort.h"
#include "eval/custom/eval_ride_smoothness.h"
#include "eval/custom/eval_set_back_check.h"
#include "eval/custom/eval_start_manu_check.h"
#include "eval/custom/eval_start_manu_time_check.h"

#include "eval_factory_custom.h"
#include "eval_factory_interface.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryCustom);

EvalFactoryCustom::EvalFactoryCustom() {
  std::cout << "EvalFactoryCustom constructed.\n";
  Register(EvalDisengageTimes::_kpi_name, EvalDisengageTimes::Build);
  Register(EvalSetBackCheck::_kpi_name, EvalSetBackCheck::Build);
  Register(EvalDrivingComfort::_kpi_name, EvalDrivingComfort::Build);
  Register(EvalDrivingSafety::_kpi_name, EvalDrivingSafety::Build);
  Register(EvalDrvingEfficiency::_kpi_name, EvalDrvingEfficiency::Build);
  Register(EvalL4InterruptCheck::_kpi_name, EvalL4InterruptCheck::Build);
  Register(EvalMemberComfort::_kpi_name, EvalMemberComfort::Build);
  Register(EvalRideSmoothness::_kpi_name, EvalRideSmoothness::Build);
  Register(EvalStartManuCheck::_kpi_name, EvalStartManuCheck::Build);
  Register(EvalStartManuTimeCheck::_kpi_name, EvalStartManuTimeCheck::Build);
}
EvalFactoryCustom::~EvalFactoryCustom() {}
}  // namespace eval
