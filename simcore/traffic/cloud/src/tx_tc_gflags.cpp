// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_gflags.h"
#include "tx_logger.h"
#include "version.h"

DEFINE_string(tc_sim_path, "", "");
DEFINE_int32(tc_probability_sample_size, 10000, "for reproduction");
DEFINE_double(tc_exit_length, 10.0, "");
DEFINE_int32(MAX_FLOW_DURATION, 10000, "");
DEFINE_double(tc_input_safe_region_radius, 10.0, "");
DEFINE_double(tc_input_scan_vision_distance, 20.0, "");
DEFINE_double(tc_ego_input_safe_region_radius, 50.0, "");
DEFINE_bool(IgnoreEgoRadius, true, "");
DEFINE_int32(VehInputIdBase, 10000, "");
DEFINE_double(tc_zero_velocity_thresold, 3.0, "");
DEFINE_double(tc_switch_velocity_thresold, 10.0, "");
DEFINE_double(RoadClosed_Threshold, 100.0, "dead end road threshod");
DEFINE_double(RoadClosedLAST_Threshold, 20.0, "");
DEFINE_double(RoutingEnd_Distance, 300.0, "");
DEFINE_int32(tc_show_statictis_info_interval, 50, "");

DEFINE_int32(tc_random_seed, 55, "");

DEFINE_double(tc_ego_length, 4.68, "Haval H7");
DEFINE_double(tc_ego_width, 2.18, "Haval H7");
DEFINE_double(tc_ego_heigth, 1.76, "Haval H7");

DEFINE_bool(LogLevel_Cloud, true, "");
DEFINE_bool(LogLevel_Cloud_Performance, false, "");
DEFINE_bool(LogLevel_Cloud_EgoInput, false, "");
DEFINE_bool(LogLevel_VehInput, true, "");
DEFINE_bool(LogLevel_NewRoute, true, "");
DEFINE_bool(LogLevel_SimulationEvent, true, "");
DEFINE_bool(LogLevel_OdInfo, true, "");
DEFINE_bool(LogLevel_MultiRegion, false, "");
DEFINE_bool(LogLevel_CloudDitw, false, "");
DEFINE_bool(LogLevel_InjectEvent, false, "");

DEFINE_bool(need_random_obs_pedes, false, "");
DEFINE_int32(rnd_obs_row, 50, "");
DEFINE_int32(rnd_obs_col, 50, "");
DEFINE_double(rnd_obs_valid_radius, 500.0, "");
DEFINE_int32(obs_rnd_seed, 55, "");
DEFINE_double(obs_valid_section_length, 100.0, "");
DEFINE_double(pedestrian_valid_section_length, 10.0, "");
DEFINE_bool(pedestrian_show, true, "");
DEFINE_double(pedestrian_velocity, 1.2, "");
DEFINE_double(pedestrian_occurrence_dist, 30.0, "");

DEFINE_bool(cloud_ditw_use_motion_traj_mgr, false, "");
DEFINE_bool(cloud_ditw_use_linear_interpolation, false, "");
DEFINE_double(cloud_3rd_aggress, 0.5, "");
DEFINE_double(cloud_ditw_max_idle_time_in_second, 0.5, "");

DEFINE_double(cloud_ditw_switch_lng, 113.19091782, "");
DEFINE_double(cloud_ditw_switch_lat, 23.35169651, "");

DEFINE_bool(need_generate_scene_on_forecast, true, "");

DEFINE_int32(event_influnce_upstream_block_cnt, 6, "");
DEFINE_int32(event_influnce_downstream_block_cnt, 2, "");
DEFINE_double(stop_crash_inverse_kill_distance, -10.0, "");

void PrintFlagsValuesTrafficCloud() TX_NOEXCEPT {
  PrintFlagsValues_TADSim();

  LOG(INFO) << "#######  txSimTraffic Complier Version ######";
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << FILE_VERSION_STR(STR_GIT_BRANCH, STR_COMMIT_ID);
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << _ST(PRO_PATH);
  LOG(INFO) << "#######  txSimTraffic Complier Version ######";

  LOG(INFO) << TX_FLAGS(tc_sim_path);
  LOG(INFO) << TX_FLAGS(tc_probability_sample_size);
  LOG(INFO) << TX_FLAGS(tc_exit_length);
  LOG(INFO) << TX_FLAGS(MAX_VEHICLE_LIMIT_COUNT);
  LOG(INFO) << TX_FLAGS(MAX_FLOW_DURATION);
  LOG(INFO) << TX_FLAGS(tc_input_safe_region_radius);
  LOG(INFO) << TX_FLAGS(tc_input_scan_vision_distance);
  LOG(INFO) << TX_FLAGS(tc_ego_input_safe_region_radius);
  LOG(INFO) << TX_FLAGS(IgnoreEgoRadius);
  LOG(INFO) << TX_FLAGS(VehInputIdBase);
  LOG(INFO) << TX_FLAGS(tc_zero_velocity_thresold);
  LOG(INFO) << TX_FLAGS(tc_switch_velocity_thresold);
  LOG(INFO) << TX_FLAGS(RoadClosed_Threshold);
  LOG(INFO) << TX_FLAGS(RoadClosedLAST_Threshold);
  LOG(INFO) << TX_FLAGS(RoutingEnd_Distance);
  LOG(INFO) << TX_FLAGS(tc_show_statictis_info_interval);
  LOG(INFO) << TX_FLAGS(tc_random_seed);
  LOG(INFO) << TX_FLAGS(tc_ego_length);
  LOG(INFO) << TX_FLAGS(tc_ego_width);
  LOG(INFO) << TX_FLAGS(tc_ego_heigth);

  LOG(INFO) << TX_FLAGS(LogLevel_Cloud);
  LOG(INFO) << TX_FLAGS(LogLevel_Cloud_Performance);
  LOG(INFO) << TX_FLAGS(LogLevel_Cloud_EgoInput);
  LOG(INFO) << TX_FLAGS(LogLevel_VehInput);
  LOG(INFO) << TX_FLAGS(LogLevel_NewRoute);
  LOG(INFO) << TX_FLAGS(LogLevel_SimulationEvent);
  LOG(INFO) << TX_FLAGS(LogLevel_OdInfo);
  LOG(INFO) << TX_FLAGS(LogLevel_MultiRegion);
  LOG(INFO) << TX_FLAGS(LogLevel_CloudDitw);
  LOG(INFO) << TX_FLAGS(LogLevel_InjectEvent);

  LOG(INFO) << TX_FLAGS(cloud_ditw_use_motion_traj_mgr);
  LOG(INFO) << TX_FLAGS(cloud_ditw_use_linear_interpolation);
  LOG(INFO) << TX_FLAGS(cloud_3rd_aggress);
  LOG(INFO) << TX_FLAGS(cloud_ditw_max_idle_time_in_second);
  LOG(INFO) << TX_FLAGS(cloud_ditw_switch_lng);
  LOG(INFO) << TX_FLAGS(cloud_ditw_switch_lat);
  LOG(INFO) << TX_FLAGS(need_generate_scene_on_forecast);

  LOG(INFO) << TX_FLAGS(event_influnce_upstream_block_cnt);
  LOG(INFO) << TX_FLAGS(event_influnce_downstream_block_cnt);
  LOG(INFO) << TX_FLAGS(stop_crash_inverse_kill_distance);
}
