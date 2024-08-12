// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_start_up_indicator.h"

namespace eval {
const char EvalStartUpIndicator::_kpi_name[] = "VehicleTurnLight_StartUp";

sim_msg::TestReport_XYPlot EvalStartUpIndicator::g_ego_su_tl_plot;
sim_msg::TestReport_XYPlot EvalStartUpIndicator::g_ego_su_tl_lamp;

EvalStartUpIndicator::EvalStartUpIndicator() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _indicator_proper_used = true;
}
bool EvalStartUpIndicator::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _lateral_thresh = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(g_ego_su_tl_plot, "startup check", "", "t", "s", {"startup state"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlot(g_ego_su_tl_lamp, "turn left lamp", "", "t", "s", {"turn left lamp"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(g_ego_su_tl_plot, "thresh upper", 0, 1, 1);
  }

  return true;
}
bool EvalStartUpIndicator::Step(eval::EvalStep &helper) {
  bool startup_state = false;
  OnOff left_lamp = OnOff::OFF;
  _indicator_proper_used = true;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // ego
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      // ego behavior
      const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
      left_lamp = veh_fb.m_turn_left_lamp;

      // output of state-flow
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();

      // check
      if (output.m_veh_behav == VehicleBehavior::StartUp) {
        // get the map information pointer and check whether the pointer is null
        auto map_info = ego_front->GetMapInfo();
        if (map_info && map_info->m_on_road) {
          double ref_lateral_offset = ego_front->GetRefLineLateralOffset();
          startup_state = ref_lateral_offset <= _lateral_thresh;
          if (startup_state) _indicator_proper_used = (veh_fb.m_turn_left_lamp == OnOff::ON);
        }

        _detector.Detect(static_cast<double>(_indicator_proper_used), 0.5);
      }
      VLOG_1 << "ego startup state:" << startup_state << ", turn lamp properly used:" << _indicator_proper_used
             << ".\n";
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      g_ego_su_tl_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      g_ego_su_tl_plot.mutable_y_axis()->at(0).add_axis_data(startup_state);

      g_ego_su_tl_lamp.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      g_ego_su_tl_lamp.mutable_y_axis()->at(0).add_axis_data(left_lamp);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalStartUpIndicator::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, g_ego_su_tl_plot);
    ReportHelper::AddXYPlot2Attach(*attach, g_ego_su_tl_lamp);
  }

  return true;
}

void EvalStartUpIndicator::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalStartUpIndicator::IsEvalPass() {
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
bool EvalStartUpIndicator::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "turn indicator check fail";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
