// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule59.h"

namespace eval {
const char EvalRule59::_kpi_name[] = "Rule59";
sim_msg::TestReport_XYPlot EvalRule59::s_rule59_plot;

EvalRule59::EvalRule59() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule59::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule59_plot, "used low beam", "", "t", "s",
                               {"result", "_cond_night", "_cond_has_traffic_light", "_cond_on_crosswalk",
                                "_cond_on_junction", "_cond_on_bridge", "_cond_on_sharp_curve", "_cond_on_steep_slope"},
                               {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 8);

    ReportHelper::ConfigXYPlotThreshold(s_rule59_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule59::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule59");
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
  _cond_night = false;
  _cond_has_traffic_light = false;
  _cond_on_crosswalk = false;
  _cond_on_junction = false;
  _cond_on_bridge = false;
  _cond_on_sharp_curve = false;
  _cond_on_steep_slope = false;
  _actual_beam_proper_used = true;
  _actual_horn_proper_used = true;
  _result = 0.0;

  // 1.Get the condition
  // whether is on the crosswork
  eval::Crosswalk crosswalk_on;
  _cond_on_crosswalk = ego_front->IsOnCrosswalk(crosswalk_on);

  // whether is night
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    auto time_of_day = environment_ptr->GetPartsOfDay();
    _cond_night = (time_of_day == eval::PARTS_OF_DAY_MIDNIGHT || time_of_day == eval::PARTS_OF_DAY_DAWN ||
                   time_of_day == eval::PARTS_OF_DAY_NIGHT);
  }

  // whether has traffic light
  SignalLightActorList traffic_light_ptrs = _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
  _cond_has_traffic_light = traffic_light_ptrs.size() > 0;
  VLOG_1 << "cond_has_traffic_light: " << _cond_has_traffic_light << ".\n";

  // whether ego is on junction
  _cond_on_junction = map_info->m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether relative distance from ego to fellow
  double rel_dist = 0;
  // get relative distance from ego milldle front to fellow milldle rear
  auto fellow = helper.GetLeadingVehicle();
  if (fellow) {
    CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
    CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
    EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
    rel_dist = ego_2_fellow.GetNormal2D();
  }

  // 2.Check Detect and report
  // check beam
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  if (_cond_night) {
    if (_cond_on_crosswalk || _cond_on_bridge || _cond_on_steep_slope ||
        (_cond_on_junction && _cond_has_traffic_light)) {
      _actual_beam_proper_used = veh_fb.m_low_beam != OnOff::ON;
    }
  }

  // check horn
  if ((_cond_on_sharp_curve || _cond_on_steep_slope) && rel_dist < 30) {
    _actual_horn_proper_used = veh_fb.m_horn != OnOff::ON;
  }

  if (!_actual_beam_proper_used && !_actual_horn_proper_used) {
    _result = 1.0;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should use low beam and horn proper, current: " << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule59_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule59_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule59_plot.mutable_y_axis()->at(1).add_axis_data(_cond_night);
    s_rule59_plot.mutable_y_axis()->at(2).add_axis_data(_cond_has_traffic_light);
    s_rule59_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_crosswalk);
    s_rule59_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_junction);
    s_rule59_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_bridge);
    s_rule59_plot.mutable_y_axis()->at(6).add_axis_data(_cond_on_sharp_curve);
    s_rule59_plot.mutable_y_axis()->at(7).add_axis_data(_cond_on_steep_slope);
  }

  return true;
}

bool EvalRule59::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule59_plot);
  }

  return true;
}

void EvalRule59::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule59::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should proper use low beam and horn");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "beam or horn proper used check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "beam or horn check skipped");
}

bool EvalRule59::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should proper use low beam and horn";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
