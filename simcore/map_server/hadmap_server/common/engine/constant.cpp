/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/engine/constant.h"
#include <boost/algorithm/string.hpp>
#include "common/log/system_logger.h"

std::string TrafficFileTypeValue2Str(int nTrafficFileType) {
  std::string strTrafficFileType = "invalid";
  if (nTrafficFileType == TRAFFIC_TYPE_XML) {
    strTrafficFileType = "xml";
  } else if (nTrafficFileType == TRAFFIC_TYPE_XOSC) {
    strTrafficFileType = "xosc";
  } else {
    SYSTEM_LOGGER_ERROR("traffic file type error!");
    strTrafficFileType = "invalid";
  }
  return strTrafficFileType;
}

int TrafficFileTypeStr2Value(std::string strTrafficFileType) {
  int nTrafficFileType = TRAFFIC_TYPE_XML;
  if (boost::iequals(strTrafficFileType, "xml")) {
    nTrafficFileType = TRAFFIC_TYPE_XML;
  } else if (boost::iequals(strTrafficFileType, "xosc")) {
    nTrafficFileType = TRAFFIC_TYPE_XOSC;
  } else {
    SYSTEM_LOGGER_ERROR("traffic file type error!");
    nTrafficFileType = TRAFFIC_TYPE_INVALID;
  }

  return nTrafficFileType;
}

std::string TrigTypeValue2Str(int nTrigType) {
  std::string strTrigType = "timeAbsolute";
  if (nTrigType == TCT_TIME_ABSOLUTE) {
    strTrigType = "timeAbsolute";
  } else if (nTrigType == TCT_TIME_RELATIVE) {
    strTrigType = "timeRelative";
  } else if (nTrigType == TCT_POSITION_ABSOLUTE) {
    strTrigType = "distanceAbsolute";
  } else if (nTrigType == TCT_POSITION_RELATIVE) {
    strTrigType = "distanceRelative";
  } else {
    SYSTEM_LOGGER_ERROR("trig type error!");
    strTrigType = "timeAbsolute";
  }
  return strTrigType;
}

int TrigTypeStr2Value(std::string strTrigType) {
  int nTrigType = TCT_TIME_ABSOLUTE;
  if (boost::iequals(strTrigType, "timeAbsolute")) {
    nTrigType = TCT_TIME_ABSOLUTE;
  } else if (boost::iequals(strTrigType, "timeRelative")) {
    nTrigType = TCT_TIME_RELATIVE;
  } else if (boost::iequals(strTrigType, "distanceAbsolute")) {
    nTrigType = TCT_POSITION_ABSOLUTE;
  } else if (boost::iequals(strTrigType, "distanceRelative")) {
    nTrigType = TCT_POSITION_RELATIVE;
  } else {
    SYSTEM_LOGGER_ERROR("trig type error!");
    nTrigType = TCT_TIME_ABSOLUTE;
  }

  return nTrigType;
}

std::string AccEndConditionTypeValue2Str(int nTrigType) {
  std::string strTrigType = "None";
  if (nTrigType == AECT_NONE) {
    strTrigType = "None";
  } else if (nTrigType == AECT_TIME) {
    strTrigType = "Time";
  } else if (nTrigType == AECT_VELOCITY) {
    strTrigType = "Velocity";
  } else {
    SYSTEM_LOGGER_ERROR("Acceleration end condition type error!");
    strTrigType = "None";
  }
  return strTrigType;
}

int AccEndConditionTypeStr2Value(std::string strTrigType) {
  int nTrigType = AECT_NONE;
  if (boost::iequals(strTrigType, "None")) {
    nTrigType = AECT_NONE;
  } else if (boost::iequals(strTrigType, "Time")) {
    nTrigType = AECT_TIME;
  } else if (boost::iequals(strTrigType, "Velocity")) {
    nTrigType = AECT_VELOCITY;
  } else {
    SYSTEM_LOGGER_ERROR("acceleration end condition type error!");
    nTrigType = AECT_NONE;
  }

  return nTrigType;
}

std::string AccEndConditionTypeValue2FrontStr(int nTrigType) {
  std::string strTrigType = "";
  if (nTrigType == AECT_NONE) {
    strTrigType = "";
  } else if (nTrigType == AECT_TIME) {
    strTrigType = "time";
  } else if (nTrigType == AECT_VELOCITY) {
    strTrigType = "velocity";
  } else {
    SYSTEM_LOGGER_ERROR("Acceleration front end condition type error!");
    strTrigType = "";
  }
  return strTrigType;
}

int AccEndConditionTypeFrontStr2Value(std::string strTrigType) {
  int nTrigType = AECT_NONE;
  // if (boost::iequals(strTrigType, "")) {
  if (strTrigType.size() == 0) {
    nTrigType = AECT_NONE;
  } else if (boost::iequals(strTrigType, "time")) {
    nTrigType = AECT_TIME;
  } else if (boost::iequals(strTrigType, "velocity")) {
    nTrigType = AECT_VELOCITY;
  } else {
    SYSTEM_LOGGER_ERROR("acceleration end condition type error!");
    nTrigType = AECT_NONE;
  }

  return nTrigType;
}

std::string GearTypeValue2Str(int nGearType) {
  std::string strGearType = "drive";
  if (nGearType == GEAR_D) {
    strGearType = "drive";
  } else if (nGearType == GEAR_P) {
    strGearType = "park";
  } else if (nGearType == GEAR_N) {
    strGearType = "neutral";
  } else if (nGearType == GEAR_R) {
    strGearType = "reverse";
  } else if (nGearType == GEAR_M1) {
    strGearType = "manual1";
  } else if (nGearType == GEAR_M2) {
    strGearType = "manual2";
  } else if (nGearType == GEAR_M3) {
    strGearType = "manual3";
  } else if (nGearType == GEAR_M4) {
    strGearType = "manual4";
  } else if (nGearType == GEAR_M5) {
    strGearType = "manual5";
  } else if (nGearType == GEAR_M6) {
    strGearType = "manual6";
  } else if (nGearType == GEAR_M7) {
    strGearType = "manual7";
  } else {
    SYSTEM_LOGGER_ERROR("gear type error!");
    strGearType = "drive";
  }
  return strGearType;
}

int GearTypeStr2Value(std::string strGear) {
  int nGearType = GEAR_D;
  if (boost::iequals(strGear, "drive")) {
    nGearType = GEAR_D;
  } else if (boost::iequals(strGear, "park")) {
    nGearType = GEAR_P;
  } else if (boost::iequals(strGear, "neutral")) {
    nGearType = GEAR_N;
  } else if (boost::iequals(strGear, "reverse")) {
    nGearType = GEAR_R;
  } else if (boost::iequals(strGear, "manual1")) {
    nGearType = GEAR_M1;
  } else if (boost::iequals(strGear, "manual2")) {
    nGearType = GEAR_M2;
  } else if (boost::iequals(strGear, "manual3")) {
    nGearType = GEAR_M3;
  } else if (boost::iequals(strGear, "manual4")) {
    nGearType = GEAR_M4;
  } else if (boost::iequals(strGear, "manual5")) {
    nGearType = GEAR_M5;
  } else if (boost::iequals(strGear, "manual6")) {
    nGearType = GEAR_M6;
  } else if (boost::iequals(strGear, "manual7")) {
    nGearType = GEAR_M7;
  } else {
    SYSTEM_LOGGER_ERROR("gear type error!");
    nGearType = GEAR_D;
  }

  return nGearType;
}

std::string TrigDistanceModeValue2Str(int nDistanceMode) {
  std::string strDistanceMode = "";
  if (nDistanceMode == TDM_NONE) {
    strDistanceMode = "";
  } else if (nDistanceMode == TDM_LANE_PROJECTION) {
    strDistanceMode = "laneprojection";
  } else if (nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
    strDistanceMode = "euclideandistance";
  } else {
    SYSTEM_LOGGER_ERROR("distance mode type error!");
    strDistanceMode = "";
  }
  return strDistanceMode;
}

int TrigDistanceModeStr2Value(std::string strDistanceMode) {
  int nDistanceType = TDM_NONE;
  if (boost::iequals(strDistanceMode, "")) {
    nDistanceType = TDM_NONE;
  } else if (boost::iequals(strDistanceMode, "laneprojection")) {
    nDistanceType = TDM_LANE_PROJECTION;
  } else if (boost::iequals(strDistanceMode, "euclideandistance")) {
    nDistanceType = TDM_EUCLIDEAN_DISTANCE;
  } else {
    SYSTEM_LOGGER_ERROR("distance mode str error!");
    nDistanceType = TDM_NONE;
  }

  return nDistanceType;
}

std::string EgoTypeValue2Str(int nEgoType) {
  std::string strEgoType = "";
  if (nEgoType == ET_INVALID) {
    strEgoType = "suv";
  } else if (nEgoType == ET_SUV) {
    strEgoType = "suv";
  } else if (nEgoType == ET_Truck) {
    strEgoType = "truck";
  } else if (nEgoType == ET_AIV_FullLoad_001) {
    strEgoType = "AIV_FullLoad_001";
  } else if (nEgoType == ET_AIV_Empty_001) {
    strEgoType = "AIV_Empty_001";
  } else if (nEgoType == ET_MPV_001) {
    strEgoType = "MPV_001";
  } else if (nEgoType == ET_AIV_FullLoad_002) {
    strEgoType = "AIV_FullLoad_002";
  } else if (nEgoType == ET_AIV_Empty_002) {
    strEgoType = "AIV_Empty_002";
  } else if (nEgoType == ET_MIFA_01) {
    strEgoType = "MIFA_01";
  } else if (nEgoType == ET_MiningDumpTruck_001) {
    strEgoType = "MiningDumpTruck_001";
  } else if (nEgoType == ET_AIV_V5_FullLoad_001) {
    strEgoType = "AIV_V5_FullLoad_001";
  } else if (nEgoType == ET_AIV_V5_Empty_001) {
    strEgoType = "AIV_V5_Empty_001";
  } else if (nEgoType == ET_Truck_004) {
    strEgoType = "Truck_004";
  } else if (nEgoType == ET_Unloaded_Trailer) {
    strEgoType = "Unloaded_Trailer";
  } else if (nEgoType == ET_Half_loaded_Trailer) {
    strEgoType = "Half_loaded_Trailer";
  } else if (nEgoType == ET_Full_loaded_Trailer) {
    strEgoType = "Full_loaded_Trailer";
  } else {
    SYSTEM_LOGGER_ERROR("ego type error!");
    strEgoType = "suv";
  }
  return strEgoType;
}

int EgoTypeStr2Value(std::string strEgoType) {
  int nEgoType = ET_SUV;
  if (boost::iequals(strEgoType, "")) {
    nEgoType = ET_SUV;
  } else if (boost::iequals(strEgoType, "suv")) {
    nEgoType = ET_SUV;
  } else if (boost::iequals(strEgoType, "truck")) {
    nEgoType = ET_SUV;
  } else if (boost::iequals(strEgoType, "AIV_FullLoad_001")) {
    nEgoType = ET_AIV_FullLoad_001;
  } else if (boost::iequals(strEgoType, "AIV_Empty_001")) {
    nEgoType = ET_AIV_Empty_001;
  } else if (boost::iequals(strEgoType, "MPV_001")) {
    nEgoType = ET_MPV_001;
  } else if (boost::iequals(strEgoType, "AIV_FullLoad_002")) {
    nEgoType = ET_AIV_FullLoad_002;
  } else if (boost::iequals(strEgoType, "AIV_Empty_002")) {
    nEgoType = ET_AIV_Empty_002;
  } else if (boost::iequals(strEgoType, "MIFA_01")) {
    nEgoType = ET_MIFA_01;
  } else if (boost::iequals(strEgoType, "MiningDumpTruck_001")) {
    nEgoType = ET_MiningDumpTruck_001;
  } else if (boost::iequals(strEgoType, "AIV_V5_FullLoad_001")) {
    nEgoType = ET_AIV_V5_FullLoad_001;
  } else if (boost::iequals(strEgoType, "AIV_V5_Empty_001")) {
    nEgoType = ET_AIV_V5_Empty_001;
  } else if (boost::iequals(strEgoType, "Truck_004")) {
    nEgoType = ET_Truck_004;
  } else if (boost::iequals(strEgoType, "Unloaded_Trailer")) {
    nEgoType = ET_Unloaded_Trailer;
  } else if (boost::iequals(strEgoType, "Half_loaded_Trailer")) {
    nEgoType = ET_Half_loaded_Trailer;
  } else if (boost::iequals(strEgoType, "Full_loaded_Trailer")) {
    nEgoType = ET_Full_loaded_Trailer;
  } else {
    SYSTEM_LOGGER_ERROR("Ego type error = %s", strEgoType.c_str());
    nEgoType = ET_INVALID;
  }

  return nEgoType;
}

int MergeModeStr2Value(std::string strMergeType) {
  int nMergeMode = MM_NONE;
  if (boost::iequals(strMergeType, "static")) {
    nMergeMode = MM_NONE;
  } else if (boost::iequals(strMergeType, "left")) {
    nMergeMode = MM_LEFT;
  } else if (boost::iequals(strMergeType, "right")) {
    nMergeMode = MM_RIGHT;
  } else if (boost::iequals(strMergeType, "laneleft")) {
    nMergeMode = MM_LANE_LEFT;
  } else if (boost::iequals(strMergeType, "laneright")) {
    nMergeMode = MM_LANE_RIGHT;
  } else {
    SYSTEM_LOGGER_ERROR("Merge Mode Str error = %s", strMergeType.c_str());
    nMergeMode = MM_NONE;
  }

  return nMergeMode;
}
