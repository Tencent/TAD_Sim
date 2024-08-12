// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "stateflow/veh_state_flow.h"
#include "actors/c_actor_ego.h"

namespace eval {
// state-flow
bool VehFlow::Switch(const StateType &state_type) {
  Veh_StatePtr state = m_flow_map[state_type];
  if (state) {
    m_active_state->OnExit();
    m_active_state = state;
    m_active_state->OnEntry();
    VLOG_2 << "vehicle flow switch state to " << state_type << "\n";
    return true;
  }
  return false;
}
bool VehFlow::Init() {
  // create all states and set default state
  m_flow_map[VehStateType::Driving] = CDrive::Build(this);
  m_flow_map[VehStateType::LaneChanging] = CLaneChange::Build(this);
  m_flow_map[VehStateType::InJunction] = CInJunction::Build(this);
  m_flow_map[VehStateType::Stopped] = CStopped::Build(this);
  m_flow_map[VehStateType::Reversing] = CReverse::Build(this);

  m_active_state = m_flow_map[VehStateType::Stopped];
  m_active_state->OnEntry();

  VLOG_2 << "vehicle state-flow created.\n";

  return true;
}
bool VehFlow::Update(const VehInput &input) {
  // copy input
  m_input = input;
  // update active state
  if (m_active_state) {
    m_active_state->Update();
    VLOG_2 << "vehicle state-flow updated, current state is " << m_active_state->GetStateType() << "\n";
  }
  return true;
}
bool VehFlow::Stop() {
  // clear all states
  m_flow_map.clear();
  VLOG_2 << "vehicle state-flow stopped.\n";
  return true;
}

// operation
bool CLaneChange::OnEntry() {
  VLOG_2 << "lane changing state entry.\n";
  // get input
  const VehInput &input = m_flow->GetInput();

  // mark entry time
  m_t1 = input.m_t_s;
  m_flow->MutableOutput()->m_start_t = m_t1;

  // mark pre lane id
  m_pre_lane_id = input.m_lane_id;

  return true;
}
bool CLaneChange::Update() {
  if (m_flow) {
    VLOG_2 << "lane changing state update.\n";
    // get input
    const VehInput &input = m_flow->GetInput();
    LaneBoundry dot_boundry;
    VehBehavType lane_change_type = VehicleBehavior::LaneChaning_Left;

    // set lane change type
    bool on_dot_boundry = input.m_ego->IsOnDottedBoundry(dot_boundry);
    if (on_dot_boundry)
      lane_change_type =
          dot_boundry.left_boundary ? VehicleBehavior::LaneChaning_Left : VehicleBehavior::LaneChaning_Right;
    m_flow->MutableOutput()->SetValue(lane_change_type);

    // get ego lateral yaw
    double lateral_yaw_abs = std::fabs(input.m_ego->GetRefLineLateralYaw());

    // switch to drive mode if not on dotted line and lateral yaw is small enough, or step into junction
    if ((!on_dot_boundry && lateral_yaw_abs < m_flow->GetParams().m_lane_change_turn_thresh) ||
        (input.m_map_info && input.m_map_info->m_in_junction)) {
      VLOG_2 << "switch to state " << VehStateType::Driving << "\n";
      m_flow->Switch(VehStateType::Driving);
    }
  }
  return true;
}
bool CLaneChange::OnExit() {
  VLOG_2 << "lane changing state exit.\n";
  // input
  const VehInput &input = m_flow->GetInput();

  // update current lane id
  m_cur_lane_id = input.m_lane_id;

  // mark time when entry driving mode
  m_t2 = input.m_t_s;

  // default output
  // m_flow->MutableOutput()->SetValue(VehicleBehavior::Driving, false, 0.0);

  m_pre_lane_id.DebugShow("lane change: ");
  m_cur_lane_id.DebugShow("lane change: ");

  // if vehicled detected (lane id valid and not equal)
  if (m_pre_lane_id.IsLaneValid() && m_cur_lane_id.IsLaneValid() && !m_pre_lane_id.Equal(m_cur_lane_id)) {
    m_flow->MutableOutput()->SetValue(m_flow->GetOutput().m_veh_behav, true, m_t2 - m_t1);
  }
  return true;
}

// operation
bool CInJunction::OnEntry() {
  VLOG_2 << "in junction state entry.\n";
  if (m_flow) {
    // input
    const VehInput &input = m_flow->GetInput();

    // mark time when leave driving mode
    m_t1 = input.m_t_s;
    m_flow->MutableOutput()->m_start_t = m_t1;

    // mark lane id and location when leave driving mode
    if (input.m_map_info && input.m_map_info->m_active_lane.m_lane) {
      auto lane_id = input.m_map_info->m_active_lane.m_lane->getTxLaneId();
      m_pre_laneid.From(lane_id.roadId, lane_id.sectionId, lane_id.laneId);
      const EvalPoints &sample_points = input.m_map_info->m_active_lane.m_ref_line.sample_points;
      if (sample_points.size() > 0) m_pre_lane_loc = sample_points.front();
    }
  }
  return true;
}
bool CInJunction::Update() {
  VLOG_2 << "in junction state update.\n";
  if (m_flow) {
    // get input
    const VehInput &input = m_flow->GetInput();

    // set default output
    m_flow->MutableOutput()->SetValue(VehicleBehavior::Driving, false, 0.0);

    // if actor leave junction
    if (input.m_map_info) {
      bool leave_junction = !input.m_map_info->m_in_junction;
      bool road_changed = false;
      LaneID cur_lane_id;
      if (input.m_map_info->m_active_lane.m_lane) {
        hadmap::txLaneId &&lane_id = input.m_map_info->m_active_lane.m_lane->getTxLaneId();

        cur_lane_id.From(lane_id.roadId, lane_id.sectionId, lane_id.laneId);
        road_changed = m_pre_laneid.IsLaneValid() && cur_lane_id.IsLaneValid() &&
                       cur_lane_id.tx_road_id != m_pre_laneid.tx_road_id;
      }
      if (leave_junction || road_changed) {
        VLOG_2 << "switch to lane drive mode, leave_junction:" << leave_junction << ", road_changed;" << road_changed
               << "\n";
        m_flow->Switch(VehStateType::Driving);
      }
    }
  }
  return true;
}
bool CInJunction::OnExit() {
  VLOG_2 << "in junction state exit.\n";
  if (m_flow) {
    // input
    const VehInput &input = m_flow->GetInput();

    // update current lane id and location
    if (input.m_map_info && input.m_map_info->m_active_lane.m_lane) {
      auto lane_id = input.m_map_info->m_active_lane.m_lane->getTxLaneId();
      m_cur_laneid.From(lane_id.roadId, lane_id.sectionId, lane_id.laneId);
      const EvalPoints &sample_points = input.m_map_info->m_active_lane.m_ref_line.sample_points;
      if (sample_points.size() > 0) m_cur_lane_loc = sample_points.back();
    }

    // mark time when entry driving mode
    m_t2 = input.m_t_s;

    // default output
    m_flow->MutableOutput()->SetValue(VehicleBehavior::Driving, false, 0.0);

    // if turnd detected (pre-lane and cur-lane valid)
    if (m_cur_laneid.IsLaneValid() && m_pre_laneid.IsLaneValid() && !m_cur_laneid.Equal(m_pre_laneid)) {
      Eigen::Matrix3d &&pre_rot = m_pre_lane_loc.GetRotMatrix();
      Eigen::Matrix3d &&cur_rot = m_cur_lane_loc.GetRotMatrix();

      Eigen::Matrix3d R_XB = CEvalMath::OrientationTransXB(pre_rot, cur_rot);
      Eigen::Vector3d axis_x = {1.0, 0.0, 0.0};
      Eigen::Vector3d &&arrow = R_XB * axis_x;

      // yaw of cur-location in pre-location coordinate (orientation of location used only)
      double yaw = std::atan2(arrow[1], arrow[0]);

      VLOG_2 << "yaw:" << yaw / M_PI * 180.0
             << " deg, yaw of cur-location in pre-location coordinate (orientation of location used only).\n";

      // vector from pre-postion to cur-position
      CPosition &&Vec_XB = CEvalMath::Sub(m_cur_lane_loc.GetPosition(), m_pre_lane_loc.GetPosition());
      Eigen::Vector3d &&Vec_X = pre_rot.inverse() * Vec_XB.GetPoint();
      CPosition Vec_X_2D(Vec_X[0], Vec_X[1], 0.0);
      Vec_X_2D.Normalize();

      // yaw from pre-position to cur-position expressed in pre-location coordinate
      double yaw1 = std::atan2(Vec_X_2D.GetY(), Vec_X_2D.GetX());
      VLOG_2 << "yaw1:" << yaw1 / M_PI * 180.0
             << " deg, yaw of vector from pre-position to cur-position expressed in pre-location coordinate.\n";

      // parameters of state-flow
      const VehParams &params = m_flow->GetParams();

      /*
              @to do , consider link information, @dhu
      */

      // check if left or right turn and output (yaw diff above thresh, also need to consider link information)
      if (yaw >= params.m_U_turn_left_thresh)
        m_flow->MutableOutput()->SetValue(VehicleBehavior::U_TurnLeft, true, m_t2 - m_t1);
      else if (yaw >= params.m_left_turn_thresh)
        m_flow->MutableOutput()->SetValue(VehicleBehavior::TurnLeft, true, m_t2 - m_t1);
      else if (yaw <= params.m_U_turn_right_thresh)
        m_flow->MutableOutput()->SetValue(VehicleBehavior::U_TurnRight, true, m_t2 - m_t1);
      else if (yaw <= params.m_right_turn_thresh)
        m_flow->MutableOutput()->SetValue(VehicleBehavior::TurnRight, true, m_t2 - m_t1);
      else if (yaw1 >= params.m_left_turn_thresh)
        m_flow->MutableOutput()->SetValue(VehicleBehavior::TurnLeft, true, m_t2 - m_t1);
      else if (yaw1 <= params.m_right_turn_thresh)
        m_flow->MutableOutput()->SetValue(VehicleBehavior::TurnRight, true, m_t2 - m_t1);
    }

    return true;
  }
  return true;
}

// operation
bool CDrive::OnEntry() {
  // when entry driving mode
  if (m_flow) {
    m_junc_detector.Reset(0);
    m_dotline_detector.Reset(0);
    VLOG_2 << "lane drive state entry.\n";
    return true;
  }

  return false;
}
bool CDrive::Update() {
  if (m_flow) {
    VLOG_2 << "lane drive state update.\n";
    // get input
    const VehInput &input = m_flow->GetInput();
    const VehParams &params = m_flow->GetParams();
    LaneBoundry dot_boundry;

    // default output value
    m_flow->MutableOutput()->SetValue(VehicleBehavior::Driving, false, 0.0);

    // if switch to stop
    auto actor = input.m_ego->GetFront();
    if (actor && actor->GetSpeed().GetNormal2D() < params.m_stop_speed_thresh) {
      VLOG_2 << "switch to " << VehStateType::Stopped << "\n";
      m_flow->Switch(VehStateType::Stopped);
    } else if (input.m_map_info && m_junc_detector.Detect(input.m_map_info->m_in_junction, 1)) {
      VLOG_2 << "switch to " << VehStateType::InJunction << "\n";
      // if actor step into junction
      m_flow->Switch(VehStateType::InJunction);
    } else if (m_dotline_detector.Detect(input.m_ego->IsOnDottedBoundry(dot_boundry), 1)) {
      VLOG_2 << "switch to " << VehStateType::LaneChanging << "\n";
      // if on dotted line rise up detected, switch to lane chane state
      m_flow->Switch(VehStateType::LaneChanging);
    }
  }

  return false;
}
bool CDrive::OnExit() {
  // when leave driving mode
  if (m_flow) {
    VLOG_2 << "lane drive state exit.\n";
    return true;
  }

  return false;
}

// operation
bool CStopped::OnEntry() {
  m_rise_check.Reset(INT32_MIN);
  m_fall_check.Reset(INT32_MAX);

  // when entry
  if (m_flow) {
    const VehInput &input = m_flow->GetInput();
    if (input.m_ego && input.m_ego->GetFront()) {
      double speed = input.m_ego->GetFront()->GetSpeed().GetNormal2D();
      m_rise_check.Reset(speed);
      m_fall_check.Reset(speed);
    }

    VLOG_2 << "stopped state entry.\n";
    return true;
  }

  return false;
}
bool CStopped::Update() {
  if (m_flow) {
    VLOG_2 << "stopped state update.\n";
    // get input
    const VehInput &input = m_flow->GetInput();

    // default output value
    m_flow->MutableOutput()->SetValue(VehicleBehavior::Stopped, false, 0.0);

    // get parameters
    const VehParams &params = m_flow->GetParams();

    // terminate if nullptr
    if (input.m_ego == nullptr) return false;

    /*
     * get actor speed and check if switch to drive or stop
     * set proper state if no state-changed
     */
    auto ego_front = input.m_ego->GetFront();
    if (ego_front) {
      const CSpeed &speed = ego_front->GetSpeed();
      const CLocation &loc = ego_front->GetLocation();

      // speed in ego coordinate
      Eigen::Vector3d &&speed_veh = loc.GetRotMatrix().inverse() * speed.GetPoint();
      double speed_abs = speed.GetNormal2D();

      // switch to driving or reversing state
      if (speed_abs > params.m_stop_speed_thresh) {
        speed_veh[0] > 0.0 ? m_flow->Switch(VehStateType::Driving) : m_flow->Switch(VehStateType::Reversing);
      } else {
        // set proper state if no state-changed
        if (speed_abs < params.m_stop_speed_thresh_A) {
          m_flow->MutableOutput()->SetValue(VehicleBehavior::Stopped, false, 0.0);
        } else {
          // default state is crawling
          m_flow->MutableOutput()->SetValue(VehicleBehavior::Crawling, false, 0.0);

          // start-up or stopping
          if (m_rise_check.IsRising(speed_abs) && m_rise_check.GetOrderCounter() >= params.m_order_couner_thresh &&
              m_rise_check.GetOrderCounter() > m_rise_check.GetDisOrderCounter()) {
            m_flow->MutableOutput()->SetValue(VehicleBehavior::StartUp, false, 0.0);
          } else if (m_fall_check.IsFalling(speed_abs) &&
                     m_fall_check.GetOrderCounter() >= params.m_order_couner_thresh &&
                     m_fall_check.GetOrderCounter() > m_fall_check.GetDisOrderCounter()) {
            m_flow->MutableOutput()->SetValue(VehicleBehavior::Stopping, false, 0.0);
          }
        }
      }
    }
  }

  return false;
}
bool CStopped::OnExit() {
  // when leave
  if (m_flow) {
    VLOG_2 << "stopped state exit.\n";
    return true;
  }

  return false;
}

// operation
bool CReverse::OnEntry() {
  // when entry
  if (m_flow) {
    VLOG_2 << "reverse state entry.\n";
    return true;
  }

  return false;
}
bool CReverse::Update() {
  if (m_flow) {
    VLOG_2 << "reverse state update.\n";
    // get input
    const VehInput &input = m_flow->GetInput();
    const VehParams &params = m_flow->GetParams();

    // default output value
    m_flow->MutableOutput()->SetValue(VehicleBehavior::Reversing, false, 0.0);

    // if switch to stop
    auto actor = input.m_ego->GetFront();
    if (actor && actor->GetSpeed().GetNormal2D() < params.m_stop_speed_thresh) {
      m_flow->Switch(VehStateType::Stopped);
    }
  }

  return false;
}
bool CReverse::OnExit() {
  // when leave
  if (m_flow) {
    VLOG_2 << "reverse state exit.\n";
    return true;
  }

  return false;
}
}  // namespace eval
