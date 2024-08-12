// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_space_check.h"
#include "parking_space.pb.h"

namespace eval {
const char EvalParkingSpaceCheck::_kpi_name[] = "ParkingSpaceCheck";

sim_msg::TestReport_XYPlot EvalParkingSpaceCheck::_s_parking_space_check_plot;

EvalParkingSpaceCheck::EvalParkingSpaceCheck() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalParkingSpaceCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_parking_space_check_plot, "if park Out", "desc", "t", "s", {"if park out"}, {"N/A"},
                               1);
    /**
     * set Threshold after ConfigXYPlot
     * @param    xy_plot         TestReport_XYPlot
     * @param    upper_desc      the desc of upper threshold.
     * @param    upper_space     the IntervalSpace of upper threshold.
     * @param    upper_type      the IntervalType of upper threshold.
     * @param    upper_value     the value of upper threshold.
     * @param    lower_desc      the desc of lower threshold.
     * @param    lower_space     the IntervalSpace of lower threshold.
     * @param    lower_type      the IntervalType of lower threshold.
     * @param    lower_value     the value of lower threshold.
     * @param    y_axis_id       the id  of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(_s_parking_space_check_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}

// 检测点在多边形内部
bool EvalParkingSpaceCheck::CheckPointInPolygon(const RectCorners &corners, Eigen::Vector3d &point) {
  double pos_x = point.x();
  double pos_y = point.y();
  bool res = false;
  double vert[3][2];
  vert[0][0] = corners.at(0).x();
  vert[0][1] = corners.at(0).y();
  vert[1][0] = corners.at(1).x();
  vert[1][1] = corners.at(1).y();
  vert[2][0] = corners.at(2).x();
  vert[2][1] = corners.at(2).y();
  vert[3][0] = corners.at(3).x();
  vert[3][1] = corners.at(3).y();

  int i, j = 0;
  for (i = 0, j = 3; i < 4; j = i++) {
    if (((vert[i][1] > pos_y) != (vert[j][1] > pos_y)) &&
        (pos_x < (vert[j][0] - vert[i][0]) * (pos_y - vert[i][1]) / (vert[j][1] - vert[i][1]) + vert[i][0])) {
      res = true;
    }
  }
  return res;
}

bool EvalParkingSpaceCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      bool parking_out = true;
      hadmap::txObjectPtr parking_space = ego_front->getClosestParkingSpace(20);
      if (parking_space) {
        VLOG_1 << "checked the parking" << "\n";
        ego_front->TransCorners2BaseCoord();
        const hadmap::txCurve *parking_curve = parking_space->getGeom()->getGeometry();
        EvalPoints parking_points;
        _map_mgr->ConvertToEvalPoints(parking_curve, parking_points);
        RectCorners &&ego_corners = ego_front->TransCorners2BaseCoord();
        for (auto j = 0; j < parking_points.size(); ++j) {
          // if ego on stop line
          const CLocation &loc = parking_points.at(j);
          if (CollisionDetection::IsInsideRect(loc.GetPosition(), ego_corners)) {
            parking_out = false;
            break;
          }
        }
        VLOG_1 << "parking point size : " << parking_points.size() << "\n";
      }
      _detector.Detect(parking_out, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_parking_space_check_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_parking_space_check_plot.mutable_y_axis()->at(0).add_axis_data(parking_out);
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
bool EvalParkingSpaceCheck::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_parking_space_check_plot);
  }
  return true;
}

void EvalParkingSpaceCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSpaceCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "park out");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "park out check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "park out check skipped");
}

bool EvalParkingSpaceCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "park  not out";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
