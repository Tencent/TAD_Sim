// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_undo_lane_change.h"

namespace eval {
const char EvalUndoLaneChange::_kpi_name[] = "UndoLaneChange";

sim_msg::TestReport_XYPlot _s_undoLc_plot;

EvalUndoLaneChange::EvalUndoLaneChange() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  changing = false;
}
bool EvalUndoLaneChange::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_undoLc_plot, "", "", "t", "s", {"if undo", "if danger"}, {"N/A", "N/A"}, 2);
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
    ReportHelper::ConfigXYPlotThreshold(_s_undoLc_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
    /**
     * [optional] clear threshold value if threshold is not constants
     * @param xy_plot         TestReport_XYPlot
     * @param y_axis_ids      y_axis id list to clear
     * @param upper_clear     whether clear upper threshold
     * @param lower_clear     whether clear lower threshold
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(_s_undoLc_plot, {0}, true, false);
  }

  return true;
}

bool EvalUndoLaneChange::Step(eval::EvalStep &helper) {
  auto CheckOnLine = [this](std::vector<LaneBoundry *> boundries, CVehicleActorPtr veh) {
    // calculate AABB of actor
    bool on_line;
    RectCorners &&ego_corners = veh->TransCorners2BaseCoord();
    CPosition bl, tr;
    CollisionDetection::CalAABB(ego_corners, bl, tr);
    for (auto i = 0; i < boundries.size(); ++i) {
      const eval::LaneBoundry &boundry = *boundries.at(i);
      VLOG_2 << "boundry_pkid::" << boundry.boundry_pkid << ", is left boudry:" << boundry.left_boundary
             << ", lane mark type:" << boundry.lane_mark_type << "\n";
      for (auto line_point : boundry.sample_points) {
        double &&abs_dist = CEvalMath::AbsoluteDistance2D(line_point.GetPosition(), veh->GetLocation().GetPosition());
        if (abs_dist >= veh->GetBaseLength()) {
          continue;
        }
        if (!CollisionDetection::IsInsideAABB(line_point.GetPosition(), bl, tr)) continue;
        if (CollisionDetection::IsInsideRect(line_point.GetPosition(), ego_corners)) {
          on_line = true;
          return on_line;
        }
      }
      // if (on_line) break;
    }
    // if(on_line) break;
  };

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    bool _on_line = false;
    bool _undo = false;
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing" << "\n";
        return false;
      }
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();
      VLOG_1 << "now driving state : " << output.m_veh_behav << ", m_duration_valid : " << output.m_duration_valid
             << "\n";
      if (!changing) {
        if (output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
            output.m_veh_behav == VehicleBehavior::LaneChaning_Right) {
          changing = true;
        }
      } else {
        if (output.m_duration_valid) changing = false;
        if (output.m_veh_behav == VehicleBehavior::Driving && !output.m_duration_valid) {
          _undo = true;
          const VehilceActorList &veh_actors = helper.FindFrontAndAdjacentVehicleActor(ego_front, 20);
          VLOG_1 << "front cars num: " << veh_actors.size() << "\n";
          if (map_info->m_on_road) {
            // copy boundry pointer
            std::vector<LaneBoundry *> boundries = {&(map_info->m_active_lane.m_left_boundry),
                                                    &(map_info->m_active_lane.m_right_boundry)};
            if (veh_actors.size() > 0) {
              for (auto fellow : veh_actors) {
                _on_line = CheckOnLine(boundries, fellow);
                if (_on_line) break;
              }
            }
            if (!_on_line) {
              auto fellow = helper.GetLeadingVehicle();
              if (fellow) {
                double dist =
                    CEvalMath::Sub(ego_front->TransMiddleFront2BaseCoord(), fellow->TransMiddleRear2BaseCoord())
                        .GetNormal2D();
                if (dist < 20) {
                  _on_line = CheckOnLine(boundries, fellow);
                }
              }
            }
            if (!_on_line) {
              DynamicActorFLUList &&dynamic_actors_flu =
                  _actor_mgr->GetFellowActorsByType<CDynamicActorFLUPtr>(Actor_Dynamic_FLU);
              // 获取主车前方半径r(<200m)行人
              CDynamicActorFLUPtr actor = nullptr;
              VLOG_1 << "dynamic actors size: " << dynamic_actors_flu.size() << "\n";
              for (const auto dynamic_actor : dynamic_actors_flu) {
                if (dynamic_actor->GetType() == Actor_Dynamic_FLU &&
                    (dynamic_actor->GetLocation().GetPosition().GetNormal2D() <= 40) &&
                    (dynamic_actor->GetLocation().GetPosition().GetX() > 0)) {
                  _on_line = true;
                  break;
                }
              }
            }
          }
          VLOG_1 << "if has car change lane: " << _on_line << "\n";
          changing = false;
          _fall_detector.Detect(_on_line, 0.5);
        }
      }
      VLOG_1 << "undo change line in no danger check \n";

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_undoLc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_undoLc_plot.mutable_y_axis()->at(0).add_axis_data(_undo);
        _s_undoLc_plot.mutable_y_axis()->at(1).add_axis_data(_on_line);
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
bool EvalUndoLaneChange::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_undoLc_plot);
  }
  return true;
}

void EvalUndoLaneChange::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalUndoLaneChange::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "changing lane is safe but undo");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "undo change lane check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "undo change lane check skipped");
}

bool EvalUndoLaneChange::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above undo change lane count";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
