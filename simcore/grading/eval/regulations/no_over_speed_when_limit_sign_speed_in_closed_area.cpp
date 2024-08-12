// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_over_speed_when_limit_sign_speed_in_closed_area.h"

namespace eval {
const char NoOverSpeedWhenLimitSignSpeedInClosedArea::_kpi_name[] = "NoOverSpeedWhenLimitSignSpeedInClosedArea";
sim_msg::TestReport_XYPlot
    NoOverSpeedWhenLimitSignSpeedInClosedArea::s_no_overspeed_when_limit_sign_speed_in_closed_area_plot;
sim_msg::TestReport_PairData
    NoOverSpeedWhenLimitSignSpeedInClosedArea::s_no_overspeed_when_limit_sign_speed_in_closed_area_pair;

NoOverSpeedWhenLimitSignSpeedInClosedArea::NoOverSpeedWhenLimitSignSpeedInClosedArea() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoOverSpeedWhenLimitSignSpeedInClosedArea::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_no_overspeed_when_limit_sign_speed_in_closed_area_pair);
    ReportHelper::ConfigXYPlot(s_no_overspeed_when_limit_sign_speed_in_closed_area_plot,
                               "no_overspeed_when_limit_sign_speed_in_closed_area", "", "t", "s",
                               {"result_speed", "_cond_on_private_road"}, {"m/s", "OnOff"}, 2);

    ReportHelper::ConfigXYPlotThreshold(s_no_overspeed_when_limit_sign_speed_in_closed_area_plot, "speed upper", 0, 0,
                                        INT32_MAX, "speed lower", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_no_overspeed_when_limit_sign_speed_in_closed_area_plot, {0}, false,
                                                  true);
  }

  return true;
}

bool NoOverSpeedWhenLimitSignSpeedInClosedArea::Step(eval::EvalStep &helper) {
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
  _cond_on_private_road = false;
  _result_speed_m_s = 0.0;
  _threshold_upper_speed_m_s = 120.0 / 3.6;

  // 1.Get the condition
  // get speed limit from map
  double speed_limit_from_map = _map_mgr->GetSpeedLimitFromMap(ego_lane);

  // whether is on private road
  _cond_on_private_road = map_info->m_active_road.m_on_private_road;

  // 2.Set thresh value
  // Speed limit sign && speed limit marking -> not exceed
  if (_cond_on_private_road && speed_limit_from_map >= 0.5) {
    _threshold_upper_speed_m_s = speed_limit_from_map;
  }

  // 3.Get actual value
  _result_speed_m_s = ego_front->GetSpeed().GetNormal();

  // 4.Check Detect and report
  _detector_upper_speed_m_s.Detect(_result_speed_m_s, _threshold_upper_speed_m_s);
  VLOG_1 << "speed limit is " << _threshold_upper_speed_m_s << " m/s." << " current ego speed is " << _result_speed_m_s
         << " m/s.\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_overspeed_when_limit_sign_speed_in_closed_area_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_overspeed_when_limit_sign_speed_in_closed_area_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_no_overspeed_when_limit_sign_speed_in_closed_area_plot.mutable_y_axis()->at(1).add_axis_data(
        _cond_on_private_road);

    s_no_overspeed_when_limit_sign_speed_in_closed_area_plot.mutable_y_axis()
        ->at(0)
        .mutable_threshold_upper()
        ->add_value(_threshold_upper_speed_m_s);
  }
  return true;
}

bool NoOverSpeedWhenLimitSignSpeedInClosedArea::Stop(eval::EvalStop &helper) {
  // calculate speed variance
  double speed_variance = 0.0;
  if (s_no_overspeed_when_limit_sign_speed_in_closed_area_plot.y_axis_size() > 0)
    speed_variance = ReportHelper::CalVariance(s_no_overspeed_when_limit_sign_speed_in_closed_area_plot.y_axis(0));
  ReportHelper::SetPairData(s_no_overspeed_when_limit_sign_speed_in_closed_area_pair, "speed variance",
                            std::to_string(speed_variance));

  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_no_overspeed_when_limit_sign_speed_in_closed_area_plot);
    ReportHelper::AddPair2Attach(*attach, s_no_overspeed_when_limit_sign_speed_in_closed_area_pair);
  }
  return true;
}

void NoOverSpeedWhenLimitSignSpeedInClosedArea::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_result_speed_m_s);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_upper_speed_m_s, _kpi_name);
}

EvalResult NoOverSpeedWhenLimitSignSpeedInClosedArea::IsEvalPass() {
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

bool NoOverSpeedWhenLimitSignSpeedInClosedArea::ShouldStopScenario(std::string &reason) {
  auto ret =
      _detector_upper_speed_m_s.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
