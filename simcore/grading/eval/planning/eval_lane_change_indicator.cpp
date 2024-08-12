// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_lane_change_indicator.h"

namespace eval {
const char EvalLaneChangeIndicator::_kpi_name[] = "EgoChangeLane_Indicator";

sim_msg::TestReport_XYPlot s_ego_lc_plot;
sim_msg::TestReport_XYPlot s_indicator_proper_use;

EvalLaneChangeIndicator::EvalLaneChangeIndicator() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _indicator_proper_used = true;
}
bool EvalLaneChangeIndicator::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_ego_lc_plot, "lane change check", "", "t", "s", {"lane change"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlot(s_indicator_proper_use, "lane change indicator usage check", "", "t", "s",
                               {"lane change indicator properly used"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_ego_lc_plot);
    ReportHelper::ConfigXYPlotThreshold(s_indicator_proper_use, "thresh upper", 0, 1, 1);
  }

  return true;
}
bool EvalLaneChangeIndicator::Step(eval::EvalStep &helper) {
  _indicator_proper_used = true;
  bool lc_left = false, lc_right = false;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // ego
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      // output of state-flow
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();

      // get vehicle state
      const VehicleBodyControl &ctrl_command = ego_front->GetVehicleBodyCmd();

      lc_left = output.m_veh_behav == VehicleBehavior::LaneChaning_Left;
      lc_right = output.m_veh_behav == VehicleBehavior::LaneChaning_Right;

      // check
      if (lc_left) _indicator_proper_used = lc_left && (ctrl_command.m_turn_left_lamp == OnOff::ON);
      if (lc_right) _indicator_proper_used = lc_right && (ctrl_command.m_turn_right_lamp == OnOff::ON);

      VLOG_1 << "ego state " << output.m_veh_behav << ", ego turn lamp properly used:" << _indicator_proper_used
             << ".\n";
      _detector.Detect(static_cast<double>(_indicator_proper_used), 0.5);
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      s_ego_lc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_ego_lc_plot.mutable_y_axis()->at(0).add_axis_data(lc_left || lc_right);
      s_indicator_proper_use.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_indicator_proper_use.mutable_y_axis()->at(0).add_axis_data(_indicator_proper_used);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalLaneChangeIndicator::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_ego_lc_plot);
    ReportHelper::AddXYPlot2Attach(*attach, s_indicator_proper_use);
  }

  return true;
}

void EvalLaneChangeIndicator::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalLaneChangeIndicator::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "lane change indicator check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "lane change indicator check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "lane change indicator check skipped");
}
bool EvalLaneChangeIndicator::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "lane change indicator check fail";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
