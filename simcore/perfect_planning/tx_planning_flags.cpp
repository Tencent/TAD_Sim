// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_flags.h"
#include "tx_logger.h"
#include "version.h"

DEFINE_bool(LogLevel_Ego_Loop, false, "");
DEFINE_bool(LogLevel_Ego_System, false, "");
DEFINE_bool(LogLevel_Ego_Element, true, "");
DEFINE_bool(LogLevel_Ego_Route, false, "");
DEFINE_bool(LogLevel_Ego_LaneChange, false, "");
DEFINE_bool(LogLevel_Ego_EnvPerception, false, "");
DEFINE_bool(LogLevel_Ego_Front, false, "");
DEFINE_bool(LogLevel_Ego_Traj, false, "");
DEFINE_bool(LogLevel_Ego_SceneEvent, false, "");
DEFINE_bool(LogLevel_Ego_Traj_ClosedLoop, false, "");

DEFINE_string(ego_map_file_path, "", "tx_sim::ResetHelper::map_file_path()");
DEFINE_double(ego_map_local_origin_lon, 0.0, "tx_sim::ResetHelper::map_local_origin().x()");
DEFINE_double(ego_map_local_origin_lat, 0.0, "tx_sim::ResetHelper::map_local_origin().y()");
DEFINE_double(ego_map_local_origin_alt, 0.0, "tx_sim::ResetHelper::map_local_origin().z()");
DEFINE_string(ego_scenario_file_path, "", "tx_sim::ResetHelper::scenario_file_path()");
DEFINE_int32(ego_rnd_seed, 55, "");
DEFINE_double(ego_aggress, 0.5, "");
DEFINE_double(ego_max_v, 0.0, "");
DEFINE_double(ego_lane_keep, 5.0, "second");
DEFINE_double(Ego_RefuseLaneChangeVelocity, 0.0, "");
DEFINE_double(Ego_RefuseLaneChangeDistance, 10.0, "");

DEFINE_double(ego_car_env_dist, 50.0, "");
DEFINE_double(ego_pede_env_dist, 50.0, "");
DEFINE_double(ego_obs_env_dist, 50.0, "");
DEFINE_double(ego_signal_env_dist, 100.0, "");

DEFINE_bool(ego_pubilc_location, false, "");
DEFINE_bool(ego_pubilc_trajectory, true, "");
DEFINE_double(ego_trajectory_duration_s, 8.0, "in second");

DEFINE_bool(ego_location_closed_loop, false, "");

DEFINE_double(ego_DeadLineRadius, 1.0, "");
DEFINE_double(ego_Signal_Reaction_Gap, -100.0, "");

DEFINE_bool(dummy_ignore_perception, true, "");

DEFINE_bool(ego_stop_system_unlive, false, "");

#if __Control_V2__
DEFINE_string(control_topic, "CONTROL_DRIVER", "");
#endif /*__Control_V2__*/
DEFINE_string(vehicle_interaction_topic, "VEHICLE_INTERACTION", "");

DEFINE_bool(ego_pubilc_veh_interaction, true, "");

DEFINE_bool(ego_use_activate, true, "");

DEFINE_bool(ego_use_lanechange, true, "");

DEFINE_bool(use_trajectory_planning_mode, false, "");

DEFINE_bool(route_end_behavior_force_stop, false, "");
DEFINE_bool(route_end_behavior_desired_stop, true, "");

DEFINE_double(decelerate_lanechange_intense_speedfactor, 0.7, "");
DEFINE_double(decelerate_lanechange_intense_lanekeepfactor, 0.7, "");

DEFINE_double(decelerate_lanechange_speedfactor, 0.7, "");
DEFINE_double(decelerate_lanechange_lanekeepfactor, 0.5, "");

#if __Control_V2__
DEFINE_bool(LogLevel_DummyDriver_Control, true, "");
DEFINE_double(looking_distance_factor, 1., "second --> m/s");
DEFINE_double(control_left_right_factor, 1.0, "left theta is positive");
DEFINE_bool(Use_Pure_Pursuit, true, "");
DEFINE_double(wheel2steer, 18.0, "18.0");
DEFINE_bool(Use_Control_Debug_Object, false, "");
DEFINE_double(control_centerline_factor, 1.0, "");
#endif /*__Control_V2__*/

DEFINE_bool(planning_const_velocity_mode, false, "");
DEFINE_bool(planning_speedlimit_mode, true, "");
void PrintFlagsValues_Ego() TX_NOEXCEPT {
  PrintFlagsValues();

  LOG(INFO) << "#######  txSimPlanning Complier Version ######";
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << FILE_VERSION_STR(STR_GIT_BRANCH, STR_COMMIT_ID);
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << _ST(PRO_PATH);
  LOG(INFO) << "#######  txSimPlanning Complier Version ######";

  LOG(INFO) << "#######  Log Level ######";
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_Loop);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_System);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_Element);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_Route);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_LaneChange);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_EnvPerception);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_Front);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_Traj);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_SceneEvent);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego_Traj_ClosedLoop);
  LOG(INFO) << "#######  Log Level ######";

  LOG(INFO) << TX_FLAGS(ego_map_file_path);
  LOG(INFO) << TX_FLAGS(ego_map_local_origin_lon);
  LOG(INFO) << TX_FLAGS(ego_map_local_origin_lat);
  LOG(INFO) << TX_FLAGS(ego_map_local_origin_alt);
  LOG(INFO) << TX_FLAGS(ego_scenario_file_path);
  LOG(INFO) << TX_FLAGS(ego_rnd_seed);
  LOG(INFO) << TX_FLAGS(ego_aggress);
  LOG(INFO) << TX_FLAGS(ego_max_v);
  LOG(INFO) << TX_FLAGS(ego_lane_keep);
  LOG(INFO) << TX_FLAGS(Ego_RefuseLaneChangeVelocity);
  LOG(INFO) << TX_FLAGS(Ego_RefuseLaneChangeDistance);

  LOG(INFO) << TX_FLAGS(ego_car_env_dist);
  LOG(INFO) << TX_FLAGS(ego_pede_env_dist);
  LOG(INFO) << TX_FLAGS(ego_obs_env_dist);
  LOG(INFO) << TX_FLAGS(ego_signal_env_dist);

  LOG(INFO) << TX_FLAGS(ego_pubilc_location);
  LOG(INFO) << TX_FLAGS(ego_pubilc_trajectory);
  LOG(INFO) << TX_FLAGS(ego_trajectory_duration_s);
  LOG(INFO) << TX_FLAGS(ego_location_closed_loop);
  LOG(INFO) << TX_FLAGS(ego_DeadLineRadius);
  LOG(INFO) << TX_FLAGS(ego_Signal_Reaction_Gap);

  LOG(INFO) << TX_FLAGS(dummy_ignore_perception);
  LOG(INFO) << TX_FLAGS(ego_stop_system_unlive);

  LOG(INFO) << TX_FLAGS(control_topic);
  LOG(INFO) << TX_FLAGS(vehicle_interaction_topic);
  LOG(INFO) << TX_FLAGS(ego_pubilc_veh_interaction);
  LOG(INFO) << TX_FLAGS(ego_use_activate);
  LOG(INFO) << TX_FLAGS(use_trajectory_planning_mode);

  LOG(INFO) << TX_FLAGS(route_end_behavior_force_stop);
  LOG(INFO) << TX_FLAGS(route_end_behavior_desired_stop);

  LOG(INFO) << TX_FLAGS(decelerate_lanechange_intense_speedfactor);
  LOG(INFO) << TX_FLAGS(decelerate_lanechange_intense_lanekeepfactor);
  LOG(INFO) << TX_FLAGS(decelerate_lanechange_speedfactor);
  LOG(INFO) << TX_FLAGS(decelerate_lanechange_lanekeepfactor);

  LOG(INFO) << TX_FLAGS(LogLevel_DummyDriver_Control);
  LOG(INFO) << TX_FLAGS(looking_distance_factor);
  LOG(INFO) << TX_FLAGS(control_left_right_factor);
  LOG(INFO) << TX_FLAGS(Use_Pure_Pursuit);
  LOG(INFO) << TX_FLAGS(wheel2steer);
  LOG(INFO) << TX_FLAGS(Use_Control_Debug_Object);
  LOG(INFO) << TX_FLAGS(control_centerline_factor);

  LOG(INFO) << TX_FLAGS(planning_const_velocity_mode);
  LOG(INFO) << TX_FLAGS(planning_speedlimit_mode);
}
