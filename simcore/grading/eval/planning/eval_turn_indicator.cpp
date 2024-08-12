// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_turn_indicator.h"

namespace eval {
const char EvalTurnIndicator::_kpi_name[] = "EgoTurn_Indicator";
sim_msg::TestReport_XYPlot EvalTurnIndicator::s_ego_turn;

EvalTurnIndicator::EvalTurnIndicator() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _indicator_proper_used = true;
}
bool EvalTurnIndicator::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_ego_turn, "turn check", "", "t", "s",
                               {"left turn", "left turn lamp", "right turn", "right turn lamp"},
                               {"OnOff", "OnOff", "OnOff", "OnOff"}, 4);
    ReportHelper::ConfigXYPlotThreshold(s_ego_turn, "thresh upper", 0, 1, 1, "", 1, 1, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(s_ego_turn, "", 0, 1, INT32_MAX, "", 1, 1, INT32_MIN, 1);
    ReportHelper::ConfigXYPlotThreshold(s_ego_turn, "thresh upper", 0, 1, 1, "", 1, 1, INT32_MIN, 2);
    ReportHelper::ConfigXYPlotThreshold(s_ego_turn, "", 0, 1, INT32_MAX, "", 1, 1, INT32_MIN, 3);
  }
  return true;
}
bool EvalTurnIndicator::Step(eval::EvalStep &helper) {
  bool turn_left = false, turn_right = false;
  OnOff left_lamp = OnOff::OFF, right_lamp = OnOff::OFF;
  _indicator_proper_used = true;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // ego
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      // ego behavior
      const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
      left_lamp = veh_fb.m_turn_left_lamp;
      right_lamp = veh_fb.m_turn_right_lamp;

      // output of state-flow
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();

      // check
      if (output.m_duration_valid) {
        turn_left =
            (output.m_veh_behav == VehicleBehavior::TurnLeft) || (output.m_veh_behav == VehicleBehavior::U_TurnLeft);
        turn_right =
            (output.m_veh_behav == VehicleBehavior::TurnRight) || (output.m_veh_behav == VehicleBehavior::U_TurnRight);

        if (turn_left) {
          // now we go back and set proper turn state
          auto &x_axis = s_ego_turn.x_axis().axis_data();
          auto y_axis = s_ego_turn.mutable_y_axis()->at(0).mutable_axis_data();
          double end_t = output.m_start_t + output.m_duration;
          for (auto i = 0; i < x_axis.size(); ++i) {
            if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
              y_axis->at(i) = turn_left;
            }
          }

          // check if turn lamp properly used
          _indicator_proper_used = (veh_fb.m_turn_left_lamp == OnOff::ON);
        } else if (turn_right) {
          // now we go back and set proper turn state
          auto &x_axis = s_ego_turn.x_axis().axis_data();
          auto y_axis = s_ego_turn.mutable_y_axis()->at(2).mutable_axis_data();
          double end_t = output.m_start_t + output.m_duration;
          for (auto i = 0; i < x_axis.size(); ++i) {
            if (output.m_start_t <= x_axis.at(i) && x_axis.at(i) <= end_t) {
              y_axis->at(i) = turn_right;
            }
          }

          // check if turn lamp properly used
          _indicator_proper_used = (veh_fb.m_turn_right_lamp == OnOff::ON);
        }

        _detector.Detect(static_cast<double>(_indicator_proper_used), 0.5);
      }
      VLOG_1 << "ego turn left state:" << turn_left << ", turn right state:" << turn_right
             << ", turn lamp properly used:" << _indicator_proper_used << ".\n";
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      s_ego_turn.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_ego_turn.mutable_y_axis()->at(0).add_axis_data(turn_left);
      s_ego_turn.mutable_y_axis()->at(1).add_axis_data(left_lamp);
      s_ego_turn.mutable_y_axis()->at(2).add_axis_data(turn_right);
      s_ego_turn.mutable_y_axis()->at(3).add_axis_data(right_lamp);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalTurnIndicator::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_ego_turn);
  }

  return true;
}

void EvalTurnIndicator::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalTurnIndicator::IsEvalPass() {
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
bool EvalTurnIndicator::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "turn indicator check fail";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
