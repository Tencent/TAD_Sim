// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_dist_to_stop_line_red_light.h"

namespace eval {
const char EvalDistToStopLineRedLight::_kpi_name[] = "DistToStopLineRedLight";

sim_msg::TestReport_XYPlot EvalDistToStopLineRedLight::_s_stop_distance_red_plot;

EvalDistToStopLineRedLight::EvalDistToStopLineRedLight() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  msg = "";
}
bool EvalDistToStopLineRedLight::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    S0 = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "S0");
    S1 = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "S1");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_stop_distance_red_plot, "dist to stopline", "dist to stopline when redlight", "t",
                               "s", {"stop on stopline", "dist"}, {"N/A", "m"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_stop_distance_red_plot, "stop on line", 0, 1, 1, "", 1, 1, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_stop_distance_red_plot, "max dist", 0, 1, S1, "", 1, 1, INT32_MIN, 1);
  }

  return true;
}
bool EvalDistToStopLineRedLight::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      double dist = -1;
      bool cur_red_light = false;
      bool stop_on_stopline = false;
      auto _fellow = helper.GetLeadingVehicle();
      auto static_actors = helper.FindFrontStaticActor(ego_front, 2);
      bool _has_front_obj = (_fellow != nullptr || static_actors.size() > 0);
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
            if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane) &&
                signal_light_ptr->GetSignalLightColor() == RED) {
              cur_red_light = true;
              break;
            }
          }
        }
      }
      if (cur_red_light && ego_front->GetSpeed().GetNormal() < 0.1 && !_has_front_obj) {
        StopLine stop_line_on;
        stop_on_stopline = ego_front->IsOnStopLine(stop_line_on);
        if (stop_on_stopline) {
          dist = S1 + 1;
          msg = "bad";
        } else {
          dist = ego_front->GetStopLineRoadDistance();
          if (dist > 0) {
            if (dist <= S0)
              msg = "perfect";
            else if (dist <= S1)
              msg = "good";
            else
              msg = "bad";
          }
        }
      }
      _detector.Detect(dist, S1);
      dist = stop_on_stopline ? -1 : dist;
      VLOG_1 << "the distance to stopline is: " << dist << "\n";
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_stop_distance_red_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_stop_distance_red_plot.mutable_y_axis()->at(0).add_axis_data(stop_on_stopline);
        _s_stop_distance_red_plot.mutable_y_axis()->at(1).add_axis_data(dist);
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
bool EvalDistToStopLineRedLight::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_stop_distance_red_plot);
  }
  return true;
}

void EvalDistToStopLineRedLight::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalDistToStopLineRedLight::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, msg);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, msg);
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, msg);
}

bool EvalDistToStopLineRedLight::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = msg;
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
