// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule79.h"

namespace eval {
const char EvalRule79::_kpi_name[] = "Rule79";
sim_msg::TestReport_XYPlot EvalRule79::s_rule79_plot;

EvalRule79::EvalRule79() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule79::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule79_plot, "turn check", "", "t", "s",
                               {"result", "_cond_on_enter_expressway", "_cond_on_exit_expressway"},
                               {"OnOff", "OnOff", "OnOff"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_rule79_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule79::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule79");
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

  hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
  if (!ego_lane) {
    VLOG_0 << _kpi_name << " ego actor missing with lane.\n";
    return false;
  }

  // param init
  _cond_on_enter_expressway = false;
  _cond_on_exit_expressway = false;
  _result = 0.0;

  // 1.get the condition
  // whether ego is on enter expressway
  _cond_on_enter_expressway = map_info->m_active_lane.m_on_entry_expressway;

  //  whether ego is on exit expressway
  _cond_on_exit_expressway = map_info->m_active_lane.m_on_exit_expressway;

  // 2.Check Detect and report
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  if (_cond_on_enter_expressway) {
    _result = veh_fb.m_turn_left_lamp != OnOff::ON;
  }
  if (_cond_on_exit_expressway) {
    _result = veh_fb.m_turn_right_lamp != OnOff::ON;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego enter expressway:" << _cond_on_enter_expressway << ", exit expressway:" << _cond_on_exit_expressway
         << ", turn lamp properly used:" << _result << ".\n";

  // report add data to xy-pot
  if (isReportEnabled()) {
    s_rule79_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule79_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule79_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_enter_expressway);
    s_rule79_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_exit_expressway);
  }

  return true;
}

bool EvalRule79::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule79_plot);
  }

  return true;
}

void EvalRule79::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule79::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "turn indicator check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "turn indicator check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "turn indicator check skipped");
}

bool EvalRule79::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "turn indicator check fail";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
