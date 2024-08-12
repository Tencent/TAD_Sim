// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_end_regulate.h"

namespace eval {
const char EvalParkingEndRegulate::_kpi_name[] = "ParkingEndRegulate";

sim_msg::TestReport_XYPlot _s_pointPark_plot;

EvalParkingEndRegulate::EvalParkingEndRegulate() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalParkingEndRegulate::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    min_distance_boundary = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "Min_Dist_Boundary");
    max_distance_boundary = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "Max_Dist_Boundary");
    angel_park = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "Yaw_Lateral");
    distance_start_detect = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "Dist_Check");
    // m_xxx_x = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MsgCycleTime");
    _endpoint_enu = helper.GetScenarioInfo().m_end_point_enu;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_pointPark_plot, "standards", "", "t", "s", {"yaw", "dist_to_right", "dist_to_end"},
                               {"deg", "m", "m"}, 3);
    ReportHelper::ConfigXYPlotThreshold(_s_pointPark_plot, "max angel", 0, 1, angel_park, "", 1, 0, -angel_park, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_pointPark_plot, "max distance", 0, 1, max_distance_boundary, "", 1, 0,
                                        min_distance_boundary, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_pointPark_plot, "distance_to_end", 0, 1, m_defaultThreshDouble, "", 1, 0,
                                        INT32_MIN, 2);
  }

  return true;
}
bool EvalParkingEndRegulate::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    double dist_to_end = 0;
    if (ego_front) {
      auto _map = ego_front->GetMapInfo();
      if (_map.get() == nullptr) {
        VLOG_1 << "map ptr is null.\n";
        return false;
      }
      double dist_to_rightline = 0.0;
      double yaw = 0.0;
      CPosition point_ego = ego_front->GetRawLocation().GetPosition();
      _map_mgr->WGS84ToENU(point_ego);
      double dist_to_end = CEvalMath::Distance2D(point_ego, _endpoint_enu);
      double dist_result = 0.0;
      if (dist_to_end < distance_start_detect && ego_front->GetSpeed().GetNormal() < 0.1) {
        auto cur_lane = _map->m_active_lane.m_right_boundry;
        EvalPoints line_points = cur_lane.sample_points;
        if (line_points.size() < 2) {
          VLOG_1 << "right boundary points size below 2.\n";
          return false;
        }
        auto q = point_ego.GetPoint();
        auto p1 = line_points.at(0).GetPosition().GetPoint();
        auto p2 = line_points.at(1).GetPosition().GetPoint();
        // double offset = (p2 - p1).cross(q - p1).norm() / (p2 - p1).norm();
        auto a = p2 - p1;
        auto b = q - p1;
        double offset = std::fabs(a.x() * b.y() - b.x() * a.y()) / sqrt(a.x() * a.x() + a.y() * a.y());
        // VLOG_1 << "-----" << (p2-p1).norm() << "," << (q-p1).norm() << "," << offset << "," <<
        // (p2-p1).cross(q-p1).norm() << "\n";
        dist_to_rightline = offset - (ego_front->GetShape().GetWidth() / 2);
        dist_result = dist_to_end;
        yaw = ego_front->GetRefLineLateralYaw() * 180 / 3.14;
        bool detect_result = (dist_to_end > m_defaultThreshDouble || std::fabs(yaw) > angel_park ||
                              dist_to_rightline < min_distance_boundary || dist_to_rightline > max_distance_boundary);
        _detector.Detect(detect_result, 0.5);
      }
      if (_map->m_active_lane.m_lane) {
        double r = _map->m_active_lane.m_lane->getLaneWidth() / 2 + ego_front->GetRefLineLateralOffset() -
                   ego_front->GetShape().GetWidth() / 2;
        VLOG_1 << "yaw: " << ego_front->GetRefLineLateralYaw() << " dist_to_end: " << dist_to_end
               << " dist to right line: " << r << "\n";
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_pointPark_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_pointPark_plot.mutable_y_axis()->at(0).add_axis_data(yaw);
        _s_pointPark_plot.mutable_y_axis()->at(1).add_axis_data(dist_to_rightline);
        _s_pointPark_plot.mutable_y_axis()->at(2).add_axis_data(dist_result);
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalParkingEndRegulate::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_pointPark_plot);
  }
  return true;
}

void EvalParkingEndRegulate::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingEndRegulate::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max distance to endpoint");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max distance to endpoint check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max distance to endpoint check skipped");
}

bool EvalParkingEndRegulate::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max distance to endpoint";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
