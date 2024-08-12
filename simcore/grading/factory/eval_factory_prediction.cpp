// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/prediction/eval_pred_acc_avg.h"
#include "eval/prediction/eval_pred_acc_max.h"
#include "eval/prediction/eval_pred_first_effective.h"
#include "eval/prediction/eval_pred_heading_avg.h"
#include "eval/prediction/eval_pred_heading_max.h"
#include "eval/prediction/eval_pred_lateral_avg.h"
#include "eval/prediction/eval_pred_lateral_max.h"
#include "eval/prediction/eval_pred_speed_avg.h"
#include "eval/prediction/eval_pred_speed_max.h"
#include "eval/prediction/eval_pred_vertical_avg.h"
#include "eval/prediction/eval_pred_vertical_max.h"

#include "eval_factory_interface.h"
#include "eval_factory_prediction.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryPrediction);

EvalFactoryPrediction::EvalFactoryPrediction() {
  std::cout << "EvalFactoryPrediction constructed.\n";
  Register(EvalPredFirstEffective::_kpi_name, EvalPredFirstEffective::Build);
  Register(EvalPredAccAvg::_kpi_name, EvalPredAccAvg::Build);
  Register(EvalPredAccMax::_kpi_name, EvalPredAccMax::Build);
  Register(EvalPredHeadingAvg::_kpi_name, EvalPredHeadingAvg::Build);
  Register(EvalPredHeadingMax::_kpi_name, EvalPredHeadingMax::Build);
  Register(EvalPredLateralAvg::_kpi_name, EvalPredLateralAvg::Build);
  Register(EvalPredLateralMax::_kpi_name, EvalPredLateralMax::Build);
  Register(EvalPredSpeedAvg::_kpi_name, EvalPredSpeedAvg::Build);
  Register(EvalPredSpeedMax::_kpi_name, EvalPredSpeedMax::Build);
  Register(EvalPredVerticalAvg::_kpi_name, EvalPredVerticalAvg::Build);
  Register(EvalPredVerticalMax::_kpi_name, EvalPredVerticalMax::Build);
}
EvalFactoryPrediction::~EvalFactoryPrediction() {}
}  // namespace eval
