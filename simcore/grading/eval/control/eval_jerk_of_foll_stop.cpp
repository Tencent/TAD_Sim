// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_jerk_of_foll_stop.h"

namespace eval {
const char EvalJerkOfFollStop::_kpi_name[] = "JerkOfFollStop";

sim_msg::TestReport_XYPlot EvalJerkOfFollStop::_s_follow_stop_jerk_plot;

EvalJerkOfFollStop::EvalJerkOfFollStop() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalJerkOfFollStop::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _above20_jerk_thresh = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ABOVE_20_JERK");
    _lower5_jerk_thresh = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LOWER_5_JERK");
    _event_follow_stop = false;
    _max_jerk = 0.0;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_follow_stop_jerk_plot, "follow stop jerk", "jerk when front cars stop", "t", "s",
                               {"dec jerk", "veh speed"}, {"m/s³", "m/s"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_follow_stop_jerk_plot, "jerk upper", 0, 1, 5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_follow_stop_jerk_plot, "", 0, 1, INT32_MAX, "", 1, 1, 1.39, 1);
    ReportHelper::ConfigXYPlotThresholdClearValue(_s_follow_stop_jerk_plot, {0}, true, false);
  }

  return true;
}
bool EvalJerkOfFollStop::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      auto _fellow = helper.GetLeadingVehicle();
      double _ego_speed = ego_front->GetSpeed().GetNormal2D();
      double _ego_jerk = 0.0;
      double _ego_acc = ego_front->GetAcc().GetX();
      double _fell_speed = 0.0;
      if (_ego_speed > 20) _max_jerk = _above20_jerk_thresh;
      if (_ego_speed < 5) _max_jerk = _lower5_jerk_thresh;
      if (_fellow) {
        _fell_speed = _fellow->GetSpeed().GetNormal();
        EVector3d ego_2_fellow =
            CEvalMath::Sub(_fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        double dist_relative = ego_2_fellow.GetNormal2D();
        double _fell_acc = _fellow->GetAcc().GetX();
        if (!_event_follow_stop && dist_relative <= m_defaultThreshDouble && _fell_speed <= 1.39 && _fell_acc < 0.0 &&
            _ego_speed > 1.39) {
          _event_follow_stop = true;
        }
        if (_event_follow_stop) {
          if (_ego_speed < 0.56 || (_fell_speed > 0.56 && _fell_acc > 0.1)) {
            _event_follow_stop = false;
          } else if (_ego_acc < 0) {
            _ego_jerk = ego_front->GetJerk().GetX();
            _detector.Detect(_ego_jerk, _max_jerk);
          }
        }
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_follow_stop_jerk_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_follow_stop_jerk_plot.mutable_y_axis()->at(0).add_axis_data(_ego_jerk);
        _s_follow_stop_jerk_plot.mutable_y_axis()->at(1).add_axis_data(_fell_speed);
        _s_follow_stop_jerk_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_max_jerk);
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
bool EvalJerkOfFollStop::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_follow_stop_jerk_plot);
  }
  return true;
}

void EvalJerkOfFollStop::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalJerkOfFollStop::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max dec jerk upper");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max dec jerk check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max dec jerk check skipped");
}

bool EvalJerkOfFollStop::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max dec jerk";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
