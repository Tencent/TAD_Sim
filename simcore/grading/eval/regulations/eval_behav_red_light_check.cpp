// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_red_light_check.h"

namespace eval {
const char EvalBehavRedLightCheck::_kpi_name[] = "BehavRedLightCheck";

sim_msg::TestReport_XYPlot EvalBehavRedLightCheck::_s_behav_red_light_plot;

EvalBehavRedLightCheck::EvalBehavRedLightCheck() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _run_traffic_light = false;
  _event_run_red = false;
  msg_res = "pass";
}
bool EvalBehavRedLightCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    min_dist = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MIN_DIST");
    max_dist = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MAX_DIST");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_behav_red_light_plot, "incorrect behavior", "", "t", "s",
                               {"if redLight", "dist_to_stopline"}, {"N/A", "m"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_red_light_plot, "detect count upper", 0, 0,
                                        m_Kpi.passcondition().value(), "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_red_light_plot, "dist upper", 0, 1, max_dist, "dist lower", 1, 1,
                                        min_dist, 1);
  }
  return true;
}
bool EvalBehavRedLightCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing" << "\n";
        return false;
      }
      bool if_in_junction = map_info->m_in_junction;
      bool if_on_road = map_info->m_on_road;
      VLOG_1 << "ego car behav : " << helper.GetVehStateFlow().GetOutput().m_veh_behav << "\n";
      VLOG_1 << "ego car if in junction : " << if_in_junction << "\n";
      VLOG_1 << "ego car if on road : " << if_on_road << "\n";
      SignalLightActorList traffic_light_ptrs =
          _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
      if (traffic_light_ptrs.size() > 0) {
        // get ego lane id
        const LaneID &ego_lane_id = ego_front->GetLaneID();
        // if signal light of ego lane is red
        for (auto signal_light_ptr : traffic_light_ptrs) {
          const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
          for (auto i = 0; i < control_lanes.size(); ++i) {
            const LaneID &control_lane = control_lanes.at(i);
            // find control lane of ego lane
            if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane)) {
              _run_traffic_light = (signal_light_ptr->GetSignalLightColor() == SignalLightColor::RED);
              if (_run_traffic_light) break;
            }
          }
        }
      }
      bool _on_stop_line = false;
      bool _un_proper_dist = false;
      bool _rush_red_light = false;
      double dist_2_stop_line = -1;

      const VehOutput &veh_out = helper.GetVehStateFlow().GetOutput();
      auto veh_ptr = helper.GetLeadingVehicle();

      VLOG_1 << "if cur traffic light is red : " << _run_traffic_light << "\n";
      if (_run_traffic_light) {
        // if on road or in junction
        bool _if_on_juction_road = map_info->m_on_road;
        // check stop line
        if (ego_front->GetSpeed().GetNormal() < 0.2) {
          // reset when stop in junction
          if (!_if_on_juction_road) _run_traffic_light = false;

          // check if stop on stop line
          StopLine closed_stop_line;
          _on_stop_line = ego_front->IsOnStopLine(closed_stop_line) || !_if_on_juction_road;

          // check the dist
          if (!_on_stop_line && !veh_ptr) {
            dist_2_stop_line = ego_front->GetStopLineRoadDistance();
            if (dist_2_stop_line != -1) _un_proper_dist = (dist_2_stop_line > max_dist || dist_2_stop_line < min_dist);
          }
        }
        VLOG_1 << "if stop stop line : " << _on_stop_line << "\n";
        VLOG_1 << "if unproper distance to stopline : " << _un_proper_dist << "\n";

        // check rush red light
        if (!_event_run_red) _event_run_red = (_run_traffic_light && !_if_on_juction_road);
        if (_event_run_red && _if_on_juction_road) {
          _rush_red_light = !(veh_out.m_veh_behav == VehicleBehavior::TurnRight ||
                              veh_out.m_veh_behav == VehicleBehavior::U_TurnRight);
          _event_run_red = false;
          _run_traffic_light = false;
        }
        VLOG_1 << "if rush red light : " << _rush_red_light << "\n";

        bool _un_proper_actions = (_on_stop_line || _un_proper_dist || _rush_red_light);
        if (_un_proper_actions) {
          msg_res = "rush_red_light : " + std::to_string(_rush_red_light) +
                    "\nstop on stopline or junction : " + std::to_string(_on_stop_line) +
                    "\nincorrect distance to stop line : " + std::to_string(_un_proper_dist);
        }
        _detector.Detect(_un_proper_actions, 0.5);
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_red_light_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_red_light_plot.mutable_y_axis()->at(0).add_axis_data(_run_traffic_light);
        _s_behav_red_light_plot.mutable_y_axis()->at(1).add_axis_data(dist_2_stop_line);
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
bool EvalBehavRedLightCheck::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("TrafficLight", std::to_string(_detector.GetCount()));

  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_behav_red_light_plot);
  }

  return true;
}

void EvalBehavRedLightCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalBehavRedLightCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, msg_res);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, msg_res);
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "run traffic light check skipped");
}
bool EvalBehavRedLightCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "incorrect behavior before red light";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
