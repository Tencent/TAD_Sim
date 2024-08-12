// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_flags.h"

DECLARE_int32(MAX_LANE_COUNT);
// ------------------ ManualVehicle START  --------------------
#if Use_ManualVehicle
DECLARE_bool(ManualVehicle);
DECLARE_string(ManualVehicleTopicName);
#endif /*Use_ManualVehicle*/
// ------------------ ManualVehicle END    --------------------
DECLARE_double(Enable_LaneChange_Length);

// ----------------- Log2Worldsim Start ----------------------------
DECLARE_string(L2W_EgoName);
DECLARE_string(L2W_EgoProperty);
DECLARE_int32(L2W_Hertz);
DECLARE_double(L2W_DisengageTime);
DECLARE_double(L2W_TrajSimplifyDistance);
// ----------------- Log2Worldsim End   ----------------------------

DECLARE_bool(LogLevel_TADSim_Loop);
DECLARE_bool(LogLevel_AI_EnvPercption);
DECLARE_bool(LogLevel_AI_Routing);
DECLARE_bool(LogLevel_AI_Arterial);
DECLARE_bool(LogLevel_XOSC_Replay);
DECLARE_bool(LogLevel_TrajFollow);
DECLARE_bool(LogLevel_L2W);
DECLARE_bool(LogLevel_RelativeTrajectoryFollow);
DECLARE_bool(LogLevel_Preview);
DECLARE_bool(LogLevel_MapObject);
DECLARE_bool(LogLevel_MapFile);
DECLARE_bool(LogLevel_Junction);
DECLARE_bool(LogLevel_DebugVertex);
DECLARE_bool(LogLevel_Sketch);
DECLARE_bool(LogLevel_Curve2Acc);
DECLARE_bool(LogLevel_L2W_EgoInfo);

DECLARE_double(vehicle_input_safe_region_radius);
DECLARE_double(vehicle_input_scan_vision_distance);

DECLARE_bool(XOSC_Replay_With_EgoCar);

DECLARE_double(OnLink_SameTo_QueryRadius);
DECLARE_bool(TADSimRouteEndStop);

DECLARE_double(event_99930_99931_trigger_time);
DECLARE_double(event_99930_heading);
DECLARE_double(event_99931_heading);
DECLARE_double(event_99930_99931_renew_time);
DECLARE_double(event_99930_99931_renew_velocity);
DECLARE_double(event_99930_99931_influnce_distance);

DECLARE_double(TAD_FCW_Radius);

DECLARE_bool(EnableRearComfortGap);
DECLARE_string(L2W_Switch_TopicName);
DECLARE_string(L2W_RawTraffic_TopicName);
DECLARE_double(L2W_Switch_SpecialTime);
DECLARE_bool(L2W_Vehicle_Position_Linear_Interpolation);
DECLARE_bool(L2W_Ego_Position_Linear_Interpolation);
DECLARE_bool(L2W_AvoidSuddenlyAppear);
DECLARE_double(L2W_LoseFrameThreshold);
DECLARE_bool(L2W_SwitchAbsorb);
DECLARE_double(L2W_Motion_Dist);
DECLARE_string(L2W_RawLocation_TopicName);
DECLARE_string(L2W_RawTrajectory_TopicName);

DECLARE_bool(EnableTrafficVisionFilter);
DECLARE_double(TrafficVisionFilterRadius);
DECLARE_double(TrafficVisionFilterAltitudeDiff);

DECLARE_double(SignalQueryRaidus);
DECLARE_string(Traffic_TopicName);
DECLARE_string(TrailerEgo_TopicName);
DECLARE_string(Location_TopicName);

DECLARE_string(EgoUnion_TopicName);
DECLARE_string(EgoUnion_Trailer_TopicName);
DECLARE_string(EgoUnion_Specified_TopicName);
DECLARE_string(EgoUnion_Specified_EgoGroup);

DECLARE_double(TAD_Obs_Subdivide_Threshold);
DECLARE_string(PreviewCachePath);

DECLARE_bool(USE_MAP_OBJECT);

DECLARE_bool(LogsimPub_Location);
DECLARE_string(LOGSIM_RawLocation_TopicName);
DECLARE_bool(LogsimPub_Traffic);
DECLARE_string(LOGSIM_RawTraffic_TopicName);
DECLARE_bool(LogsimPub_Trajectory);
DECLARE_string(LOGSIM_RawTrajectory_TopicName);
DECLARE_bool(Logsim_FrameInterpolation);
DECLARE_bool(Logsim_HeadingInterpolation);

DECLARE_double(AI_NonRoute_LaneChangeThreshold);

DECLARE_double(L2W_StopRouteLen);

DECLARE_double(EgoInfo_AngleCenterLine_LeftAngle);
DECLARE_double(EgoInfo_AngleCenterLine_RightAngle);
DECLARE_double(TrajStartAngleDuration);
DECLARE_bool(VehTrajEventTrigger);

DECLARE_double(_74_param_1);
DECLARE_double(_74_param_2);
DECLARE_double(_74_param_3);

DECLARE_bool(Enable_Sketch);
DECLARE_double(roadsurface_water_radius);
DECLARE_double(sign_radius);
DECLARE_int32(ego_cutin_history_traj_size);
DECLARE_double(ego_3_2_lanechange_dist);
DECLARE_double(curve2acc_a);
DECLARE_double(curve2acc_b);
DECLARE_double(curve2acc_active_factor);

DECLARE_bool(Enable_Road_Limit);
DECLARE_double(Speed_Max_OnCurve); /* acc is limited to 0 when speed exceeds Speed_Max_OnCurve (curv > 1e-2 ) */
DECLARE_double(StaticLaneChange_Acc);
DECLARE_string(ConsistencyRecPath);

extern void PrintFlagsValues_TADSim() TX_NOEXCEPT;
