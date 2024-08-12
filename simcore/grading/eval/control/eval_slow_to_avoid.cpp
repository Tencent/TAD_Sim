// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_slow_to_avoid.h"

namespace eval {
const char EvalSlowToAvoid::_kpi_name[] = "SlowToAvoid";

sim_msg::TestReport_XYPlot _s_sdfl_plot;

EvalSlowToAvoid::EvalSlowToAvoid() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  distanceFront = 0.0;
  accFront = 0.0;
}
bool EvalSlowToAvoid::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    distanceFront = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "distanceToFront");
    accFront = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "AccFront");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_sdfl_plot, "decelerate", "has or not decelearte by front cars", "t", "s",
                               {"speed", "dist"}, {"m/s", "m"}, 2);
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
    ReportHelper::ConfigXYPlotThreshold(_s_sdfl_plot, "dist upper", 0, 1, distanceFront, "", 1, 0, INT32_MIN, 1);
    /**
     * [optional] clear threshold value if threshold is not constants
     * @param xy_plot         TestReport_XYPlot
     * @param y_axis_ids      y_axis id list to clear
     * @param upper_clear     whether clear upper threshold
     * @param lower_clear     whether clear lower threshold
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(_s_sdfl_plot, {0}, true, false);
  }

  return true;
}

bool EvalSlowToAvoid::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      double speed = ego_front->GetSpeed().GetNormal2D();
      auto fellow = helper.GetLeadingVehicle();
      double dist_relative = 0.0;
      if (fellow) {
        EVector3d ego_2_fellow =
            CEvalMath::Sub(fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        dist_relative = ego_2_fellow.GetNormal2D();
        if (fellow->GetSpeed().GetNormal2D() > 0 && dist_relative <= distanceFront &&
            fellow->GetAcc().GetX() <= -accFront) {
          VLOG_0 << "slow down to follow the fellow cat successful";
          _detector.Detect(ego_front->GetAcc().GetX() >= 0.0, 0.5);
        }
      }
      VLOG_1 << "min deceleration limit is " << 0 << " m/s^2." << " current ego speed is " << speed << " m/s^2.\n";

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_sdfl_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_sdfl_plot.mutable_y_axis()->at(0).add_axis_data(speed);
        _s_sdfl_plot.mutable_y_axis()->at(1).add_axis_data(dist_relative);
        /**
         * [optional] set threshold on step
         */
        // _s_sdfl_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(_speed_deceleration_limit);
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
bool EvalSlowToAvoid::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_sdfl_plot);
  }
  return true;
}

void EvalSlowToAvoid::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalSlowToAvoid::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "count of do not decelerate not pass");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "min deceleration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "min deceleration check skipped");
}

bool EvalSlowToAvoid::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "below min deceleration";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
