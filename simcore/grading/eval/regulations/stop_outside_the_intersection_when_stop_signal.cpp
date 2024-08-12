// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "stop_outside_the_intersection_when_stop_signal.h"

namespace eval {
const char StopOutsideTheIntersectionWhenStopSignal::_kpi_name[] = "StopOutsideTheIntersectionWhenStopSignal";
sim_msg::TestReport_XYPlot
    StopOutsideTheIntersectionWhenStopSignal::s_stop_outside_the_intersection_when_stop_signal_plot;

StopOutsideTheIntersectionWhenStopSignal::StopOutsideTheIntersectionWhenStopSignal() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _cond_on_red_light = false;
  _has_stop_line = true;
}

bool StopOutsideTheIntersectionWhenStopSignal::Init(eval::EvalInit &helper) {
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
        s_stop_outside_the_intersection_when_stop_signal_plot, "StopOutsideTheIntersectionWhenStopSignal", "", "t", "s",
        {"_result", "_cond_on_junction", "_cond_on_stop_line", "_cond_has_traffic_light", "_cond_on_red_light"},
        {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 5);

    ReportHelper::ConfigXYPlotThreshold(s_stop_outside_the_intersection_when_stop_signal_plot,
                                        "StopOutsideTheIntersectionWhenStopSignal Violation ", 0, 1, _threshold, "", 0,
                                        0, INT32_MIN, 0);
  }

  return true;
}

bool StopOutsideTheIntersectionWhenStopSignal::Step(eval::EvalStep &helper) {
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
  _cond_on_junction = false;
  _cond_on_stop_line = false;
  _cond_has_traffic_light = false;
  _result = 0.0;

  // 1.get the condition
  // todo: 停在路口以外是指？
  // whether ego is on jection
  _cond_on_junction = map_info->m_in_junction && !map_info->m_on_road;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether has traffic light and is red light
  auto traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
  _cond_has_traffic_light = traffic_light_ptrs.size() > 0;
  VLOG_1 << "cond_has_traffic_light: " << _cond_has_traffic_light << ".\n";

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

  // check if on stop line and has traffic light, stopped
  if (_cond_on_red_light && _cond_on_junction) {
    _result = true;
  }
  _detector.Detect(_result, _threshold);
  VLOG_1 << "_result: " << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_stop_outside_the_intersection_when_stop_signal_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_stop_outside_the_intersection_when_stop_signal_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_stop_outside_the_intersection_when_stop_signal_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_junction);
    s_stop_outside_the_intersection_when_stop_signal_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_stop_line);
    s_stop_outside_the_intersection_when_stop_signal_plot.mutable_y_axis()->at(3).add_axis_data(
        _cond_has_traffic_light);
    s_stop_outside_the_intersection_when_stop_signal_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_red_light);
  }

  return true;
}

bool StopOutsideTheIntersectionWhenStopSignal::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_stop_outside_the_intersection_when_stop_signal_plot);
  }

  return true;
}

void StopOutsideTheIntersectionWhenStopSignal::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult StopOutsideTheIntersectionWhenStopSignal::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should not run red light and follow lane direction");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, " not run red light check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "not run red light check skipped");
}

bool StopOutsideTheIntersectionWhenStopSignal::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should not run red light";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
