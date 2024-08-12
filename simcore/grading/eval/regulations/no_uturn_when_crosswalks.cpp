// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_uturn_when_crosswalks.h"

namespace eval {
const char NoUTurnWhenCrosswalks::_kpi_name[] = "NoUTurnWhenCrosswalks";
sim_msg::TestReport_XYPlot NoUTurnWhenCrosswalks::s_no_u_turn_when_crosswalks_plot;

NoUTurnWhenCrosswalks::NoUTurnWhenCrosswalks() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool NoUTurnWhenCrosswalks::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_u_turn_when_crosswalks_plot, "NoUTurnWhenCrosswalks", "", "t", "s",
                               {"result", "_cond_on_crosswalk", "_cond_u_turn"}, {"OnOff", "OnOff", "OnOff"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_no_u_turn_when_crosswalks_plot, "NoUTurnWhenCrosswalks", 0, 1, _threshold, "",
                                        0, 0, INT32_MIN, 0);
  }

  return true;
}

bool NoUTurnWhenCrosswalks::Step(eval::EvalStep &helper) {
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

  hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
  if (!ego_road) {
    VLOG_0 << _kpi_name << " ego actor missing with road.\n";
    return false;
  }

  // param init
  _cond_on_crosswalk = false;
  _cond_u_turn = false;
  _result = 0.0;

  // 1.get ego stat
  eval::Crosswalk crosswalk_on;
  _cond_on_crosswalk = ego_front->IsOnCrosswalk(crosswalk_on);
  VLOG_1 << "cond_on_crosswalk: " << _cond_on_crosswalk << "\n";

  // 2.Get actual value
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  _cond_u_turn =
      (output.m_veh_behav == VehicleBehavior::U_TurnLeft || output.m_veh_behav == VehicleBehavior::U_TurnRight);

  // 4.Check Detect and report
  // check if the car is u-turn under conditions
  if (_cond_on_crosswalk && _cond_u_turn) {
    _result = 1.0;
  }

  // Detect and report
  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should no u-turn, current ego u-turn status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_u_turn_when_crosswalks_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_u_turn_when_crosswalks_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_no_u_turn_when_crosswalks_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_crosswalk);
    s_no_u_turn_when_crosswalks_plot.mutable_y_axis()->at(2).add_axis_data(_cond_u_turn);
  }

  return true;
}

bool NoUTurnWhenCrosswalks::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_no_u_turn_when_crosswalks_plot);
  }

  return true;
}

void NoUTurnWhenCrosswalks::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult NoUTurnWhenCrosswalks::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should no u-turn");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no u-turn check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "no u-turn check skipped");
}

bool NoUTurnWhenCrosswalks::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no u-turn";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
