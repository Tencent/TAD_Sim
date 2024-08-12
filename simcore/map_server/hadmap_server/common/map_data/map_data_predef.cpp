/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/map_data/map_data_predef.h"

std::map<std::string, hadmap::MAP_DATA_TYPE> kHadmapTypeDict = {
    {".sqlite", hadmap::MAP_DATA_TYPE::SQLITE},
    {".xodr", hadmap::MAP_DATA_TYPE::OPENDRIVE},
};

LaneID::LaneID() {
  m_roadID = -1;
  m_sectionID = -1;
  m_laneID = -1;
}

LaneID::LaneID(const hadmap::txLaneId& txlaneid) { (*this) = txlaneid; }

LaneID& LaneID::operator=(const hadmap::txLaneId& txlaneid) {
  m_roadID = txlaneid.roadId;
  m_sectionID = txlaneid.sectionId;
  m_laneID = txlaneid.laneId;

  return (*this);
}

bool LaneID::operator<(const LaneID& other) const {
  if (m_roadID < other.m_roadID) {
    return true;
  } else if (m_roadID == other.m_roadID) {
    if (m_sectionID < other.m_sectionID) {
      return true;
    } else if (m_sectionID == other.m_sectionID) {
      if (m_laneID < other.m_laneID) {
        return true;
      }
    }
  }

  return false;
}

inline bool LaneID::operator==(const LaneID& other) const {
  return (m_roadID == other.m_roadID) && (m_sectionID == other.m_sectionID) && (m_laneID == other.m_laneID);
}

const char* HadMapRoadType(HADMAP_ROAD_TYPE type) {
  switch (type) {
    case HADMAP_ROAD_TYPE_UNKNOWN:
      return "UNKNOWN";
    case HADMAP_ROAD_TYPE_Normal:
      return "Normal";
    case HADMAP_ROAD_TYPE_OTHER:
      return "OTHER";
    case HADMAP_ROAD_TYPE_SA:
      return "SA";
    case HADMAP_ROAD_TYPE_PA:
      return "PA";
    case HADMAP_ROAD_TYPE_ControlAccess:
      return "ControlAccess";
    case HADMAP_ROAD_TYPE_GenerapRamp:
      return "GenerapRamp";
    case HADMAP_ROAD_TYPE_Overpass:
      return "Overpass";
    case HADMAP_ROAD_TYPE_Underpass:
      return "Underpass";
    case HADMAP_ROAD_TYPE_PrivateRoad:
      return "PrivateRoad";
    case HADMAP_ROAD_TYPE_PedestrainRoad:
      return "PedestrainRoad";
    case HADMAP_ROAD_TYPE_OverpassForP:
      return "OverpassForP";
    case HADMAP_ROAD_TYPE_BusOnly:
      return "BusOnly";
    case HADMAP_ROAD_TYPE_BicycleOnly:
      return "BicycleOnly";
    case HADMAP_ROAD_TYPE_Tunnel:
      return "Tunnel";
    case HADMAP_ROAD_TYPE_UTA:
      return "UTA";
    case HADMAP_ROAD_TYPE_ICEntrance:
      return "ICEntrance";
    case HADMAP_ROAD_TYPE_TollBooth:
      return "TollBooth";
    case HADMAP_ROAD_TYPE_ConstructionRoad:
      return "ConstructionRoad";
  }

  return "Invalid";
}

const char* HadMapRoadMaterial(HADMAP_ROAD_MATERIAL mat) {
  switch (mat) {
    case HADMAP_ROAD_MATERIAL_General:
      return "General";
    case HADMAP_ROAD_MATERIAL_Asphalt:
      return "Asphalt";
    case HADMAP_ROAD_MATERIAL_Concrete:
      return "Concrete";
    case HADMAP_ROAD_MATERIAL_Cement:
      return "Cement";
    case HADMAP_ROAD_MATERIAL_Soil:
      return "Soil";
    case HADMAP_ROAD_MATERIAL_Sand:
      return "Sand";
    case HADMAP_ROAD_MATERIAL_Gravel:
      return "Gravel";
    case HADMAP_ROAD_MATERIAL_Brick:
      return "Brick";
    case HADMAP_ROAD_MATERIAL_Cobblestone:
      return "Cobblestone";
    case HADMAP_ROAD_MATERIAL_Composite:
      return "Composite";
  }

  return "Invalid";
}
const char* HadMapObjectType(HADMAP_OBJECT_TYPE type) {
  switch (type) {
    case HADMAP_OBJECT_TYPE_None:
      return "None";
    case HADMAP_OBJECT_TYPE_Curb:
      return "Curb";
    case HADMAP_OBJECT_TYPE_TrafficBarrier:
      return "TrafficBarrier";
    case HADMAP_OBJECT_TYPE_OverheadCrossing:
      return "OverheadCrossing";
    case HADMAP_OBJECT_TYPE_Tunnel:
      return "Tunnel";
    case HADMAP_OBJECT_TYPE_Gantry:
      return "Gantry";
    case HADMAP_OBJECT_TYPE_TrafficSign:
      return "TrafficSign";
    case HADMAP_OBJECT_TYPE_VariableMessageSign:
      return "VariableMessageSign";
    case HADMAP_OBJECT_TYPE_PermanentDelineator:
      return "PermanentDelineator";
    case HADMAP_OBJECT_TYPE_TollBooth:
      return "TollBooth";
    case HADMAP_OBJECT_TYPE_EmergencyCallbox:
      return "EmergencyCallbox";
    case HADMAP_OBJECT_TYPE_Ditch:
      return "Ditch";
    case HADMAP_OBJECT_TYPE_Pole:
      return "Pole";
    case HADMAP_OBJECT_TYPE_Building:
      return "Building";
    case HADMAP_OBJECT_TYPE_RoadSign:
      return "RoadSign";
    case HADMAP_OBJECT_TYPE_CrossWalk:
      return "CrossWalk";
    case HADMAP_OBJECT_TYPE_TrafficLights:
      return "TrafficLights";
    case HADMAP_OBJECT_TYPE_TunnelWall:
      return "TunnelWall";
    case HADMAP_OBJECT_TYPE_Arrow:
      return "Arrow";
    case HADMAP_OBJECT_TYPE_Text:
      return "Text";
    case HADMAP_OBJECT_TYPE_RoadSign_Text:
      return "RoadSign_Text";
    case HADMAP_OBJECT_TYPE_Stop:
      return "Stop";
    case HADMAP_OBJECT_TYPE_LaneBoundary:
      return "LaneBoundary";
    case HADMAP_OBJECT_TYPE_ParkingSpace:
      return "ParkingSpace";
    case HADMAP_OBJECT_TYPE_DrivingArea:
      return "DrivingArea";
  }

  return "Invalid";
}
const char* HadMapLaneType(HADMAP_LANE_TYPE type) {
  switch (type) {
    case HADMAP_LANE_TYPE_None:
      return "None";
    case HADMAP_LANE_TYPE_Driving:
      return "Driving";
    case HADMAP_LANE_TYPE_Stop:
      return "Stop";
    case HADMAP_LANE_TYPE_Shoulder:
      return "Shoulder";
    case HADMAP_LANE_TYPE_Biking:
      return "Biking";
    case HADMAP_LANE_TYPE_Sidewalk:
      return "Sidewalk";
    case HADMAP_LANE_TYPE_Border:
      return "Border";
    case HADMAP_LANE_TYPE_Restricted:
      return "Restricted";
    case HADMAP_LANE_TYPE_Parking:
      return "Parking";
    case HADMAP_LANE_TYPE_MwyEntry:
      return "MwyEntry";
    case HADMAP_LANE_TYPE_MwyExit:
      return "MwyExit";
    case HADMAP_LANE_TYPE_Emergency:
      return "Emergency";
    case HADMAP_LANE_TYPE_TurnWaiting:
      return "TurnWaiting";
  }
  return "Invalid";
}
const char* HadMapLaneArrow(HADMAP_LANE_ARROW arrow) {
  switch (arrow) {
    case HADMAP_LANE_ARROW_None:
      return "None";
    case HADMAP_LANE_ARROW_Straight:
      return "Straight";
    case HADMAP_LANE_ARROW_Left:
      return "Left";
    case HADMAP_LANE_ARROW_Right:
      return "Right";
    case HADMAP_LANE_ARROW_Uturn:
      return "Uturn";
    case HADMAP_LANE_ARROW_SkewLeft:
      return "SkewLeft";
    case HADMAP_LANE_ARROW_SkewRight:
      return "SkewRight";
    case HADMAP_LANE_ARROW_StraightLeft:
      return "StraightLeft";
    case HADMAP_LANE_ARROW_StraightRight:
      return "StraightRight";
    case HADMAP_LANE_ARROW_LeftRight:
      return "LeftRight";
    case HADMAP_LANE_ARROW_LeftUturn:
      return "LeftUturn";
    case HADMAP_LANE_ARROW_StraightUturn:
      return "StraightUturn";
  }
  return "Invalid";
}
const char* HadMapLaneMark(HADMAP_LANE_MARK mark) {
  switch (mark) {
    case HADMAP_LANE_MARK_None:
      return "None";
    case HADMAP_LANE_MARK_Solid:
      return "Solid";
    case HADMAP_LANE_MARK_Broken:
      return "Broken";
    case HADMAP_LANE_MARK_Solid2:
      return "Solid2";
    case HADMAP_LANE_MARK_Broken2:
      return "Broken2";
    case HADMAP_LANE_MARK_SolidSolid:
      return "SolidSolid";
    case HADMAP_LANE_MARK_SolidBroken:
      return "SolidBroken";
    case HADMAP_LANE_MARK_BrokenSolid:
      return "BrokenSolid";
    case HADMAP_LANE_MARK_BrokenBroken:
      return "BrokenBroken";
    case HADMAP_LANE_MARK_Curb:
      return "Curb";
    case HADMAP_LANE_MARK_Fence:
      return "Fence";
    case HADMAP_LANE_MARK_Bold:
      return "Bold";
    case HADMAP_LANE_MARK_Yellow:
      return "Yellow";
  }
  return "Invalid";
}
const char* HadMapObjectColor(HADMAP_OBJECT_COLOR color) {
  switch (color) {
    case HADMAP_OBJECT_COLOR_Unknown:
      return "unknown";
    case HADMAP_OBJECT_COLOR_White:
      return "white";
    case HADMAP_OBJECT_COLOR_Red:
      return "red";
    case HADMAP_OBJECT_COLOR_Yellow:
      return "yellow";
    case HADMAP_OBJECT_COLOR_Blue:
      return "blue";
    case HADMAP_OBJECT_COLOR_Green:
      return "green";
    case HADMAP_OBJECT_COLOR_Black:
      return "black";
    default:
      break;
  }
  return "Invalid";
}
