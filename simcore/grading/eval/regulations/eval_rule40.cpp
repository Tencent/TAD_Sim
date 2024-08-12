// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule40.h"

namespace eval {
const char EvalRule40::_kpi_name[] = "Rule40";
sim_msg::TestReport_XYPlot EvalRule40::s_rule40_plot;

EvalRule40::EvalRule40() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule40::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule40_plot, "rule40", "", "t", "s",
                               {"result", "cond_on_stop_lane", "cond_on_junction", "cond_on_red_light"},
                               {"OnOff", "OnOff", "OnOff", "OnOff"}, 4);

    ReportHelper::ConfigXYPlotThreshold(s_rule40_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule40::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule40");
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
  _result = 0.0;
  _cond_on_stop_lane = false;
  _cond_on_junction = false;
  _cond_on_red_light = false;
  hadmap::LANE_ARROW _cond_lane_arrow = hadmap::LANE_ARROW_None;

  // 1.get the condition
  // whether ego on stop lane
  StopLine stop_line;
  _cond_on_stop_lane = ego_front->IsOnStopLine(stop_line);
  VLOG_1 << "cond_on_stop_lane: " << _cond_on_stop_lane << ".\n";

  // whether ego on junction
  _cond_on_junction = !map_info->m_on_road;  // m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether signal light of ego lane is red
  auto traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
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

  // get lane arrow
  _cond_lane_arrow = ego_lane->getLaneArrow();
  VLOG_1 << "cond_lane_arrow: " << _cond_lane_arrow;

  // get vehicle behavior
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();

  // 2.Check Detect and report
  if (_cond_on_red_light && (_cond_on_stop_lane || _cond_on_junction)) {
    // check if follow direction driving
    if (_cond_lane_arrow == hadmap::LANE_ARROW_Straight && output.m_veh_behav != VehicleBehavior::Driving) {
      _result = 1.0;
    }
    if (output.m_duration_valid) {
      if ((_cond_lane_arrow == hadmap::LANE_ARROW_Left && output.m_veh_behav != VehicleBehavior::TurnLeft) ||
          (_cond_lane_arrow == hadmap::LANE_ARROW_Right && output.m_veh_behav != VehicleBehavior::TurnRight)) {
        _result = 1.0;
      }
    }
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego not follow direction status: " << _result << ".\n";

  // report add data to xy-pot
  if (isReportEnabled()) {
    s_rule40_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule40_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule40_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_stop_lane);
    s_rule40_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_junction);
    s_rule40_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_red_light);
  }

  return true;
}

bool EvalRule40::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule40_plot);
  }

  return true;
}

void EvalRule40::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule40::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should not run red light and follow lane direction");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "not run red light and follow lane direction check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "not run red light and follow lane direction check skipped");
}

bool EvalRule40::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego is run red light or not follow lane direction";

  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
