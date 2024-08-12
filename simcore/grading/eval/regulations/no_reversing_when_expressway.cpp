// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_reversing_when_expressway.h"

namespace eval {
const char NoReversingWhenExpressway::_kpi_name[] = "NoReversingWhenExpressway";
sim_msg::TestReport_XYPlot NoReversingWhenExpressway::s_no_reversing_when_expressway_plot;

NoReversingWhenExpressway::NoReversingWhenExpressway() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoReversingWhenExpressway::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_reversing_when_expressway_plot, "no_reversing_when_expressway", "", "t", "s",
                               {"result", "_cond_on_expressway"}, {"OnOff", "OnOff"}, 2);

    ReportHelper::ConfigXYPlotThreshold(s_no_reversing_when_expressway_plot, "Rule Violation ", 0, 1, 0.5, "", 0, 0,
                                        INT32_MIN, 0);
  }

  return true;
}

bool NoReversingWhenExpressway::Step(eval::EvalStep &helper) {
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
    VLOG_1 << "map info missing" << "\n";
    return false;
  }
  // param init
  bool _on_expressway = false;
  bool _result = false;

  // 1.get the condition
  // whether ego is on enter expressway
  _on_expressway = map_info->m_active_road.m_on_motorway;

  // 4.Check Detect and report
  _result = _on_expressway && ego_front->IsReverse() && ego_front->GetSpeed().GetNormal() > 0.56;
  // const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  // if (output.m_duration_valid) {
  //     _result = _cond_on_enter_expressway && (output.m_veh_behav == VehicleBehavior::Reversing);
  // }

  _detector.Detect(_result, 0.5);
  VLOG_1 << "ego should no reverse, current ego bo reverse status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_reversing_when_expressway_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_reversing_when_expressway_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_no_reversing_when_expressway_plot.mutable_y_axis()->at(1).add_axis_data(_on_expressway);
  }

  return true;
}

bool NoReversingWhenExpressway::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_no_reversing_when_expressway_plot);
  }

  return true;
}

void NoReversingWhenExpressway::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult NoReversingWhenExpressway::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego is reversing");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "ego is not reversing on expressway");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "ego is reversing on expressway");
}

bool NoReversingWhenExpressway::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego is reversing";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
