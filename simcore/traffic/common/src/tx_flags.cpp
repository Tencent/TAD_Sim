// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_flags.h"
#include <limits>
#include "tx_logger.h"

DEFINE_string(app_name, "Traffic_clone", "the Application Name");
DEFINE_string(app_type, "txMsgModule", "txMsgModule  txVisualizer");
DEFINE_string(tx_log_dir, "", "log default path");
DEFINE_string(ip_addr_port, "", "rpc server location");
DEFINE_string(scene_type, "TAD", "Traffic Flow Scene Type : OpenScenario, TAD, RAW");
DEFINE_string(scene_file_path, "", "");
DEFINE_double(default_altitude, 0.0, "default altitude is 0.0, fake 3D");
DEFINE_double(world_max_coord, 10000, "scene max coord, mile");
DEFINE_double(world_lowest_coord, -10000, "scene lowest coord, mile");
DEFINE_int32(default_random_seed, 55, "");

DEFINE_int64(max_step_count, std::numeric_limits<int64_t>::max(), "");
DEFINE_string(inject_vehicle_behavior_db, "", "");

DEFINE_double(default_velocity, 20.0, "");
DEFINE_double(max_lift_time, 99999999, "");
DEFINE_double(vehicle_max_speed, 27.8, "");

DEFINE_double(default_height_of_obstacle_pedestrian, 1.5, "");

DEFINE_int32(invalid_id, -1, "");
DEFINE_double(default_lane_width, 3.5, "");
DEFINE_double(MapLocationInterval, 2.0, "RTree2D_NearestReferenceLine");

DEFINE_string(GlobalCoordinate, "WGS84", "");
DEFINE_string(LocalFrame, "ENU", "Local GPS rectangular coordinates with respect to the reference point");
DEFINE_string(VehicleFrame, "RFU, Right-Forward-Up", "");

DEFINE_double(Average_LaneChanging_Duration, 4.5, "");
DEFINE_double(Variance_LaneChanging_Duration, 4.5, "");
DEFINE_double(Average_LaneChanging_Duration_Short, 2.0, "");
DEFINE_double(Variance_LaneChanging_Duration_Short, 2.0, "");
#ifdef ON_CLOUD
DEFINE_bool(Kill_After_Stop, true,
            "the action when an element arrive at the destination. False : do not kill. True : kill");
#else
DEFINE_bool(Kill_After_Stop, false,
            "the action when an element arrive at the destination. False : do not kill. True : kill");
#endif /*ON_CLOUD*/

DEFINE_double(Envelope_Offset, 10.0, "");
DEFINE_double(AI_VehicleLaneKeepTime, 0.5, "");
DEFINE_double(LK_A, 1.0, "a * _laneKeep +  b* (1.0 - _aggress);");
DEFINE_double(LK_B, 3.0, "a * _laneKeep +  b* (1.0 - _aggress);");

DEFINE_int32(Default_EgoVehicle_Id, 2147483647, "int32 max value.");
DEFINE_int32(Default_TrailerVehicle_Id, 2147483646, "int32 max value -1.");

DEFINE_int32(Default_EgoIdPrefix, 1000000000, "");

DEFINE_int32(Topological_Root_Id, 0, "");
DEFINE_double(SUDOKU_GRID_MAX_DISTANCE, 999999.0, "");
DEFINE_double(Finish_Radius, 10.0, "");
DEFINE_double(Deceleration_Safe_Distance, 2.0, "Using in CheckPedestrianWalking");

DEFINE_double(LaneChangeDuration, 4.5, "");
DEFINE_double(OffsetChangeDuration, 3.0, "");
DEFINE_double(RefuseLaneChangeDistance, 0.0, "");
DEFINE_double(SignalDetectionDistance, 100.0, "");

DEFINE_double(EGO_Length, 4.68, "Haval H7");
DEFINE_double(EGO_Width, 2.18, "Haval H7");
DEFINE_double(EGO_Height, 1.76, "Haval H7");
DEFINE_double(EGO_MassStep, 0.014405, "Haval H7");

DEFINE_double(Truck_Length, 5.17, "");
DEFINE_double(Truck_Width, 2.18, "");
DEFINE_double(Truck_Height, 2.76, "");
DEFINE_double(Truck_MassStep, 0.1935, "");

DEFINE_double(Trailer_Length, 12.31, "");
DEFINE_double(Trailer_Width, 2.18, "");
DEFINE_double(Trailer_Height, 3.5, "");
DEFINE_double(Trailer_MassStep, -6.04379, "");

DEFINE_double(Pedestrian_Length, 1.0, "");
DEFINE_double(Pedestrian_Width, 1.0, "");
DEFINE_double(Pedestrian_Height, 1.5, "");

DEFINE_double(PEDESTRIAN_UPDATE_LANE_INFO_TIME, 0.2, "");

/*DEFINE_double(Verbose, false, "");*/

DEFINE_string(EventTypeTTC, "ttc", "Case sensitive");
DEFINE_string(EventTypeEgoDistance, "egodistance", "Case sensitive");
DEFINE_string(DistProjectionTypeEuclid, "euclideandistance", "Case sensitive");
DEFINE_string(DistProjectionTypeLane, "laneprojection", "Case sensitive");
DEFINE_int32(DefaultTriggerIndex, 1, "");
DEFINE_double(DefaultMergeDuration, 4.5, "");
DEFINE_double(DefaultMergeOffset, 0.0, "");

DEFINE_string(FollowEgoFlag, "ego", "Case sensitive");
DEFINE_double(MAP_NearLaneDist, 15.0, "");
DEFINE_double(Connect_Lane_Dist, 0.5, "");
DEFINE_double(Ignore_LaneLink_Length, 0.00001, "");

DEFINE_int32(MAX_VEHICLE_LIMIT_COUNT, 2000, "");

// ---------------- Log Level Start ------------------------------------------------------------
DEFINE_bool(LogLevel_Acc, false, "");
DEFINE_bool(LogLevel_Merge, false, "");
DEFINE_bool(LogLevel_Vehicle, false, "");
DEFINE_bool(LogLevel_Pedestrian, false, "");
DEFINE_bool(LogLevel_Obstacle, false, "");
DEFINE_bool(LogLevel_Signal, false, "");
DEFINE_bool(LogLevel_SendMsg, false, "");
DEFINE_bool(LogLevel_Statistics, false, "");
DEFINE_bool(LogLevel_AiTraffic, false, "");
DEFINE_bool(LogLevel_EventTrigger, false, "");
DEFINE_bool(LogLevel_VehicleDir, false, "");
DEFINE_bool(LogLevel_Routing, GeneralCheckBool, "");
DEFINE_bool(LogLevel_SceneReader, false, "");
DEFINE_bool(LogLevel_SceneLoader, false, "");
DEFINE_bool(LogLevel_LaneChange, LogSurroundInfo, "");
DEFINE_bool(LogLevel_MultiThread, false, "");
DEFINE_bool(LogLevel_Statistics_MultiThread, false, "");
DEFINE_bool(LogLevel_Ai_CrossRoad, GeneralCheckBool, "");
DEFINE_bool(LogLevel_MapSDK, false, "GetLane");
DEFINE_bool(LogLevel_EgoTrailer, false, "");
DEFINE_bool(LogLevel_LinkSpline, false, "");
DEFINE_bool(LogLevel_RTree, false, "");
DEFINE_bool(LogLevel_Kinetics, false, "");
DEFINE_bool(LogLevel_XOSC, false, "");
DEFINE_bool(LogLevel_ManualVehicle, false, "");
DEFINE_bool(LogLevel_Visualizer, false, "");
DEFINE_bool(LogLevel_ShowMapCacheInfo, false, "");
DEFINE_bool(LogLevel_UserDefine, false, "");
DEFINE_bool(LogLevel_UserDefine_StateMachine, false, "");
DEFINE_bool(LogLevel_AI_Traffic, GeneralCheckBool, "");
DEFINE_bool(LogLevel_AI_StateMachine, false, "");
DEFINE_bool(LogLevel_Ego, false, "");
DEFINE_bool(LogLevel_RoadNetwork, false, "");
DEFINE_bool(LogLevel_EnvPerception, GeneralCheckBool, "");
DEFINE_bool(LogLevel_SystemLoop, GeneralCheckBool, "");
DEFINE_bool(LogLevel_IDM_Front, LogSurroundInfo, "");
DEFINE_bool(LogLevel_Console, true, "");
DEFINE_bool(LogLevel_VehExit, false, "");
DEFINE_bool(LogLevel_FCW, false, "");
DEFINE_bool(LogLevel_Serialize, false, "");
DEFINE_bool(LogLevel_Vehicle_Traj, false, "");
DEFINE_bool(LogLevel_VehMeeting, false, "");
DEFINE_bool(LogLevel_Traffic_PB_Output, false, "");
DEFINE_bool(LogLevel_Vehicle_Input, false, "");
DEFINE_bool(LogLevel_Surrounding_Info, false, "");
DEFINE_bool(LogLevel_Rear_Axle, false, "");
DEFINE_bool(LogLevel_ExternInfo, false, "");
DEFINE_bool(LogLevel_Catalog, false, "");
DEFINE_bool(LogLevel_SceneEvent_1100, false, "");
DEFINE_bool(LogLevel_SceneEvent_1200, true, "");
DEFINE_bool(LogLevel_SpeedLimit, false, "");
DEFINE_bool(LogLevel_GeomShape, false, "");
DEFINE_bool(LogLevel_Catalogs, false, "");

// ---------------- Log Level End   ------------------------------------------------------------

// ----------------- TAD AI TRAFFIC PARAM START ---------------------
DEFINE_double(DETECT_OBJECT_DISTANCE, 50.0, "");
DEFINE_double(DETECT_FOCUS_OBJECT_DISTANCE, 30.0, "");
DEFINE_double(Mandatory_Aware_Distance, 500.0, "");
DEFINE_double(DETECT_OBJECT_DISTANCE_IN_JUNCTION, 100.0, "TAD_AI_VehicleElement::SearchNextCrossVehicle");
// ----------------- TAD AI TRAFFIC PARAM END   ---------------------

// ----------------- High Performance START ---------------------
DEFINE_int32(MAX_THREAD_NUM, 0, "");
// ----------------- High Performance END   ---------------------

// ----------------- Cross Road Debug START ---------------------
DEFINE_double(Debug_DistanceToLink, 10.0, "");
DEFINE_double(Debug_RatioInLink_1, 0.25, "");
DEFINE_double(Debug_RatioInLink_2, 0.5, "");
DEFINE_double(Debug_RatioInLink_3, 0.75, "");

DEFINE_double(GeomShapeFactor1, 0.139, "0.139 * PI = 25");
DEFINE_double(GeomShapeFactor2, 0.86, "0.86 * pi = 154");
// ----------------- Cross Road Debug END   ---------------------

// ----------------- Lane Change START ---------------------
DEFINE_double(RefuseLaneChangeDistanceInLane, 40.0, "");
// ----------------- Lane Change   END ---------------------

// ----------------- Traffic Light START ---------------------
DEFINE_double(LeftTurnWaitingBuffer, 15.0, "");
DEFINE_double(RightTurnWaitingBuffer, 15.0, "");
// ----------------- Traffic Light END   ---------------------

DEFINE_double(TopologyUpdatePeriod, 1.0, "");

// ----------------- RTree START ---------------------
DEFINE_double(SearchAngleHalf_NearestDynamicObj, 25.0, "");
DEFINE_double(SearchAngleHalf_NearestObject, 15.0, "");
// ----------------- RTree END   ---------------------

// ------------------ MapManager START -------------------
DEFINE_bool(LaneCacheOnInit, true, "");
// ------------------ MapManager END -------------------

// ------------------ Trailer START  --------------------
#if Use_TrailerEgo
DEFINE_bool(ForcedTrailer, false, "");
#endif /*Use_TrailerEgo*/
// ------------------ Trailer END    --------------------

// ------------------ VIL START -------------------------
DEFINE_bool(SimulateWithPnC, true, "");
// ------------------ VIL END   -------------------------

DEFINE_int32(MaxSignalControlLaneIterateCount, 10, "");
DEFINE_double(SignalMaxVisionDistance, 100.0, "");

DEFINE_double(HdMap_3D_Alt, -100000.0, "");

DEFINE_int32(SubSectionPower, 4, "2^SubSectionPower = sub section length ");

DEFINE_double(LaneInfoSampleStep, 2.0, "bug_ID90550417");
DEFINE_bool(CheckAbsorbLaneInfo, false, "");

DEFINE_double(stop_on_junction_duration, 10.0, "second");
DEFINE_double(stop_duration, 300.0, "second");

DEFINE_double(exit_region_length, 20.0, "m");
DEFINE_double(deadline_minimum_length, 10.0, "");

DEFINE_bool(export_hashed_debug_info, false, "");
DEFINE_string(export_hashed_debug_info_path, "D:/1.HashedRelated", "");

DEFINE_bool(CacheLaneMarkLR, true, "");

DEFINE_double(lanelink_nudge_delta, 0.2, "");

DEFINE_int32(AiVehEnvPercptionPeriod, 30, "env perception ");

DEFINE_bool(EnableCrashJam, false, "trick");
DEFINE_double(Crash_TimeHead, 3.0, "");
DEFINE_double(CrashJam_TimeHead, 0.5, "");
DEFINE_double(Active_TimeHead, 0.5, "");

DEFINE_double(CentripetalCatMull_Alpha, 0.25, "chordal = 0.5. centripetal = 0.25");
DEFINE_double(CentripetalCatMull_Tension, 0.5, "0-1, 1 = polyline");

DEFINE_int32(VehicleInputMaxSize, (int32_t)1e7, "922337203685.4775808");

DEFINE_bool(VehicleOutputTrajectory, false, "");
DEFINE_double(vehicle_trajectory_duration_s, 8.0, "in second");

DEFINE_bool(VehicleInputEnable, true, "");
DEFINE_double(VehicleInputVelocityVariance, 0.3, "");

DEFINE_bool(Use_Arterial, false, "");
DEFINE_string(behavior_cfg_name, "vehicle_behavior_db.cfg", "tad_vehicle.cfg");

DEFINE_double(RefuseLaneChangeVelocity, 0.0, "");
DEFINE_int64(DebugVehId, -1, "");

DEFINE_bool(Filling_ExternInfo, true, "");
DEFINE_double(ExternInfo_Search_Distance, 200.0, "");
DEFINE_double(DefaultRuleComplianceProportion, 1.0, "");

// ------------------ Catalog START -------------------------
DEFINE_string(Catalog_VehicleCatalog, "../Catalogs/Vehicles/VehicleCatalog.xosc", "");
DEFINE_string(Catalog_PedestrianCatalog, "../Catalogs/Pedestrians/PedestrianCatalog.xosc", "");
DEFINE_string(Catalog_MiscObjectCatalog, "../Catalogs/MiscObjects/MiscObjectCatalog.xosc", "");

DEFINE_string(Catalog_Sys_VehicleCatalog, "../../../sys/scenario/Catalogs/Vehicles/VehicleCatalog.xosc", "");
DEFINE_string(Catalog_Sys_PedestrianCatalog, "../../../sys/scenario/Catalogs/Pedestrians/PedestrianCatalog.xosc", "");
DEFINE_string(Catalog_Sys_MiscObjectCatalog, "../../../sys/scenario/Catalogs/MiscObjects/MiscObjectCatalog.xosc", "");
// ------------------ Catalog END   -------------------------

DEFINE_bool(Generate_Meeting, true, "");
DEFINE_double(SpeedLimitThreshold, 0.01, "");
DEFINE_string(error_info, "", "preview init error info.");

DEFINE_string(Logsim_Extension, ".simrec", "");
DEFINE_string(Worldsim_Extension, ".xml", "");
DEFINE_string(L2W_Extension, ".simrec", "");

DEFINE_string(params_vehicle_behavior_db, "_vehicle_behavior_db", "");

DEFINE_bool(EnableSysInfo, true, "");

DEFINE_double(MapMultiRangeStep, 200.0, "");
DEFINE_bool(EnableCheckRefuseLaneChange, false, "");
DEFINE_double(EgoCircleSearchRadius, 20.0, "");
DEFINE_double(elem_near_junction_dist, 5.0, "");
DEFINE_string(VehicleBehaviorMapping, "", "eg: TrafficVehicle");
void PrintFlagsValues() TX_NOEXCEPT {
  LOG(INFO) << TX_FLAGS(app_name);
  LOG(INFO) << TX_FLAGS(tx_log_dir);
  LOG(INFO) << TX_FLAGS(ip_addr_port);
  LOG(INFO) << TX_FLAGS(scene_type);
  LOG(INFO) << TX_FLAGS(scene_file_path);
  LOG(INFO) << TX_FLAGS(default_altitude);
  LOG(INFO) << TX_FLAGS(world_max_coord);
  LOG(INFO) << TX_FLAGS(world_lowest_coord);
  LOG(INFO) << TX_FLAGS(default_random_seed);
  LOG(INFO) << TX_FLAGS(default_velocity);
  LOG(INFO) << TX_FLAGS(max_lift_time);
  LOG(INFO) << TX_FLAGS(vehicle_max_speed);
  LOG(INFO) << TX_FLAGS(default_height_of_obstacle_pedestrian);
  LOG(INFO) << TX_FLAGS(invalid_id);
  LOG(INFO) << TX_FLAGS(default_lane_width);
  LOG(INFO) << TX_FLAGS(MapLocationInterval);

  LOG(INFO) << TX_FLAGS(GlobalCoordinate);
  LOG(INFO) << TX_FLAGS(LocalFrame);
  LOG(INFO) << TX_FLAGS(VehicleFrame);

  LOG(INFO) << TX_FLAGS(EGO_Length);
  LOG(INFO) << TX_FLAGS(EGO_Width);
  LOG(INFO) << TX_FLAGS(EGO_Height);
  LOG(INFO) << TX_FLAGS(EGO_MassStep);

  LOG(INFO) << TX_FLAGS(Truck_Length);
  LOG(INFO) << TX_FLAGS(Truck_Width);
  LOG(INFO) << TX_FLAGS(Truck_Height);
  LOG(INFO) << TX_FLAGS(Truck_MassStep);

  LOG(INFO) << TX_FLAGS(Trailer_Length);
  LOG(INFO) << TX_FLAGS(Trailer_Width);
  LOG(INFO) << TX_FLAGS(Trailer_Height);
  LOG(INFO) << TX_FLAGS(Trailer_MassStep);

  LOG(INFO) << TX_FLAGS(Pedestrian_Length);
  LOG(INFO) << TX_FLAGS(Pedestrian_Width);
  LOG(INFO) << TX_FLAGS(Pedestrian_Height);

  LOG(INFO) << TX_FLAGS(Average_LaneChanging_Duration);
  LOG(INFO) << TX_FLAGS(Variance_LaneChanging_Duration);
  LOG(INFO) << TX_FLAGS(Average_LaneChanging_Duration_Short);
  LOG(INFO) << TX_FLAGS(Variance_LaneChanging_Duration_Short);
  LOG(INFO) << TX_FLAGS(Kill_After_Stop);

  LOG(INFO) << TX_FLAGS(Envelope_Offset);
  LOG(INFO) << TX_FLAGS(AI_VehicleLaneKeepTime);
  LOG(INFO) << TX_FLAGS(LK_A);
  LOG(INFO) << TX_FLAGS(LK_B);

  LOG(INFO) << TX_FLAGS(Default_EgoVehicle_Id);
  LOG(INFO) << TX_FLAGS(Default_TrailerVehicle_Id);
  LOG(INFO) << TX_FLAGS(Topological_Root_Id);
  txAssert(FLAGS_Topological_Root_Id != FLAGS_Default_EgoVehicle_Id);
  LOG(INFO) << TX_FLAGS(SUDOKU_GRID_MAX_DISTANCE);
  LOG(INFO) << TX_FLAGS(Finish_Radius);
  LOG(INFO) << TX_FLAGS(Deceleration_Safe_Distance);

  LOG(INFO) << TX_FLAGS(LaneChangeDuration);
  LOG(INFO) << TX_FLAGS(OffsetChangeDuration);
  LOG(INFO) << TX_FLAGS(RefuseLaneChangeDistance);
  LOG(INFO) << TX_FLAGS(SignalDetectionDistance);

  LOG(INFO) << TX_FLAGS(EventTypeTTC);
  LOG(INFO) << TX_FLAGS(EventTypeEgoDistance);
  LOG(INFO) << TX_FLAGS(DistProjectionTypeEuclid);
  LOG(INFO) << TX_FLAGS(DistProjectionTypeLane);
  LOG(INFO) << TX_FLAGS(DefaultTriggerIndex);
  LOG(INFO) << TX_FLAGS(DefaultMergeDuration);
  LOG(INFO) << TX_FLAGS(DefaultMergeOffset);

  LOG(INFO) << TX_FLAGS(FollowEgoFlag);
  LOG(INFO) << TX_FLAGS(MAP_NearLaneDist);
  LOG(INFO) << TX_FLAGS(Connect_Lane_Dist);
  LOG(INFO) << TX_FLAGS(Ignore_LaneLink_Length);
  LOG(INFO) << TX_FLAGS(MAX_VEHICLE_LIMIT_COUNT);
  LOG(INFO) << "#######  Log Level ######";
  LOG(INFO) << TX_FLAGS(LogLevel_Acc);
  LOG(INFO) << TX_FLAGS(LogLevel_Merge);
  LOG(INFO) << TX_FLAGS(LogLevel_Vehicle);
  LOG(INFO) << TX_FLAGS(LogLevel_Pedestrian);
  LOG(INFO) << TX_FLAGS(LogLevel_Obstacle);
  LOG(INFO) << TX_FLAGS(LogLevel_Signal);
  LOG(INFO) << TX_FLAGS(LogLevel_SendMsg);
  LOG(INFO) << TX_FLAGS(LogLevel_Statistics);
  LOG(INFO) << TX_FLAGS(LogLevel_AiTraffic);
  LOG(INFO) << TX_FLAGS(LogLevel_EventTrigger);
  LOG(INFO) << TX_FLAGS(LogLevel_VehicleDir);
  LOG(INFO) << TX_FLAGS(LogLevel_Routing);
  LOG(INFO) << TX_FLAGS(LogLevel_SceneReader);
  LOG(INFO) << TX_FLAGS(LogLevel_SceneLoader);
  LOG(INFO) << TX_FLAGS(LogLevel_LaneChange);
  LOG(INFO) << TX_FLAGS(LogLevel_MultiThread);
  LOG(INFO) << TX_FLAGS(LogLevel_Ai_CrossRoad);
  LOG(INFO) << TX_FLAGS(LogLevel_MapSDK);
  LOG(INFO) << TX_FLAGS(LogLevel_EgoTrailer);
  LOG(INFO) << TX_FLAGS(LogLevel_Statistics_MultiThread);
  LOG(INFO) << TX_FLAGS(LogLevel_LinkSpline);
  LOG(INFO) << TX_FLAGS(LogLevel_RTree);
  LOG(INFO) << TX_FLAGS(LogLevel_XOSC);
  LOG(INFO) << TX_FLAGS(LogLevel_ManualVehicle);
  LOG(INFO) << TX_FLAGS(LogLevel_Kinetics);
  LOG(INFO) << TX_FLAGS(LogLevel_Visualizer);
  LOG(INFO) << TX_FLAGS(LogLevel_ShowMapCacheInfo);
  LOG(INFO) << TX_FLAGS(LogLevel_UserDefine);
  LOG(INFO) << TX_FLAGS(LogLevel_UserDefine_StateMachine);
  LOG(INFO) << TX_FLAGS(LogLevel_AI_Traffic);
  LOG(INFO) << TX_FLAGS(LogLevel_AI_StateMachine);
  LOG(INFO) << TX_FLAGS(LogLevel_Ego);
  LOG(INFO) << TX_FLAGS(LogLevel_RoadNetwork);
  LOG(INFO) << TX_FLAGS(LogLevel_EnvPerception);
  LOG(INFO) << TX_FLAGS(LogLevel_SystemLoop);
  LOG(INFO) << TX_FLAGS(LogLevel_IDM_Front);
  LOG(INFO) << TX_FLAGS(LogLevel_Console);
  LOG(INFO) << TX_FLAGS(LogLevel_VehExit);
  LOG(INFO) << TX_FLAGS(LogLevel_FCW);
  LOG(INFO) << TX_FLAGS(LogLevel_Serialize);
  LOG(INFO) << TX_FLAGS(LogLevel_Vehicle_Traj);
  LOG(INFO) << TX_FLAGS(LogLevel_VehMeeting);
  LOG(INFO) << TX_FLAGS(LogLevel_Traffic_PB_Output);
  LOG(INFO) << TX_FLAGS(LogLevel_Vehicle_Input);
  LOG(INFO) << TX_FLAGS(LogLevel_Surrounding_Info);
  LOG(INFO) << TX_FLAGS(LogLevel_Rear_Axle);
  LOG(INFO) << TX_FLAGS(LogLevel_ExternInfo);
  LOG(INFO) << TX_FLAGS(LogLevel_Catalog);
  LOG(INFO) << TX_FLAGS(LogLevel_SceneEvent_1100);
  LOG(INFO) << TX_FLAGS(LogLevel_SceneEvent_1200);
  LOG(INFO) << TX_FLAGS(LogLevel_SpeedLimit);
  LOG(INFO) << TX_FLAGS(LogLevel_GeomShape);
  LOG(INFO) << TX_FLAGS(LogLevel_Catalogs);
  LOG(INFO) << "#######  Log Level ######";

  LOG(INFO) << "#######  TAD AI TRAFFIC PARAM ######";
  LOG(INFO) << TX_FLAGS(DETECT_OBJECT_DISTANCE);
  LOG(INFO) << TX_FLAGS(DETECT_FOCUS_OBJECT_DISTANCE);
  LOG(INFO) << TX_FLAGS(Mandatory_Aware_Distance);
  LOG(INFO) << TX_FLAGS(DETECT_OBJECT_DISTANCE_IN_JUNCTION);
  LOG(INFO) << "#######  TAD AI TRAFFIC PARAM ######";

  LOG(INFO) << "#######  txSimTraffic Git Version ######";
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  git commit id : " << STR_COMMIT_ID << ", git branch name : " << STR_GIT_BRANCH;
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  txSimTraffic Git Version ######";

  LOG(INFO) << TX_FLAGS(MAX_THREAD_NUM);

  LOG(INFO) << TX_FLAGS(Debug_DistanceToLink);
  LOG(INFO) << TX_FLAGS(Debug_RatioInLink_1);
  LOG(INFO) << TX_FLAGS(Debug_RatioInLink_2);
  LOG(INFO) << TX_FLAGS(Debug_RatioInLink_3);
  LOG(INFO) << TX_FLAGS(GeomShapeFactor1);
  LOG(INFO) << TX_FLAGS(GeomShapeFactor2);

  LOG(INFO) << TX_FLAGS(RefuseLaneChangeDistanceInLane);

  LOG(INFO) << TX_FLAGS(LeftTurnWaitingBuffer);
  LOG(INFO) << TX_FLAGS(RightTurnWaitingBuffer);
  LOG(INFO) << TX_FLAGS(TopologyUpdatePeriod);
  LOG(INFO) << TX_FLAGS(SearchAngleHalf_NearestDynamicObj);
  LOG(INFO) << TX_FLAGS(SearchAngleHalf_NearestObject);

  LOG(INFO) << TX_FLAGS(LaneCacheOnInit);
  // ------------------ Trailer START  --------------------
#if Use_TrailerEgo
  LOG(INFO) << TX_FLAGS(ForcedTrailer);
#endif /*Use_TrailerEgo*/
  // ------------------ Trailer END    --------------------
  LOG(INFO) << TX_FLAGS(SimulateWithPnC);

  LOG(INFO) << TX_FLAGS(MaxSignalControlLaneIterateCount);
  LOG(INFO) << TX_FLAGS(SignalMaxVisionDistance);
  LOG(INFO) << TX_FLAGS(SubSectionPower);
  LOG(INFO) << TX_FLAGS(LaneInfoSampleStep);
  LOG(INFO) << TX_FLAGS(CheckAbsorbLaneInfo);
  LOG(INFO) << TX_FLAGS(stop_on_junction_duration);
  LOG(INFO) << TX_FLAGS(stop_duration);

  LOG(INFO) << TX_FLAGS(exit_region_length);
  LOG(INFO) << TX_FLAGS(CacheLaneMarkLR);
  LOG(INFO) << TX_FLAGS(deadline_minimum_length);
  LOG(INFO) << TX_FLAGS(export_hashed_debug_info);
  LOG(INFO) << TX_FLAGS(export_hashed_debug_info_path);
  LOG(INFO) << TX_FLAGS(lanelink_nudge_delta);

  LOG(INFO) << TX_FLAGS(EnableCrashJam);
  LOG(INFO) << TX_FLAGS(Crash_TimeHead);
  LOG(INFO) << TX_FLAGS(CrashJam_TimeHead);

  LOG(INFO) << TX_FLAGS(AiVehEnvPercptionPeriod);

  LOG(INFO) << TX_FLAGS(CentripetalCatMull_Alpha);
  LOG(INFO) << TX_FLAGS(CentripetalCatMull_Tension);
  LOG(INFO) << TX_FLAGS(VehicleInputMaxSize);

  LOG(INFO) << TX_FLAGS(VehicleOutputTrajectory);
  LOG(INFO) << TX_FLAGS(vehicle_trajectory_duration_s);

  LOG(INFO) << TX_FLAGS(VehicleInputEnable);
  LOG(INFO) << TX_FLAGS(VehicleInputVelocityVariance);

  LOG(INFO) << TX_FLAGS(Use_Arterial);
  LOG(INFO) << TX_FLAGS(behavior_cfg_name);
  LOG(INFO) << TX_FLAGS(RefuseLaneChangeVelocity);
  LOG(INFO) << TX_FLAGS(DebugVehId);
  LOG(INFO) << TX_FLAGS(Filling_ExternInfo);
  LOG(INFO) << TX_FLAGS(ExternInfo_Search_Distance);

  LOG(INFO) << TX_FLAGS(Catalog_VehicleCatalog);
  LOG(INFO) << TX_FLAGS(Catalog_PedestrianCatalog);
  LOG(INFO) << TX_FLAGS(Catalog_MiscObjectCatalog);
  LOG(INFO) << TX_FLAGS(Catalog_Sys_VehicleCatalog);
  LOG(INFO) << TX_FLAGS(Catalog_Sys_PedestrianCatalog);
  LOG(INFO) << TX_FLAGS(Catalog_Sys_MiscObjectCatalog);

  LOG(INFO) << TX_FLAGS(SpeedLimitThreshold);
  LOG(INFO) << TX_FLAGS(params_vehicle_behavior_db);
  LOG(INFO) << TX_FLAGS(EnableSysInfo);
  LOG(INFO) << TX_FLAGS(MapMultiRangeStep);
  LOG(INFO) << TX_FLAGS(EnableCheckRefuseLaneChange);
  LOG(INFO) << TX_FLAGS(EgoCircleSearchRadius);
  LOG(INFO) << TX_FLAGS(elem_near_junction_dist);
  LOG(INFO) << TX_FLAGS(VehicleBehaviorMapping);
  LOG(INFO) << TX_FLAGS(DefaultRuleComplianceProportion);
}
