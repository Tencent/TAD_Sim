// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule82.h"

namespace eval {
const char EvalRule82::_kpi_name[] = "Rule82";
sim_msg::TestReport_XYPlot EvalRule82::s_rule82_plot;

EvalRule82::EvalRule82() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule82::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(
        s_rule82_plot, "Rule82", "", "t", "s",
        {"_result", "_cond_on_lane_boundary", "_cond_on_enter_expressway", "_cond_on_exit_expressway",
         "_cond_on_lane_acceleration", "_cond_on_lane_deceleration", "_cond_on_lane_shoulder",
         "_cond_on_lane_emergency", "_actual_parking", "_actual_reverse", "_actual_overtaking",
         "_actual_driving_shoulder", "_actual_cross_solid_line"},
        {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff",
         "OnOff"},
        13);

    ReportHelper::ConfigXYPlotThreshold(s_rule82_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule82::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule82");
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
  _cond_on_lane_boundary = false;
  _cond_on_enter_expressway = false;
  _cond_on_exit_expressway = false;
  _cond_on_lane_acceleration = false;
  _cond_on_lane_deceleration = false;
  _cond_on_lane_shoulder = false;
  _cond_on_lane_emergency = false;

  _actual_parking = false;
  _actual_reverse = false;
  _actual_overtaking = false;
  _actual_driving_shoulder = false;
  _actual_cross_solid_line = false;
  _actual_driving_emergency = false;

  _result = 0.0;

  // 1.get the condition
  // whether on lane boundary
  eval::LaneBoundry boundry_on;
  _cond_on_lane_boundary = ego_front->IsOnSolidBoundry(boundry_on);
  auto ego_trailer = _actor_mgr->GetEgoTrailerActorPtr();
  if (!_cond_on_lane_boundary && ego_trailer) _cond_on_lane_boundary = ego_trailer->IsOnSolidBoundry(boundry_on);

  // whether ego is on enter expressway
  _cond_on_enter_expressway = map_info->m_active_lane.m_on_entry_expressway;

  //  whether ego is on exit expressway
  _cond_on_exit_expressway = map_info->m_active_lane.m_on_exit_expressway;

  // whether ego is on acceleration lane
  _cond_on_lane_acceleration = map_info->m_active_lane.m_on_lane_acceleration;

  // whether ego is on deceleration lane
  _cond_on_lane_deceleration = map_info->m_active_lane.m_on_lane_deceleration;

  // whether ego is on shoulder lane
  _cond_on_lane_shoulder = map_info->m_active_lane.m_on_lane_shoulder;

  // whether ego is on emergency lane
  _cond_on_lane_emergency = map_info->m_active_lane.m_on_lane_emergency;

  // 4.Check Detect and report
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  // if (output.m_duration_valid) {
  if (_cond_on_enter_expressway) {
    _actual_parking = (output.m_veh_behav == VehicleBehavior::Stopped);
    _actual_reverse = (ego_front->IsReverse());
  }

  if (_cond_on_enter_expressway || _cond_on_exit_expressway || _cond_on_lane_acceleration ||
      _cond_on_lane_deceleration) {
    _actual_overtaking = (output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
                          output.m_veh_behav == VehicleBehavior::LaneChaning_Right) &&
                         output.m_duration_valid;
  }

  if (_cond_on_lane_shoulder) {
    _actual_driving_shoulder = true;
  }

  if (_cond_on_lane_boundary) {
    _actual_cross_solid_line = true;
  }

  if (_cond_on_lane_emergency) {
    _actual_driving_emergency = true;
  }
  // }

  // if (!_actual_parking && !_actual_reverse && !_actual_overtaking && !_actual_driving_shoulder &&
  //     !_actual_cross_solid_line && !_actual_driving_emergency) {
  //     _result = 1.0;
  // }

  if (_actual_parking || _actual_reverse || _actual_overtaking || _actual_driving_shoulder ||
      _actual_cross_solid_line || _actual_driving_emergency) {
    _result = 1.0;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should no parking/reverse/overtaking/driving shoulder/cross solid line/driving emergency, current ego "
            "status: "
         << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule82_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule82_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule82_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_lane_boundary);
    s_rule82_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_enter_expressway);
    s_rule82_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_exit_expressway);
    s_rule82_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_lane_acceleration);
    s_rule82_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_lane_deceleration);
    s_rule82_plot.mutable_y_axis()->at(6).add_axis_data(_cond_on_lane_shoulder);
    s_rule82_plot.mutable_y_axis()->at(7).add_axis_data(_cond_on_lane_emergency);
    s_rule82_plot.mutable_y_axis()->at(8).add_axis_data(_actual_parking);
    s_rule82_plot.mutable_y_axis()->at(9).add_axis_data(_actual_reverse);
    s_rule82_plot.mutable_y_axis()->at(10).add_axis_data(_actual_overtaking);
    s_rule82_plot.mutable_y_axis()->at(11).add_axis_data(_actual_driving_shoulder);
    s_rule82_plot.mutable_y_axis()->at(12).add_axis_data(_actual_cross_solid_line);
  }

  return true;
}

bool EvalRule82::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule82_plot);
  }

  return true;
}

void EvalRule82::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule82::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego is Violation of rule82");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS,
                        "ego is not parking, reversing or overtaking or driving on shoulder or cross solid line");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS,
                    "ego is not parking, reversing or overtaking or driving on shoulder or cross solid line");
}

bool EvalRule82::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego is Violation of rule82";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
