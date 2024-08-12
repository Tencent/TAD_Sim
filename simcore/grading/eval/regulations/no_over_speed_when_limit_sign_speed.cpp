// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_over_speed_when_limit_sign_speed.h"

namespace eval {
const char NoOverSpeedWhenLimitSignSpeed::_kpi_name[] = "NoOverSpeedWhenLimitSignSpeed";
sim_msg::TestReport_XYPlot NoOverSpeedWhenLimitSignSpeed::s_no_over_speed_when_limit_sign_speed_plot;
sim_msg::TestReport_PairData NoOverSpeedWhenLimitSignSpeed::s_no_over_speed_when_limit_sign_speed_pair;

NoOverSpeedWhenLimitSignSpeed::NoOverSpeedWhenLimitSignSpeed() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoOverSpeedWhenLimitSignSpeed::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    limit_speed_upper = 120;
    limit_speed_lower = -4;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_no_over_speed_when_limit_sign_speed_pair);
    ReportHelper::ConfigXYPlot(s_no_over_speed_when_limit_sign_speed_plot, "overspeed check(limit sign speed) ", "",
                               "t", "s", {"result_speed"}, {"m/s"}, 1);

    ReportHelper::ConfigXYPlotThreshold(s_no_over_speed_when_limit_sign_speed_plot, "speed upper", 0, 0, INT32_MAX,
                                        "speed lower", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_no_over_speed_when_limit_sign_speed_plot, {0}, true, true);
  }

  return true;
}

bool NoOverSpeedWhenLimitSignSpeed::Step(eval::EvalStep &helper) {
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
  // Parm init
  _result_speed_m_s = ego_front->GetSpeed().GetNormal();

  // 1.Get the condition
  double speed_limit_from_map = 0.0;
  if (ego_lane) speed_limit_from_map = _map_mgr->GetSpeedLimitFromMap(ego_lane);
  VLOG_1 << "speed limit from hdmap: " << speed_limit_from_map << " m/s.\n";
  uint64_t ego_roadid = ego_front->GetLaneID().tx_road_id;
  EvalMapObjectPtr speed_limit_signs = std::make_shared<EvalMapObject>();
  ego_front->GetSpeedLimitSign(speed_limit_signs);
  for (auto &speed_sign : speed_limit_signs->m_map_objects) {
    uint64_t speed_sign_roadid = speed_sign->getRoadId();
    VLOG_2 << "ego_roadid: " << ego_roadid << ", this speed_sign_roadid: " << speed_sign_roadid << "\n";
    if (ego_roadid != speed_sign_roadid) continue;  // only care about ego's roadid's speed sign

    auto _speed_upper_ptr = speed_upper.find(speed_sign->getObjectSubType());
    auto _speed_lower_ptr = speed_lower.find(speed_sign->getObjectSubType());
    limit_speed_upper = _speed_upper_ptr == speed_upper.end() ? limit_speed_upper : _speed_upper_ptr->second;
    limit_speed_lower = _speed_lower_ptr == speed_lower.end() ? limit_speed_lower : _speed_lower_ptr->second;
    VLOG_1 << "have checked sign of speed : " << speed_sign->getObjectSubType() << "\n";
  }

  double _threshold_upper_speed_m_s = limit_speed_upper / 3.6f;
  double _threshold_lower_speed_m_s = limit_speed_lower / 3.6f;

  // 2.Set thresh value
  if (speed_limit_from_map >= 0.5) {
    _threshold_upper_speed_m_s = std::min(speed_limit_from_map, _threshold_upper_speed_m_s);
  }

  double add_limit_eps = _threshold_upper_speed_m_s + const_limit_eps;
  double reduce_limit_eps = _threshold_lower_speed_m_s - const_limit_eps;

  // 4.Check Detect and report
  _detector_upper_speed_m_s.Detect(_result_speed_m_s, add_limit_eps);
  _detector_lower_speed_m_s.Detect(_result_speed_m_s, reduce_limit_eps);
  VLOG_1 << "speed limit is " << _threshold_upper_speed_m_s << " m/s." << " current ego speed is " << _result_speed_m_s
         << " m/s.\n";

  if (!map_info->m_on_road) {
    VLOG_1 << "pass juncction, reset speed limit to 120 km/h.\n";
    limit_speed_upper = 120;
    limit_speed_lower = -4;
  }
  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_over_speed_when_limit_sign_speed_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_over_speed_when_limit_sign_speed_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_no_over_speed_when_limit_sign_speed_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(
        _threshold_upper_speed_m_s);
    s_no_over_speed_when_limit_sign_speed_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(
        _threshold_lower_speed_m_s);
  }

  return true;
}

bool NoOverSpeedWhenLimitSignSpeed::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_no_over_speed_when_limit_sign_speed_plot.y_axis_size() > 0)
      speed_variance = ReportHelper::CalVariance(s_no_over_speed_when_limit_sign_speed_plot.y_axis(0));
    ReportHelper::SetPairData(s_no_over_speed_when_limit_sign_speed_pair, "speed variance",
                              std::to_string(speed_variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_no_over_speed_when_limit_sign_speed_plot);
    ReportHelper::AddPair2Attach(*attach, s_no_over_speed_when_limit_sign_speed_pair);
  }

  return true;
}

void NoOverSpeedWhenLimitSignSpeed::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_result_speed_m_s);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_upper_speed_m_s, _kpi_name);
}

EvalResult NoOverSpeedWhenLimitSignSpeed::IsEvalPass() {
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

bool NoOverSpeedWhenLimitSignSpeed::ShouldStopScenario(std::string &reason) {
  auto ret =
      _detector_upper_speed_m_s.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
