// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_dec.h"

namespace eval {
const char EvalDec::_kpi_name[] = "MaxDeceleration_V";

sim_msg::TestReport_XYPlot s_dec_plot;

EvalDec::EvalDec() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _ax = 0.0;
}
bool EvalDec::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);  // speed upper
    threshold_speed_lower = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "SPEED_LOWER");
    threshold_dec_above_upper = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "DEC_ABOVE_UPPER");
    threshold_dec_below_lower = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "DEC_BELOW_LOWER");
    threshold_dec_low2up = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "DEC_LOW2UP");
    real_dec_thresh - threshold_dec_below_lower;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_dec_plot, "deceleration", "", "t", "s", {"acc"}, {"m/s2"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_dec_plot, "thresh upper", 0, 1, INT32_MAX, "thresh lower", 1, 1,
                                        threshold_dec_below_lower);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_dec_plot, {0}, false, true);
  }

  return true;
}
bool EvalDec::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_deceleration_above_thresh(
      sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front == nullptr) {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }

    _ax = ego_front->GetAcc().GetX();
    double speed = ego_front->GetSpeed().GetNormal();
    if (speed < threshold_speed_lower) {
      real_dec_thresh = -threshold_dec_below_lower;
    } else if (speed < m_defaultThreshDouble) {
      real_dec_thresh = -threshold_dec_low2up;
    } else {
      real_dec_thresh = -threshold_dec_above_upper;
    }
    if (_detector.Detect(_ax, real_dec_thresh)) {
      grading.mutable_event_detector()->set_deceleration_above_thresh(
          sim_msg::Grading_EventDetector_EventState_EventDetected);
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      s_dec_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_dec_plot.mutable_y_axis()->at(0).add_axis_data(_ax);
      s_dec_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(real_dec_thresh);
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalDec::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_dec_plot);
  }

  return true;
}

EvalResult EvalDec::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max deceleration");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max deceleration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max deceleration check skipped");
}
bool EvalDec::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max deceleration";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalDec::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_dec_acc()->set_acceleration(_ax);
  msg.mutable_dec_acc()->set_state(sim_msg::GRADING_ACCELERATION_NORMAL);
  if (_detector.GetLatestState()) {
    msg.mutable_dec_acc()->set_state(sim_msg::GRADING_ACCELERATION_RAPIDDEACCELERATE);
  }
  msg.mutable_dec_acc()->set_threshold(real_dec_thresh);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalDec::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_overdecelleration()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_overdecelleration()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_overdecelleration()->set_is_pass(IsEvalPass()._state ==
                                                                       sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
