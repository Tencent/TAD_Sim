// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_rule63.h"

namespace eval {
const char EvalRule63::_kpi_name[] = "Rule63";
sim_msg::TestReport_XYPlot EvalRule63::s_rule63_plot;

EvalRule63::EvalRule63() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalRule63::Init(eval::EvalInit &helper) {
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
        s_rule63_plot, "rule63", "", "t", "s",
        {"_result", "_cond_on_crosswalk", "_cond_on_junction", "_cond_on_narrow_road", "_cond_on_tunnel",
         "_cond_on_bus_station", "_cond_has_parking_sign", "_cond_has_parking_marking"},
        {"OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff", "OnOff"}, 8);

    ReportHelper::ConfigXYPlotThreshold(s_rule63_plot, "Rule Violation ", 0, 1, 0.5, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalRule63::Step(eval::EvalStep &helper) {
  // CElapsedTime e1("EvalRule63");
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
  _cond_on_crosswalk = false;
  _cond_on_junction = false;
  _cond_on_narrow_road = false;
  _cond_on_tunnel = false;
  _cond_on_bus_station = false;
  _cond_on_bridge = false;
  _cond_on_sharp_curve = false;
  _cond_on_steep_slope = false;
  _cond_has_parking_sign = false;
  _cond_has_parking_marking = false;

  _result = 0.0;

  // 1.get the condition
  // get the crosswork
  eval::Crosswalk crosswalk_on;
  _cond_on_crosswalk = ego_front->IsOnCrosswalk(crosswalk_on);

  // whether ego is on junction
  _cond_on_junction = map_info->m_in_junction;
  VLOG_1 << "cond_on_junction: " << _cond_on_junction << ".\n";

  // whether on narrow road
  _cond_on_narrow_road = map_info->m_active_lane.m_on_narrow_lane;

  // whether is on tunnel
  _cond_on_tunnel = map_info->m_active_road.m_on_tunnel;
  VLOG_1 << "cond_on_tunnel: " << _cond_on_tunnel << "\n";

  // whether is on bus station
  eval::EvalMapObjectPtr bus_station_objs =
      _map_mgr->GetMapObjects(hadmap::OBJECT_TYPE_BusStation, ego_front->GetLocation().GetPosition());

  if (bus_station_objs) {
    const std::vector<eval::EvalPoints> &map_obj_samples = bus_station_objs->m_samples;
    for (auto i = 0; i < map_obj_samples.size(); ++i) {
      const EvalPoints &sample_points = map_obj_samples.at(i);
      for (auto j = 0; j < sample_points.size(); ++j) {
        const CLocation &obj_loc = sample_points.at(j);
        if (_map_mgr->IsInRange(ego_front->GetLocation(), obj_loc, 30.0f)) {
          _cond_on_bus_station = true;
        }
      }
    }
  }

  // whether on bridge
  _cond_on_bridge = map_info->m_active_road.m_on_bridge;
  VLOG_1 << "cond_on_bridge: " << _cond_on_bridge << "\n";

  // whether is on sharp_curve
  _cond_on_sharp_curve = map_info->m_active_road.m_on_sharp_curve;
  VLOG_1 << "cond_on_sharp_curve: " << _cond_on_sharp_curve << "\n";

  // whether is on steep_slope
  _cond_on_steep_slope = map_info->m_active_road.m_on_steep_slope;
  VLOG_1 << "cond_on_steep_slope: " << _cond_on_steep_slope << "\n";

  // check whether has_parking_sign
  _cond_has_parking_sign = map_info->m_active_road.m_has_parking_marking;

  // check whether has_parking_marking
  _cond_has_parking_marking = map_info->m_active_road.m_has_parking_sign;

  // 2.Check Detect and report
  if (_cond_has_parking_sign || _cond_has_parking_marking || _cond_on_crosswalk || _cond_on_junction ||
      _cond_on_sharp_curve || _cond_on_narrow_road || _cond_on_bridge || _cond_on_steep_slope || _cond_on_tunnel ||
      _cond_on_bus_station) {
    const VehOutput &output = helper.GetVehStateFlow().GetOutput();
    _result = (output.m_veh_behav == VehicleBehavior::Stopped);
  }

  // Detect and report
  _detector.Detect(_result, 0.5);
  VLOG_1 << "ego should no parking, current ego parking status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_rule63_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_rule63_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_rule63_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_crosswalk);
    s_rule63_plot.mutable_y_axis()->at(2).add_axis_data(_cond_on_junction);
    s_rule63_plot.mutable_y_axis()->at(3).add_axis_data(_cond_on_narrow_road);
    s_rule63_plot.mutable_y_axis()->at(4).add_axis_data(_cond_on_tunnel);
    s_rule63_plot.mutable_y_axis()->at(5).add_axis_data(_cond_on_bus_station);
    s_rule63_plot.mutable_y_axis()->at(6).add_axis_data(_cond_has_parking_sign);
    s_rule63_plot.mutable_y_axis()->at(7).add_axis_data(_cond_has_parking_marking);
  }

  return true;
}

bool EvalRule63::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_rule63_plot);
  }

  return true;
}

void EvalRule63::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRule63::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should no parking");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no parking check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "no parking check skipped");
}

bool EvalRule63::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no parking";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
