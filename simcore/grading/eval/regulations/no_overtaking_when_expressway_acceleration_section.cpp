// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_overtaking_when_expressway_acceleration_section.h"

namespace eval {
const char NoOvertakingWhenExpresswayAccelerationSection::_kpi_name[] = "NoOvertakingWhenExpresswayAccelerationSection";
sim_msg::TestReport_XYPlot
    NoOvertakingWhenExpresswayAccelerationSection::s_no_overtaking_when_expressway_acceleration_section_plot;

NoOvertakingWhenExpresswayAccelerationSection::NoOvertakingWhenExpresswayAccelerationSection() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoOvertakingWhenExpresswayAccelerationSection::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_overtaking_when_expressway_acceleration_section_plot,
                               "no_overtaking_when_expressway_acceleration_section", "", "t", "s",
                               {"result_overtaking", "_cond_on_expressway", "_cond_on_lane_acceleration"},
                               {"OnOff", "OnOff", "OnOff"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_no_overtaking_when_expressway_acceleration_section_plot,
                                        "overtaking Violation ", 0, 1, 0.5, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool NoOvertakingWhenExpresswayAccelerationSection::Step(eval::EvalStep &helper) {
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
    VLOG_1 << "map info missing" << "\n";
    return false;
  }

  // param init
  bool _cond_on_expressway = false;
  bool _cond_on_acc_lane = false;
  bool _result = false;

  // 1.get the condition
  // whether ego is on enter expressway
  _cond_on_expressway = map_info->m_active_road.m_on_motorway;

  // whether ego is on acceleration lane
  _cond_on_acc_lane = map_info->m_active_lane.m_on_lane_acceleration;

  // 4.Check Detect and report
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  if (output.m_duration_valid) {
    if (_cond_on_expressway && _cond_on_acc_lane) {
      _result = (output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
                 output.m_veh_behav == VehicleBehavior::LaneChaning_Right);
    }
  }

  _detector.Detect(_result, 0.5);
  VLOG_1 << "ego should no overtaking, current ego no overtaking status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_overtaking_when_expressway_acceleration_section_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_overtaking_when_expressway_acceleration_section_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_no_overtaking_when_expressway_acceleration_section_plot.mutable_y_axis()->at(1).add_axis_data(
        _cond_on_expressway);
    s_no_overtaking_when_expressway_acceleration_section_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_acc_lane);
  }

  return true;
}

bool NoOvertakingWhenExpresswayAccelerationSection::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(),
                                   s_no_overtaking_when_expressway_acceleration_section_plot);
  }

  return true;
}

void NoOvertakingWhenExpresswayAccelerationSection::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult NoOvertakingWhenExpresswayAccelerationSection::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego is overtaking");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "ego is overtaking check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "ego is overtaking check skipped");
}

bool NoOvertakingWhenExpresswayAccelerationSection::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego is overtaking when _cond_on_enter_expressway or _cond_on_lane_acceleration";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
