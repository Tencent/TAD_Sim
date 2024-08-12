// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_reversing_when_intersections.h"

namespace eval {
const char NoReversingWhenIntersections::_kpi_name[] = "NoReversingWhenIntersections";
sim_msg::TestReport_XYPlot NoReversingWhenIntersections::s_no_reversing_when_intersections_plot;

NoReversingWhenIntersections::NoReversingWhenIntersections() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoReversingWhenIntersections::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_reversing_when_intersections_plot, "no_reversing_when_expressway", "", "t", "s",
                               {"result", "_cond_on_junction"}, {"OnOff", "OnOff"}, 2);

    ReportHelper::ConfigXYPlotThreshold(s_no_reversing_when_intersections_plot, "Rule Violation ", 0, 1, 0.5, "", 0, 0,
                                        INT32_MIN, 0);
  }

  return true;
}

bool NoReversingWhenIntersections::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (!IsModuleValid() || !m_KpiEnabled) {
    VLOG_0 << _kpi_name << " kpi not enabled.\n";
    return false;
  }

  // get the ego pointer and check whether the pointer is null
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  if (!ego_front) {
    VLOG_0 << _kpi_name << " ego actor missing.\n";
    return false;
  }

  // get the map information pointer and check whether the pointer is null
  auto map_info = ego_front->GetMapInfo();
  if (map_info.get() == nullptr) {
    VLOG_0 << _kpi_name << " ego actor missing with map.\n";
    return false;
  }

  // 1.get the condition
  // whether ego is on jection
  bool _cond_on_junction = map_info->m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // 2.Get actual value
  // const VehOutput &output = helper.GetVehStateFlow().GetOutput();

  // 3.Check if the car is reversing under conditions
  // _result = output.m_duration_valid && (output.m_veh_behav == VehicleBehavior::Reversing) && _cond_on_junction;
  bool _result = ego_front->IsReverse() && _cond_on_junction;

  // 4.Detect and report
  _detector.Detect(_result, 0.5);
  VLOG_1 << "ego should no reverse, current ego reverse status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_reversing_when_intersections_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_reversing_when_intersections_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_no_reversing_when_intersections_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_junction);
  }

  return true;
}

bool NoReversingWhenIntersections::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_no_reversing_when_intersections_plot);
  }

  return true;
}

void NoReversingWhenIntersections::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult NoReversingWhenIntersections::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should no reverse");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no reverse check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "no reverse check skipped");
}

bool NoReversingWhenIntersections::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no reverse";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
