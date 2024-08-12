/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include <mapengine/engine_defs.h>
#include <types/map_defs.h>
#include <map>
#include <set>
#include <vector>

extern std::map<std::string, hadmap::MAP_DATA_TYPE> kHadmapTypeDict;

enum HADMAP_ROAD_TYPE {
  HADMAP_ROAD_TYPE_UNKNOWN = hadmap::ROAD_TYPE_UNKNOWN,
  HADMAP_ROAD_TYPE_Normal = hadmap::ROAD_TYPE_Normal,
  HADMAP_ROAD_TYPE_OTHER = hadmap::ROAD_TYPE_OTHER,
  HADMAP_ROAD_TYPE_SA = hadmap::ROAD_TYPE_SA,
  HADMAP_ROAD_TYPE_PA = hadmap::ROAD_TYPE_PA,
  HADMAP_ROAD_TYPE_ControlAccess = hadmap::ROAD_TYPE_ControlAccess,
  HADMAP_ROAD_TYPE_GenerapRamp = hadmap::ROAD_TYPE_GenerapRamp,
  HADMAP_ROAD_TYPE_Overpass = hadmap::ROAD_TYPE_Overpass,
  HADMAP_ROAD_TYPE_Underpass = hadmap::ROAD_TYPE_Underpass,
  HADMAP_ROAD_TYPE_PrivateRoad = hadmap::ROAD_TYPE_PrivateRoad,
  HADMAP_ROAD_TYPE_PedestrainRoad = hadmap::ROAD_TYPE_PedestrainRoad,
  HADMAP_ROAD_TYPE_OverpassForP = hadmap::ROAD_TYPE_OverpassForP,
  HADMAP_ROAD_TYPE_BusOnly = hadmap::ROAD_TYPE_BusOnly,
  HADMAP_ROAD_TYPE_BicycleOnly = hadmap::ROAD_TYPE_BicycleOnly,
  HADMAP_ROAD_TYPE_Tunnel = hadmap::ROAD_TYPE_Tunnel,
  HADMAP_ROAD_TYPE_UTA = hadmap::ROAD_TYPE_UTA,
  HADMAP_ROAD_TYPE_ICEntrance = hadmap::ROAD_TYPE_ICEntrance,
  HADMAP_ROAD_TYPE_TollBooth = hadmap::ROAD_TYPE_TollBooth,
  HADMAP_ROAD_TYPE_ConstructionRoad = hadmap::ROAD_TYPE_ConstructionRoad,
};

enum HADMAP_LINK_TYPE {
  HADMAP_LINK_TYPE_ROAD = hadmap::LINK_TYPE_ROAD,
  HADMAP_LINK_TYPE_LANE = hadmap::LINK_TYPE_LANE,
  HADMAP_LINK_TYPE_JUNCTION = hadmap::LINK_TYPE_JUNCTION,
  HADMAP_LINK_TYPE_CONNECTION = hadmap::LINK_TYPE_CONNECTION,
  HADMAP_LINK_TYPE_LANELINK = hadmap::LINK_TYPE_LANELINK,
};

enum HADMAP_LANE_TYPE {
  HADMAP_LANE_TYPE_None = hadmap::LANE_TYPE_None,
  HADMAP_LANE_TYPE_Driving = hadmap::LANE_TYPE_Driving,
  HADMAP_LANE_TYPE_Stop = hadmap::LANE_TYPE_Stop,
  HADMAP_LANE_TYPE_Shoulder = hadmap::LANE_TYPE_Shoulder,
  HADMAP_LANE_TYPE_Biking = hadmap::LANE_TYPE_Biking,
  HADMAP_LANE_TYPE_Sidewalk = hadmap::LANE_TYPE_Sidewalk,
  HADMAP_LANE_TYPE_Border = hadmap::LANE_TYPE_Border,
  HADMAP_LANE_TYPE_Restricted = hadmap::LANE_TYPE_Restricted,
  HADMAP_LANE_TYPE_Parking = hadmap::LANE_TYPE_Parking,
  HADMAP_LANE_TYPE_MwyEntry = hadmap::LANE_TYPE_MwyEntry,
  HADMAP_LANE_TYPE_MwyExit = hadmap::LANE_TYPE_MwyExit,
  HADMAP_LANE_TYPE_Emergency = hadmap::LANE_TYPE_Emergency,
  HADMAP_LANE_TYPE_TurnWaiting = hadmap::LANE_TYPE_TurnWaiting,
};

enum HADMAP_ROAD_MATERIAL {
  HADMAP_ROAD_MATERIAL_General = hadmap::ROAD_MATERIAL_General,
  HADMAP_ROAD_MATERIAL_Asphalt = hadmap::ROAD_MATERIAL_Asphalt,
  HADMAP_ROAD_MATERIAL_Concrete = hadmap::ROAD_MATERIAL_Concrete,
  HADMAP_ROAD_MATERIAL_Cement = hadmap::ROAD_MATERIAL_Cement,
  HADMAP_ROAD_MATERIAL_Soil = hadmap::ROAD_MATERIAL_Soil,
  HADMAP_ROAD_MATERIAL_Sand = hadmap::ROAD_MATERIAL_Sand,
  HADMAP_ROAD_MATERIAL_Gravel = hadmap::ROAD_MATERIAL_Gravel,
  HADMAP_ROAD_MATERIAL_Brick = hadmap::ROAD_MATERIAL_Brick,
  HADMAP_ROAD_MATERIAL_Cobblestone = hadmap::ROAD_MATERIAL_Cobblestone,
  HADMAP_ROAD_MATERIAL_Composite = hadmap::ROAD_MATERIAL_Composite,
};

enum HADMAP_LANE_MARK {
  HADMAP_LANE_MARK_None = hadmap::LANE_MARK_None,
  HADMAP_LANE_MARK_Solid = hadmap::LANE_MARK_Solid,
  HADMAP_LANE_MARK_Broken = hadmap::LANE_MARK_Broken,
  HADMAP_LANE_MARK_Solid2 = hadmap::LANE_MARK_Solid2,
  HADMAP_LANE_MARK_Broken2 = hadmap::LANE_MARK_Broken2,
  HADMAP_LANE_MARK_SolidSolid = hadmap::LANE_MARK_SolidSolid,
  HADMAP_LANE_MARK_SolidBroken = hadmap::LANE_MARK_SolidBroken,
  HADMAP_LANE_MARK_BrokenSolid = hadmap::LANE_MARK_BrokenSolid,
  HADMAP_LANE_MARK_BrokenBroken = hadmap::LANE_MARK_BrokenBroken,
  // botts dots
  // grass (meaning a grass edge)
  HADMAP_LANE_MARK_Curb = hadmap::LANE_MARK_Curb,
  HADMAP_LANE_MARK_Fence = hadmap::LANE_MARK_Fence,
  HADMAP_LANE_MARK_Bold = hadmap::LANE_MARK_Bold,
  HADMAP_LANE_MARK_Yellow = hadmap::LANE_MARK_Yellow,
  HADMAP_LANE_MARK_Blue = hadmap::LANE_MARK_Blue,
  HADMAP_LANE_MARK_Green = hadmap::LANE_MARK_Green,
  HADMAP_LANE_MARK_Red = hadmap::LANE_MARK_Red,
  // HADMAP_LANE_MARK_Yellow = 0xffff00,
  // HADMAP_LANE_MARK_Blue   = 0x0000ff,
  // HADMAP_LANE_MARK_Green  = 0x00ff00,
  // HADMAP_LANE_MARK_Red    = 0xff0000,
  // HADMAP_LANE_MARK_White  = 0xffffff,
};

enum HADMAP_LANE_ARROW {
  HADMAP_LANE_ARROW_None = hadmap::LANE_ARROW_None,
  HADMAP_LANE_ARROW_Straight = hadmap::LANE_ARROW_Straight,
  HADMAP_LANE_ARROW_Left = hadmap::LANE_ARROW_Left,
  HADMAP_LANE_ARROW_Right = hadmap::LANE_ARROW_Right,
  HADMAP_LANE_ARROW_Uturn = hadmap::LANE_ARROW_Uturn,
  HADMAP_LANE_ARROW_SkewLeft = hadmap::LANE_ARROW_SkewLeft,
  HADMAP_LANE_ARROW_SkewRight = hadmap::LANE_ARROW_SkewRight,
  HADMAP_LANE_ARROW_StraightLeft = hadmap::LANE_ARROW_StraightLeft,
  HADMAP_LANE_ARROW_StraightRight = hadmap::LANE_ARROW_StraightRight,
  HADMAP_LANE_ARROW_LeftRight = hadmap::LANE_ARROW_LeftRight,
  HADMAP_LANE_ARROW_LeftUturn = hadmap::LANE_ARROW_LeftUturn,
  HADMAP_LANE_ARROW_StraightUturn = hadmap::LANE_ARROW_StraightUturn,
};

enum HADMAP_DIR_TYPE {
  HADMAP_DIR_None = hadmap::DIR_None,
  HADMAP_DIR_Straight = hadmap::DIR_Straight,
  HADMAP_DIR_Left = hadmap::DIR_Left,
  HADMAP_DIR_Right = hadmap::DIR_Right,
  HADMAP_DIR_Uturn = hadmap::DIR_Uturn,
};

enum HADMAP_CONTACT_POINT {
  HADMAP_CONTACT_POINT_Start = hadmap::CONTACT_POINT_Start,
  HADMAP_CONTACT_POINT_End = hadmap::CONTACT_POINT_End,
  HADMAP_CONTACT_POINT_RStart = hadmap::CONTACT_POINT_RStart,
  HADMAP_CONTACT_POINT_REnd = hadmap::CONTACT_POINT_REnd,
};

enum HADMAP_OBJECT_TYPE {
  HADMAP_OBJECT_TYPE_None = hadmap::OBJECT_TYPE_None,
  HADMAP_OBJECT_TYPE_Curb = hadmap::OBJECT_TYPE_Curb,
  HADMAP_OBJECT_TYPE_TrafficBarrier = hadmap::OBJECT_TYPE_TrafficBarrier,
  HADMAP_OBJECT_TYPE_OverheadCrossing = hadmap::OBJECT_TYPE_OverheadCrossing,
  HADMAP_OBJECT_TYPE_Tunnel = hadmap::OBJECT_TYPE_Tunnel,
  HADMAP_OBJECT_TYPE_Gantry = hadmap::OBJECT_TYPE_Gantry,
  HADMAP_OBJECT_TYPE_TrafficSign = hadmap::OBJECT_TYPE_TrafficSign,
  HADMAP_OBJECT_TYPE_VariableMessageSign = hadmap::OBJECT_TYPE_VariableMessageSign,
  HADMAP_OBJECT_TYPE_PermanentDelineator = hadmap::OBJECT_TYPE_PermanentDelineator,
  HADMAP_OBJECT_TYPE_TollBooth = hadmap::OBJECT_TYPE_TollBooth,
  HADMAP_OBJECT_TYPE_EmergencyCallbox = hadmap::OBJECT_TYPE_EmergencyCallbox,
  HADMAP_OBJECT_TYPE_Ditch = hadmap::OBJECT_TYPE_Ditch,
  HADMAP_OBJECT_TYPE_Pole = hadmap::OBJECT_TYPE_Pole,
  HADMAP_OBJECT_TYPE_Building = hadmap::OBJECT_TYPE_Building,
  HADMAP_OBJECT_TYPE_RoadSign = hadmap::OBJECT_TYPE_RoadSign,
  HADMAP_OBJECT_TYPE_CrossWalk = hadmap::OBJECT_TYPE_CrossWalk,
  HADMAP_OBJECT_TYPE_TrafficLights = hadmap::OBJECT_TYPE_TrafficLights,
  HADMAP_OBJECT_TYPE_TunnelWall = hadmap::OBJECT_TYPE_TunnelWall,
  HADMAP_OBJECT_TYPE_Arrow = hadmap::OBJECT_TYPE_Arrow,
  HADMAP_OBJECT_TYPE_Text = hadmap::OBJECT_TYPE_Text,
  HADMAP_OBJECT_TYPE_RoadSign_Text = hadmap::OBJECT_TYPE_RoadSign_Text,
  HADMAP_OBJECT_TYPE_Stop = hadmap::OBJECT_TYPE_Stop,
  HADMAP_OBJECT_TYPE_LaneBoundary = hadmap::OBJECT_TYPE_LaneBoundary,
  HADMAP_OBJECT_TYPE_ParkingSpace = hadmap::OBJECT_TYPE_ParkingSpace,
  HADMAP_OBJECT_TYPE_DrivingArea = hadmap::OBJECT_TYPE_DrivingArea,
  HADMAP_OBJECT_TYPE_RR_Parking = hadmap::OBJECT_TYPE_RR_Parking,
};

enum HADMAP_OBJECT_STYLE {
  HADMAP_OBJECT_STYLE_None = hadmap::OBJECT_STYLE_None,
  HADMAP_OBJECT_STYLE_Rectangle = hadmap::OBJECT_STYLE_Rectangle,
  HADMAP_OBJECT_STYLE_Triangle = hadmap::OBJECT_STYLE_Triangle,
  HADMAP_OBJECT_STYLE_Circle = hadmap::OBJECT_STYLE_Circle,
  HADMAP_OBJECT_STYLE_Diamond = hadmap::OBJECT_STYLE_Diamond,
  HADMAP_OBJECT_STYLE_RTriangle = hadmap::OBJECT_STYLE_RTriangle,
  HADMAP_OBJECT_STYLE_Arrow = hadmap::OBJECT_STYLE_Arrow,
  HADMAP_OBJECT_STYLE_Bicycle = hadmap::OBJECT_STYLE_Bicycle,
  HADMAP_OBJECT_STYLE_WheelChair = hadmap::OBJECT_STYLE_WheelChair,
  HADMAP_OBJECT_STYLE_Cross = hadmap::OBJECT_STYLE_Cross,
  HADMAP_OBJECT_STYLE_Polyline = hadmap::OBJECT_STYLE_Polyline,
  HADMAP_OBJECT_STYLE_Polygon = hadmap::OBJECT_STYLE_Polygon,
};

enum HADMAP_OBJECT_GEOMETRY_TYPE {
  HADMAP_OBJECT_GEOMETRY_TYPE_None = hadmap::OBJECT_GEOMETRY_TYPE_None,
  HADMAP_OBJECT_GEOMETRY_TYPE_Point = hadmap::OBJECT_GEOMETRY_TYPE_Point,
  HADMAP_OBJECT_GEOMETRY_TYPE_Polyline = hadmap::OBJECT_GEOMETRY_TYPE_Polyline,
  HADMAP_OBJECT_GEOMETRY_TYPE_Polygon = hadmap::OBJECT_GEOMETRY_TYPE_Polygon,
};

enum HADMAP_OBJECT_COLOR {
  HADMAP_OBJECT_COLOR_Unknown = hadmap::OBJECT_COLOR_Unknown,
  HADMAP_OBJECT_COLOR_White = hadmap::OBJECT_COLOR_White,
  HADMAP_OBJECT_COLOR_Red = hadmap::OBJECT_COLOR_Red,
  HADMAP_OBJECT_COLOR_Yellow = hadmap::OBJECT_COLOR_Yellow,
  HADMAP_OBJECT_COLOR_Blue = hadmap::OBJECT_COLOR_Blue,
  HADMAP_OBJECT_COLOR_Green = hadmap::OBJECT_COLOR_Green,
  HADMAP_OBJECT_COLOR_Black = hadmap::OBJECT_COLOR_Black,
};

enum HADMAP_ROAD_DIRECTION {
  HADMAP_ROAD_DIR_Unknown = 0,
  HADMAP_ROAD_DIR_Two = 1,
  HADMAP_ROAD_DIR_Same = 2,
  HADMAP_ROAD_DIR_Reverse = 3,
};

enum HADMAP_ROAD_FUNCTION {
  HADMAP_ROAD_FUNC_High = 0,
  HADMAP_ROAD_FUNC_City_High = 1,
  HADMAP_ROAD_FUNC_State = 2,
  HADMAP_ROAD_FUNC_Other = 3,
};

enum HADMAP_ROAD_TOLL_STATE {
  HADMAP_ROAD_TOLL_Toll = 1,
  HADMAP_ROAD_TOLL_Free = 2,
  HADMAP_ROAD_TOLL_Free_Along_Toll = 3,
};

typedef hadmap::pkid_t pkid_t;
typedef hadmap::taskpkid taskpkid;
typedef hadmap::roadpkid roadpkid;
typedef hadmap::junctionpkid junctionpkid;
typedef hadmap::sectionpkid sectionpkid;
typedef hadmap::lanepkid lanepkid;
typedef hadmap::lanelinkpkid lanelinkpkid;
typedef hadmap::lanedivisionpkid lanedivisionpkid;
typedef hadmap::laneboundarypkid laneboundarypkid;
typedef hadmap::objectpkid objectpkid;
typedef hadmap::objectdivisionpkid objectdivisionpkid;
typedef hadmap::objectrelpkid objectrelpkid;
typedef hadmap::objectgeompkid objectgeompkid;
typedef hadmap::objectgeomrelpkid objectgeomrelpkid;

struct LaneID {
  roadpkid m_roadID;
  sectionpkid m_sectionID;
  lanepkid m_laneID;

  LaneID();
  explicit LaneID(const hadmap::txLaneId& txlaneid);

  inline LaneID& operator=(const hadmap::txLaneId& txlaneid);
  bool operator<(const LaneID& other) const;
  inline bool operator==(const LaneID& other) const;
};

const char* HadMapRoadType(HADMAP_ROAD_TYPE type);
const char* HadMapRoadMaterial(HADMAP_ROAD_MATERIAL mat);
const char* HadMapObjectType(HADMAP_OBJECT_TYPE type);
const char* HadMapLaneType(HADMAP_LANE_TYPE type);
const char* HadMapLaneArrow(HADMAP_LANE_ARROW arrow);
const char* HadMapLaneMark(HADMAP_LANE_MARK mark);
const char* HadMapObjectColor(HADMAP_OBJECT_COLOR color);

class CRoadData;
class CSectionData;
class CLaneData;
class CLaneBoundaryData;
class CLaneLink;
class CMapObject;
class CEntity;

typedef std::vector<CRoadData*> RoadVec;
typedef std::map<LaneID, CRoadData*> RoadMap;
typedef std::vector<CSectionData*> SectionVec;
typedef std::map<LaneID, CSectionData*> SectionMap;
typedef std::vector<CLaneData*> LaneVec;
typedef std::map<LaneID, CLaneData*> LaneMap;
typedef std::vector<CLaneBoundaryData*> LaneBoundaryVec;
typedef std::map<laneboundarypkid, CLaneBoundaryData*> LaneBoundaryMap;
typedef std::vector<CLaneLink*> LaneLinkVec;
typedef std::map<lanelinkpkid, CLaneLink*> LaneLinkIDMap;
typedef std::map<LaneID, std::set<CLaneLink*> > LaneLinkMap;
typedef std::vector<CMapObject*> MapObjectVec;
typedef std::map<objectpkid, CMapObject*> MapObjectMap;
typedef std::vector<CEntity*> EntityVec;
typedef std::vector<LaneID> LaneIDVec;
