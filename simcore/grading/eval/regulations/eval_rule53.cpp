// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule53.h"

namespace eval {
const char EvalRule53::_kpi_name[] = "Rule53";
sim_msg::TestReport_XYPlot EvalRule53::s_rule53_plot;

EvalRule53::EvalRule53() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule53::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule53_plot, "rule53", "", "t", "s",
                               {"_result", "_cond_on_junction", "_cond_on_crosswalk", "_cond_on_control_access"},
                               {"OnOff", "OnOff", "OnOff", "OnOff"}, 4);

    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule53_plot, {0}, false, true);
  }

  return true;
}

bool EvalRule53::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule53");
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

  hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
  if (!ego_road) {
    VLOG_0 << _kpi_name << " ego actor missing with road.\n";
    return false;
  }

  // param init
  _cond_on_junction = false;
  _cond_on_crosswalk = false;
  _cond_on_control_access = false;
  _result = 0.0;

  // 1.get the condition
  // whether ego is on junction
  _cond_on_junction = !map_info->m_on_road;  // m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // get on the crosswork
  eval::Crosswalk crosswalk_on;
  _cond_on_crosswalk = ego_front->IsOnCrosswalk(crosswalk_on);
  VLOG_1 << "cond_on_crosswalk: " << _cond_on_crosswalk << "\n";

  // whether on control access
  _cond_on_control_access = map_info->m_active_road.m_on_control_access;
  VLOG_1 << "cond_on_control_access: " << _cond_on_control_access << "\n";

  // 2.Check Detect and report
  if (_cond_on_junction || _cond_on_crosswalk || _cond_on_control_access) {
    // check if the car is reversing
    const VehOutput &output = helper.GetVehStateFlow().GetOutput();
    // if (output.m_duration_valid) {
    _result = ego_front->GetSpeed().GetNormal() <= 0.56;  // (output.m_veh_behav == VehicleBehavior::Stopped);
                                                          // }
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should no parking, current ego weather parking: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule53_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule53_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule53_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_junction);
    s_rule53_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_crosswalk);
    s_rule53_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_control_access);
  }

  return true;
}

bool EvalRule53::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule53_plot);
  }

  return true;
}

void EvalRule53::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule53::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should no parking");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no parking check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "no parking check skipped");
}

bool EvalRule53::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no parking";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
