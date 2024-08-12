// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_jerk_of_foll_start.h"

namespace eval {
const char EvalJerkOfFollStart::_kpi_name[] = "JerkOfFollStart";

sim_msg::TestReport_XYPlot EvalJerkOfFollStart::_s_start_jerk_plot;

EvalJerkOfFollStart::EvalJerkOfFollStart() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalJerkOfFollStart::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _above20_jerk_thresh = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ABOVE_20_JERK");
    _lower5_jerk_thresh = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LOWER_5_JERK");
    _event_follow_start = false;
    _max_jerk = 0.0;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_start_jerk_plot, "follow start jerk", "jerk when front cars start", "t", "s",
                               {"acc_jerk"}, {"m/s³"}, 1);
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
    ReportHelper::ConfigXYPlotThreshold(_s_start_jerk_plot, "jerk upper", 0, 1, 5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThresholdClearValue(_s_start_jerk_plot, {0}, true, false);
  }

  return true;
}
bool EvalJerkOfFollStart::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      auto _fellow = helper.GetLeadingVehicle();
      double _ego_speed = ego_front->GetSpeed().GetNormal2D();
      double _ego_jerk = 0.0;
      double _ego_acc = ego_front->GetAcc().GetX();
      if (_ego_speed > 20) _max_jerk = _above20_jerk_thresh;
      if (_ego_speed < 5) _max_jerk = _lower5_jerk_thresh;
      if (_fellow) {
        EVector3d ego_2_fellow =
            CEvalMath::Sub(_fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        double dist_relative = ego_2_fellow.GetNormal2D();
        double _fell_speed = _fellow->GetSpeed().GetNormal2D();
        double _fell_acc = _fellow->GetAcc().GetX();
        if (!_event_follow_start && dist_relative <= m_defaultThreshDouble && _fell_speed <= 1.39 && _fell_acc > 0.1 &&
            _ego_speed <= 0.56) {
          _event_follow_start = true;
        }
        if (_event_follow_start) {
          if (_fell_speed > 1.39 || _ego_speed > 1.39) {
            _event_follow_start = false;
          } else if (_ego_acc >= 0) {
            _ego_jerk = ego_front->GetJerk().GetNormal2D();
            _detector.Detect(_ego_jerk, _max_jerk);
          }
        }
      } else {
        _event_follow_start = false;
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_start_jerk_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_start_jerk_plot.mutable_y_axis()->at(0).add_axis_data(_ego_jerk);
        _s_start_jerk_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_max_jerk);
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
bool EvalJerkOfFollStart::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_start_jerk_plot);
  }
  return true;
}

void EvalJerkOfFollStart::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalJerkOfFollStart::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max jerk of starting");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max jerk of staring check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max jerk of starting check skipped");
}

bool EvalJerkOfFollStart::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max jerk";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
