// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule46.h"

namespace eval {
const char EvalRule46::_kpi_name[] = "Rule46";
sim_msg::TestReport_XYPlot EvalRule46::s_rule46_plot;
sim_msg::TestReport_PairData EvalRule46::s_speed_variance_pair;

EvalRule46::EvalRule46() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule46::Init(eval::EvalInit &helper) {
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
        s_rule46_plot, "rule46", "", "t", "s",
        {"result_speed", "_cond_rain", "_cond_snow", "_cond_fog", "_cond_on_ramp", "_cond_on_narrow_road",
         "_cond_on_bridge", "_cond_on_sharp_curve", "_cond_on_steep_slope", "_cond_turn_left", "_cond_turn_right"},
        {"m/s", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 11);
    ReportHelper::ConfigPairData(s_speed_variance_pair);

    ReportHelper::ConfigXYPlotThreshold(s_rule46_plot, "speed upper", 0, 0, INT32_MAX, "speed lower", 1, 0, INT32_MIN,
                                        0);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule46_plot, {0}, false, true);
  }

  return true;
}

bool EvalRule46::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule46");
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

  // Parm init
  _cond_rain = false;
  _cond_snow = false;
  _cond_fog = false;
  _cond_on_ramp = false;
  _cond_on_narrow_road = false;
  _cond_on_bridge = false;
  _cond_on_sharp_curve = false;
  _cond_on_steep_slope = false;
  _cond_turn_left = false;
  _cond_turn_right = false;
  _result_speed_m_s = 0.0;
  _threshold_upper_speed_m_s = 120.0 / 3.6;

  // 1.Get the condition
  // get weather
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    _cond_rain = environment_ptr->GetWeather() == eval::WEATHER_RAIN;
    _cond_snow = environment_ptr->GetWeather() == eval::WEATHER_SNOW;
    _cond_fog = environment_ptr->GetWeather() == eval::WEATHER_FOG;
    VLOG_1 << "cond_rain: " << _cond_rain << "\n";
    VLOG_1 << "cond_snow: " << _cond_snow << "\n";
    VLOG_1 << "cond_fog: " << _cond_fog << "\n";
  }

  // whether on ramp
  _cond_on_ramp = map_info->m_active_road.m_on_ramp;
  VLOG_1 << "cond_on_ramp: " << _cond_on_ramp << "\n";

  // whether on narrow road
  _cond_on_narrow_road = map_info->m_active_lane.m_on_narrow_lane;
  VLOG_1 << "cond_on_narrow_road: " << _cond_on_narrow_road << "\n";

  // whether on bridge
  _cond_on_bridge = map_info->m_active_road.m_on_bridge;
  VLOG_1 << "cond_on_bridge: " << _cond_on_bridge << "\n";

  // whether on sharp curve
  _cond_on_sharp_curve = map_info->m_active_road.m_on_sharp_curve;
  VLOG_1 << "cond_on_sharp_curve: " << _cond_on_sharp_curve << "\n";

  // whether on steep slope
  _cond_on_steep_slope = map_info->m_active_road.m_on_steep_slope;
  VLOG_1 << "cond_on_steep_slope: " << _cond_on_steep_slope << "\n";

  // whether is turn
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  _cond_turn_left = (output.m_veh_behav == VehicleBehavior::TurnLeft);
  VLOG_1 << "cond_turn_left: " << _cond_turn_left << "\n";
  _cond_turn_right = (output.m_veh_behav == VehicleBehavior::TurnRight);
  VLOG_1 << "cond_turn_right: " << _cond_turn_right << "\n";

  // 2.Set thresh value
  if (_cond_rain || _cond_snow || _cond_fog || _cond_on_ramp || _cond_on_narrow_road || _cond_on_sharp_curve ||
      _cond_on_bridge || _cond_on_steep_slope || _cond_turn_left || _cond_turn_right) {
    _threshold_upper_speed_m_s = 30.0 / 3.6;
  }

  // 3.Get actual value (ego speed)
  _result_speed_m_s = ego_front->GetSpeed().GetNormal();

  // 4.Check Detect and report
  _detector.Detect(_result_speed_m_s, _threshold_upper_speed_m_s);
  VLOG_1 << "speed limit is " << _threshold_upper_speed_m_s << " m/s." << " current ego speed is " << _result_speed_m_s
         << " m/s.\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule46_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule46_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_rule46_plot.mutable_y_axis()->at(1).add_axis_data(_cond_rain);
    s_rule46_plot.mutable_y_axis()->at(2).add_axis_data(_cond_snow);
    s_rule46_plot.mutable_y_axis()->at(3).add_axis_data(_cond_fog);
    s_rule46_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_ramp);
    s_rule46_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_narrow_road);
    s_rule46_plot.mutable_y_axis()->at(6).add_axis_data(_cond_on_bridge);
    s_rule46_plot.mutable_y_axis()->at(7).add_axis_data(_cond_on_sharp_curve);
    s_rule46_plot.mutable_y_axis()->at(8).add_axis_data(_cond_on_steep_slope);
    s_rule46_plot.mutable_y_axis()->at(9).add_axis_data(_cond_turn_left);
    s_rule46_plot.mutable_y_axis()->at(10).add_axis_data(_cond_turn_right);
    //
    s_rule46_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_threshold_upper_speed_m_s);
  }

  return true;
}

bool EvalRule46::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_rule46_plot.y_axis_size() > 0) speed_variance = ReportHelper::CalVariance(s_rule46_plot.y_axis(0));
    ReportHelper::SetPairData(s_speed_variance_pair, "speed variance", std::to_string(speed_variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule46_plot);
    ReportHelper::AddPair2Attach(*attach, s_speed_variance_pair);
  }

  return true;
}

void EvalRule46::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule46::IsEvalPass() {
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

bool EvalRule46::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
