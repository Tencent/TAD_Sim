// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule80.h"

namespace eval {
const char EvalRule80::_kpi_name[] = "Rule80";

sim_msg::TestReport_XYPlot EvalRule80::s_rule80_plot;
sim_msg::TestReport_PairData EvalRule80::s_distance_variance_pair;

EvalRule80::EvalRule80() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalRule80::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_rule80_plot, "rule80", "", "t", "s", {"distance", "on_expressway", "speed"},
                               {"m", "OnOff", "m/s"}, 3);

    ReportHelper::ConfigXYPlotThreshold(s_rule80_plot, "", 0, 0, INT32_MAX, "distance lower", 1, 0, INT32_MIN, 0);

    ReportHelper::ConfigXYPlotThresholdClearValue(s_rule80_plot, {0}, false, true);
  }

  return true;
}

bool EvalRule80::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule80");
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

  // param init
  _cond_on_enter_expressway = false;
  _result_distance = INT16_MAX;

  // 1.get the condition
  // whether is on expressway
  _cond_on_enter_expressway = map_info->m_active_road.m_on_motorway;

  // get relative distance from ego milldle front to fellow milldle rear
  CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
  auto fellow = helper.GetLeadingVehicle();
  if (fellow && _cond_on_enter_expressway) {
    CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
    CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
    EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
    _result_distance = ego_2_fellow.GetNormal2D();
  }

  // 2.Set thresh value
  double speed_m_s = ego_front->GetSpeed().GetNormal();
  // get limit of relative distance
  if (speed_m_s > 100 / 3.6) {
    _threshold_lower_distance = 100;
  } else {
    _threshold_lower_distance = 50;
  }

  // 3.Check Detect and report
  _detector_lower_distance.Detect(_result_distance, _threshold_lower_distance);
  VLOG_1 << "_rel_dist is " << _result_distance << " m.\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    _result_distance = _result_distance > const_max_actor_size ? -1.0 : _result_distance;
    s_rule80_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule80_plot.mutable_y_axis()->at(0).add_axis_data(_result_distance);
    s_rule80_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_enter_expressway);
    s_rule80_plot.mutable_y_axis()->at(2).add_axis_data(speed_m_s);

    s_rule80_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(_threshold_lower_distance);
  }

  return true;
}

bool EvalRule80::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate distance variance
    double distance_variance = 0.0;
    if (s_rule80_plot.y_axis_size() > 0) distance_variance = ReportHelper::CalVariance(s_rule80_plot.y_axis(0));
    ReportHelper::SetPairData(s_distance_variance_pair, "speed variance", std::to_string(distance_variance));

    // add report
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule80_plot);
    ReportHelper::SetPairData(s_distance_variance_pair, "relative distance variance",
                              std::to_string(distance_variance));
  }

  return true;
}

void EvalRule80::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_lower_distance, _kpi_name);
}

EvalResult EvalRule80::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector_lower_distance.GetCount());

    if (_detector_lower_distance.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "relative distance too low");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "relative distance check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "relative distance check skipped");
}

bool EvalRule80::ShouldStopScenario(std::string &reason) {
  auto ret =
      _detector_lower_distance.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "relative distance too low";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
