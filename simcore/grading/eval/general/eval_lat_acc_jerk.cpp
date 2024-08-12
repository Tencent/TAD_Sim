// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_lat_acc_jerk.h"

namespace eval {
const char EvalLatAccJerk::_kpi_name[] = "Jerk_Lateral_MaxAcc";

sim_msg::TestReport_XYPlot EvalLatAccJerk::sLatAccJerk;

EvalLatAccJerk::EvalLatAccJerk() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  mJerk = 0.0;
}
bool EvalLatAccJerk::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(sLatAccJerk, "lateral accel jerk", "", "t", "s", {"acc jerk"}, {"m/s3"}, 1);
    ReportHelper::ConfigXYPlotThreshold(sLatAccJerk, "thresh upper", 0, 1, m_defaultThreshDouble);
  }

  return true;
}
bool EvalLatAccJerk::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      mJerk = ego_front->GetJerk().GetY();
      VLOG_1 << "lateral jerk is " << mJerk << " m/s3.\n";
      _detector.Detect(mJerk, m_defaultThreshDouble);

      // add data to xy-pot
      if (isReportEnabled()) {
        sLatAccJerk.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        sLatAccJerk.mutable_y_axis()->at(0).add_axis_data(mJerk);
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
bool EvalLatAccJerk::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), sLatAccJerk);
  }

  return true;
}

void EvalLatAccJerk::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalLatAccJerk::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max lateral jerk");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral jerk check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral jerk check skipped");
}
bool EvalLatAccJerk::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max lateral jerk";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
