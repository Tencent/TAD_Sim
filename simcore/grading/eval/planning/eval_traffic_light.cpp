// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_traffic_light.h"

namespace eval {
const char EvalTrafficLight::_kpi_name[] = "RunTrafficLight";

sim_msg::TestReport_XYPlot s_trafficlight_plot;

EvalTrafficLight::EvalTrafficLight() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _on_stop_line = false;
  _run_traffic_light = false;
}
bool EvalTrafficLight::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_trafficlight_plot, "run traffic light", "", "t", "s", {"run traffic light"}, {"N/A"},
                               1);
    ReportHelper::ConfigXYPlotThreshold(s_trafficlight_plot, "thresh upper", 0, 1, 1);
  }
  return true;
}
bool EvalTrafficLight::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_run_trafficlight(sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    _on_stop_line = false;
    _run_traffic_light = false;
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      SignalLightActorList traffic_light_ptrs =
          _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);

      if (traffic_light_ptrs.size() > 0) {
        // get ego lane id
        const LaneID &ego_lane_id = ego_front->GetLaneID();

        // ego on stop line
        StopLine stop_line;
        if (ego_front->IsOnStopLine(stop_line)) {
          VLOG_1 << "ego on stop line:" << stop_line.object_pkid << ".\n";
          // if signal light of ego lane is red
          for (auto signal_light_ptr : traffic_light_ptrs) {
            const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
            for (auto i = 0; i < control_lanes.size(); ++i) {
              const LaneID &control_lane = control_lanes.at(i);
              // find control lane of ego lane
              if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane) &&
                  signal_light_ptr->GetSignalLightColor() == SignalLightColor::RED) {
                _run_traffic_light = true;
                break;
              }
            }
          }
        }
      }

      if (_detector.Detect(_run_traffic_light, 1)) {
        grading.mutable_event_detector()->set_run_trafficlight(sim_msg::Grading_EventDetector_EventState_EventDetected);
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        s_trafficlight_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_trafficlight_plot.mutable_y_axis()->at(0).add_axis_data(_run_traffic_light);
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalTrafficLight::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("TrafficLight", std::to_string(_detector.GetCount()));

  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_trafficlight_plot);
  }

  return true;
}

EvalResult EvalTrafficLight::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "run traffic light");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "run traffic light check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "run traffic light check skipped");
}
bool EvalTrafficLight::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "run traffic light";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalTrafficLight::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_lane()->set_isonstopline(_on_stop_line);

  msg.mutable_trafficelight()->set_run_trafficlight_count(_detector.GetCount());
  msg.mutable_trafficelight()->set_state(sim_msg::GRADING_TRAFFICLIGHT_RUNNING);
  if (_run_traffic_light) {
    msg.mutable_trafficelight()->set_state(sim_msg::GRADING_TRAFFICLIGHT_JUMPREDLIGHT);
  }
  msg.mutable_trafficelight()->set_run_trafficlight_thresh(1);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalTrafficLight::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_ranredlight()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_ranredlight()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_ranredlight()->set_is_pass(IsEvalPass()._state ==
                                                                 sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
