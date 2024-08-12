// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_long_acc_jerk.h"

namespace eval {
const char EvalLongAccJerk::_kpi_name[] = "Jerk_Longitudinal_MaxAcc";

sim_msg::TestReport_XYPlot EvalLongAccJerk::sLongAccJerk;

EvalLongAccJerk::EvalLongAccJerk() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  mJerk = 0.0;
}
bool EvalLongAccJerk::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(sLongAccJerk, "longitudinal accel jerk", "", "t", "s", {"acc jerk"}, {"m/s3"}, 1);
    ReportHelper::ConfigXYPlotThreshold(sLongAccJerk, "thresh upper", 0, 1, m_defaultThreshDouble);
  }

  return true;
}
bool EvalLongAccJerk::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      mJerk = ego_front->GetJerk().GetX();
      VLOG_1 << "longitudinal jerk is " << mJerk << " m/s3.\n";
      _detector.Detect(mJerk, m_defaultThreshDouble);

      // add data to xy-pot
      if (isReportEnabled()) {
        sLongAccJerk.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        sLongAccJerk.mutable_y_axis()->at(0).add_axis_data(mJerk);
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalLongAccJerk::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), sLongAccJerk);
  }

  return true;
}

void EvalLongAccJerk::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalLongAccJerk::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max longitudinal jerk");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max longitudinal jerk check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max longitudinal jerk check skipped");
}
bool EvalLongAccJerk::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max longitudinal jerk";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
