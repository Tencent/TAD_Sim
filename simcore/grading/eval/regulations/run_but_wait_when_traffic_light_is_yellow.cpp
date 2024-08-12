// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "run_but_wait_when_traffic_light_is_yellow.h"

namespace eval {
const char RunButWaitWhenTrafficLightIsYellow::_kpi_name[] = "RunButWaitWhenTrafficLightIsYellow";
sim_msg::TestReport_XYPlot RunButWaitWhenTrafficLightIsYellow::s_RunButWaitWhenTrafficLightIsYellow_plot;

RunButWaitWhenTrafficLightIsYellow::RunButWaitWhenTrafficLightIsYellow() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool RunButWaitWhenTrafficLightIsYellow::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _count = 0;
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(
        s_RunButWaitWhenTrafficLightIsYellow_plot, "RunButWaitWhenTrafficLightIsYellow", "", "t", "s",
        {"result", "_cond_has_close_vehicle_ahead", "_cond_on_stop_lane", "_cond_on_junction", "_cond_on_yellow_light"},
        {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 5);

    ReportHelper::ConfigXYPlotThreshold(s_RunButWaitWhenTrafficLightIsYellow_plot,
                                        "RunButWaitWhenTrafficLightIsYellow Violation ", 0, 1, _threshold, "", 0, 0,
                                        INT32_MIN, 0);
  }

  return true;
}

bool RunButWaitWhenTrafficLightIsYellow::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("RunButWaitWhenTrafficLightIsYellow");
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
  _cond_has_close_vehicle_ahead = false;
  _cond_on_stop_lane = false;
  _cond_on_junction = false;
  _cond_on_yellow_light = false;
  _result = 0.0;

  double _rel_dist = 500.0;

  // 1.get the condition
  // Whether there is a leading vehicle
  auto fellow = helper.GetLeadingVehicle();

  // _cond_has_close_vehicle_ahead
  if (fellow) {
    CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
    CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
    EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
    _rel_dist = ego_2_fellow.GetNormal2D();
  } else {
    _rel_dist = 9999;
  }

  if (_rel_dist >= 0.1 && _rel_dist <= 20) {
    _cond_has_close_vehicle_ahead = true;
  }

  // whether ego on stop lane
  StopLine stop_line;
  _cond_on_stop_lane = ego_front->IsOnStopLine(stop_line);
  VLOG_1 << "cond_on_stop_lane: " << _cond_on_stop_lane << ".\n";

  // whether ego on junction
  _cond_on_junction = !map_info->m_on_road;  // m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether signal light of ego lane is yellow
  auto traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
  const LaneID &ego_lane_id = ego_front->GetLaneID();

  // find control lane of ego lane and if light input color
  if (traffic_light_ptrs.size() > 0) {
    for (auto signal_light_ptr : traffic_light_ptrs) {
      const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
      for (auto i = 0; i < control_lanes.size(); ++i) {
        const LaneID &control_lane = control_lanes.at(i);
        // find control lane of ego lane and if light input color
        if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane) &&
            signal_light_ptr->GetSignalLightColor() == SignalLightColor::YELLOW) {
          _cond_on_yellow_light = true;
        }
      }
    }
  }
  VLOG_1 << "cond_on_yellow_light: " << _cond_on_yellow_light << ".\n";

  // 2.Check Detect and report
  // param init
  static double wait_time_s = 0.0;
  // Condition when traffic light is yellow and no leading vehicle
  if (!_cond_has_close_vehicle_ahead && _cond_on_yellow_light) {
    wait_time_s = _count++ * helper.GetStepTime().GetSecond();
    // need wait _thresh_value second and run
    if (wait_time_s <= m_defaultThreshDouble) {
      _result = _cond_on_stop_lane || _cond_on_junction;
    } else {
      _result = false;  // _cond_on_stop_lane || _cond_on_junction;
    }
  }

  // FallingDetection<int> detector = MakeFallingDetection<int>(INT32_MAX);
  _detector.Detect(_result, _threshold);
  VLOG_1 << "RunButWaitWhenTrafficLightIsYellow status: " << _result << ".\n";

  // report add data to xy-pot
  if (isReportEnabled()) {
    s_RunButWaitWhenTrafficLightIsYellow_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_RunButWaitWhenTrafficLightIsYellow_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_RunButWaitWhenTrafficLightIsYellow_plot.mutable_y_axis()->at(1).add_axis_data(_cond_has_close_vehicle_ahead);
    s_RunButWaitWhenTrafficLightIsYellow_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_stop_lane);
    s_RunButWaitWhenTrafficLightIsYellow_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_junction);
    s_RunButWaitWhenTrafficLightIsYellow_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_yellow_light);
  }

  return true;
}

bool RunButWaitWhenTrafficLightIsYellow::Stop(eval::EvalStop &helper) {
  _count = 0;
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_RunButWaitWhenTrafficLightIsYellow_plot);
  }

  return true;
}

void RunButWaitWhenTrafficLightIsYellow::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult RunButWaitWhenTrafficLightIsYellow::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should wait and then run when trafficlight is yellow");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "RunButWaitWhenTrafficLightIsYellow check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "RunButWaitWhenTrafficLightIsYellow check skipped");
}

bool RunButWaitWhenTrafficLightIsYellow::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego not wait and then run when trafficlight is yellow";
  _case.mutable_info()->set_request_stop(ret);

  return (ret);
}
}  // namespace eval
