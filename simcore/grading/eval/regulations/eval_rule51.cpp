// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule51.h"

namespace eval {
const char EvalRule51::_kpi_name[] = "Rule51";
sim_msg::TestReport_XYPlot EvalRule51::s_rule51_plot;

EvalRule51::EvalRule51() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule51::Init(eval::EvalInit &helper) {
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
        s_rule51_plot, "rule51", "", "t", "s",
        {"_result", "_cond_night", "_cond_on_junction", "_cond_on_stop_lane", "_cond_has_traffic_light",
         "_cond_on_red_light", "_cond_turn_left", "_result_beam_proper_used", "_result_indicator_proper_used",
         "_result_run_red_light", "_result_follow_dir"},
        {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 11);

    ReportHelper::ConfigXYPlotThreshold(s_rule51_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule51::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule51");
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
  _cond_night = false;
  _cond_on_junction = false;
  _cond_on_stop_lane = false;
  _cond_has_traffic_light = false;
  _cond_on_red_light = false;
  _cond_turn_left = false;
  _result_beam_proper_used = true;
  _result_indicator_proper_used = true;
  _result_run_red_light = true;
  _result_follow_dir = true;
  _result = 0.0;

  // 1.get the condition
  // whether is night
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    auto time_of_day = environment_ptr->GetPartsOfDay();
    _cond_night = (time_of_day == eval::PARTS_OF_DAY_MIDNIGHT || time_of_day == eval::PARTS_OF_DAY_DAWN ||
                   time_of_day == eval::PARTS_OF_DAY_NIGHT);
  }

  // whether ego is on jection
  _cond_on_junction = map_info->m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether on stop lane
  StopLine stop_line;
  _cond_on_stop_lane = ego_front->IsOnStopLine(stop_line);
  VLOG_1 << "cond_on_stop_lane: " << _cond_on_stop_lane << ".\n";

  // get lane arrow
  _cond_lane_arrow = ego_lane->getLaneArrow();

  // whether has traffic light and is red light
  auto traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
  _cond_has_traffic_light = traffic_light_ptrs.size() > 0;
  VLOG_1 << "cond_has_traffic_light: " << _cond_has_traffic_light << ".\n";

  // whether signal light of ego lane is red
  LaneID ego_lane_id = ego_front->GetLaneID();
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

  // whether is turn
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  VLOG_2 << "ego behavior is " << output.m_veh_behav << ".\n";
  if (output.m_duration_valid) {
    _cond_turn_left = output.m_veh_behav == VehicleBehavior::TurnLeft;
  }

  // 4.Check Detect and report
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  // check if beam properly used
  if (_cond_night) _result_beam_proper_used = veh_fb.m_low_beam == OnOff::ON;
  VLOG_1 << "ego should use beam, current ego beam status: " << _result_beam_proper_used << " \n";

  // check if indicator properly used
  if (_cond_has_traffic_light && _cond_on_junction && _cond_turn_left) {
    _result_indicator_proper_used = veh_fb.m_turn_left_lamp == OnOff::ON;
  }
  VLOG_1 << "ego turn left state:" << _cond_turn_left
         << ", turn left lamp properly used:" << _result_indicator_proper_used << ".\n";

  // check if on stop line and has traffic light, stopped
  if (_cond_on_red_light && _cond_on_stop_lane) {
    _result_run_red_light = false;
  }
  VLOG_1 << "_result_run_red_light: " << _result_run_red_light << ".\n";

  // check if follow direction driving
  if (output.m_duration_valid) {
    VLOG_1 << "_cond_lane_arrow: " << _cond_lane_arrow << ".\n";
    if ((_cond_lane_arrow == hadmap::LANE_ARROW_Straight && output.m_veh_behav != VehicleBehavior::Driving) ||
        (_cond_lane_arrow == hadmap::LANE_ARROW_Left && output.m_veh_behav != VehicleBehavior::TurnLeft) ||
        (_cond_lane_arrow == hadmap::LANE_ARROW_Right && output.m_veh_behav != VehicleBehavior::TurnRight)) {
      _result_follow_dir = false;
    }
  }

  if (!_result_beam_proper_used || !_result_indicator_proper_used || !_result_run_red_light || !_result_follow_dir) {
    _result = 1.0;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego follow rule51 status: " << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule51_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule51_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule51_plot.mutable_y_axis()->at(1).add_axis_data(_cond_night);
    s_rule51_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_junction);
    s_rule51_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_stop_lane);
    s_rule51_plot.mutable_y_axis()->at(4).add_axis_data(_cond_has_traffic_light);
    s_rule51_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_red_light);
    s_rule51_plot.mutable_y_axis()->at(6).add_axis_data(_cond_turn_left);
    s_rule51_plot.mutable_y_axis()->at(7).add_axis_data(_result_beam_proper_used);
    s_rule51_plot.mutable_y_axis()->at(8).add_axis_data(_result_indicator_proper_used);
    s_rule51_plot.mutable_y_axis()->at(9).add_axis_data(_result_run_red_light);
    s_rule51_plot.mutable_y_axis()->at(10).add_axis_data(_result_follow_dir);
  }

  return true;
}

bool EvalRule51::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule51_plot);
  }

  return true;
}

void EvalRule51::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule51::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL,
                        "ego should beam/indicator proper used and not run red light and follow lane direction");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS,
                        "beam and indicator and not run red light and follow lane direction check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS,
                    "beam and indicator and not run red light and follow lane direction check skipped");
}

bool EvalRule51::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should beam/indicator proper used and not run red light and follow lane direction";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
