// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_veh_cut_out.h"

namespace eval {
const char EvalBehavVehCutOut::_kpi_name[] = "BehavVehCutOut";

sim_msg::TestReport_XYPlot _s_preCutOut_plot;

EvalBehavVehCutOut::EvalBehavVehCutOut() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalBehavVehCutOut::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(_s_preCutOut_plot, "preCutOut", "", "t", "s", {"acc", "if detect cut out"},
                               {"m/s²", "N/A"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_preCutOut_plot, "", 0, 1, 0, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalBehavVehCutOut::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      auto fellow = helper.GetLeadingVehicle();
      double _ego_speed = ego_front->GetSpeed().GetNormal();
      double _ego_acc = -0.1;
      bool on_line = false;
      if (map_info->m_on_road && fellow) {
        // calculate AABB of actor
        RectCorners &&ego_corners = fellow->TransCorners2BaseCoord();
        double dist =
            CEvalMath::Sub(ego_front->TransMiddleFront2BaseCoord(), fellow->TransMiddleRear2BaseCoord()).GetNormal2D();
        CPosition bl, tr;
        CollisionDetection::CalAABB(ego_corners, bl, tr);
        double angle = CEvalMath::YawDiff(ego_front->GetLocation().GetEuler(), fellow->GetLocation().GetEuler());
        LaneBoundry *boundry_ptr = nullptr;
        if (angle > 0.2)
          boundry_ptr = &(map_info->m_active_lane.m_left_boundry);
        else if (angle < -0.2)
          boundry_ptr = &(map_info->m_active_lane.m_right_boundry);
        if (boundry_ptr && dist <= m_defaultThreshDouble) {
          LaneBoundry &boundry = *boundry_ptr;
          VLOG_2 << "boundry_pkid::" << boundry.boundry_pkid << ", is left boudry:" << boundry.left_boundary
                 << ", lane mark type:" << boundry.lane_mark_type << "\n";
          for (auto line_point : boundry.sample_points) {
            double &&abs_dist =
                CEvalMath::AbsoluteDistance2D(line_point.GetPosition(), fellow->GetLocation().GetPosition());
            if (abs_dist >= fellow->GetBaseLength()) {
              continue;
            }
            if (!CollisionDetection::IsInsideAABB(line_point.GetPosition(), bl, tr)) continue;
            if (CollisionDetection::IsInsideRect(line_point.GetPosition(), ego_corners)) {
              on_line = true;
              break;
            }
          }
        }
        if (on_line && _ego_speed > 0.56) _ego_acc = ego_front->GetAcc().GetX();
      }
      _detector.Detect(_ego_acc, 0);
      VLOG_1 << "fellow car cross the solid,dotted,solidsolid line: " << on_line << "\n";
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_preCutOut_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_preCutOut_plot.mutable_y_axis()->at(0).add_axis_data(_ego_acc);
        _s_preCutOut_plot.mutable_y_axis()->at(1).add_axis_data(on_line);
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
bool EvalBehavVehCutOut::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_preCutOut_plot);
  }
  return true;
}

void EvalBehavVehCutOut::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalBehavVehCutOut::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "traffic cars cut out ego lane,but do not decelerate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "traffic cars cut out ego lane,decelerate pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "traffic cars cut out,deceleration check skipped");
}

bool EvalBehavVehCutOut::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "trffic cars cut out ego lane,do not decelerate";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
