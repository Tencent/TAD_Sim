// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule38.h"

namespace eval {
const char EvalRule38::_kpi_name[] = "Rule38";
sim_msg::TestReport_XYPlot EvalRule38::s_rule38_plot;

EvalRule38::EvalRule38() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule38::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _cond_on_red_light = false;
    _cond_on_right_lane = false;
    _event_rush_red_light = false;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule38_plot, "rule38", "", "t", "s",
                               {"result_run_red_light_without_turn_right", "cond_on_red_light"}, {"OnOff", "OnOff"}, 2);

    ReportHelper::ConfigXYPlotThreshold(s_rule38_plot, "Rule Violation ", 0, 1, 0.5, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule38::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule38");
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
  auto _map_ptr = ego_front->GetMapInfo();
  if (_map_ptr.get() == nullptr) {
    VLOG_1 << "map info missing\n";
    return false;
  }
  // enter junction and exit junction status
  bool _enter_junction = (_map_ptr->m_in_junction && !_map_ptr->m_on_road);
  bool _exit_junction = (_map_ptr->m_in_junction && _map_ptr->m_on_road);
  bool _result = false;
  VLOG_1 << "if event of entering junction happens when red light : " << _event_rush_red_light << "\n";

  // begin to check when red light and entering a junction
  if (!_event_rush_red_light) _event_rush_red_light = (_enter_junction && _cond_on_red_light);

  // check whether not right turn on the most right lane
  if (_event_rush_red_light && _exit_junction) {
    _cond_on_red_light = false;
    _event_rush_red_light = false;
    const VehOutput &output = helper.GetVehStateFlow().GetOutput();
    bool _turn_right = (_cond_on_right_lane && output.m_veh_behav == VehicleBehavior::TurnRight);
    _result = !_turn_right;
  }
  VLOG_1 << "if red light : " << _cond_on_red_light << "\n";
  VLOG_1 << "if enter the junction : " << _enter_junction << "\n";
  VLOG_1 << "if exit the junction : " << _exit_junction << "\n";
  VLOG_1 << "if on the most right lane in junction: " << _cond_on_right_lane << "\n";
  VLOG_1 << "result_run_red_light_without_turn_right status: " << _result << ".\n";

  // judge if the most right mortor lane
  if (_exit_junction) {
    hadmap::txLanePtr right_lane_ptr = _map_ptr->m_active_lane.m_right_lane;
    _cond_on_right_lane = _map_ptr->m_active_lane.m_lane->getLaneType() == hadmap::LANE_TYPE_Driving;
    _cond_on_right_lane = right_lane_ptr
                              ? _cond_on_red_light && (right_lane_ptr->getLaneType() != hadmap::LANE_TYPE_Driving)
                              : _cond_on_right_lane;
  }

  _detector.Detect(_result, 0.5);

  // report add data to xy-pot
  if (isReportEnabled()) {
    s_rule38_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule38_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule38_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_red_light);
  }
  return true;
}

bool EvalRule38::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule38_plot);
  }

  return true;
}

void EvalRule38::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule38::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "run red traffic light without turn right");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "run red traffic light without turn right check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "run red traffic light without turn right check skipped");
}

bool EvalRule38::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "run red traffic light without turn right";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
