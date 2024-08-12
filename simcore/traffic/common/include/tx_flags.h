// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <gflags/gflags.h>
#include "tx_marco.h"

DECLARE_string(app_name);
DECLARE_string(app_type);
DECLARE_string(tx_log_dir);
DECLARE_string(ip_addr_port);
DECLARE_string(scene_type);
DECLARE_string(scene_file_path);
DECLARE_double(default_altitude);
DECLARE_double(world_max_coord);
DECLARE_double(world_lowest_coord);
DECLARE_int32(default_random_seed);

DECLARE_int64(max_step_count);
DECLARE_string(inject_vehicle_behavior_db);

DECLARE_double(default_velocity);
DECLARE_double(max_lift_time);
DECLARE_double(vehicle_max_speed);
DECLARE_double(default_height_of_obstacle_pedestrian);
DECLARE_int32(invalid_id);
DECLARE_double(default_lane_width);
DECLARE_double(MapLocationInterval);

DECLARE_string(GlobalCoordinate);
DECLARE_string(LocalFrame);
DECLARE_string(VehicleFrame);

DECLARE_double(EGO_Length);
DECLARE_double(EGO_Width);
DECLARE_double(EGO_Height);
DECLARE_double(EGO_MassStep);

DECLARE_double(Truck_Length);
DECLARE_double(Truck_Width);
DECLARE_double(Truck_Height);
DECLARE_double(Truck_MassStep);

DECLARE_double(Trailer_Length);
DECLARE_double(Trailer_Width);
DECLARE_double(Trailer_Height);
DECLARE_double(Trailer_MassStep);

DECLARE_double(Pedestrian_Length);
DECLARE_double(Pedestrian_Width);
DECLARE_double(Pedestrian_Height);

DECLARE_double(Average_LaneChanging_Duration);
DECLARE_double(Variance_LaneChanging_Duration);
DECLARE_double(Average_LaneChanging_Duration_Short);
DECLARE_double(Variance_LaneChanging_Duration_Short);

DECLARE_bool(Kill_After_Stop);

DECLARE_double(Envelope_Offset);
DECLARE_double(AI_VehicleLaneKeepTime);
DECLARE_double(LK_A);
DECLARE_double(LK_B);

DECLARE_int32(Default_EgoVehicle_Id);
DECLARE_int32(Default_TrailerVehicle_Id);

#if USE_EgoGroup
DECLARE_int32(Default_EgoIdPrefix);
#endif /*USE_EgoGroup*/

DECLARE_int32(Topological_Root_Id);
DECLARE_double(SUDOKU_GRID_MAX_DISTANCE);
DECLARE_double(Finish_Radius);
DECLARE_double(Deceleration_Safe_Distance);

DECLARE_double(LaneChangeDuration);
DECLARE_double(OffsetChangeDuration);
DECLARE_double(RefuseLaneChangeDistance);
DECLARE_double(SignalDetectionDistance);

DECLARE_string(EventTypeTTC);
DECLARE_string(EventTypeEgoDistance);
DECLARE_string(DistProjectionTypeEuclid);
DECLARE_string(DistProjectionTypeLane);
DECLARE_int32(DefaultTriggerIndex);
DECLARE_double(DefaultMergeDuration);
DECLARE_double(DefaultMergeOffset);

DECLARE_string(FollowEgoFlag);
DECLARE_double(MAP_NearLaneDist);
DECLARE_double(Connect_Lane_Dist);
DECLARE_double(Ignore_LaneLink_Length);

DECLARE_int32(MAX_VEHICLE_LIMIT_COUNT);
// ---------------- Log Level Start ---------------------------------
DECLARE_bool(LogLevel_Acc);
DECLARE_bool(LogLevel_Merge);
DECLARE_bool(LogLevel_Vehicle);
DECLARE_bool(LogLevel_Pedestrian);
DECLARE_bool(LogLevel_Obstacle);
DECLARE_bool(LogLevel_Signal);
DECLARE_bool(LogLevel_SendMsg);
DECLARE_bool(LogLevel_Statistics);
DECLARE_bool(LogLevel_AiTraffic);
DECLARE_bool(LogLevel_EventTrigger);
DECLARE_bool(LogLevel_VehicleDir);
DECLARE_bool(LogLevel_Routing);
DECLARE_bool(LogLevel_SceneReader);
DECLARE_bool(LogLevel_SceneLoader);
DECLARE_bool(LogLevel_LaneChange);
DECLARE_bool(LogLevel_MultiThread);
DECLARE_bool(LogLevel_Statistics_MultiThread);
DECLARE_bool(LogLevel_Ai_CrossRoad);
DECLARE_bool(LogLevel_MapSDK);
DECLARE_bool(LogLevel_EgoTrailer);
DECLARE_bool(LogLevel_LinkSpline);
DECLARE_bool(LogLevel_RTree);
DECLARE_bool(LogLevel_Kinetics);
DECLARE_bool(LogLevel_XOSC);
DECLARE_bool(LogLevel_ManualVehicle);
DECLARE_bool(LogLevel_Visualizer);
DECLARE_bool(LogLevel_ShowMapCacheInfo);
DECLARE_bool(LogLevel_UserDefine);
DECLARE_bool(LogLevel_UserDefine_StateMachine);
DECLARE_bool(LogLevel_AI_Traffic);
DECLARE_bool(LogLevel_AI_StateMachine);
DECLARE_bool(LogLevel_Ego);
DECLARE_bool(LogLevel_RoadNetwork);
DECLARE_bool(LogLevel_EnvPerception);
DECLARE_bool(LogLevel_SystemLoop);
DECLARE_bool(LogLevel_IDM_Front);
DECLARE_bool(LogLevel_Console);
DECLARE_bool(LogLevel_VehExit);
DECLARE_bool(LogLevel_FCW);
DECLARE_bool(LogLevel_Serialize);
DECLARE_bool(LogLevel_Vehicle_Traj);
DECLARE_bool(LogLevel_VehMeeting);
DECLARE_bool(LogLevel_Traffic_PB_Output);
DECLARE_bool(LogLevel_Vehicle_Input);
DECLARE_bool(LogLevel_Surrounding_Info);
DECLARE_bool(LogLevel_Rear_Axle);
DECLARE_bool(LogLevel_ExternInfo);
DECLARE_bool(LogLevel_Catalog);
DECLARE_bool(LogLevel_SceneEvent_1100);
DECLARE_bool(LogLevel_SceneEvent_1200);
DECLARE_bool(LogLevel_SpeedLimit);
DECLARE_bool(LogLevel_GeomShape);
DECLARE_bool(LogLevel_Catalogs);

// ---------------- Log Level End   ---------------------------------

// ----------------- TAD AI TRAFFIC PARAM START ---------------------
DECLARE_double(DETECT_OBJECT_DISTANCE);
DECLARE_double(DETECT_FOCUS_OBJECT_DISTANCE);
DECLARE_double(Mandatory_Aware_Distance);
DECLARE_double(DETECT_OBJECT_DISTANCE_IN_JUNCTION);
// ----------------- TAD AI TRAFFIC PARAM END   ---------------------

// ----------------- High Performance START ---------------------
DECLARE_int32(MAX_THREAD_NUM);
// ----------------- High Performance END   ---------------------

// ----------------- Cross Road Debug START ---------------------
DECLARE_double(Debug_DistanceToLink);
DECLARE_double(Debug_RatioInLink_1);
DECLARE_double(Debug_RatioInLink_2);
DECLARE_double(Debug_RatioInLink_3);

DECLARE_double(GeomShapeFactor1);
DECLARE_double(GeomShapeFactor2);
// ----------------- Cross Road Debug END   ---------------------

// ----------------- Lane Change START ---------------------
DECLARE_double(RefuseLaneChangeDistanceInLane);
// ----------------- Lane Change   END ---------------------

// ----------------- Traffic Light START ---------------------
DECLARE_double(LeftTurnWaitingBuffer);
DECLARE_double(RightTurnWaitingBuffer);
// ----------------- Traffic Light END   ---------------------

DECLARE_double(TopologyUpdatePeriod);

// ----------------- RTree START ---------------------
DECLARE_double(SearchAngleHalf_NearestDynamicObj);
DECLARE_double(SearchAngleHalf_NearestObject);
// ----------------- RTree END   ---------------------

// ------------------ MapManager START -------------------
DECLARE_bool(LaneCacheOnInit);
// ------------------ MapManager END -------------------

// ------------------ Trailer START  --------------------
#if Use_TrailerEgo
DECLARE_bool(ForcedTrailer);
#endif /*Use_TrailerEgo*/
// ------------------ Trailer END    --------------------

// ------------------ VIL START -------------------------
DECLARE_bool(SimulateWithPnC);
// ------------------ VIL END   -------------------------

DECLARE_int32(MaxSignalControlLaneIterateCount);
DECLARE_double(SignalMaxVisionDistance);

DECLARE_double(HdMap_3D_Alt);

DECLARE_int32(SubSectionPower); /*2^SubSectionPower = sub section length */

DECLARE_double(LaneInfoSampleStep);
DECLARE_bool(CheckAbsorbLaneInfo);

DECLARE_double(stop_on_junction_duration);
DECLARE_double(stop_duration);

DECLARE_double(exit_region_length);
DECLARE_bool(CacheLaneMarkLR);
DECLARE_double(deadline_minimum_length);
DECLARE_bool(export_hashed_debug_info);
DECLARE_string(export_hashed_debug_info_path);

DECLARE_double(lanelink_nudge_delta);

DECLARE_int32(AiVehEnvPercptionPeriod);

DECLARE_bool(EnableCrashJam);
DECLARE_double(Crash_TimeHead);
DECLARE_double(CrashJam_TimeHead);
DECLARE_double(Active_TimeHead);

DECLARE_double(CentripetalCatMull_Alpha);
DECLARE_double(CentripetalCatMull_Tension);

DECLARE_int32(VehicleInputMaxSize);

DECLARE_bool(VehicleOutputTrajectory);
DECLARE_double(vehicle_trajectory_duration_s);

DECLARE_bool(VehicleInputEnable);
DECLARE_double(VehicleInputVelocityVariance);

DECLARE_bool(Use_Arterial);
DECLARE_string(behavior_cfg_name);

DECLARE_double(RefuseLaneChangeVelocity);
DECLARE_int64(DebugVehId);

DECLARE_bool(Filling_ExternInfo);
DECLARE_double(ExternInfo_Search_Distance);

// ------------------ Catalog START -------------------------
DECLARE_string(Catalog_VehicleCatalog);
DECLARE_string(Catalog_PedestrianCatalog);
DECLARE_string(Catalog_MiscObjectCatalog);
DECLARE_string(Catalog_Sys_VehicleCatalog);
DECLARE_string(Catalog_Sys_PedestrianCatalog);
DECLARE_string(Catalog_Sys_MiscObjectCatalog);
// ------------------ Catalog END   -------------------------

DECLARE_bool(Generate_Meeting);
DECLARE_double(SpeedLimitThreshold);

DECLARE_string(error_info);

DECLARE_string(Logsim_Extension);
DECLARE_string(Worldsim_Extension);
DECLARE_string(L2W_Extension);

DECLARE_string(params_vehicle_behavior_db);

DECLARE_bool(EnableSysInfo);

DECLARE_double(MapMultiRangeStep);
DECLARE_bool(EnableCheckRefuseLaneChange);
DECLARE_double(EgoCircleSearchRadius);
DECLARE_double(elem_near_junction_dist);
DECLARE_double(DefaultRuleComplianceProportion);

DECLARE_string(VehicleBehaviorMapping);
extern void PrintFlagsValues() TX_NOEXCEPT;
