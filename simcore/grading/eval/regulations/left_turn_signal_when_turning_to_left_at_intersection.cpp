// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "left_turn_signal_when_turning_to_left_at_intersection.h"

namespace eval {
const char LeftTurnSignalWhenTurningToLeftAtIntersection::_kpi_name[] = "LeftTurnSignalWhenTurningToLeftAtIntersection";
sim_msg::TestReport_XYPlot s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot;

LeftTurnSignalWhenTurningToLeftAtIntersection::LeftTurnSignalWhenTurningToLeftAtIntersection() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool LeftTurnSignalWhenTurningToLeftAtIntersection::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
    _result = true;
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot, "turn left check", "", "t", "s",
                               {"_result", "_cond_turn_left", "_cond_left_indicator"}, {"OnOff", "OnOff", "OnOff"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot, "Rule Violation ", 0, 1,
                                        _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool LeftTurnSignalWhenTurningToLeftAtIntersection::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("LeftTurnSignalWhenTurningToLeftAtIntersection");
  // check whether the module is valid and whether the indicator is enabled
  if (!IsModuleValid() || !m_KpiEnabled) {
    VLOG_0 << _kpi_name << " kpi not enabled.\n";
    return false;
  }

  // param init
  _cond_turn_left = false;
  _cond_left_indicator = false;
  _result = 0.0;

  // ego
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  if (!ego_front) {
    VLOG_0 << _kpi_name << " ego actor missing.\n";
    return false;
  }

  // output of state-flow and get vehicle state
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  const VehicleBodyControl &ctrl_command = ego_front->GetVehicleBodyCmd();

  // get vehicle state
  _cond_turn_left = (output.m_veh_behav == VehicleBehavior::TurnLeft);

  // get lamp state
  _cond_left_indicator = ctrl_command.m_turn_left_lamp == OnOff::ON;

  // check proper
  if (_cond_turn_left && !_cond_left_indicator) _result = 1.0;
  VLOG_1 << "ego state " << output.m_veh_behav << ", ego turn lamp misuse used:" << _result << ".\n";
  _detector.Detect(_result, _threshold);

  // add data to xy-pot
  if (isReportEnabled()) {
    s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot.mutable_y_axis()->at(1).add_axis_data(_cond_turn_left);
    s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot.mutable_y_axis()->at(2).add_axis_data(_cond_left_indicator);
  }

  return true;
}

bool LeftTurnSignalWhenTurningToLeftAtIntersection::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_LeftTurnSignalWhenTurningToLeftAtIntersection_plot);
  }

  return true;
}

void LeftTurnSignalWhenTurningToLeftAtIntersection::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult LeftTurnSignalWhenTurningToLeftAtIntersection::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "turn left with left lamp usage check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "turn left with left lamp usage check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "turn left with left lamp usage check skipped");
}

bool LeftTurnSignalWhenTurningToLeftAtIntersection::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "turn left with left lamp usage check fail";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
