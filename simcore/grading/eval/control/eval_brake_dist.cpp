// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_brake_dist.h"

namespace eval {
const char EvalBrakeDist::_kpi_name[] = "BrakeDistance";

sim_msg::TestReport_XYPlot s_brake_dist;

EvalBrakeDist::EvalBrakeDist() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalBrakeDist::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_brake_dist, "acceleration", "", "t", "s", {"acc"}, {"m/s2"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_brake_dist, "thresh upper", 0, 1, m_defaultThreshDouble);
  }

  return true;
}
bool EvalBrakeDist::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // add data to xy-pot
      if (isReportEnabled()) {
        s_brake_dist.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_brake_dist.mutable_y_axis()->at(0).add_axis_data(0.0);
      }
    } else {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalBrakeDist::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_brake_dist);

  return true;
}

void EvalBrakeDist::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalBrakeDist::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max brake distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, " max brake distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, " max brake distance check skipped");
}
bool EvalBrakeDist::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max acceleration";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
