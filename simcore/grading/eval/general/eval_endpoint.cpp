// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_endpoint.h"
#include <iomanip>
#include <iostream>

namespace eval {
const char EvalEndPoint::_kpi_name[] = "EndPoint";

sim_msg::TestReport_XYPlot s_endpoint_plot;

EvalEndPoint::EvalEndPoint() {
  _end_speed = 0.5;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalEndPoint::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();

    _end_pos_wgs84 = helper.GetScenarioInfo().m_end_point_enu;
    _endpoint_enu = helper.GetScenarioInfo().m_end_point_enu;

    _map_mgr->ENUToWGS84(_end_pos_wgs84);
    VLOG_0 << "eval | ego endpoint: " << std::setprecision(14) << std::fixed
           << " endpoint.x = " << _end_pos_wgs84.GetX() << ", endpoint.y = " << _end_pos_wgs84.GetY() << "\n";
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_endpoint_plot, "reach endpoint", "", "t", "s", {"reach endpoint"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_endpoint_plot, "thresh upper", 1, 0, 1, "", 0);
  }

  m_needParking = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "NeedParking");
  m_parkingRadius = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ParkingRadius");

  return true;
}
bool EvalEndPoint::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    auto ego_ptr = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_ptr) {
      _cur_pos_wgs84 = ego_ptr->GetLocation().GetPosition();
      _map_mgr->ENUToWGS84(_cur_pos_wgs84);
      if (helper.GetSimTime() <= helper.GetStepTime().GetSecond()) {
        VLOG_0 << "eval | ego startpoint: " << std::setprecision(14) << std::fixed
               << " startpoint.x = " << _cur_pos_wgs84.GetX() << ", startpoint.y = " << _cur_pos_wgs84.GetY() << "\n";
      }
      _end_speed = ego_ptr->GetSpeed().GetNormal2D();
      double dist_to_end = CEvalMath::Distance2D(ego_ptr->GetLocation().GetPosition(), _endpoint_enu);

      bool parking = true;
      // if (m_needParking > 0.5) parking = _end_speed <= 0.5;
      parking = (_end_speed <= (m_needParking + const_limit_eps));
      auto reached = CompareDetection<double>::EqualOrBelow(dist_to_end, m_parkingRadius);
      bool reach_endpoint = reached && parking;

      _detector.Detect(reach_endpoint, 1);
      VLOG_1 << "dist to end " << dist_to_end << " m.\n";
      if (reach_endpoint) {
        VLOG_0 << "eval | reach endpoint: " << std::setprecision(14) << std::fixed
               << " endpoint.x = " << _end_pos_wgs84.GetX() << ", endpoint.y = " << _end_pos_wgs84.GetY() << "\n";
        VLOG_0 << "eval | dist to end " << dist_to_end << " m.\n";
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        s_endpoint_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_endpoint_plot.mutable_y_axis()->at(0).add_axis_data(reach_endpoint);
      }
    } else {
      VLOG_1 << "ego actor missing.\n";
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalEndPoint::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("IsEndPointArrivaled", std::to_string(_detector.GetCount()));

  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_endpoint_plot);
  }

  return true;
}

EvalResult EvalEndPoint::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "reach endpoint");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "not reach endpoint");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "reach endpoint check skipped");
}
bool EvalEndPoint::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "reach endpoint";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalEndPoint::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_planning()->set_is_reachendpoint(_detector.GetCount() >= 1);

  msg.mutable_planning()->mutable_current_point()->set_x(_cur_pos_wgs84.GetX());
  msg.mutable_planning()->mutable_current_point()->set_y(_cur_pos_wgs84.GetY());
  msg.mutable_planning()->mutable_current_point()->set_z(_cur_pos_wgs84.GetZ());

  msg.mutable_planning()->mutable_endpoint()->set_x(_end_pos_wgs84.GetX());
  msg.mutable_planning()->mutable_endpoint()->set_y(_end_pos_wgs84.GetY());
  msg.mutable_planning()->mutable_endpoint()->set_z(_end_pos_wgs84.GetZ());
}
void EvalEndPoint::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_reachendpoint()->set_eval_value(_detector.GetCount() >= 1);
  msg.mutable_detail()->mutable_reachendpoint()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_reachendpoint()->set_is_pass(IsEvalPass()._state == sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
