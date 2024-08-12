// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule50.h"

namespace eval {
const char EvalRule50::_kpi_name[] = "Rule50";
sim_msg::TestReport_XYPlot EvalRule50::s_rule50_plot;

EvalRule50::EvalRule50() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule50::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule50_plot, "rule50", "", "t", "s",
                               {"_result", "_cond_on_junction", "_cond_unbidirectional", "_cond_on_tunnel",
                                "_cond_on_sharp_curve", "_cond_on_bridge", "_cond_on_steep_slope"},
                               {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 7);
    ReportHelper::ConfigXYPlotThreshold(s_rule50_plot, "", 0, 0, 0.5, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule50::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule50");
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

  // param init
  bool _result = false;

  // 1.get the condition
  // whether ego is on jection
  bool _cond_on_junction = map_info->m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether ego is on unbidirectional
  bool _cond_unbidirectional = map_info->m_active_road.m_is_unbidirectional;
  VLOG_1 << "cond_unbidirectional: " << _cond_unbidirectional;

  // whether is on tunnel
  bool _cond_on_tunnel = map_info->m_active_road.m_on_tunnel;
  VLOG_1 << "cond_on_tunnel: " << _cond_on_tunnel << "\n";

  // whether is on sharp_curve
  bool _cond_on_sharp_curve = map_info->m_active_road.m_on_sharp_curve;
  VLOG_1 << "cond_on_sharp_curve: " << _cond_on_sharp_curve << "\n";

  // whether is on bridge
  bool _cond_on_bridge = map_info->m_active_road.m_on_bridge;
  VLOG_1 << "cond_on_bridge: " << _cond_on_bridge << "\n";

  // whether is on steep_slope
  bool _cond_on_steep_slope = map_info->m_active_road.m_on_steep_slope;
  VLOG_1 << "cond_on_steep_slope: " << _cond_on_steep_slope << "\n";

  // 2.Check if the car is reversing under conditions
  if ((_cond_on_junction || _cond_unbidirectional || _cond_on_bridge || _cond_on_sharp_curve || _cond_on_steep_slope ||
       _cond_on_tunnel)) {
    _result = ego_front->IsReverse();
  }

  // 3.Detect and report
  _detector.Detect(_result, 0.5);
  VLOG_1 << "ego should no reverse, current ego reverse status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule50_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule50_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule50_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_junction);
    s_rule50_plot.mutable_y_axis()->at(2).add_axis_data(_cond_unbidirectional);
    s_rule50_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_tunnel);
    s_rule50_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_sharp_curve);
    s_rule50_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_bridge);
    s_rule50_plot.mutable_y_axis()->at(6).add_axis_data(_cond_on_steep_slope);
  }

  return true;
}

bool EvalRule50::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule50_plot);
  }

  return true;
}

void EvalRule50::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule50::IsEvalPass() {
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

bool EvalRule50::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no reverse";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
