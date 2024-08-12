// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "stop_outside_the_stop_line_when_stop_signal_and_stop_lane.h"

namespace eval {
const char StopOutsideTheStopLineWhenStopSignalAndStopLane::_kpi_name[] =
    "StopOutsideTheStopLineWhenStopSignalAndStopLane";
sim_msg::TestReport_XYPlot
    StopOutsideTheStopLineWhenStopSignalAndStopLane::s_stop_outside_when_stop_signal_and_stoplane_plot;

StopOutsideTheStopLineWhenStopSignalAndStopLane::StopOutsideTheStopLineWhenStopSignalAndStopLane() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool StopOutsideTheStopLineWhenStopSignalAndStopLane::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_stop_outside_when_stop_signal_and_stoplane_plot,
                               "stop_outside_when_stop_signal_and_stoplane check", "", "t", "s",
                               {"_result", "_cond_on_red_light", "_cond_on_stop_lane"}, {"OnOff", "OnOff", "OnOff"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_stop_outside_when_stop_signal_and_stoplane_plot,
                                        "StopOutsideTheStopLineWhenStopSignalAndStopLane Violation ", 0, 1, 0.5, "", 0,
                                        0, INT32_MIN, 0);
  }

  return true;
}

bool StopOutsideTheStopLineWhenStopSignalAndStopLane::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("StopOutsideTheStopLineWhenStopSignalAndStopLane");
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

  // param init
  bool _cond_on_red_light = false;
  bool _cond_on_stop_lane = false;
  bool _result = false;

  // 1.get the condition
  // whether has traffic light and is red light
  auto traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);

  // whether signal light of ego lane is red
  const LaneID &ego_lane_id = ego_front->GetLaneID();
  // find control lane of ego lane and if light input color
  if (traffic_light_ptrs.size() > 0) {
    for (auto signal_light_ptr : traffic_light_ptrs) {
      const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
      for (auto i = 0; i < control_lanes.size(); ++i) {
        const LaneID &control_lane = control_lanes.at(i);
        // find control lane of ego lane and if light input color
        if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane) &&
            signal_light_ptr->GetSignalLightColor() == SignalLightColor::RED) {
          _cond_on_red_light = true;
        }
      }
    }
  }
  VLOG_1 << "cond_on_red_light: " << _cond_on_red_light << ".\n";

  //  whether on stop lane
  StopLine stop_line;
  _cond_on_stop_lane = ego_front->IsOnStopLine(stop_line);
  VLOG_1 << "cond_on_stop_lane: " << _cond_on_stop_lane << ".\n";

  // check if on stop line and has traffic light, stopped
  if (_cond_on_red_light && _cond_on_stop_lane) {
    _result = true;
  }
  _detector.Detect(_result, 0.5);
  VLOG_1 << "_result: " << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_stop_outside_when_stop_signal_and_stoplane_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_stop_outside_when_stop_signal_and_stoplane_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_stop_outside_when_stop_signal_and_stoplane_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_red_light);
    s_stop_outside_when_stop_signal_and_stoplane_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_stop_lane);
  }

  return true;
}

bool StopOutsideTheStopLineWhenStopSignalAndStopLane::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_stop_outside_when_stop_signal_and_stoplane_plot);
  }

  return true;
}

void StopOutsideTheStopLineWhenStopSignalAndStopLane::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult StopOutsideTheStopLineWhenStopSignalAndStopLane::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should not run red light");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, " not run red light check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "not run red light check skipped");
}

bool StopOutsideTheStopLineWhenStopSignalAndStopLane::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should not run red light";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
