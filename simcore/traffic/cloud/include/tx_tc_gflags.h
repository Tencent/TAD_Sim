// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_tadsim_flags.h"

DECLARE_string(tc_sim_path);
DECLARE_int32(tc_probability_sample_size);
DECLARE_double(tc_exit_length);
DECLARE_int32(MAX_FLOW_DURATION);
DECLARE_double(tc_input_safe_region_radius);
DECLARE_double(tc_input_scan_vision_distance);
DECLARE_double(tc_ego_input_safe_region_radius);
DECLARE_bool(IgnoreEgoRadius);
DECLARE_int32(VehInputIdBase);
DECLARE_double(tc_zero_velocity_thresold);
DECLARE_double(tc_switch_velocity_thresold);

DECLARE_double(tc_ego_length);
DECLARE_double(tc_ego_width);
DECLARE_double(tc_ego_heigth);

DECLARE_double(RoadClosed_Threshold);
DECLARE_double(RoadClosedLAST_Threshold);
DECLARE_double(RoutingEnd_Distance);

DECLARE_int32(tc_show_statictis_info_interval);

DECLARE_int32(tc_random_seed);

DECLARE_bool(LogLevel_Cloud);
DECLARE_bool(LogLevel_Cloud_Performance);
DECLARE_bool(LogLevel_Cloud_EgoInput);
DECLARE_bool(LogLevel_VehInput);
DECLARE_bool(LogLevel_NewRoute);
DECLARE_bool(LogLevel_SimulationEvent);
DECLARE_bool(LogLevel_OdInfo);
DECLARE_bool(LogLevel_MultiRegion);
DECLARE_bool(LogLevel_CloudDitw);
DECLARE_bool(LogLevel_InjectEvent);

DECLARE_bool(need_random_obs_pedes);
DECLARE_int32(rnd_obs_row);
DECLARE_int32(rnd_obs_col);
DECLARE_double(rnd_obs_valid_radius);
DECLARE_int32(obs_rnd_seed);
DECLARE_double(obs_valid_section_length);
DECLARE_double(pedestrian_valid_section_length);

DECLARE_bool(pedestrian_show);
DECLARE_double(pedestrian_velocity);
DECLARE_double(pedestrian_occurrence_dist);

DECLARE_bool(cloud_ditw_use_motion_traj_mgr);
DECLARE_bool(cloud_ditw_use_linear_interpolation);
DECLARE_double(cloud_3rd_aggress);
DECLARE_double(cloud_ditw_max_idle_time_in_second);
DECLARE_double(cloud_ditw_switch_lng);
DECLARE_double(cloud_ditw_switch_lat);
DECLARE_bool(need_generate_scene_on_forecast);

DECLARE_int32(event_influnce_upstream_block_cnt);
DECLARE_int32(event_influnce_downstream_block_cnt);

DECLARE_double(stop_crash_inverse_kill_distance);

extern void PrintFlagsValuesTrafficCloud() TX_NOEXCEPT;
