/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

enum TrafficFileType {
  TRAFFIC_TYPE_INVALID = -1,
  TRAFFIC_TYPE_XML = 0,
  TRAFFIC_TYPE_XOSC,
  TRAFFIC_TYPE_COUNT,
};

std::string TrafficFileTypeValue2Str(int nTrafficFileType);

int TrafficFileTypeStr2Value(std::string strTrafficFileType);

enum AABBIntersection {
  AABB_Invalid = -1,
  AABB_Inside = 0,
  AABB_Intersect = 1,
  AABB_Outside = 2,
};

enum TrigConditionType {
  TCT_INVALID = -1,
  TCT_START = 0,
  TCT_TIME_ABSOLUTE = 0,
  TCT_TIME_RELATIVE = 1,
  TCT_POSITION_RELATIVE = 2,
  TCT_POSITION_ABSOLUTE = 3,
  TCT_END = TCT_POSITION_ABSOLUTE,
};

std::string TrigTypeValue2Str(int nTrigType);

int TrigTypeStr2Value(std::string strTrigType);

enum AccelerationEndConditionType { AECT_INVALID = -1, AECT_NONE = 0, AECT_TIME = 1, AECT_VELOCITY = 2 };

std::string AccEndConditionTypeValue2Str(int nTrigType);

int AccEndConditionTypeStr2Value(std::string strTrigType);

std::string AccEndConditionTypeValue2FrontStr(int nType);

int AccEndConditionTypeFrontStr2Value(std::string strType);

enum SceneBatchGenExportFlagType {
  NOT_EXPORT_TO_TADSIM = 0,
  EXPORT_TO_TADSIM = 1,
};

enum SceneBatchGenType {
  SCENE_BATCH_GEN_TYPE_SINGLE = 0,
  SCENE_BATCH_GEN_TYPE_ALL = 1,
};

enum SceneType {
  SCENE_TYPE_INVALID = -1,
  SCENE_TYPE_SIM = 0,
  SCENE_TYPE_XOSC = 1,
  SCENE_TYPE_COUNT,
};

enum ControlGear {
  GEAR_INVALID = -1,
  GEAR_D = 0,
  GEAR_P = 1,
  GEAR_R = 2,
  GEAR_N = 3,
  GEAR_M1 = 4,
  GEAR_M2 = 5,
  GEAR_M3 = 6,
  GEAR_M4 = 7,
  GEAR_M5 = 8,
  GEAR_M6 = 9,
  GEAR_M7 = 10,
  GEAR_M8 = 11,
  GEAR_M9 = 12,
};

std::string GearTypeValue2Str(int nTrigType);

int GearTypeStr2Value(std::string strTrigType);

enum TrigDistanceMode {
  TDM_INVALID = -1,
  TDM_NONE = 0,
  TDM_LANE_PROJECTION = 1,
  TDM_EUCLIDEAN_DISTANCE = 2,
  TDM_END = TDM_EUCLIDEAN_DISTANCE
};

std::string TrigDistanceModeValue2Str(int nTrigType);

int TrigDistanceModeStr2Value(std::string strTrigType);

enum EgoType {
  ET_INVALID = -1,
  ET_SUV = 0,
  ET_Truck = 1,
  ET_AIV_FullLoad_001 = 2,
  ET_AIV_Empty_001 = 3,
  ET_MPV_001 = 4,
  ET_AIV_FullLoad_002 = 5,
  ET_AIV_Empty_002 = 6,
  ET_MIFA_01 = 7,
  ET_MiningDumpTruck_001 = 8,
  ET_AIV_V5_FullLoad_001 = 9,
  ET_AIV_V5_Empty_001 = 10,
  ET_Truck_004 = 12,
  ET_Unloaded_Trailer = 13,
  ET_Half_loaded_Trailer = 14,
  ET_Full_loaded_Trailer = 15,
};

std::string EgoTypeValue2Str(int nEgoType);

int EgoTypeStr2Value(std::string strEgoType);

enum MergeMode { MM_INVALID = -999, MM_NONE = 0, MM_LEFT = 1, MM_RIGHT = -1, MM_LANE_LEFT = 2, MM_LANE_RIGHT = -2 };
int MergeModeStr2Value(std::string strMergeType);
