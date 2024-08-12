// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_speed.h"

namespace eval {
const char EvalSpeed::_kpi_name[] = "MaxSpeed_V";

sim_msg::TestReport_XYPlot s_speed_plot;
sim_msg::TestReport_PairData g_speed_variance_pair;

EvalSpeed::EvalSpeed() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _speed = 0.0;
  _speed_limit = 33.0;
}
bool EvalSpeed::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  m_speed_limit_from_scene = helper.GetScenarioInfo().m_ego_speed_limit;

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_speed_plot, "speed", "", "t", "s", {"speed"}, {"m/s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_speed_plot, "thresh upper", 0, 0, INT32_MAX, "thresh lower", 1, 0, INT32_MIN);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_speed_plot, {0}, true, false);
    ReportHelper::ConfigPairData(g_speed_variance_pair);
  }

  return true;
}
bool EvalSpeed::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_speed_above_thresh(sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get lane speed limit from hadmap
      double speed_limit_from_map = -1.0;
      _speed_limit = m_speed_limit_from_scene;
      if (ego_front->GetMapInfo()) {
        auto ego_lane = ego_front->GetMapInfo()->m_active_lane.m_lane;
        if (ego_lane) {
          speed_limit_from_map = ego_lane->getSpeedLimit() / 3.6;
          speed_limit_from_map = speed_limit_from_map > 1.0 ? speed_limit_from_map : -1.0;
        }
      }
      if (speed_limit_from_map >= 0.5) {
        _speed_limit = m_defaultThreshDouble < speed_limit_from_map ? m_defaultThreshDouble : speed_limit_from_map;
      }

      _speed = ego_front->GetSpeed().GetNormal();

      VLOG_1 << "speed limit is " << _speed_limit << " m/s." << " current ego speed is " << _speed << " m/s.\n";

      double limit_add_eps = _speed_limit + const_limit_eps;
      if (_detector.Detect(_speed, limit_add_eps)) {
        grading.mutable_event_detector()->set_speed_above_thresh(
            sim_msg::Grading_EventDetector_EventState_EventDetected);
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        s_speed_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_speed_plot.mutable_y_axis()->at(0).add_axis_data(_speed);
        s_speed_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_speed_limit);
      }
    } else {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalSpeed::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("OverSpeed", std::to_string(_detector.GetCount()));
  helper.SetFeedback("OverspeedTime", std::to_string(0));

  // calculate speed variance
  double speed_variance = 0.0;
  if (s_speed_plot.y_axis_size() > 0) speed_variance = ReportHelper::CalVariance(s_speed_plot.y_axis(0));
  ReportHelper::SetPairData(g_speed_variance_pair, "speed variance", std::to_string(speed_variance));

  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_speed_plot);
    ReportHelper::AddPair2Attach(*attach, g_speed_variance_pair);
  }

  return true;
}

EvalResult EvalSpeed::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check skipped");
}
bool EvalSpeed::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalSpeed::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  msg.mutable_speed()->set_state(sim_msg::GRADING_SPEED_NORMAL);
  if (_detector.GetLatestState()) {
    msg.mutable_speed()->set_state(sim_msg::GRADING_SPEED_OVERHIGH);
  }
  msg.mutable_speed()->set_threshold(_speed_limit);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalSpeed::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_overspeed()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_overspeed()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_overspeed()->set_is_pass(IsEvalPass()._state ==
                                                               sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
