// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule47.h"

namespace eval {
const char EvalRule47::_kpi_name[] = "Rule47";
sim_msg::TestReport_XYPlot EvalRule47::s_rule47_plot;

EvalRule47::EvalRule47() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule47::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule47_plot, "rule47", "", "t", "s",
                               {"_cond_lane_change_left", "_cond_lane_change_right", "_actual_indicator_proper_used",
                                "_actual_beam_proper_used"},
                               {"OnOff", "OnOff", "OnOff", "OnOff"}, 4);

    ReportHelper::ConfigXYPlotThreshold(s_rule47_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule47::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule47");
  // check whether the module is valid and whether the indicator is enabled
  if (!IsModuleValid() || !m_KpiEnabled) {
    VLOG_0 << _kpi_name << " kpi not enabled.\n";
    return false;
  }

  // get the ego pointer and check whether the pointer is null
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  // if (!ego_front || !ego_trailer) {
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
  _cond_lane_change_left = false;
  _cond_lane_change_right = false;
  _actual_indicator_proper_used = true;
  _actual_beam_proper_used = true;
  // 1.Get the condition
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  if (output.m_duration_valid) {
    if (output.m_veh_behav == VehicleBehavior::LaneChaning_Left) {
      _cond_lane_change_left = true;
    }
    if (output.m_veh_behav == VehicleBehavior::LaneChaning_Right) {
      _cond_lane_change_right = true;
    }
  }
  VLOG_1 << "cond_lane_change_left: " << _cond_lane_change_left << "\n";
  VLOG_1 << "cond_lane_change_right: " << _cond_lane_change_right << "\n";

  // 3.Check
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  // check the car's indicator proper used under conditions
  // check use low beam when car turn left
  if (_cond_lane_change_left) {
    _actual_indicator_proper_used = veh_fb.m_turn_left_lamp == OnOff::ON;
    _actual_beam_proper_used = veh_fb.m_low_beam == OnOff::ON;
  }
  if (_cond_lane_change_right) {
    _actual_indicator_proper_used = veh_fb.m_turn_right_lamp == OnOff::ON;
  }

  // if (!_actual_beam_proper_used || !_actual_beam_proper_used) {
  if (_cond_lane_change_left && (!_actual_indicator_proper_used || !_actual_beam_proper_used)) {
    _result = 1.0;
  }

  if (_cond_lane_change_right && (!_actual_indicator_proper_used)) {
    _result = 1.0;
  }

  // 4.Detect and report
  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should use indicator and beam proper, current: " << _result << " .\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule47_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule47_plot.mutable_y_axis()->at(0).add_axis_data(_cond_lane_change_left);
    s_rule47_plot.mutable_y_axis()->at(1).add_axis_data(_cond_lane_change_right);
    s_rule47_plot.mutable_y_axis()->at(2).add_axis_data(_actual_indicator_proper_used);
    s_rule47_plot.mutable_y_axis()->at(3).add_axis_data(_actual_beam_proper_used);
  }

  return true;
}

bool EvalRule47::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule47_plot);
  }

  return true;
}

void EvalRule47::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule47::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should turn indicator and use low beam");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "beam and indicator proper used check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "beam and indicator proper used check skipped");
}

bool EvalRule47::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should turn indicator and use low beam";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
