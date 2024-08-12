// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_collision_passive.h"

namespace eval {
const char EvalCollisionPassive::_kpi_name[] = "CollisionPassive";

sim_msg::TestReport_XYPlot EvalCollisionPassive::_s_passive_collision_plot;

EvalCollisionPassive::EvalCollisionPassive() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalCollisionPassive::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _collision_type = "no collision";
    is_collision = false;
    _event_collision = false;
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_passive_collision_plot, "collsion passive", "", "t", "s", {"if collision"}, {"N/A"},
                               1);
    ReportHelper::ConfigXYPlotThreshold(_s_passive_collision_plot, "", 0, 1, 1, "", 1, 0, INT32_MIN, 0);
  }
  return true;
}

bool EvalCollisionPassive::CheckPointInPolygon(const RectCorners &corners, Eigen::Vector3d &point) {
  double pos_x = point.x();
  double pos_y = point.y();
  double vert[4][2];
  vert[0][0] = corners.at(0).x();
  vert[0][1] = corners.at(0).y();
  vert[1][0] = corners.at(1).x();
  vert[1][1] = corners.at(1).y();
  vert[2][0] = corners.at(2).x();
  vert[2][1] = corners.at(2).y();
  vert[3][0] = corners.at(3).x();
  vert[3][1] = corners.at(3).y();

  int i, j = 0;
  double cur_value = 0, last_value = 0;
  for (i = 0, j = 3; i < 4; j = i++) {
    cur_value = (pos_x - vert[j][0]) * (vert[i][1] - vert[j][1]) - (vert[i][0] - vert[j][0]) * (pos_y - vert[j][1]);
    if (i > 0 && cur_value * last_value < 1e-10) return false;
    last_value = cur_value;
  }
  return true;
}

bool EvalCollisionPassive::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    auto CheckCollisionWithVehicleFellows = [this](CEgoActorPtr ego_ptr, VehilceActorList &vehilce_actors) {
      // Calculate the collision between ego and traffic vehicle
      auto collision_actor = EvalStep::FindCollisionFellow(ego_ptr, vehilce_actors);
      if (collision_actor) {
        _event_collision = true;
        double ego_speed = ego_ptr->GetSpeed().GetNormal();
        if (ego_speed <= 0.1 + const_limit_eps) {
          is_collision = true;
          _collision_type = "Stopped_collision";
          return;
        }
        VLOG_0 << "passive collision with vechile actor whose id is " << collision_actor->GetID() << "\n";
        double collision_angle =
            CEvalMath::YawDiff(ego_ptr->GetLocation().GetEuler(), collision_actor->GetLocation().GetEuler());
        bool ego_points[4] = {0};
        bool veh_points[4] = {0};
        RectCorners ego_corners = ego_ptr->TransCorners2BaseCoord();
        RectCorners veh_corners = collision_actor->TransCorners2BaseCoord();
        for (int i = 0; i < ego_corners.size(); i++) {
          veh_points[i] = CheckPointInPolygon(ego_corners, veh_corners.at(i));
          ego_points[i] = CheckPointInPolygon(veh_corners, ego_corners.at(i));
          VLOG_0 << "veh and ego point " << i << " : " << veh_points[i] << ", " << ego_points[i] << "\n";
        }
        bool is_reverse = ego_ptr->IsReverse();
        VLOG_0 << "*****collision messages : " << "angel yaw beteween cars: " << collision_angle
               << ", ego car if reverse: " << is_reverse << "\n";
        if (collision_angle < 1.308996 + const_limit_eps && collision_angle > 0.0) {
          if (!is_reverse && (ego_points[2] || veh_points[1])) {
            _collision_type = "car and car-Rear_end(passive)";
            is_collision = true;
          } else if (is_reverse && (ego_points[0] || veh_points[3])) {
            _collision_type = "car and car-Rear_end(Reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > -1.308996 - const_limit_eps && collision_angle < const_limit_eps) {
          if (!is_reverse && (ego_points[3] || veh_points[0])) {
            _collision_type = "car and car-Rear_end(passive)";
            is_collision = true;
          } else if (is_reverse && (ego_points[1] || veh_points[2])) {
            _collision_type = "car and car-Rear_end(Reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > 1.308996 && collision_angle < 1.570796 + const_limit_eps) {
          if (!is_reverse && (ego_points[2] || veh_points[0] || veh_points[1])) {
            _collision_type = "car and car-vertical(passive)";
            is_collision = true;
          } else if (is_reverse && (ego_points[0] || veh_points[2] || veh_points[3])) {
            _collision_type = "car and car-vertical(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > 1.570796 && collision_angle < 1.832595 + const_limit_eps) {
          if (!is_reverse && (veh_points[1])) {
            _collision_type = "car and car-vertical(passive)";
            is_collision = true;
          } else if (is_reverse && (ego_points[1] || veh_points[0] || veh_points[1])) {
            _collision_type = "car and car-vertical(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > -1.570796 - const_limit_eps && collision_angle < -1.308996) {
          if (!is_reverse && (ego_points[3] || veh_points[1] || veh_points[0])) {
            _collision_type = "car and car-vertical(passive)";
            is_collision = true;
          } else if (is_reverse && (veh_points[1])) {
            _collision_type = "car and car-vertical(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > -1.832595 - const_limit_eps && collision_angle < -1.570796) {
          if (!is_reverse && (veh_points[0])) {
            _collision_type = "car and car-vertical(passive)";
            is_collision = true;
          } else if (is_reverse && (ego_points[0] || veh_points[0] || veh_points[1])) {
            _collision_type = "car and car-vertical(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > 1.832595 && collision_angle < 3.141592 + const_limit_eps) {
          if (is_reverse && (ego_points[1] || veh_points[0])) {
            _collision_type = "car and car-Frontal(passive)";
            is_collision = true;
          }
        }
        if (collision_angle > -3.141592 - const_limit_eps && collision_angle < -1.832595) {
          if (is_reverse && (ego_points[0] || veh_points[1])) {
            _collision_type = "car and car-Frontal(passive)";
            is_collision = true;
          }
        }
        if (collision_angle > 0.0 && collision_angle < 1.308996 + const_limit_eps) {
          if (!is_reverse && (veh_points[0])) {
            _collision_type = "car and car-Bevel(passive)";
            is_collision = true;
          } else if (is_reverse && (veh_points[2])) {
            _collision_type = "car and car-Bevel(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > 1.832595 && collision_angle < 3.141592 + const_limit_eps) {
          if (!is_reverse && (veh_points[1] || veh_points[3])) {
            _collision_type = "car and car-Bevel(passive)";
            is_collision = true;
          } else if (is_reverse && (veh_points[1] || veh_points[3])) {
            _collision_type = "car and car-Bevel(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > -1.308996 - const_limit_eps && collision_angle < const_limit_eps) {
          if (!is_reverse && (veh_points[1])) {
            _collision_type = "car and car-Bevel(passive)";
            is_collision = true;
          } else if (is_reverse && (veh_points[3])) {
            _collision_type = "car and car-Bevel(passive-reversing)";
            is_collision = true;
          }
        }
        if (collision_angle > -3.141592 - const_limit_eps && collision_angle < -1.832595) {
          if (!is_reverse && (veh_points[0] || veh_points[2])) {
            _collision_type = "car and car-Bevel(passive)";
            is_collision = true;
          } else if (is_reverse && (veh_points[0] || veh_points[2])) {
            _collision_type = "car and car-Bevel(passive-reversing)";
            is_collision = true;
          }
        }
      }
    };
    auto CheckCollisionWithDynamicFellows = [this, &helper](CEgoActorPtr ego_ptr, DynamicActorList &dynamic_actors) {
      // Calculate the collision between ego and obstacle
      auto collision_actor = EvalStep::FindCollisionFellow(ego_ptr, dynamic_actors);
      if (collision_actor) {
        _event_collision = true;
        double ego_speed = ego_ptr->GetSpeed().GetNormal();
        if (ego_speed <= 0.1 + const_limit_eps) {
          is_collision = true;
          _collision_type = "Stopped_collision";
          return;
        }
        VLOG_0 << "passive collision with dynamic actor whose id is " << collision_actor->GetID() << "\n";
        double collision_angle =
            std::abs(CEvalMath::YawDiff(ego_ptr->GetLocation().GetEuler(), collision_actor->GetLocation().GetEuler()));
        bool ego_points[4] = {0};
        bool dynamic_points[4] = {0};
        RectCorners ego_corners = ego_ptr->TransCorners2BaseCoord();
        RectCorners veh_corners = collision_actor->TransCorners2BaseCoord();
        for (int i = 0; i < ego_corners.size(); i++) {
          dynamic_points[i] = CheckPointInPolygon(ego_corners, veh_corners.at(i));
          ego_points[i] = CheckPointInPolygon(veh_corners, ego_corners.at(i));
          VLOG_0 << "dynamic and ego point " << i << " : " << dynamic_points[i] << ", " << ego_points[i] << "\n";
        }
        bool is_reverse = ego_ptr->IsReverse();
        VLOG_0 << "collision angel : " << collision_angle << " reversing: " << is_reverse << "\n";
        DynamicActorType dynamic_type = helper.GetDynamicActorType(collision_actor);
        std::string actor_type = "";
        if (dynamic_type == DynamicActorType::Pedestrian) {
          actor_type = "Pedestrian";
        } else if (dynamic_type == DynamicActorType::Bike) {
          actor_type = "Non_Motor";
        } else if (dynamic_type == DynamicActorType::Motor) {
          actor_type = "Motor";
        }
        if (!is_reverse && (ego_points[2] || ego_points[3])) {
          _collision_type = "Rear_end-" + actor_type;
          is_collision = true;
        }
        if (is_reverse && (ego_points[0] || ego_points[1])) {
          _collision_type = "head Collision(passive)-" + actor_type;
          is_collision = true;
        }
        if (!is_reverse && collision_angle < 1.570796 + const_limit_eps && (dynamic_points[0] || dynamic_points[1])) {
          _collision_type = "Same Direction Collision-" + actor_type;
          is_collision = true;
        }
        if (is_reverse && collision_angle > 1.570796 && (dynamic_points[0] || dynamic_points[1])) {
          _collision_type = "Opposite Collision-" + actor_type;
          is_collision = true;
        }
      }
    };
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      VehilceActorList &&vehicle_actors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);
      DynamicActorList &&dynamic_actors = _actor_mgr->GetFellowActorsByType<CDynamicActorPtr>(Actor_Dynamic);
      if (_event_collision) {
        auto collision_dynamic = EvalStep::FindCollisionFellow(ego_front, dynamic_actors);
        auto collision_vehicle = EvalStep::FindCollisionFellow(ego_front, vehicle_actors);
        if (!collision_dynamic && !collision_vehicle) {
          VLOG_0 << "Reset _event_collision, is_collision\n";
          _event_collision = false;
          is_collision = false;
        }
      }
      if (!_event_collision) {
        CheckCollisionWithVehicleFellows(ego_front, vehicle_actors);
        CheckCollisionWithDynamicFellows(ego_front, dynamic_actors);
      }
      VLOG_0 << "event collision : " << _event_collision << "\n";
      VLOG_0 << "if passive collision : " << is_collision << "\n";
      VLOG_0 << "last collision passive msg : " << _collision_type << "\n";
      _detector.Detect(is_collision, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_passive_collision_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_passive_collision_plot.mutable_y_axis()->at(0).add_axis_data(is_collision);
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
bool EvalCollisionPassive::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_passive_collision_plot);
  }
  return true;
}

void EvalCollisionPassive::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalCollisionPassive::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, _collision_type);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "collision not happen");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "collision not happen");
}

bool EvalCollisionPassive::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = _collision_type;
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
