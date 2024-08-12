// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule49.h"

namespace eval {
const char EvalRule49::_kpi_name[] = "Rule49";
sim_msg::TestReport_XYPlot EvalRule49::s_rule49_plot;

EvalRule49::EvalRule49() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule49::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule49_plot, "rule49", "", "t", "s",
                               {"u-turn", "_cond_on_crosswalk", "_cond_on_tunnel", "_cond_on_sharp_curve",
                                "_cond_on_bridge", "_cond_on_steep_slope"},
                               {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 6);

    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule49_plot, {0}, false, true);
  }

  return true;
}

bool EvalRule49::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule49");
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
  _cond_on_tunnel = false;
  _cond_on_sharp_curve = false;
  _cond_on_bridge = false;
  _cond_on_steep_slope = false;

  _result = 0.0;

  // 1.get the condition
  // whether is on crosswalk
  eval::Crosswalk crosswalk_on;
  _cond_on_crosswalk = ego_front->IsOnCrosswalk(crosswalk_on);
  VLOG_1 << "cond_on_crosswalk: " << _cond_on_crosswalk << "\n";

  // whether is on tunnel
  _cond_on_tunnel = map_info->m_active_road.m_on_tunnel;
  VLOG_1 << "cond_on_tunnel: " << _cond_on_tunnel << "\n";

  // whether is on sharp_curve
  _cond_on_sharp_curve = map_info->m_active_road.m_on_sharp_curve;
  VLOG_1 << "cond_on_sharp_curve: " << _cond_on_sharp_curve << "\n";

  // whether is on bridge
  _cond_on_bridge = map_info->m_active_road.m_on_bridge;
  VLOG_1 << "cond_on_bridge: " << _cond_on_bridge << "\n";

  // whether is on steep_slope
  _cond_on_steep_slope = map_info->m_active_road.m_on_steep_slope;
  VLOG_1 << "cond_on_steep_slope: " << _cond_on_steep_slope << "\n";

  // 2.Get actual value
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();

  // 4.Check Detect and report
  // check if the car is u-turn under conditions
  if (_cond_on_crosswalk || _cond_on_sharp_curve || _cond_on_bridge || _cond_on_steep_slope || _cond_on_tunnel) {
    _result = (output.m_veh_behav == VehicleBehavior::TurnLeft || output.m_veh_behav == VehicleBehavior::TurnRight);
  }

  // Detect and report
  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should no u-turn, current ego u-turn status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule49_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule49_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule49_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_crosswalk);
    s_rule49_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_tunnel);
    s_rule49_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_sharp_curve);
    s_rule49_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_bridge);
    s_rule49_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_steep_slope);
  }

  return true;
}

bool EvalRule49::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule49_plot);
  }

  return true;
}

void EvalRule49::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule49::IsEvalPass() {
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

bool EvalRule49::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no u-turn";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
