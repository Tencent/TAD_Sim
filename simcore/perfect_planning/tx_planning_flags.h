// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_flags.h"

DECLARE_bool(LogLevel_Ego_Loop);
DECLARE_bool(LogLevel_Ego_System);
DECLARE_bool(LogLevel_Ego_Element);
DECLARE_bool(LogLevel_Ego_Route);
DECLARE_bool(LogLevel_Ego_LaneChange);
DECLARE_bool(LogLevel_Ego_EnvPerception);
DECLARE_bool(LogLevel_Ego_Front);
DECLARE_bool(LogLevel_Ego_Traj);
DECLARE_bool(LogLevel_Ego_SceneEvent);
DECLARE_bool(LogLevel_Ego_Traj_ClosedLoop);

DECLARE_string(ego_map_file_path);
DECLARE_double(ego_map_local_origin_lon);
DECLARE_double(ego_map_local_origin_lat);
DECLARE_double(ego_map_local_origin_alt);
DECLARE_string(ego_scenario_file_path);
DECLARE_int32(ego_rnd_seed);
DECLARE_double(ego_aggress);
DECLARE_double(ego_max_v);
DECLARE_double(ego_lane_keep);
DECLARE_double(Ego_RefuseLaneChangeVelocity);
DECLARE_double(Ego_RefuseLaneChangeDistance);

DECLARE_double(ego_car_env_dist);
DECLARE_double(ego_pede_env_dist);
DECLARE_double(ego_obs_env_dist);
DECLARE_double(ego_signal_env_dist);

DECLARE_bool(ego_pubilc_location);
DECLARE_bool(ego_pubilc_trajectory);
DECLARE_double(ego_trajectory_duration_s);

DECLARE_bool(ego_location_closed_loop);

DECLARE_double(ego_DeadLineRadius);
DECLARE_double(ego_Signal_Reaction_Gap);

DECLARE_bool(dummy_ignore_perception);

DECLARE_bool(ego_stop_system_unlive);
#if __Control_V2__
DECLARE_string(control_topic);
#endif /*__Control_V2__*/
DECLARE_string(vehicle_interaction_topic);

DECLARE_bool(ego_pubilc_veh_interaction);

DECLARE_bool(ego_use_activate);

DECLARE_bool(ego_use_lanechange);

DECLARE_bool(use_trajectory_planning_mode);

DECLARE_bool(route_end_behavior_force_stop);
DECLARE_bool(route_end_behavior_desired_stop);

DECLARE_double(decelerate_lanechange_intense_speedfactor);
DECLARE_double(decelerate_lanechange_intense_lanekeepfactor);

DECLARE_double(decelerate_lanechange_speedfactor);
DECLARE_double(decelerate_lanechange_lanekeepfactor);

#if __Control_V2__
DECLARE_bool(LogLevel_DummyDriver_Control);
DECLARE_double(looking_distance_factor);
DECLARE_double(control_left_right_factor);
DECLARE_bool(Use_Pure_Pursuit);
DECLARE_double(wheel2steer);
DECLARE_bool(Use_Control_Debug_Object);
DECLARE_double(control_centerline_factor);
#endif /*__Control_V2__*/
DECLARE_bool(planning_const_velocity_mode);
DECLARE_bool(planning_speedlimit_mode);

extern void PrintFlagsValues_Ego() TX_NOEXCEPT;
