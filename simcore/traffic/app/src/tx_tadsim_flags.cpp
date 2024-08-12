// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tadsim_flags.h"
#include "tx_logger.h"
#include "version.h"

DEFINE_int32(MAX_LANE_COUNT, 10, "");
// ------------------ ManualVehicle START  --------------------
#if Use_ManualVehicle
DEFINE_bool(ManualVehicle, false, "");
DEFINE_string(ManualVehicleTopicName, "TX_Manual_Vehicle", "");
#endif /*Use_ManualVehicle*/
// ------------------ ManualVehicle END    --------------------
DEFINE_double(Enable_LaneChange_Length, 20.0, "RouteAI");

// ----------------- Log2Worldsim Start ----------------------------
DEFINE_string(L2W_EgoName, "egocar", "");
DEFINE_string(L2W_EgoProperty, "external", "");
DEFINE_int32(L2W_Hertz, 100, "");
DEFINE_double(L2W_DisengageTime, 100000.0, "switch state from l2w to ai");
DEFINE_double(L2W_TrajSimplifyDistance, 15.0, "boost geometry line_interpolate max_distance");
// ----------------- Log2Worldsim End   ----------------------------.

DEFINE_bool(LogLevel_TADSim_Loop, GeneralCheckBool, "");
DEFINE_bool(LogLevel_AI_EnvPercption, LogSurroundInfo, "");
DEFINE_bool(LogLevel_AI_Routing, GeneralCheckBool, "");
DEFINE_bool(LogLevel_AI_Arterial, false, "");
DEFINE_bool(LogLevel_TrajFollow, false, "");
DEFINE_bool(LogLevel_RelativeTrajectoryFollow, false, "");
DEFINE_bool(LogLevel_Preview, false, "");
DEFINE_bool(LogLevel_XOSC_Replay, false, "");
DEFINE_bool(LogLevel_MapObject, false, "");
DEFINE_bool(LogLevel_MapFile, false, "");
DEFINE_bool(LogLevel_Junction, false, "");
DEFINE_bool(LogLevel_DebugVertex, false, "");
DEFINE_bool(LogLevel_Sketch, false, "");
DEFINE_bool(LogLevel_Curve2Acc, false, "");
DEFINE_bool(LogLevel_L2W_EgoInfo, false, "");

DEFINE_double(vehicle_input_safe_region_radius, 10.0, "");
DEFINE_double(vehicle_input_scan_vision_distance, 20.0, "");

DEFINE_bool(XOSC_Replay_With_EgoCar, false, "");

DEFINE_double(OnLink_SameTo_QueryRadius, 25.0, "");
DEFINE_bool(TADSimRouteEndStop, true, "hzdemo 1180 stop when routegroup end.");

DEFINE_double(event_99930_99931_trigger_time, 1.0, "");
DEFINE_double(event_99930_heading, 40.0, "");
DEFINE_double(event_99931_heading, -37.0, "");
DEFINE_double(event_99930_99931_renew_time, 60.0, "");
DEFINE_double(event_99930_99931_renew_velocity, 15.0, "");
DEFINE_double(event_99930_99931_influnce_distance, 150.0, "");

DEFINE_double(TAD_FCW_Radius, 200.0, "");

DEFINE_bool(EnableRearComfortGap, true, "");
DEFINE_string(L2W_Switch_TopicName, ".log2world_trigger", "");
DEFINE_string(L2W_RawTraffic_TopicName, "TRAFFIC_REPLAY", "");
DEFINE_bool(LogLevel_L2W, false, "");
DEFINE_double(L2W_Switch_SpecialTime, -1.0, "");
DEFINE_bool(L2W_Vehicle_Position_Linear_Interpolation, true, "l2w scene vehicle replay with smooth interpolation.");
DEFINE_bool(L2W_Ego_Position_Linear_Interpolation, true, "");
DEFINE_bool(L2W_AvoidSuddenlyAppear, true, "car suddenly appear in front of lane.");
DEFINE_double(L2W_LoseFrameThreshold, 2000.0, "duration great than 2 sec. two same-id vehicle.");
DEFINE_bool(L2W_SwitchAbsorb, false, "");
DEFINE_double(L2W_Motion_Dist, 10.0, "");
DEFINE_string(L2W_RawLocation_TopicName, "LOCATION_REPLAY", "");
DEFINE_string(L2W_RawTrajectory_TopicName, "TRAJECTORY_REPLAY", "");

DEFINE_bool(EnableTrafficVisionFilter, false, "");
DEFINE_double(TrafficVisionFilterRadius, 100000.0, "");
DEFINE_double(TrafficVisionFilterAltitudeDiff, 3.0, "");

DEFINE_double(SignalQueryRaidus, 2.0, "");
DEFINE_string(Traffic_TopicName, "TRAFFIC", R"(const std::string kTraffic = "TRAFFIC")");
DEFINE_string(Location_TopicName, "LOCATION", R"(const std::string kLocation = "LOCATION")");
DEFINE_string(TrailerEgo_TopicName, "LOCATION_TRAILER", "");

DEFINE_string(EgoUnion_TopicName, "EgoUnion/LOCATION", "");
DEFINE_string(EgoUnion_Trailer_TopicName, "EgoUnion/LOCATION_TRAILER", "");
DEFINE_string(EgoUnion_Specified_TopicName, ".hightlight_group", "");
DEFINE_string(EgoUnion_Specified_EgoGroup, "", "");

DEFINE_double(TAD_Obs_Subdivide_Threshold, 5.0, "");
DEFINE_string(PreviewCachePath, ".", "");

DEFINE_bool(USE_MAP_OBJECT, true, "");

DEFINE_bool(LogsimPub_Location, true, "");
DEFINE_string(LOGSIM_RawLocation_TopicName, "LOCATION_REPLAY", "");
DEFINE_bool(LogsimPub_Traffic, true, "");
DEFINE_string(LOGSIM_RawTraffic_TopicName, "TRAFFIC", "");
DEFINE_bool(LogsimPub_Trajectory, true, "");
DEFINE_string(LOGSIM_RawTrajectory_TopicName, "TRAJECTORY_REPLAY", "");
DEFINE_bool(Logsim_FrameInterpolation, true, "");
DEFINE_bool(Logsim_HeadingInterpolation, false, "");

DEFINE_double(AI_NonRoute_LaneChangeThreshold, 0.4, "");

DEFINE_double(L2W_StopRouteLen, 10.0, "");

DEFINE_double(EgoInfo_AngleCenterLine_LeftAngle, 0.0, "");
DEFINE_double(EgoInfo_AngleCenterLine_RightAngle, 360.0, "");
DEFINE_double(TrajStartAngleDuration, 2.0, "");
DEFINE_bool(VehTrajEventTrigger, false, "ID112321149");

DEFINE_double(_74_param_1, 0.0, "0.0");
DEFINE_double(_74_param_2, -0.05, "-0.05");
DEFINE_double(_74_param_3, 1000.0, "300.0");

DEFINE_bool(Enable_Sketch, false, "");
DEFINE_double(roadsurface_water_radius, 20.0, "");
DEFINE_double(sign_radius, 30.0, "");
DEFINE_int32(ego_cutin_history_traj_size, 50, "");
DEFINE_double(ego_3_2_lanechange_dist, 15.0, "");

DEFINE_double(curve2acc_a, 1.0, "");
DEFINE_double(curve2acc_b, 0.0, "");
DEFINE_double(curve2acc_active_factor, 0.5, "");

DEFINE_bool(Enable_Road_Limit, true, "");
DEFINE_double(Speed_Max_OnCurve, 10.0, "");
DEFINE_double(StaticLaneChange_Acc, 2.0, "");

DEFINE_string(ConsistencyRecPath, "D:/work/traffic/consisty", "");

void PrintFlagsValues_TADSim() TX_NOEXCEPT {
  PrintFlagsValues();

  LOG(INFO) << "#######  txSimTraffic Complier Version ######";
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << FILE_VERSION_STR(STR_GIT_BRANCH, STR_COMMIT_ID);
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << _ST(PRO_PATH);
  LOG(INFO) << "#######  txSimTraffic Complier Version ######";

  LOG(INFO) << TX_FLAGS(MAX_LANE_COUNT);
  LOG(INFO) << TX_FLAGS(ManualVehicle);
  LOG(INFO) << TX_FLAGS(ManualVehicleTopicName);
  LOG(INFO) << TX_FLAGS(Enable_LaneChange_Length);
  LOG(INFO) << TX_FLAGS(vehicle_input_safe_region_radius);
  LOG(INFO) << TX_FLAGS(vehicle_input_scan_vision_distance);

  LOG(INFO) << TX_FLAGS(LogLevel_TADSim_Loop);
  LOG(INFO) << TX_FLAGS(LogLevel_AI_EnvPercption);
  LOG(INFO) << TX_FLAGS(LogLevel_AI_Routing);
  LOG(INFO) << TX_FLAGS(LogLevel_AI_Arterial);

  // ----------------- Log2Worldsim Start ----------------------------
  LOG(INFO) << TX_FLAGS(L2W_EgoName);
  LOG(INFO) << TX_FLAGS(L2W_EgoProperty);
  LOG(INFO) << TX_FLAGS(L2W_Hertz);
  LOG(INFO) << TX_FLAGS(L2W_DisengageTime);
  LOG(INFO) << TX_FLAGS(L2W_TrajSimplifyDistance);
  LOG(INFO) << TX_FLAGS(LogLevel_RelativeTrajectoryFollow);
  LOG(INFO) << TX_FLAGS(LogLevel_Preview);
  LOG(INFO) << TX_FLAGS(LogLevel_MapObject);
  LOG(INFO) << TX_FLAGS(LogLevel_MapFile);
  LOG(INFO) << TX_FLAGS(LogLevel_Junction);
  LOG(INFO) << TX_FLAGS(LogLevel_DebugVertex);
  LOG(INFO) << TX_FLAGS(LogLevel_Sketch);
  LOG(INFO) << TX_FLAGS(LogLevel_Curve2Acc);
  LOG(INFO) << TX_FLAGS(LogLevel_L2W_EgoInfo);
  // ----------------- Log2Worldsim End   ----------------------------

  LOG(INFO) << TX_FLAGS(LogLevel_XOSC_Replay);

  LOG(INFO) << TX_FLAGS(XOSC_Replay_With_EgoCar);
  LOG(INFO) << TX_FLAGS(LogLevel_TrajFollow);

  LOG(INFO) << TX_FLAGS(OnLink_SameTo_QueryRadius);
  LOG(INFO) << TX_FLAGS(TADSimRouteEndStop);

  LOG(INFO) << TX_FLAGS(event_99930_99931_trigger_time);
  LOG(INFO) << TX_FLAGS(event_99930_heading);
  LOG(INFO) << TX_FLAGS(event_99931_heading);
  LOG(INFO) << TX_FLAGS(event_99930_99931_renew_time);
  LOG(INFO) << TX_FLAGS(event_99930_99931_renew_velocity);
  LOG(INFO) << TX_FLAGS(event_99930_99931_influnce_distance);

  LOG(INFO) << TX_FLAGS(TAD_FCW_Radius);
  LOG(INFO) << TX_FLAGS(EnableRearComfortGap);
  LOG(INFO) << TX_FLAGS(L2W_Switch_TopicName);
  LOG(INFO) << TX_FLAGS(L2W_RawTraffic_TopicName);
  LOG(INFO) << TX_FLAGS(LogLevel_L2W);
  LOG(INFO) << TX_FLAGS(L2W_Switch_SpecialTime);
  LOG(INFO) << TX_FLAGS(L2W_Vehicle_Position_Linear_Interpolation);
  LOG(INFO) << TX_FLAGS(L2W_Ego_Position_Linear_Interpolation);
  LOG(INFO) << TX_FLAGS(L2W_AvoidSuddenlyAppear);
  LOG(INFO) << TX_FLAGS(L2W_LoseFrameThreshold);
  LOG(INFO) << TX_FLAGS(L2W_SwitchAbsorb);
  LOG(INFO) << TX_FLAGS(L2W_Motion_Dist);
  LOG(INFO) << TX_FLAGS(L2W_RawLocation_TopicName);
  LOG(INFO) << TX_FLAGS(L2W_RawTrajectory_TopicName);

  LOG(INFO) << TX_FLAGS(EnableTrafficVisionFilter);
  LOG(INFO) << TX_FLAGS(TrafficVisionFilterRadius);
  LOG(INFO) << TX_FLAGS(TrafficVisionFilterAltitudeDiff);
  LOG(INFO) << TX_FLAGS(SignalQueryRaidus);

  LOG(INFO) << TX_FLAGS(Traffic_TopicName);
  LOG(INFO) << TX_FLAGS(Location_TopicName);
  LOG(INFO) << TX_FLAGS(TrailerEgo_TopicName);

  LOG(INFO) << TX_FLAGS(EgoUnion_TopicName);
  LOG(INFO) << TX_FLAGS(EgoUnion_Trailer_TopicName);
  LOG(INFO) << TX_FLAGS(EgoUnion_Specified_TopicName);
  LOG(INFO) << TX_FLAGS(EgoUnion_Specified_EgoGroup);

  LOG(INFO) << TX_FLAGS(TAD_Obs_Subdivide_Threshold);
  LOG(INFO) << TX_FLAGS(PreviewCachePath);
  LOG(INFO) << TX_FLAGS(Filling_ExternInfo);
  LOG(INFO) << TX_FLAGS(ExternInfo_Search_Distance);
  LOG(INFO) << TX_FLAGS(USE_MAP_OBJECT);

  LOG(INFO) << TX_FLAGS(LogsimPub_Location);
  LOG(INFO) << TX_FLAGS(LOGSIM_RawLocation_TopicName);
  LOG(INFO) << TX_FLAGS(LogsimPub_Traffic);
  LOG(INFO) << TX_FLAGS(LOGSIM_RawTraffic_TopicName);
  LOG(INFO) << TX_FLAGS(LogsimPub_Trajectory);
  LOG(INFO) << TX_FLAGS(LOGSIM_RawTrajectory_TopicName);
  LOG(INFO) << TX_FLAGS(Logsim_FrameInterpolation);
  LOG(INFO) << TX_FLAGS(Logsim_HeadingInterpolation);
  LOG(INFO) << TX_FLAGS(AI_NonRoute_LaneChangeThreshold);
  LOG(INFO) << TX_FLAGS(L2W_StopRouteLen);

  LOG(INFO) << TX_FLAGS(EgoInfo_AngleCenterLine_LeftAngle);
  LOG(INFO) << TX_FLAGS(EgoInfo_AngleCenterLine_RightAngle);
  LOG(INFO) << TX_FLAGS(TrajStartAngleDuration);
  LOG(INFO) << TX_FLAGS(VehTrajEventTrigger);

  LOG(INFO) << TX_FLAGS(_74_param_1);
  LOG(INFO) << TX_FLAGS(_74_param_2);
  LOG(INFO) << TX_FLAGS(_74_param_3);

  LOG(INFO) << TX_FLAGS(Enable_Sketch);
  LOG(INFO) << TX_FLAGS(roadsurface_water_radius);
  LOG(INFO) << TX_FLAGS(sign_radius);
  LOG(INFO) << TX_FLAGS(ego_cutin_history_traj_size);
  LOG(INFO) << TX_FLAGS(ego_3_2_lanechange_dist);

  LOG(INFO) << TX_FLAGS(curve2acc_a);
  LOG(INFO) << TX_FLAGS(curve2acc_b);
  LOG(INFO) << TX_FLAGS(curve2acc_active_factor);
  LOG(INFO) << TX_FLAGS(StaticLaneChange_Acc);
  LOG(INFO) << TX_FLAGS(ConsistencyRecPath);
}
