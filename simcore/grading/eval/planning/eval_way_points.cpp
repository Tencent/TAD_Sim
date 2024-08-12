// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_way_points.h"
#include "trajectory.pb.h"

namespace eval {
const char EvalWayPoints::_kpi_name[] = "Planning_WayPoint";

sim_msg::TestReport_XlsxSheet EvalWayPoints::m_waypoint_sheet;

EvalWayPoints::EvalWayPoints() {
  m_travelled.reserve(eval::const_MSPS);
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalWayPoints::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXLSXSheet(m_waypoint_sheet, "planning waypoint");
  }

  // travelled points
  m_index.set_header("index");
  m_coord.set_header("coord");
  m_has_travelled.set_header("has_passed_through");

  // init output xlsx sheet data
  m_travelled.clear();
  m_travelled_all = true;
  m_waypoints = helper.GetScenarioInfo().m_waypoints;
  for (auto i = 0; i < m_waypoints.size() && _map_mgr; ++i) {
    CPosition way_point = m_waypoints.at(i).GetPosition();
    std::string coord_str;
    _map_mgr->ENUToWGS84(way_point);
    coord_str = std::to_string(way_point.GetX()) + "," + std::to_string(way_point.GetY()) + "," +
                std::to_string(way_point.GetZ());
    m_index.add_data()->assign(std::to_string(i).c_str());
    m_coord.add_data()->assign(coord_str.c_str());
    m_has_travelled.add_data()->assign((const char *)("0"));
    m_travelled.push_back(false);
  }

  VLOG_2 << "waypoints radius thresh:" << m_defaultThreshDouble << "\n";

  return true;
}
bool EvalWayPoints::Step(eval::EvalStep &helper) {
  double travelled_all = true;

  if (IsModuleValid() && m_KpiEnabled && m_waypoints.size() > 0) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      const CPosition &ego_enu = ego_front->GetLocation().GetPosition();
      for (auto i = 0; i < m_waypoints.size() && _map_mgr; ++i) {
        const CPosition &way_point = m_waypoints.at(i).GetPosition();
        if (CEvalMath::Distance2D(ego_enu, way_point) <= m_defaultThreshDouble) {
          m_has_travelled.set_data(i, (const char *)("1"));
          m_travelled.at(i) = true;
          VLOG_1 << "ego pass through waypoint " << m_coord.data().at(i) << ", index:" << i << "\n";
        }

        travelled_all = travelled_all && m_travelled.at(i);
      }

      m_travelled_all = travelled_all;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalWayPoints::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddSheetData2XLSX(m_waypoint_sheet, m_index);
    ReportHelper::AddSheetData2XLSX(m_waypoint_sheet, m_coord);
    ReportHelper::AddSheetData2XLSX(m_waypoint_sheet, m_has_travelled);
    ReportHelper::AddXLSXSheet2Attach(*_case.add_steps()->add_attach(), m_waypoint_sheet);
  }

  return true;
}

void EvalWayPoints::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalWayPoints::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "planning waypoints check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "planning waypoints check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "planning waypoints check skipped");
}
bool EvalWayPoints::ShouldStopScenario(std::string &reason) {
  auto ret = m_travelled_all && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego has travelled all waypoints";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
