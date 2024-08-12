// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_car_speed_statistics.h"

namespace eval {
const char EvalCarSpeedStatistics::_kpi_name[] = "CarSpeedStatistics";

sim_msg::TestReport_XYPlot EvalCarSpeedStatistics::_s_speed_plot;

EvalCarSpeedStatistics::EvalCarSpeedStatistics() {
  _speed = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalCarSpeedStatistics::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_speed_plot, "speed", "speed", "t", "s", {"speed"}, {"m/s"}, 1);
    /**
     * set Threshold after ConfigXYPlot
     * @param    xy_plot         TestReport_XYPlot
     * @param    upper_desc      the desc of upper threshold.
     * @param    upper_space     the IntervalSpace of upper threshold.
     * @param    upper_type      the IntervalType of upper threshold.
     * @param    upper_value     the value of upper threshold.
     * @param    lower_desc      the desc of lower threshold.
     * @param    lower_space     the IntervalSpace of lower threshold.
     * @param    lower_type      the IntervalType of lower threshold.
     * @param    lower_value     the value of lower threshold.
     * @param    y_axis_id       the id  of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(_s_speed_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
  }
  return true;
}
bool EvalCarSpeedStatistics::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      _speed = ego_front->GetSpeed().GetNormal();
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_speed_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_speed_plot.mutable_y_axis()->at(0).add_axis_data(_speed);
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
bool EvalCarSpeedStatistics::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_speed_plot);
  }
  return true;
}

void EvalCarSpeedStatistics::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalCarSpeedStatistics::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check skipped");
}

bool EvalCarSpeedStatistics::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
