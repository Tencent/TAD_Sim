// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_smooth_start.h"

namespace eval {
const char EvalSmoothStart::_kpi_name[] = "SmoothStart";

sim_msg::TestReport_XYPlot _s_smooth_plot;

EvalSmoothStart::EvalSmoothStart() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalSmoothStart::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    statusStart = false;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_smooth_plot, "lateral acc", "", "t", "s", {"lateral acc"}, {"m/s²"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_smooth_plot, "lateral acc upper", 0, 1, m_defaultThreshDouble, "", 1, 0,
                                        INT32_MIN, 0);
  }

  return true;
}
bool EvalSmoothStart::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // compute ACC Y start
      double lateral_acc = 0;
      VLOG_1 << "start lateral_acc limit is " << " m/s^2." << m_defaultThreshDouble << " current lateral_acc is "
             << lateral_acc << "m/s^2.\n";
      double long_acc = ego_front->GetAcc().GetX();
      double ego_speed = ego_front->GetSpeed().GetNormal();
      if (ego_speed < 1.39 && long_acc > 0) {
        lateral_acc = std::fabs(ego_front->GetAcc().GetY());
        _detector.Detect(lateral_acc, m_defaultThreshDouble);
      }
      if (isReportEnabled()) {
        _s_smooth_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_smooth_plot.mutable_y_axis()->at(0).add_axis_data(lateral_acc);
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
bool EvalSmoothStart::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_smooth_plot);
  }
  return true;
}

void EvalSmoothStart::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalSmoothStart::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above start lateral_acc");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max start lateral_acc check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max start lateral_acc check skipped");
}

bool EvalSmoothStart::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above start lateral_acc limit";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
