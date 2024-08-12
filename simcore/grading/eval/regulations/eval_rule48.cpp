// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule48.h"

namespace eval {
const char EvalRule48::_kpi_name[] = "Rule48";
sim_msg::TestReport_XYPlot EvalRule48::s_rule48_plot;
sim_msg::TestReport_PairData EvalRule48::s_distance_variance_pair;

EvalRule48::EvalRule48() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule48::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_rule48_plot, "rule48", "", "t", "s",
                               {"_result", "_cond_no_center_lane", "_cond_night", "_cond_on_narrow_lane",
                                "_cond_on_bridge", "_actual_beam_proper_used", "_actual_keep_safe_distance"},
                               {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 7);

    ReportHelper::ConfigXYPlotThreshold(s_rule48_plot, "Rule Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule48::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule48");
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
  _cond_no_center_lane = false;
  _cond_night = false;
  _cond_on_narrow_lane = false;
  _cond_on_bridge = false;
  _actual_beam_proper_used = true;
  _actual_keep_safe_distance = true;

  _result = 0.0;

  double _opposite_nearest_distance = 500.0;
  double _follow_nearest_distance = 500.0;

  // 1.Get the condition
  // whether there have no center lane
  _cond_no_center_lane = map_info->m_active_lane.m_no_center_lane;
  VLOG_1 << "cond_no_center_lane: " << _cond_no_center_lane << "\n";

  // wheather night
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    // get time of day
    auto time_of_day = environment_ptr->GetPartsOfDay();
    _cond_night = (time_of_day == eval::PARTS_OF_DAY_MIDNIGHT || time_of_day == eval::PARTS_OF_DAY_DAWN ||
                   time_of_day == eval::PARTS_OF_DAY_NIGHT);
  }
  VLOG_1 << "cond_night: " << _cond_night << "\n";

  // wheather narrow lane
  _cond_on_narrow_lane = map_info->m_active_lane.m_on_narrow_lane;
  VLOG_1 << "cond_on_narrow_lane: " << _cond_on_narrow_lane << "\n";

  // wheather on bridge
  _cond_on_bridge = map_info->m_active_road.m_on_bridge;
  VLOG_1 << "cond_on_bridge: " << _cond_on_bridge << "\n";

  // get distance of nearest opposite vehicle relative ego_front
  VehilceActorList &&vehicle_actors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);
  const CLocation &loc_ego = ego_front->GetLocation();
  for (auto &fellow : vehicle_actors) {
    const CLocation &loc_fellow = fellow->GetLocation();
    if (_map_mgr->IsOppositeDirection(loc_ego, loc_fellow)) {
      // calculate delta distance, see if these 2 actors may overlaps
      // todo(xingboliu): postive ane negtive, current always postive
      double dist_diff = CEvalMath::Distance3D(loc_ego.GetPosition(), loc_fellow.GetPosition());
      // VLOG_1 << "dist_diff: " << dist_diff << "\n";
      if (dist_diff < _opposite_nearest_distance) {
        _opposite_nearest_distance = dist_diff;
      }
    }
  }
  VLOG_1 << "_opposite_nearest_distance: " << _opposite_nearest_distance << "\n";

  // 3.Get actual value
  // get relative distance from ego milldle front to fellow milldle rear
  auto fellow = helper.GetLeadingVehicle();
  if (fellow) {
    CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
    CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
    EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
    _follow_nearest_distance = ego_2_fellow.GetNormal2D();
  }

  // if the car's beam proper used under conditions
  if (_cond_no_center_lane) {
    if (_cond_on_narrow_lane || _cond_on_bridge || (_cond_night && _opposite_nearest_distance <= 150.0)) {
      const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
      _actual_beam_proper_used = veh_fb.m_low_beam == OnOff::ON;
    }

    _actual_keep_safe_distance = _follow_nearest_distance < _const_min_safe_distance;
  }

  // 4.Check Detect and report
  if (!_actual_beam_proper_used || !_actual_keep_safe_distance) {
    _result = 1.0;
  }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should use low beam proper and keep safe distance, current: " << _result << ".\n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule48_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule48_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule48_plot.mutable_y_axis()->at(1).add_axis_data(_cond_no_center_lane);
    s_rule48_plot.mutable_y_axis()->at(2).add_axis_data(_cond_night);
    s_rule48_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_narrow_lane);
    s_rule48_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_bridge);
    s_rule48_plot.mutable_y_axis()->at(5).add_axis_data(_actual_beam_proper_used);
    s_rule48_plot.mutable_y_axis()->at(6).add_axis_data(_actual_keep_safe_distance);
  }

  return true;
}

bool EvalRule48::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_rule48_plot);
  }

  return true;
}

void EvalRule48::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule48::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "relative distance too low or ego should use low beam");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "beam proper used and relative distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "beam and relative distance check skipped");
}

bool EvalRule48::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "relative distance too low or ego should use low beam";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
