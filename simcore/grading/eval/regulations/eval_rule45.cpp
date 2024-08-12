// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule45.h"

namespace eval {
const char EvalRule45::_kpi_name[] = "Rule45";
sim_msg::TestReport_XYPlot EvalRule45::s_rule45_plot;
sim_msg::TestReport_PairData EvalRule45::s_speed_variance_pair;

EvalRule45::EvalRule45() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule45::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(
        s_rule45_plot, "rule45", "", "t", "s",
        {"result_speed", "_cond_on_urban", "_cond_on_motorway", "_cond_no_center_lane", "_cond_only_one_driving_lane"},
        {"m/s", "OnOff", "OnOff", "OnOff", "OnOff"}, 5);
    ReportHelper::ConfigPairData(s_speed_variance_pair);

    ReportHelper::ConfigXYPlotThreshold(s_rule45_plot, "speed upper", 0, 0, INT32_MAX, "speed lower", 1, 0, INT32_MIN,
                                        0);

    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule45_plot, {0}, true, false);
  }

  return true;
}

bool EvalRule45::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule45");
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
  _cond_on_urban = false;
  _cond_on_motorway = false;
  _cond_no_center_lane = false;
  _cond_only_one_driving_lane = false;
  _threshold_upper_speed_m_s = 120 / 3.6;

  // 1.Get the condition
  // whether is on urban
  _cond_on_urban = map_info->m_active_road.m_on_urban;
  VLOG_1 << "cond_on_urban: " << _cond_on_urban << "\n";

  // whether is on motorway
  _cond_on_motorway = map_info->m_active_road.m_on_motorway;
  VLOG_1 << "cond_on_motorway: " << _cond_on_motorway << "\n";

  // get speed limit from map
  double speed_limit_from_map = _map_mgr->GetSpeedLimitFromMap(ego_lane);
  VLOG_1 << "speed limit from hdmap: " << speed_limit_from_map << " m/s.\n";

  // whether there have center lane
  _cond_no_center_lane = map_info->m_active_lane.m_no_center_lane;
  VLOG_1 << "cond_no_center_lane: " << _cond_no_center_lane << "\n";

  // whether there is only one driving lane in the same direction
  _cond_only_one_driving_lane = _map_mgr->CalDrivingLaneNum(ego_lane->getTxLaneId()) == 1;
  VLOG_1 << "cond_only_one_driving_lane: " << _cond_only_one_driving_lane << "\n";

  // 2.Set thresh value
  if (_cond_on_urban && _cond_no_center_lane) {
    _threshold_upper_speed_m_s = 30 / 3.6;
  } else if (_cond_on_motorway && _cond_no_center_lane) {
    _threshold_upper_speed_m_s = 40 / 3.6;
  } else if (_cond_on_urban && _cond_only_one_driving_lane) {
    _threshold_upper_speed_m_s = 50 / 3.6;
  } else if (_cond_on_motorway && _cond_only_one_driving_lane) {
    _threshold_upper_speed_m_s = 70 / 3.6;
  } else {
    _threshold_upper_speed_m_s = 120 / 3.6;
  }
  if (speed_limit_from_map >= 0.5) {
    _threshold_upper_speed_m_s = std::min(speed_limit_from_map, _threshold_upper_speed_m_s);
  }

  // 3.Get actual value
  double _result_speed_m_s = ego_front->GetSpeed().GetNormal();
  _threshold_upper_speed_m_s = static_cast<int>(_threshold_upper_speed_m_s * 100) / 100.00 + 0.01;

  // 4.Check Detect and report
  _detector.Detect(_result_speed_m_s, _threshold_upper_speed_m_s);
  VLOG_1 << "speed limit is " << _threshold_upper_speed_m_s << " m/s." << " current ego speed is " << _result_speed_m_s
         << " m/s.\n";
  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule45_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule45_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_rule45_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_urban);
    s_rule45_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_motorway);
    s_rule45_plot.mutable_y_axis()->at(3).add_axis_data(_cond_no_center_lane);
    s_rule45_plot.mutable_y_axis()->at(4).add_axis_data(_cond_only_one_driving_lane);
    //
    s_rule45_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_threshold_upper_speed_m_s);
  }

  return true;
}

bool EvalRule45::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_rule45_plot.y_axis_size() > 0) speed_variance = ReportHelper::CalVariance(s_rule45_plot.y_axis(0));
    ReportHelper::SetPairData(s_speed_variance_pair, "speed variance", std::to_string(speed_variance));
    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule45_plot);
    ReportHelper::AddPair2Attach(*attach, s_speed_variance_pair);
  }

  return true;
}

void EvalRule45::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule45::IsEvalPass() {
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

bool EvalRule45::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
