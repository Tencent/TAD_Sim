// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_over_speed_when_poor_visibility.h"

namespace eval {
const char NoOverSpeedWhenPoorVisibility::_kpi_name[] = "NoOverSpeedWhenPoorVisibility";
sim_msg::TestReport_XYPlot NoOverSpeedWhenPoorVisibility::s_NoOverSpeedWhenPoorVisibility_plot;
sim_msg::TestReport_PairData NoOverSpeedWhenPoorVisibility::s_speed_variance_pair;

NoOverSpeedWhenPoorVisibility::NoOverSpeedWhenPoorVisibility() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoOverSpeedWhenPoorVisibility::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _threshold_upper_speed_m_s = 120 / 3.6;
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_speed_variance_pair);
    ReportHelper::ConfigXYPlot(s_NoOverSpeedWhenPoorVisibility_plot, "NoOverSpeedWhenPoorVisibility", "", "t", "s",
                               {"result_speed", "_cond_rain", "_cond_snow", "_cond_fog"},
                               {"m/s", "OnOff", "OnOff", "OnOff"}, 4);

    ReportHelper::ConfigXYPlotThreshold(s_NoOverSpeedWhenPoorVisibility_plot, "speed upper", 0, 0, INT32_MAX,
                                        "speed lower", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_NoOverSpeedWhenPoorVisibility_plot, {0}, true, false);
  }

  return true;
}

bool NoOverSpeedWhenPoorVisibility::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("NoOverSpeedWhenPoorVisibility");
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

  // Parm init
  _cond_rain = false;
  _cond_snow = false;
  _cond_fog = false;
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

  // 2.Set thresh value
  if (_cond_rain || _cond_snow || _cond_fog) {
    // _threshold_upper_speed_m_s = 30.0 / 3.6;
    _threshold_upper_speed_m_s = m_defaultThreshDouble / 3.6;
  }

  // 3.Get actual value (ego speed)
  _result_speed_m_s = ego_front->GetSpeed().GetNormal();

  // 4.Check Detect and report
  _detector_upper_speed_m_s.Detect(_result_speed_m_s, _threshold_upper_speed_m_s);
  VLOG_1 << "speed limit is " << _threshold_upper_speed_m_s << " m/s." << "current ego speed is " << _result_speed_m_s
         << " m/s.\n";
  VLOG_1 << "detect count : " << _detector_upper_speed_m_s.GetCount() << "\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_NoOverSpeedWhenPoorVisibility_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_NoOverSpeedWhenPoorVisibility_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_NoOverSpeedWhenPoorVisibility_plot.mutable_y_axis()->at(1).add_axis_data(_cond_rain);
    s_NoOverSpeedWhenPoorVisibility_plot.mutable_y_axis()->at(2).add_axis_data(_cond_snow);
    s_NoOverSpeedWhenPoorVisibility_plot.mutable_y_axis()->at(3).add_axis_data(_cond_fog);

    s_NoOverSpeedWhenPoorVisibility_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(
        _threshold_upper_speed_m_s);
  }
  return true;
}

bool NoOverSpeedWhenPoorVisibility::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_NoOverSpeedWhenPoorVisibility_plot.y_axis_size() > 0)
      speed_variance = ReportHelper::CalVariance(s_NoOverSpeedWhenPoorVisibility_plot.y_axis(0));
    ReportHelper::SetPairData(s_speed_variance_pair, "speed variance", std::to_string(speed_variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_NoOverSpeedWhenPoorVisibility_plot);
    ReportHelper::AddPair2Attach(*attach, s_speed_variance_pair);
  }

  return true;
}

void NoOverSpeedWhenPoorVisibility::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_result_speed_m_s);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_upper_speed_m_s, _kpi_name);
}

EvalResult NoOverSpeedWhenPoorVisibility::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector_upper_speed_m_s.GetCount());

    if (_detector_upper_speed_m_s.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check skipped");
}

bool NoOverSpeedWhenPoorVisibility::ShouldStopScenario(std::string &reason) {
  auto ret =
      _detector_upper_speed_m_s.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
