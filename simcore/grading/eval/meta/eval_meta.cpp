// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_meta.h"
#include "eval/general/eval_acc.h"
#include "eval/general/eval_collision.h"
#include "eval/general/eval_lateral_acc.h"
#include "eval/general/eval_speed.h"
#include "eval/planning/eval_thw.h"
#include "eval/planning/eval_ttc.h"

namespace eval {
const char EvalMeta::_kpi_name[] = "ReportMetaData";

EvalMeta::EvalMeta() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalMeta::Init(eval::EvalInit &helper) {
  s_ax_plot.Clear();
  s_lateral_acc_plot.Clear();
  s_speed_plot.Clear();
  s_ttc_plot.Clear();
  s_thw_plot.Clear();
  s_collision_plot.Clear();

  return true;
}
bool EvalMeta::Step(eval::EvalStep &helper) { return true; }
bool EvalMeta::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    if (s_ax_plot.x_axis().axis_data_size() > 0) ReportHelper::AddXYPlot2Meta(*helper.GetEgoMeta(), s_ax_plot);
    if (s_lateral_acc_plot.x_axis().axis_data_size() > 0)
      ReportHelper::AddXYPlot2Meta(*helper.GetEgoMeta(), s_lateral_acc_plot);
    if (s_speed_plot.x_axis().axis_data_size() > 0) ReportHelper::AddXYPlot2Meta(*helper.GetEgoMeta(), s_speed_plot);
    if (s_ttc_plot.x_axis().axis_data_size() > 0) ReportHelper::AddXYPlot2Meta(*helper.GetEgoMeta(), s_ttc_plot);
    if (s_thw_plot.x_axis().axis_data_size() > 0) ReportHelper::AddXYPlot2Meta(*helper.GetEgoMeta(), s_thw_plot);
    if (s_collision_plot.x_axis().axis_data_size() > 0)
      ReportHelper::AddXYPlot2Meta(*helper.GetEgoMeta(), s_collision_plot);
  }

  return true;
}
}  // namespace eval
