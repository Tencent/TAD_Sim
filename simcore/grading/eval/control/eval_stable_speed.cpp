// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_stable_speed.h"

namespace eval {
const char EvalStableSpeed::_kpi_name[] = "StableSpeed";

sim_msg::TestReport_XYPlot _s_stableSpeed_plot;

EvalStableSpeed::EvalStableSpeed() {
  _st_min_v = 30.0;
  _st_max_v = 35.0;
  _speed = 0.0;
  _stable_time = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalStableSpeed::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _st_min_v = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "SpeedLower");
    _st_max_v = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "SpeedUpper");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_stableSpeed_plot, "stable speed", "", "t", "s", {"acc", "speed", "stable time"},
                               {"m/s²", "m/s", "s"}, 3);
    ReportHelper::ConfigXYPlotThreshold(_s_stableSpeed_plot, "acc upper", 0, 1, 1, "acc lower", 1, 1, -1, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_stableSpeed_plot, "stable speed upper", 0, 1, _st_max_v,
                                        "stable speed lower", 1, 1, _st_min_v, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_stableSpeed_plot, "", 0, 1, INT32_MAX, "", 1, 1, INT32_MIN, 2);
  }

  return true;
}
bool EvalStableSpeed::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get lane speed limit from hadmap
      _speed = ego_front->GetSpeed().GetNormal();
      double _acc = ego_front->GetAcc().GetX();
      bool _stabled = true;
      if (std::fabs(_acc) < 1) {
        _stabled = (_speed > _st_min_v && _speed < _st_max_v);
        _stable_time += getModuleStepTime();
      } else {
        _stable_time = 0.0;
      }
      _detector.Detect(!_stabled, 0.5);
      VLOG_1 << "stable speed lower limit is " << _st_min_v << " m/s." << " stable speed lower limit is " << _st_max_v
             << " m/s." << " current speed is " << _speed << " m/s." << " stable time is " << _stable_time << " s\n";
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_stableSpeed_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_stableSpeed_plot.mutable_y_axis()->at(0).add_axis_data(_acc);
        _s_stableSpeed_plot.mutable_y_axis()->at(1).add_axis_data(_speed);
        _s_stableSpeed_plot.mutable_y_axis()->at(2).add_axis_data(_stable_time);
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
bool EvalStableSpeed::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_stableSpeed_plot);
  }
  return true;
}

void EvalStableSpeed::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalStableSpeed::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "out range of speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "range of speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "range of speed check skipped");
}

bool EvalStableSpeed::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "out range of speed";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
