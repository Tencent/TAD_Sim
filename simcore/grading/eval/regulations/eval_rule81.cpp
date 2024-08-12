// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule81.h"

namespace eval {
const char EvalRule81::_kpi_name[] = "Rule81";
sim_msg::TestReport_XYPlot EvalRule81::s_rule81_plot;
sim_msg::TestReport_PairData EvalRule81::s_distance_variance_pair;
sim_msg::TestReport_PairData EvalRule81::s_speed_variance_pair;

EvalRule81::EvalRule81() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule81::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_distance_variance_pair);
    ReportHelper::ConfigPairData(s_speed_variance_pair);
    ReportHelper::ConfigXYPlot(
        s_rule81_plot, "Rule81", "", "t", "s",
        {"_result_speed_m_s", "result_distance", "_cond_on_enter_expressway", "_cond_visibility"},
        {"m/s", "m", "OnOff", "m"}, 4);

    ReportHelper::ConfigXYPlotThreshold(s_rule81_plot, "speed upper", 0, 0, INT32_MAX, "speed lower", 1, 0, INT32_MIN,
                                        0);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule81_plot, {0}, true, false);

    ReportHelper::ConfigXYPlotThreshold(s_rule81_plot, "distance upper", 0, 0, INT32_MAX, "distance lower", 1, 0,
                                        INT32_MIN, 1);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule81_plot, {1}, false, true);
  }

  return true;
}

bool EvalRule81::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule81");
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

  // param init
  _cond_visibility = 500;
  _cond_on_enter_expressway = false;

  _threshold_lower_distance = 150.0;
  _threshold_upper_speed_m_s = 120.0;

  _result_speed_m_s = 0.0;
  _result_distance = INT16_MAX;

  // 1.Get the condition
  // get weather
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    // visibility unit m
    _cond_visibility = environment_ptr->GetFogVisibility();
  }

  // whether is on expressway
  _cond_on_enter_expressway = map_info->m_active_road.m_on_motorway;  // m_active_lane.m_on_entry_expressway;

  // get speed of ego
  _result_speed_m_s = ego_front->GetSpeed().GetNormal2D();

  // get relative distance from ego milldle front to fellow milldle rear
  auto fellow = helper.GetLeadingVehicle();
  if (fellow && _cond_on_enter_expressway) {
    CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
    CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
    EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
    _result_distance = ego_2_fellow.GetNormal2D();
  }

  // 2.Set thresh value
  if (_cond_on_enter_expressway && _cond_visibility < 200) {
    _threshold_lower_distance = 100;
    _threshold_upper_speed_m_s = 60;
  }
  if (_cond_on_enter_expressway && _cond_visibility < 100) {
    _threshold_lower_distance = 50;
    _threshold_upper_speed_m_s = 40;
  }
  if (_cond_on_enter_expressway && _cond_visibility < 50) {
    // todo: Drive away from the road
    _threshold_lower_distance = 30;
    _threshold_upper_speed_m_s = 20;
  }

  // 3.Check Detect and report
  _detector_lower_distance.Detect(_result_distance, _threshold_lower_distance);
  VLOG_1 << "_rel_dist is " << _result_distance << " m.\n";
  _detector_upper_speed_m_s.Detect(_result_speed_m_s, _threshold_upper_speed_m_s / 3.6);
  VLOG_1 << "_speed is " << _result_distance << " m.\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    _result_distance = _result_distance > const_max_actor_size ? -1.0 : _result_distance;
    s_rule81_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule81_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_rule81_plot.mutable_y_axis()->at(1).add_axis_data(_result_distance);
    s_rule81_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_enter_expressway);
    s_rule81_plot.mutable_y_axis()->at(3).add_axis_data(_threshold_upper_speed_m_s);

    s_rule81_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_threshold_upper_speed_m_s / 3.6);
    s_rule81_plot.mutable_y_axis()->at(1).mutable_threshold_lower()->add_value(_threshold_lower_distance);
  }

  return true;
}

bool EvalRule81::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_rule81_plot.y_axis_size() > 0) speed_variance = ReportHelper::CalVariance(s_rule81_plot.y_axis(0));
    ReportHelper::SetPairData(s_speed_variance_pair, "speed variance", std::to_string(speed_variance));

    // calculate distance variance
    double distance_variance = 0.0;
    if (s_rule81_plot.y_axis_size() > 0) distance_variance = ReportHelper::CalVariance(s_rule81_plot.y_axis(0));
    ReportHelper::SetPairData(s_distance_variance_pair, "distance variance", std::to_string(distance_variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule81_plot);
    ReportHelper::AddPair2Attach(*attach, s_distance_variance_pair);
    ReportHelper::AddPair2Attach(*attach, s_speed_variance_pair);
  }

  return true;
}

void EvalRule81::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_upper_speed_m_s, _kpi_name);
}

EvalResult EvalRule81::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector_upper_speed_m_s.GetCount() +
                                             _detector_lower_distance.GetCount());

    if (_detector_lower_distance.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "relative distance too low");
    } else if (_detector_upper_speed_m_s.GetCount() >= m_Kpi.passcondition().value() &&
               m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "relative distance and max speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "relative distance and max speed check skipped");
}

bool EvalRule81::ShouldStopScenario(std::string &reason) {
  auto ret_distance =
      _detector_lower_distance.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  auto ret_speed =
      _detector_upper_speed_m_s.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;

  if (ret_distance) {
    reason = "relative distance too low";
  }
  if (ret_speed) {
    reason = "above max speed";
  }
  _case.mutable_info()->set_request_stop(ret_distance || ret_speed);

  return (ret_distance || ret_speed);
}
}  // namespace eval
