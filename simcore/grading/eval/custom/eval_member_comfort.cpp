// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_member_comfort.h"

namespace eval {
const char EvalMemberComfort::_kpi_name[] = "MemberComfort";

sim_msg::TestReport_XYPlot _s_comfort_plot;

EvalMemberComfort::EvalMemberComfort() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  v_sum = 0.0;
  v2_sum = 0.0;
  count = 0.0;
}
bool EvalMemberComfort::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(_s_comfort_plot, "long coefficient desc", "", "t", "s", {"long coefficient"}, {""}, 1);
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

    ReportHelper::ConfigXYPlotThreshold(_s_comfort_plot, "coefficient upper", 0, 1, m_defaultThreshDouble, "", 1, 0,
                                        INT32_MIN, 0);
    /**
     * [optional] clear threshold value if threshold is not constants
     * @param xy_plot         TestReport_XYPlot
     * @param y_axis_ids      y_axis id list to clear
     * @param upper_clear     whether clear upper threshold
     * @param lower_clear     whether clear lower threshold
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(_s_rateChAcc_plot, {0, 1}, true, false);
  }

  return true;
}
bool EvalMemberComfort::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // compute coefficient
      double _speed = ego_front->GetSpeed().GetNormal2D();
      _speed = std::abs(_speed);
      count++;
      v2_sum += _speed * _speed;
      v_sum += _speed;
      double av_v = v_sum / count;
      double tmp = (v2_sum - 2 * av_v * v_sum) / count + av_v * av_v;
      double res = av_v == 0 ? 0 : sqrt(tmp) / av_v;
      VLOG_1 << "speed: " << _speed << " v_sum: " << v_sum << "count: " << count << " v2_sum:" << v2_sum
             << " coefficient limit is " << m_defaultThreshDouble << " current coefficient is " << res
             << " zhuncha is: " << tmp << "\n";

      _detector.Detect(res, m_defaultThreshDouble);

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_comfort_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_comfort_plot.mutable_y_axis()->at(0).add_axis_data(res);
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
bool EvalMemberComfort::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_comfort_plot);
  }
  return true;
}

void EvalMemberComfort::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalMemberComfort::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above long coefficient");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max long coefficient check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max long coefficient check skipped");
}

bool EvalMemberComfort::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above long coefficient";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
