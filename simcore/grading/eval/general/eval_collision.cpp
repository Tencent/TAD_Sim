// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_collision.h"
#include <iomanip>
#include <iostream>

namespace eval {
const char EvalCollision::_kpi_name[] = "Collision";

sim_msg::TestReport_XYPlot s_collision_plot;

EvalCollision::EvalCollision() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _collision_actor = nullptr;
}
bool EvalCollision::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_collision_plot, "collision", "", "t", "s", {"collision", "no response"},
                               {"N/A", "N/A"}, 2);
    ReportHelper::ConfigXYPlotThreshold(s_collision_plot, "thresh upper", 0, 1, 1);
    ReportHelper::ConfigXYPlotThreshold(s_collision_plot, "", 0, 1, 1.0, "", 1, 0, INT32_MIN, 1);
  }

  return true;
}

void EvalCollision::CheckCollisonTurnResponse(CEgoActorPtr ego_ptr) {
  const VehicleBodyControl &ctrl_command = ego_ptr->GetVehicleBodyCmd();
  _turn_response = (ctrl_command.m_turn_left_lamp == OnOff::ON || ctrl_command.m_turn_right_lamp == OnOff::ON);
}

void EvalCollision::CheckCollisonReduceResponse(CEgoActorPtr ego_ptr) {
  double x_acc = ego_ptr->GetAcc().GetX();
  if (x_acc > -1.0) {
    _reduce_response = false;
  }
}

bool EvalCollision::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_if_collision(sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  _collision_actor = nullptr;
  _is_collision = false;
  _turn_response = true;
  _reduce_response = true;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    auto ego_trailer = _actor_mgr->GetEgoTrailerActorPtr();

    DynamicActorList &&dynamic_actors = _actor_mgr->GetFellowActorsByType<CDynamicActorPtr>(Actor_Dynamic);
    VehilceActorList &&vehicle_actors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);
    StaticActorList &&static_actors = _actor_mgr->GetFellowActorsByType<CStaticActorPtr>(Actor_Static);

    auto CheckCollisionWithFellows = [this](CEgoActorPtr ego_ptr, DynamicActorList &dynamic_actors,
                                            VehilceActorList &vehilce_actors, StaticActorList &static_actors) {
      auto collision_actor = EvalStep::FindCollisionFellow(ego_ptr, dynamic_actors);
      if (collision_actor) {
        _collision_actor = collision_actor;
        eval::CPosition c_actor84 = _collision_actor->GetLocation().GetPosition();
        eval::CPosition e_actor84 = ego_ptr->GetLocation().GetPosition();
        eval::CPosition c_actor84_raw = _collision_actor->GetRawLocation().GetPosition();
        eval::CPosition e_actor84_raw = ego_ptr->GetRawLocation().GetPosition();
        double abs_dist = CEvalMath::Distance2D(e_actor84, c_actor84);

        _map_mgr->ENUToWGS84(c_actor84);
        _map_mgr->ENUToWGS84(e_actor84);

        VLOG_0 << "eval | collision dist = " << std::setprecision(14) << std::fixed << abs_dist;
        VLOG_0 << "eval | ego collision: " << "; sim_t = " << ego_ptr->GetSimTime().GetSecond() << std::setprecision(14)
               << std::fixed << "; bbx loc.x = " << e_actor84.GetX() << "; bbx loc.y = " << e_actor84.GetY()
               << "; bbx len = " << ego_ptr->GetShape().GetLength() << "; bbx wid = " << ego_ptr->GetShape().GetWidth()
               << "; raw loc.x = " << e_actor84_raw.GetX() << "; raw loc.y = " << e_actor84_raw.GetY() << "\n";
        VLOG_0 << "eval | collision with dynamic actor whose id is " << _collision_actor->GetID()
               << "; sim_t = " << _collision_actor->GetSimTime().GetSecond() << std::setprecision(14) << std::fixed
               << "; bbx loc.x = " << c_actor84.GetX() << "; bbx loc.y = " << c_actor84.GetY()
               << "; bbx len = " << _collision_actor->GetShape().GetLength()
               << "; bbx wid = " << _collision_actor->GetShape().GetWidth() << "; raw loc.x = " << c_actor84_raw.GetX()
               << "; raw loc.y = " << c_actor84_raw.GetY() << "\n";
        _collision_type = "collision with dynamic actors include motor or pedestrian...";
        CheckCollisonReduceResponse(ego_ptr);
      }

      collision_actor = EvalStep::FindCollisionFellow(ego_ptr, vehilce_actors);
      if (collision_actor) {
        _collision_actor = collision_actor;
        eval::CPosition c_actor84 = _collision_actor->GetLocation().GetPosition();
        eval::CPosition e_actor84 = ego_ptr->GetLocation().GetPosition();
        eval::CPosition c_actor84_raw = _collision_actor->GetRawLocation().GetPosition();
        eval::CPosition e_actor84_raw = ego_ptr->GetRawLocation().GetPosition();

        double abs_dist = CEvalMath::Distance2D(e_actor84, c_actor84);

        _map_mgr->ENUToWGS84(c_actor84);
        _map_mgr->ENUToWGS84(e_actor84);

        VLOG_0 << "eval | collision dist = " << std::setprecision(14) << std::fixed << abs_dist;
        VLOG_0 << "eval | ego collision: " << "; sim_t = " << ego_ptr->GetSimTime().GetSecond() << std::setprecision(14)
               << std::fixed << "; bbx loc.x = " << e_actor84.GetX() << "; bbx loc.y = " << e_actor84.GetY()
               << "; bbx len = " << ego_ptr->GetShape().GetLength() << "; bbx wid = " << ego_ptr->GetShape().GetWidth()
               << "; raw loc.x = " << e_actor84_raw.GetX() << "; raw loc.y = " << e_actor84_raw.GetY() << "\n";
        VLOG_0 << "eval | collision with vehicle actor whose id is " << _collision_actor->GetID()
               << "; sim_t = " << _collision_actor->GetSimTime().GetSecond() << std::setprecision(14) << std::fixed
               << "; bbx loc.x = " << c_actor84.GetX() << "; bbx loc.y = " << c_actor84.GetY()
               << "; bbx len = " << _collision_actor->GetShape().GetLength()
               << "; bbx wid = " << _collision_actor->GetShape().GetWidth() << "; raw loc.x = " << c_actor84_raw.GetX()
               << "; raw loc.y = " << c_actor84_raw.GetY() << "\n";

        double collision_angle =
            std::abs(CEvalMath::YawDiff(ego_ptr->GetLocation().GetEuler(), collision_actor->GetLocation().GetEuler()));
        if (collision_angle < 0.2618) {  // [-15,15] (0.2618)
          CPosition fellow_enu = collision_actor->GetLocation().GetPosition();
          Eigen::Vector3d &&G_Vec = fellow_enu.GetPoint();
          const Eigen::Matrix3d &R_GB = ego_ptr->GetLocation().GetRotMatrix();
          const Eigen::Matrix3d &R_BG = R_GB.inverse();
          Eigen::Vector3d &&B_vec = R_BG * G_Vec;
          if (B_vec[0] > 0) {
            _collision_type = "car and car-Rear_end ";
          } else {
            _collision_type = "car and car-Rear_ended ";
          }
        } else if (collision_angle > 1.309 && collision_angle < 1.833) {  // [-105,-75] or [75,105] (1.309,1.833)
          _collision_type = "car and car-Vertical ";
          CheckCollisonTurnResponse(ego_ptr);
        } else if (collision_angle > 2.879 && collision_angle < 3.142) {  // [-180,-165] or [165,180] (2.879,3.142)
          _collision_type = "car and car-Frontal collision ";
          CheckCollisonTurnResponse(ego_ptr);
        } else {  // [-75,-15] or [15,75] or [-165,-105] or [105,165]
          _collision_type = "car and car-Bevel collision ";
          CheckCollisonTurnResponse(ego_ptr);
        }
        CheckCollisonReduceResponse(ego_ptr);
      }

      auto collision_static_actor = EvalStep::FindCollisionFellow(ego_ptr, static_actors);
      if (collision_static_actor) {
        _collision_actor = collision_static_actor;
        eval::CPosition c_actor84 = _collision_actor->GetLocation().GetPosition();
        eval::CPosition e_actor84 = ego_ptr->GetLocation().GetPosition();
        eval::CPosition c_actor84_raw = _collision_actor->GetRawLocation().GetPosition();
        eval::CPosition e_actor84_raw = ego_ptr->GetRawLocation().GetPosition();

        double abs_dist = CEvalMath::Distance2D(e_actor84, c_actor84);

        _map_mgr->ENUToWGS84(c_actor84);
        _map_mgr->ENUToWGS84(e_actor84);

        VLOG_0 << "eval | collision dist = " << std::setprecision(14) << std::fixed << abs_dist;
        VLOG_0 << "eval | ego collision: " << "; sim_t = " << ego_ptr->GetSimTime().GetSecond() << std::setprecision(14)
               << std::fixed << "; bbx loc.x = " << e_actor84.GetX() << "; bbx loc.y = " << e_actor84.GetY()
               << "; bbx len = " << ego_ptr->GetShape().GetLength() << "; bbx wid = " << ego_ptr->GetShape().GetWidth()
               << "; raw loc.x = " << e_actor84_raw.GetX() << "; raw loc.y = " << e_actor84_raw.GetY() << "\n";
        VLOG_0 << "eval | collision with static actor whose id is " << _collision_actor->GetID()
               << "; sim_t = " << _collision_actor->GetSimTime().GetSecond() << std::setprecision(14) << std::fixed
               << "; bbx loc.x = " << c_actor84.GetX() << "; bbx loc.y = " << c_actor84.GetY()
               << "; bbx len = " << _collision_actor->GetShape().GetLength()
               << "; bbx wid = " << _collision_actor->GetShape().GetWidth() << "; raw loc.x = " << c_actor84_raw.GetX()
               << "; raw loc.y = " << c_actor84_raw.GetY() << "\n";
        _collision_type = "Static-Obstacles collision";
        CheckCollisonReduceResponse(ego_ptr);
        CheckCollisonTurnResponse(ego_ptr);
      }

      {
        EvalMapStatActorPtr map_stat_actor =
            _map_mgr->GetMapStatActors(hadmap::OBJECT_TYPE_Pole, ego_ptr->GetLocation().GetPosition());
        bool is_collison = false;
        if (map_stat_actor) {
          is_collison = is_collison || EvalStep::FindCollisionObject(ego_ptr, map_stat_actor->m_map_actors);
        }
        map_stat_actor =
            _map_mgr->GetMapStatActors(hadmap::OBJECT_TYPE_Building, ego_ptr->GetLocation().GetPosition(), 30);
        if (map_stat_actor) {
          is_collison = is_collison || EvalStep::FindCollisionObject(ego_ptr, map_stat_actor->m_map_actors);
        }
        map_stat_actor = _map_mgr->GetMapStatActors(hadmap::OBJECT_TYPE_Ditch, ego_ptr->GetLocation().GetPosition());
        if (map_stat_actor) {
          is_collison = is_collison || EvalStep::FindCollisionObject(ego_ptr, map_stat_actor->m_map_actors);
        }
        map_stat_actor = _map_mgr->GetMapStatActors(hadmap::OBJECT_TYPE_Surface, ego_ptr->GetLocation().GetPosition());
        if (map_stat_actor) {
          is_collison = is_collison || EvalStep::FindCollisionObject(ego_ptr, map_stat_actor->m_map_actors);
        }
        map_stat_actor = _map_mgr->GetMapStatActors(hadmap::OBJECT_TYPE_Tree, ego_ptr->GetLocation().GetPosition());
        if (map_stat_actor) {
          is_collison = is_collison || EvalStep::FindCollisionObject(ego_ptr, map_stat_actor->m_map_actors);
        }
        map_stat_actor =
            _map_mgr->GetMapStatActors(hadmap::OBJECT_TYPE_Obstacle, ego_ptr->GetLocation().GetPosition(), 30);
        if (map_stat_actor) {
          is_collison = is_collison || EvalStep::FindCollisionObject(ego_ptr, map_stat_actor->m_map_actors);
        }
        if (is_collison) {
          _is_collision = is_collison;
          _collision_type = "collision with map objects";
          CheckCollisonReduceResponse(ego_ptr);
          CheckCollisonTurnResponse(ego_ptr);
        }
      }
    };

    if (ego_front) CheckCollisionWithFellows(ego_front, dynamic_actors, vehicle_actors, static_actors);
    if (ego_trailer) CheckCollisionWithFellows(ego_trailer, dynamic_actors, vehicle_actors, static_actors);

    // grading
    if (_detector.Detect(_collision_actor != nullptr || _is_collision, 1)) {
      grading.mutable_event_detector()->set_if_collision(sim_msg::Grading_EventDetector_EventState_EventDetected);
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      s_collision_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_collision_plot.mutable_y_axis()->at(0).add_axis_data(_collision_actor != nullptr || _is_collision);
      s_collision_plot.mutable_y_axis()->at(1).add_axis_data(!(_reduce_response && _turn_response));
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalCollision::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("Collision", std::to_string(_detector.GetCount()));
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_collision_plot);
  }
  return true;
}

EvalResult EvalCollision::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "collision");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "collision check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "collision check skipped");
}
bool EvalCollision::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "collision";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalCollision::SetGradingMsg(sim_msg::Grading &msg) {
  if (_collision_actor) {
    auto collision_ptr = msg.mutable_collision()->Add();
    CPosition ego_pos = _collision_actor->GetLocation().GetPosition();

    _map_mgr->ENUToWGS84(ego_pos);

    collision_ptr->mutable_location()->set_x(ego_pos.GetX());
    collision_ptr->mutable_location()->set_y(ego_pos.GetY());
    collision_ptr->mutable_location()->set_z(ego_pos.GetZ());

    auto ptr = dynamic_cast<CVehicleActorPtr>(_collision_actor);
    if (ptr) {
      collision_ptr->set_state(sim_msg::GRADING_COLLISION_VEHICLE);
    } else {
      collision_ptr->set_state(sim_msg::GRADING_COLLISION_PEDESTRIAN);
    }
  }

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalCollision::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_collision()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_collision()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_collision()->set_is_pass(IsEvalPass()._state == sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
