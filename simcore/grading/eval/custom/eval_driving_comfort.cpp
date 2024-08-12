// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_driving_comfort.h"

namespace eval {
const char EvalDrivingComfort::_kpi_name[] = "DrivingComfort";

sim_msg::TestReport_XYPlot _s_drive_comfort_plot;

EvalDrivingComfort::EvalDrivingComfort() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  frames_numbers = 0;
  time_cloth_sum = 0;
  i_comfort_sum = 0.0;
  lateral_acc2_sum = 0.0;
  long_acc2_sum = 0.0;
  angle2_sum = 0.0;
  jerk2_sum = 0.0;
}
bool EvalDrivingComfort::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_drive_comfort_plot, "driving comfort", "", "t", "s", {"I_comfort"}, {"N/A"}, 1);
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
    ReportHelper::ConfigXYPlotThreshold(_s_drive_comfort_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
    /**
     * [optional] clear threshold value if threshold is not constants
     * @param xy_plot         TestReport_XYPlot
     * @param y_axis_ids      y_axis id list to clear
     * @param upper_clear     whether clear upper threshold
     * @param lower_clear     whether clear lower threshold
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(_s_drive_comfort_plot, {0}, true, false);
  }

  return true;
}
bool EvalDrivingComfort::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      frames_numbers++;
      double _ego_speed = ego_front->GetSpeed().GetNormal2D();
      double lateral_acc = ego_front->GetAcc().GetY();
      double long_acc = ego_front->GetAcc().GetX();
      double long_jerk = ego_front->GetJerk().GetX();
      double yaw_rate = ego_front->GetAngularV().GetZ();

      lateral_acc2_sum += lateral_acc * lateral_acc;
      long_acc2_sum += long_acc * long_acc;
      jerk2_sum += long_jerk * long_jerk;
      angle2_sum += yaw_rate * yaw_rate;
      double res = 0.0;
      if (frames_numbers == 20) {
        time_cloth_sum++;
        i_comfort_sum += sqrt((lateral_acc2_sum + long_acc2_sum + 0.66 * 0.66 * jerk2_sum + 0.66 * 0.66 * angle2_sum) /
                              frames_numbers);
        res = i_comfort_sum / time_cloth_sum;

        // add data to xy-pot
        if (isReportEnabled()) {
          _s_drive_comfort_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
          _s_drive_comfort_plot.mutable_y_axis()->at(0).add_axis_data(res);
        }

        // reset
        frames_numbers = 0;
        lateral_acc2_sum = 0;
        long_acc2_sum = 0;
        jerk2_sum = 0;
        angle2_sum = 0;
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
bool EvalDrivingComfort::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_drive_comfort_plot);
  }
  return true;
}

void EvalDrivingComfort::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalDrivingComfort::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above no check");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "no check skipped");
}

bool EvalDrivingComfort::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above dring comfort value";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
