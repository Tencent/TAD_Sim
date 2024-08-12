// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule58.h"

namespace eval {
const char EvalRule58::_kpi_name[] = "Rule58";
sim_msg::TestReport_XYPlot EvalRule58::s_rule58_plot;

EvalRule58::EvalRule58() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule58::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule58_plot, "beam and harzard check", "", "t", "s",
                               {"result", "_cond_night", "_cond_fog", "_cond_rain", "_cond_snow",
                                "_cond_low_visibility", "_actual_beam_proper_used", "_actual_harzard_proper_used"},
                               {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 8);

    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule58_plot, {0}, false, true);
  }

  return true;
}

bool EvalRule58::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule58");
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
  _cond_night = false;
  _cond_fog = false;
  _cond_rain = false;
  _cond_snow = false;
  _cond_low_visibility = false;
  _actual_beam_proper_used = true;
  _actual_harzard_proper_used = true;
  _result = 0.0;

  // 1.Get the condition
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    // get time of day
    auto time_of_day = environment_ptr->GetPartsOfDay();
    _cond_night = (time_of_day == eval::PARTS_OF_DAY_MIDNIGHT || time_of_day == eval::PARTS_OF_DAY_DAWN ||
                   time_of_day == eval::PARTS_OF_DAY_NIGHT);
    // get weather
    // visibility unit m
    _cond_low_visibility = (environment_ptr->GetFogVisibility() < 200);
    // get weather
    _cond_fog = (environment_ptr->GetWeather() == eval::WEATHER_FOG);
    _cond_rain = (environment_ptr->GetWeather() == eval::WEATHER_RAIN);
    _cond_snow = (environment_ptr->GetWeather() == eval::WEATHER_SNOW);
  }

  // get relative distance from ego milldle front to fellow milldle rear
  double rel_dist = 0;
  if (map_info) {
    auto ego_lane = map_info->m_active_lane.m_lane;
    auto fellow = helper.GetLeadingVehicle();
    if (fellow) {
      CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
      CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
      EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
      rel_dist = ego_2_fellow.GetNormal2D();
    }
  }

  // 2.Check Detect and report
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  // check beam
  if (_cond_night || _cond_fog || _cond_rain || _cond_snow || _cond_low_visibility || rel_dist < 150.0) {
    _actual_beam_proper_used = veh_fb.m_low_beam != OnOff::ON;
  }
  // check harzard
  if (_cond_fog) {
    _actual_harzard_proper_used = veh_fb.m_harzard != OnOff::ON;
  }

  if (!_actual_beam_proper_used || !_actual_harzard_proper_used) {
    _result = 1.0;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should use low beam and harzard proper , current: " << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule58_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule58_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule58_plot.mutable_y_axis()->at(1).add_axis_data(_cond_night);
    s_rule58_plot.mutable_y_axis()->at(2).add_axis_data(_cond_fog);
    s_rule58_plot.mutable_y_axis()->at(3).add_axis_data(_cond_rain);
    s_rule58_plot.mutable_y_axis()->at(4).add_axis_data(_cond_snow);
    s_rule58_plot.mutable_y_axis()->at(5).add_axis_data(_cond_low_visibility);
    s_rule58_plot.mutable_y_axis()->at(6).add_axis_data(_actual_beam_proper_used);
    s_rule58_plot.mutable_y_axis()->at(7).add_axis_data(_actual_harzard_proper_used);
  }

  return true;
}

bool EvalRule58::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule58_plot);
  }

  return true;
}

void EvalRule58::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule58::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL,
                        "ego should use low beam or ego should turn on harzard beam when meet fog");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "beam and harzard proper used check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "beam check skipped");
}

bool EvalRule58::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should use low beam or ego should turn on harzard beam when meet fog";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
