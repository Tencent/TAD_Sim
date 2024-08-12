// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule78.h"

namespace eval {
const char EvalRule78::_kpi_name[] = "Rule78";
sim_msg::TestReport_XYPlot EvalRule78::s_rule78_plot;
sim_msg::TestReport_PairData EvalRule78::s_speed_variance_pair;

EvalRule78::EvalRule78() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule78::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_rule78_plot, "rule78", "", "t", "s", {"_result_speed_m_s", "_on_expressway"},
                               {"m/s", "OnOff"}, 2);
  }

  ReportHelper::ConfigXYPlotThreshold(s_rule78_plot, "speed upper", 0, 0, INT32_MAX, "speed lower", 1, 0, INT32_MIN, 0);

  ReportHelper::ConfigXYPlotThresholdClearValue(s_rule78_plot, {0}, true, true);

  return true;
}

bool EvalRule78::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule78");
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
    VLOG_1 << "map info missing.\n";
    return false;
  }

  double _result_speed_m_s = ego_front->GetSpeed().GetNormal();
  bool on_expressway = map_info->m_active_road.m_on_motorway;
  double _threshold_upper_speed_m_s = 120 / 3.6;
  double _threshold_lower_speed_m_s = -1.0;

  if (on_expressway) {
    _threshold_lower_speed_m_s = 60.0 / 3.6;
    if (map_info->m_active_lane.m_lane) {
      hadmap::txLanePtr lane_ptr = map_info->m_active_lane.m_lane;
      int lane_nums = _map_mgr->CalDrivingLaneNum(map_info->m_active_lane.m_lane->getTxLaneId());
      int lane_id = lane_ptr->getId();
      if (lane_nums == 2) {
        if (lane_id == -1) _threshold_lower_speed_m_s = 100.0 / 3.6;
      } else if (lane_nums == 3) {
        if (lane_id == -1) _threshold_lower_speed_m_s = 110.0 / 3.6;
        if (lane_id == -2) _threshold_lower_speed_m_s = 90.0 / 3.6;
      }
      double speed_limit_from_map = _map_mgr->GetSpeedLimitFromMap(lane_ptr);
      if (speed_limit_from_map >= 0.5)
        _threshold_upper_speed_m_s = std::min(_threshold_upper_speed_m_s, speed_limit_from_map);
    }
  }
  _detector_upper_speed_m_s.Detect(_result_speed_m_s, _threshold_upper_speed_m_s);
  _detector_lower_speed_m_s.Detect(_result_speed_m_s, _threshold_lower_speed_m_s);
  VLOG_1 << "speed limit is " << _threshold_lower_speed_m_s << " m/s to " << _threshold_upper_speed_m_s
         << " m/s, current ego speed is " << _result_speed_m_s << " m/s.\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule78_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule78_plot.mutable_y_axis()->at(0).add_axis_data(_result_speed_m_s);
    s_rule78_plot.mutable_y_axis()->at(1).add_axis_data(on_expressway);
    s_rule78_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(_threshold_upper_speed_m_s);
    s_rule78_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(_threshold_lower_speed_m_s);
  }

  return true;
}

bool EvalRule78::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate speed variance
    double speed_variance = 0.0;
    if (s_rule78_plot.y_axis_size() > 0) speed_variance = ReportHelper::CalVariance(s_rule78_plot.y_axis(0));
    ReportHelper::SetPairData(s_speed_variance_pair, "speed variance", std::to_string(speed_variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule78_plot);
    ReportHelper::AddPair2Attach(*attach, s_speed_variance_pair);
  }

  return true;
}

void EvalRule78::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_upper_speed_m_s, _kpi_name);
}

EvalResult EvalRule78::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector_upper_speed_m_s.GetCount() +
                                             _detector_lower_speed_m_s.GetCount());

    if (_detector_upper_speed_m_s.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max speed");
    } else if (_detector_lower_speed_m_s.GetCount() >= m_Kpi.passcondition().value() &&
               m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "below min speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "speed range check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "speed range check skipped");
}

bool EvalRule78::ShouldStopScenario(std::string &reason) {
  auto ret_upper =
      _detector_upper_speed_m_s.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  auto ret_lower =
      _detector_lower_speed_m_s.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret_upper) {
    reason = "above max speed";
  } else if (ret_lower) {
    reason = "below min speed";
  }
  _case.mutable_info()->set_request_stop(ret_upper || ret_lower);

  return (ret_upper || ret_lower);
}
}  // namespace eval
