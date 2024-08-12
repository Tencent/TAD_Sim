// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "run_when_traffic_light_is_green.h"

namespace eval {
const char RunWhenTrafficLightIsGreen::_kpi_name[] = "RunWhenTrafficLightIsGreen";
sim_msg::TestReport_XYPlot RunWhenTrafficLightIsGreen::s_RunWhenTrafficLightIsGreen_plot;

RunWhenTrafficLightIsGreen::RunWhenTrafficLightIsGreen() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _start_duration = 0.0;
  _wait_red_light = true;
}

bool RunWhenTrafficLightIsGreen::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_RunWhenTrafficLightIsGreen_plot, "incorrect behavior", "", "t", "s",
                               {"green light and no car", "speed", "start duration"}, {"N/A", "m/s", "s"}, 3);
    ReportHelper::ConfigXYPlotThreshold(s_RunWhenTrafficLightIsGreen_plot, "", 0, 0, INT32_MAX,
                                        "green light and no car", 1, 0, 1, 0);
    ReportHelper::ConfigXYPlotThreshold(s_RunWhenTrafficLightIsGreen_plot, "", 0, 1, INT32_MAX, "speed lower", 1, 1,
                                        0.1, 1);
    ReportHelper::ConfigXYPlotThreshold(s_RunWhenTrafficLightIsGreen_plot, "duration upper", 0, 1,
                                        m_defaultThreshDouble, "", 1, 0, INT32_MIN, 2);
  }

  return true;
}

bool RunWhenTrafficLightIsGreen::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      bool if_can_run = true;
      SignalLightColor _light_color = SignalLightColor::YELLOW;
      SignalLightActorList traffic_light_ptrs =
          _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
      if (traffic_light_ptrs.size() > 0) {
        // get ego lane id
        const LaneID &ego_lane_id = ego_front->GetLaneID();
        // if signal light of ego lane is green
        for (auto signal_light_ptr : traffic_light_ptrs) {
          const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
          for (auto i = 0; i < control_lanes.size(); ++i) {
            const LaneID &control_lane = control_lanes.at(i);
            // find control lane of ego lane
            if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane)) {
              _light_color = signal_light_ptr->GetSignalLightColor();
              break;
            }
          }
        }
      }
      auto _fellow_car = helper.GetLeadingVehicle();
      auto static_actors = helper.FindFrontStaticActor(ego_front, 10);
      if (_fellow_car) {
        EVector3d back2d =
            CEvalMath::Sub(ego_front->TransMiddleFront2BaseCoord(), _fellow_car->TransMiddleRear2BaseCoord());
        // too close distance
        if_can_run = (back2d.GetNormal2D() > 10.0);
      }
      if (if_can_run) if_can_run = (static_actors.size() == 0);
      double _ego_speed = ego_front->GetSpeed().GetNormal();
      bool _red_light = (_light_color == SignalLightColor::RED);
      bool _green_light = (_light_color == SignalLightColor::GREEN);
      if (_red_light) _wait_red_light = (_ego_speed < 0.1 && if_can_run);
      if (_wait_red_light) {
        if (_green_light) _start_duration += getModuleStepTime();
        if (_ego_speed > 0.1) {
          _wait_red_light = false;
          _start_duration = 0;
        }
      }
      if (!_wait_red_light && _green_light && if_can_run) _transit_detector.Detect(_ego_speed, 0.1);
      _duration_detector.Detect(_start_duration, m_defaultThreshDouble);
      // add data to xy-pot
      if (isReportEnabled()) {
        s_RunWhenTrafficLightIsGreen_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_RunWhenTrafficLightIsGreen_plot.mutable_y_axis()->at(0).add_axis_data(_green_light && if_can_run);
        s_RunWhenTrafficLightIsGreen_plot.mutable_y_axis()->at(1).add_axis_data(_ego_speed);
        s_RunWhenTrafficLightIsGreen_plot.mutable_y_axis()->at(2).add_axis_data(_start_duration);
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

bool RunWhenTrafficLightIsGreen::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_RunWhenTrafficLightIsGreen_plot);
  }

  return true;
}

void RunWhenTrafficLightIsGreen::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _duration_detector, _kpi_name);
}

EvalResult RunWhenTrafficLightIsGreen::IsEvalPass() {
  if (m_KpiEnabled) {
    auto _all_count = _duration_detector.GetCount() + _transit_detector.GetCount();
    _case.mutable_info()->set_detected_count(_all_count);

    if (_all_count >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "behave before green light check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "RunWhenTrafficLightIsGreen check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "RunWhenTrafficLightIsGreen check skipped");
}

bool RunWhenTrafficLightIsGreen::ShouldStopScenario(std::string &reason) {
  auto _all_count = _duration_detector.GetCount() + _transit_detector.GetCount();
  auto ret = _all_count >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "behave before green light check fail";
  _case.mutable_info()->set_request_stop(ret);

  return (ret);
}
}  // namespace eval
