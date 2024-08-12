// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule57.h"

namespace eval {
const char EvalRule57::_kpi_name[] = "Rule57";
sim_msg::TestReport_XYPlot EvalRule57::s_rule57_plot;

EvalRule57::EvalRule57() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule57::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule57_plot, "turn check", "", "t", "s",
                               {"no_open_lamp", "_cond_turn_left", "_cond_turn_right", "_cond_lane_change_left",
                                "_cond_lane_change_right", "_cond_uturn_left", "_cond_start"},
                               {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 7);
    ReportHelper::ConfigXYPlotThreshold(s_rule57_plot, "threshold", 0, 1, 1, "", 1, 1, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule57::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule57");
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

  // param init
  _cond_turn_left = false;
  _cond_turn_right = false;
  _cond_lane_change_left = false;
  _cond_lane_change_right = false;
  _cond_uturn_left = false;
  _cond_start = false;
  _result = 0.0;

  // 1.get the condition
  // get lane change and turn
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  // if (output.m_duration_valid) {
  _cond_turn_left = (output.m_veh_behav == VehicleBehavior::TurnLeft);
  // if (_cond_turn_left) {
  //   // now we go back and set proper turn state
  //   auto &x_axis = s_rule57_plot.x_axis().axis_data();
  //   auto y_axis = s_rule57_plot.mutable_y_axis()->at(1).mutable_axis_data();
  //   double end_t = output.m_start_t + output.m_duration;
  //   for (auto i = 0; i < x_axis.size(); ++i) {
  //     if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
  //       y_axis->at(i) = _cond_turn_left;
  //     }
  //   }
  // }
  _cond_turn_right = (output.m_veh_behav == VehicleBehavior::TurnRight);
  // if (_cond_turn_right) {
  //   // now we go back and set proper turn state
  //   auto &x_axis = s_rule57_plot.x_axis().axis_data();
  //   auto y_axis = s_rule57_plot.mutable_y_axis()->at(2).mutable_axis_data();
  //   double end_t = output.m_start_t + output.m_duration;
  //   for (auto i = 0; i < x_axis.size(); ++i) {
  //     if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
  //       y_axis->at(i) = _cond_turn_right;
  //     }
  //   }
  // }
  _cond_lane_change_left = output.m_veh_behav == VehicleBehavior::LaneChaning_Left;
  // if (_cond_lane_change_left) {
  //   // now we go back and set proper turn state
  //   auto &x_axis = s_rule57_plot.x_axis().axis_data();
  //   auto y_axis = s_rule57_plot.mutable_y_axis()->at(3).mutable_axis_data();
  //   double end_t = output.m_start_t + output.m_duration;
  //   for (auto i = 0; i < x_axis.size(); ++i) {
  //     if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
  //       y_axis->at(i) = _cond_lane_change_left;
  //     }
  //   }
  // }
  _cond_lane_change_right = output.m_veh_behav == VehicleBehavior::LaneChaning_Right;
  // if (_cond_lane_change_right) {
  //   // now we go back and set proper turn state
  //   auto &x_axis = s_rule57_plot.x_axis().axis_data();
  //   auto y_axis = s_rule57_plot.mutable_y_axis()->at(4).mutable_axis_data();
  //   double end_t = output.m_start_t + output.m_duration;
  //   for (auto i = 0; i < x_axis.size(); ++i) {
  //     if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
  //       y_axis->at(i) = _cond_lane_change_right;
  //     }
  //   }
  // }
  _cond_uturn_left = output.m_veh_behav == VehicleBehavior::U_TurnLeft;
  // if (_cond_uturn_left) {
  //   // now we go back and set proper turn state
  //   auto &x_axis = s_rule57_plot.x_axis().axis_data();
  //   auto y_axis = s_rule57_plot.mutable_y_axis()->at(5).mutable_axis_data();
  //   double end_t = output.m_start_t + output.m_duration;
  //   for (auto i = 0; i < x_axis.size(); ++i) {
  //     if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
  //       y_axis->at(i) = _cond_uturn_left;
  //     }
  //   }
  // }
  // }
  _cond_start = (output.m_veh_behav == VehicleBehavior::StartUp) && ego_front->CurLaneIsRightmostLane();
  // if (_cond_start) {
  //   // now we go back and set proper turn state
  //   auto &x_axis = s_rule57_plot.x_axis().axis_data();
  //   auto y_axis = s_rule57_plot.mutable_y_axis()->at(6).mutable_axis_data();
  //   double end_t = output.m_start_t + output.m_duration;
  //   for (auto i = 0; i < x_axis.size(); ++i) {
  //     if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
  //       y_axis->at(i) = _cond_start;
  //     }
  //   }
  // }
  // }
  VLOG_1 << "m_start_t: " << output.m_start_t << "\n";

  // 2.check
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  if (_cond_turn_left || _cond_lane_change_left || _cond_uturn_left || _cond_start) {
    _result = veh_fb.m_turn_left_lamp != OnOff::ON;
  } else if (_cond_turn_right || _cond_lane_change_right) {
    _result = veh_fb.m_turn_right_lamp != OnOff::ON;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego turn left state:" << _cond_turn_left << ", turn right state:" << _cond_turn_right
         << ", lane change to left:" << _cond_lane_change_left << ", lane change to right:" << _cond_lane_change_right
         << ", uturn left state:" << _cond_uturn_left << ", start state:" << _cond_start
         << ", turn lamp properly used:" << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule57_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule57_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule57_plot.mutable_y_axis()->at(1).add_axis_data(_cond_turn_left);
    s_rule57_plot.mutable_y_axis()->at(2).add_axis_data(_cond_turn_right);
    s_rule57_plot.mutable_y_axis()->at(3).add_axis_data(_cond_lane_change_left);
    s_rule57_plot.mutable_y_axis()->at(4).add_axis_data(_cond_lane_change_right);
    s_rule57_plot.mutable_y_axis()->at(5).add_axis_data(_cond_uturn_left);
    s_rule57_plot.mutable_y_axis()->at(6).add_axis_data(_cond_start);
  }

  return true;
}

bool EvalRule57::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule57_plot);
  }

  return true;
}

void EvalRule57::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule57::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "turn indicator check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "turn indicator check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "turn indicator check skipped");
}

bool EvalRule57::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "turn indicator check fail";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
