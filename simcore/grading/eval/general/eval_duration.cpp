// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_duration.h"

namespace eval {
const char EvalDuration::_kpi_name[] = "TotalTime";

sim_msg::TestReport_PairData s_timeout_pair;

EvalDuration::EvalDuration() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _total_t_s = 0.0;
}
bool EvalDuration::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_timeout_pair);
  }

  return true;
}
bool EvalDuration::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    _total_t_s = helper.GetSimTime();
    VLOG_1 << "current simulation time " << _total_t_s << " s.\n";
    _detector.Detect(_total_t_s, m_defaultThreshDouble);
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalDuration::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("TimeOut", std::to_string(_detector.GetCount()));

  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_timeout_pair, "is timeout", std::to_string(_detector.GetCount()));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_timeout_pair);
  }

  return true;
}

EvalResult EvalDuration::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "timout");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "timout check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "timout check skipped");
}
bool EvalDuration::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "timout";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalDuration::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_timestamp()->set_timestamp(static_cast<uint64_t>(_total_t_s * 1000));
}
void EvalDuration::SetLegacyReport(sim_msg::Grading_Statistics &msg) { msg.set_is_timeout(_detector.GetCount() >= 1); }
}  // namespace eval
