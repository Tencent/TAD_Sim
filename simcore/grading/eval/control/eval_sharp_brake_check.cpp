// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_sharp_brake_check.h"

namespace eval {
const char EvalSharpBrakeCheck::_kpi_name[] = "SharpBrakeCheck";

sim_msg::TestReport_XYPlot _s_sharpbrake_plot;

EvalSharpBrakeCheck::EvalSharpBrakeCheck() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalSharpBrakeCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_sharpbrake_plot, "sharp brakes", "", "t", "s", {"urgent_brake", "high_brake"},
                               {"m/s²", "m/s²"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_sharpbrake_plot, "", 0, 0, INT32_MAX, "urgent_brake lower", 1, 0,
                                        urgent_brake_low, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_sharpbrake_plot, "", 0, 0, INT32_MAX, "high_brake lower", 1, 0,
                                        high_brake_low, 1);
  }

  return true;
}
bool EvalSharpBrakeCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // compute ACC Y start
      double _acc = ego_front->GetAcc().GetX();
      VLOG_1 << "sharp brake acc limit is " << 0.3 << " m/s^2." << " current sharp brake acc is " << _acc << "m/s^2.\n";
      _detector.Detect(_acc, high_brake_low);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_sharpbrake_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_sharpbrake_plot.mutable_y_axis()->at(0).add_axis_data(_acc);
        _s_sharpbrake_plot.mutable_y_axis()->at(1).add_axis_data(_acc);
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
bool EvalSharpBrakeCheck::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_sharpbrake_plot);
  }
  return true;
}

void EvalSharpBrakeCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalSharpBrakeCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above sharp brake");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max sharp brake check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max sharp brake check skipped");
}

bool EvalSharpBrakeCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above sharp brake limit";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
