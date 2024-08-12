// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rush_yellow_light.h"

namespace eval {
const char EvalRushYellowLight::_kpi_name[] = "RushYellowLight";

sim_msg::TestReport_XYPlot EvalRushYellowLight::_s_rush_yellow_light_plot;

EvalRushYellowLight::EvalRushYellowLight() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _on_stop_line = false;
  _run_traffic_light = false;
}
bool EvalRushYellowLight::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_rush_yellow_light_plot, "run traffic light", "", "t", "s",
                               {"yellow traffic light", "run traffic light"}, {"N/A", "N/A"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_rush_yellow_light_plot, "whether rush", 0, 1, 1, "", 1, 1, INT32_MIN, 1);
  }

  return true;
}
bool EvalRushYellowLight::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    _on_stop_line = false;
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      _run_traffic_light = false;
      SignalLightActorList traffic_light_ptrs =
          _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
      if (traffic_light_ptrs.size() > 0) {
        // get ego lane id
        const LaneID &ego_lane_id = ego_front->GetLaneID();
        // if signal light of ego lane is yellow
        for (auto signal_light_ptr : traffic_light_ptrs) {
          const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
          for (auto i = 0; i < control_lanes.size(); ++i) {
            const LaneID &control_lane = control_lanes.at(i);
            // find control lane of ego lane
            if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane)) {
              _run_traffic_light = (signal_light_ptr->GetSignalLightColor() == SignalLightColor::YELLOW);
              break;
            }
          }
        }
      }
      StopLine stop_line;
      bool _stop_on_line = ego_front->IsOnStopLine(stop_line);
      bool _above_speed = (ego_front->GetSpeed().GetNormal() > 2.0);
      bool _whether_rush_yellow_light = _stop_on_line && _above_speed && _run_traffic_light;
      _detector.Detect(_whether_rush_yellow_light, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_rush_yellow_light_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_rush_yellow_light_plot.mutable_y_axis()->at(0).add_axis_data(_run_traffic_light);
        _s_rush_yellow_light_plot.mutable_y_axis()->at(1).add_axis_data(_whether_rush_yellow_light);
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
bool EvalRushYellowLight::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_rush_yellow_light_plot);
  }
  return true;
}

void EvalRushYellowLight::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRushYellowLight::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "run traffic light");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "run traffic light check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "run traffic light check skipped");
}

bool EvalRushYellowLight::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "run traffic light";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
