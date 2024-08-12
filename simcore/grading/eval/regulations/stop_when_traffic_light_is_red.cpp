// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "stop_when_traffic_light_is_red.h"

namespace eval {
const char StopWhenTrafficLightIsRed::_kpi_name[] = "StopWhenTrafficLightIsRed";
sim_msg::TestReport_XYPlot StopWhenTrafficLightIsRed::s_StopWhenTrafficLightIsRed_plot;

StopWhenTrafficLightIsRed::StopWhenTrafficLightIsRed() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool StopWhenTrafficLightIsRed::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _cond_on_junction = false;
    _cond_on_red_light = false;
    _cond_on_stop_lane = false;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    // ReportHelper::ConfigXYPlot(s_StopWhenTrafficLightIsRed_plot, "StopWhenTrafficLightIsRed", "", "t", "s",
    //                         {"run traffic light"}, {"OnOff"}, 1);
    ReportHelper::ConfigXYPlot(s_StopWhenTrafficLightIsRed_plot, "StopWhenTrafficLightIsRed", "", "t", "s",
                               {"result", "cond_on_red_light", "_cond_on_stop_lane", "_cond_on_junction"},
                               {"OnOff", "OnOff", "OnOff", "OnOff"}, 4);

    ReportHelper::ConfigXYPlotThreshold(s_StopWhenTrafficLightIsRed_plot, "StopWhenTrafficLightIsRed Violation ", 0, 1,
                                        0.5, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool StopWhenTrafficLightIsRed::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("StopWhenTrafficLightIsRed");
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
    VLOG_0 << _kpi_name << "map info misssing with ego actor";
    return false;
  }

  // 1.get the condition
  // whether ego on stop lane
  StopLine stop_line;
  _cond_on_stop_lane = ego_front->IsOnStopLine(stop_line);
  VLOG_1 << "cond_on_stop_lane: " << _cond_on_stop_lane << ".\n";

  // reset red light while has passed junction
  if (_cond_on_junction && map_info->m_on_road) _cond_on_red_light = false;

  // whether ego on junction
  _cond_on_junction = !map_info->m_on_road && map_info->m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether signal light of ego lane is red
  auto traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
  const LaneID &ego_lane_id = ego_front->GetLaneID();

  // find control lane of ego lane and if light input color
  if (traffic_light_ptrs.size() > 0) {
    for (auto signal_light_ptr : traffic_light_ptrs) {
      const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
      for (auto i = 0; i < control_lanes.size(); ++i) {
        const LaneID &control_lane = control_lanes.at(i);
        // find control lane of ego lane and if light input color
        if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane)) {
          _cond_on_red_light = (signal_light_ptr->GetSignalLightColor() == SignalLightColor::RED);
        }
      }
    }
  }
  VLOG_1 << "cond_on_red_light: " << _cond_on_red_light << ".\n";

  // 2.Check Detect and report
  // check if on stop line and has traffic light, stopped
  bool result = (_cond_on_red_light && (_cond_on_stop_lane || _cond_on_junction));
  _detector.Detect(result, 0.5);

  VLOG_1 << "StopWhenTrafficLightIsRed status: " << result << ".\n";

  // report add data to xy-pot
  if (isReportEnabled()) {
    s_StopWhenTrafficLightIsRed_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_StopWhenTrafficLightIsRed_plot.mutable_y_axis()->at(0).add_axis_data(result);
    s_StopWhenTrafficLightIsRed_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_red_light);
    s_StopWhenTrafficLightIsRed_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_stop_lane);
    s_StopWhenTrafficLightIsRed_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_junction);
  }

  return true;
}

bool StopWhenTrafficLightIsRed::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_StopWhenTrafficLightIsRed_plot);
  }

  return true;
}

void StopWhenTrafficLightIsRed::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult StopWhenTrafficLightIsRed::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should not run when trafficlight is red");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "StopWhenTrafficLightIsRed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "StopWhenTrafficLightIsRed check skipped");
}

bool StopWhenTrafficLightIsRed::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego is run when trafficlight is red";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
