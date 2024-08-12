// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_max_speed_on_ramp.h"

namespace eval {
const char EvalMaxSpeedOnRamp::_kpi_name[] = "MaxSpeedOnRamp";

sim_msg::TestReport_XYPlot EvalMaxSpeedOnRamp::_s_ramp_check_plot;

EvalMaxSpeedOnRamp::EvalMaxSpeedOnRamp() {
  _speed = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalMaxSpeedOnRamp::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_ramp_check_plot, "ramp speed", "", "t", "s", {"speed_when_ramp"}, {"m/s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_ramp_check_plot, "", 0, 1, 16.67, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalMaxSpeedOnRamp::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      _speed = ego_front->GetSpeed().GetNormal();
      double on_ramp_speed = _speed;
      bool is_ramp = map_info->m_active_lane.m_on_entry_expressway || map_info->m_active_lane.m_on_exit_expressway ||
                     map_info->m_active_lane.m_on_connecting_ramp;
      VLOG_2 << "ego is on entry_expressway: " << map_info->m_active_lane.m_on_entry_expressway
             << ", on exit expressway: " << map_info->m_active_lane.m_on_exit_expressway
             << ", on connecting ramp: " << map_info->m_active_lane.m_on_connecting_ramp << "\n";
      if (is_ramp) {
        _detector.Detect(on_ramp_speed, 16.67);
      } else {
        on_ramp_speed = 0.0;
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_ramp_check_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_ramp_check_plot.mutable_y_axis()->at(0).add_axis_data(on_ramp_speed);
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
bool EvalMaxSpeedOnRamp::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_ramp_check_plot);
  }
  return true;
}

void EvalMaxSpeedOnRamp::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalMaxSpeedOnRamp::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max ramp speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max ramp speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max ramp speed check skipped");
}

bool EvalMaxSpeedOnRamp::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max ramp speed";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
