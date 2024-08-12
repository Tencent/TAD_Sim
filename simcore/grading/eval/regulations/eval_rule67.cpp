// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule67.h"

namespace eval {
const char EvalRule67::_kpi_name[] = "Rule67";
sim_msg::TestReport_XYPlot EvalRule67::s_rule67_plot;
sim_msg::TestReport_PairData EvalRule67::s_speed_variance_pair;

EvalRule67::EvalRule67() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule67::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_speed_variance_pair);
    ReportHelper::ConfigXYPlot(s_rule67_plot, "rule67", "", "t", "s",
                               {"_result_speed_m_s", "_cond_on_private_road", "_cond_on_parking_area"},
                               {"m/s", "OnOff", "OnOff"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_rule67_plot, "speed upper", 0, 0, INT32_MAX, "speed lower", 1, 0, INT32_MIN,
                                        0);

    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule67_plot, {0}, false, true);
  }

  return true;
}

bool EvalRule67::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule67");
  // check whether the module is valid and whether the indicator is enabled
  if (!IsModuleValid() || !m_KpiEnabled) {
    VLOG_0 << _kpi_name << " kpi not enabled.\n";
    return false;
  }

  // get the ego pointer and check whether the pointer is null
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  if (!ego_front) {
    VLOG_0 << _kpi_name << " ego actor missing.\n";
    return false;
  }

  // get the map information pointer and check whether the pointer is null
  auto map_info = ego_front->GetMapInfo();
  if (map_info.get() == nullptr) {
    VLOG_0 << _kpi_name << " ego actor missing with map.\n";
    return false;
  }

  hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
  if (!ego_lane) {
    VLOG_0 << _kpi_name << " ego actor missing with lane.\n";
    return false;
  }

  hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
  if (!ego_road) {
    VLOG_0 << _kpi_name << " ego actor missing with road.\n";
    return false;
  }

  // param init
  _cond_on_private_road = false;
  _cond_on_parking_area = false;
  _result_speed_m_s = 0.0;
  _threshold_upper_speed_m_s = 120 / 3.6;

  // 1.Get the condition
  // get speed limit from map
  double speed_limit_from_map = _map_mgr->GetSpeedLimitFromMap(ego_lane);

  // whether is on private road
  _cond_on_private_road = map_info->m_active_road.m_on_private_road;

  // whether is on parking area
  _cond_on_parking_area = map_info->m_active_road.m_on_parking_area;

  // 2.Set thresh value
  // Speed limit sign && speed limit marking -> not exceed
  if ((_cond_on_private_road || _cond_on_parking_area) && speed_limit_from_map >= 0.5) {
    _threshold_upper_speed_m_s = speed_limit_from_map;
  }

  // 3.Get actual value
  _result_speed_m_s = ego_front->GetSpeed().GetNormal();

  // 4.Check Detect and report
  _detector.Detect(_result_speed_m_s, _threshold_upper_speed_m_s);
  VLOG_1 << "speed limit is " << _threshold_upper_speed_m_s << " m/s." << " current ego speed is " << _result_speed_m_s
         << " m/s.\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule67_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule67_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_rule67_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_private_road);
    s_rule67_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_parking_area);

    s_rule67_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_threshold_upper_speed_m_s);
  }

  return true;
}

bool EvalRule67::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_rule67_plot.y_axis_size() > 0) speed_variance = ReportHelper::CalVariance(s_rule67_plot.y_axis(0));
    ReportHelper::SetPairData(s_speed_variance_pair, "speed variance", std::to_string(speed_variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule67_plot);
    ReportHelper::AddPair2Attach(*attach, s_speed_variance_pair);
  }

  return true;
}

void EvalRule67::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule67::IsEvalPass() {
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

bool EvalRule67::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
