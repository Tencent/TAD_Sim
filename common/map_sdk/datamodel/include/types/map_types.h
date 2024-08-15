// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

namespace hadmap {
enum ROAD_TYPE {
  ROAD_TYPE_UNKNOWN = 9999,
  ROAD_TYPE_Normal = 0,
  ROAD_TYPE_OTHER = 1,
  ROAD_TYPE_SA = 2,  // Service Area
  ROAD_TYPE_PA = 3,  // Parking Area
  ROAD_TYPE_ControlAccess = 4,
  ROAD_TYPE_GenerapRamp = 5,
  ROAD_TYPE_Overpass = 6,
  ROAD_TYPE_Underpass = 7,
  ROAD_TYPE_PrivateRoad = 8,
  ROAD_TYPE_PedestrainRoad = 9,
  ROAD_TYPE_OverpassForP = 10,  // Overpass for passenger
  ROAD_TYPE_BusOnly = 11,
  ROAD_TYPE_BicycleOnly = 12,  // Bicycle only30 Bridge
  ROAD_TYPE_Tunnel = 13,
  ROAD_TYPE_UTA = 14,  // Undefined Traffic Figure
  ROAD_TYPE_ICEntrance = 15,
  ROAD_TYPE_TollBooth = 16,
  ROAD_TYPE_ConstructionRoad = 17,
  ROAD_TYPE_Roundabout = 18,
  ROAD_TYPE_Intersection = 19,
  ROAD_TYPE_OccpationAvailable = 20,
  ROAD_TYPE_ODR_Rural = 21,
  ROAD_TYPE_ODR_Motorway = 22,
  ROAD_TYPE_ODR_Town = 23,
  ROAD_TYPE_ODR_LowSpeed = 24,
  ROAD_TYPE_ODR_Pedestrian = 25,
};

enum ROAD_DIRECTION {
  ROAD_DIRECTION_UNKNONW = 0,
  ROAD_DIRECTION_BOTH = 1,
  ROAD_DIRECTION_FORWARD = 2,
  ROAD_DIRECTION_BACKWARD = 3,
};

enum ROAD_TOLL {
  ROAD_TOLL_UNKNONW = 0,
  ROAD_TOLL_NEED = 1,
  ROAD_TOLL_FREE = 2,
  ROAD_TOLL_FREESECTION = 3,
};

enum ROAD_FUNCLASS {
  ROAD_FUNCLASS_EXPRESS_WAY = 0,
  ROAD_FUNCLASS_CITYEXPRESS_WAY = 1,
  ROAD_FUNCLASS_NATION_ROAD = 2,
  ROAD_FUNCLASS_PROVINCE_ROAD = 3,
  ROAD_FUNCLASS_COUNTY_ROAD = 4,
  ROAD_FUNCLASS_TOWN_ROAD = 6,
  ROAD_FUNCLASS_OTHER_ROAD = 8,
  ROAD_FUNCLASS_FRONTAGE_ROAD = 9,
  ROAD_FUNCLASS_FERRY_WAY = 10,
  ROAD_FUNCLASS_WALK_WAY = 11,
};

enum ROAD_URBAN {
  ROAD_URBAN_NO = 0,
  ROAD_URBAN_YES = 1,
};

enum ROAD_PAVED {
  ROAD_PAVED_NO = 0,
  ROAD_PAVED_YES = 1,
};

enum BOUNDARY_WIDTH {
  BOUNDARY_WIDTH_5cm = 5,
  BOUNDARY_WIDTH_8cm = 8,
  BOUNDARY_WIDTH_15cm = 15,
  BOUNDARY_WIDTH_30cm = 30,
  BOUNDARY_WIDTH_45cm = 45,
};

enum LINK_TYPE {
  LINK_TYPE_ROAD = 0,
  LINK_TYPE_LANE = 1,
  LINK_TYPE_JUNCTION = 2,
  LINK_TYPE_CONNECTION = 3,
  LINK_TYPE_LANELINK = 4
};

enum LANE_TYPE {
  LANE_TYPE_Others = 9999,
  LANE_TYPE_None = 0,
  LANE_TYPE_Driving = 1,
  LANE_TYPE_Stop = 2,
  LANE_TYPE_Shoulder = 3,
  LANE_TYPE_Biking = 4,
  LANE_TYPE_Sidewalk = 5,
  LANE_TYPE_Border = 6,
  LANE_TYPE_Restricted = 7,  // BUS
  LANE_TYPE_Parking = 8,
  LANE_TYPE_MwyEntry = 9,
  LANE_TYPE_MwyExit = 10,
  LANE_TYPE_Acceleration = 11,
  LANE_TYPE_Deceleration = 12,
  LANE_TYPE_Compound = 13,
  LANE_TYPE_HOV = 14,
  LANE_TYPE_Slow = 15,
  LANE_TYPE_Reversible = 16,
  LANE_TYPE_EmergencyParking = 17,
  LANE_TYPE_Emergency = 20,
  LANE_TYPE_TurnWaiting = 21,
  LANE_TYPE_TollBooth = 22,
  LANE_TYPE_ETC = 23,
  LANE_TYPE_CheckStation = 24,
  LANE_TYPE_MixVehicle = 25,
  LANE_TYPE_BusOnly = 26,
  LANE_TYPE_BIDIRECTIONAL = 27,
  LANE_TYPE_CONNECTINGRAMP = 29,
  LANE_TYPE_CURB = 30,
  LANE_TYPE_ENTRY = 31,
  LANE_TYPE_EXIT = 32,
  LANE_TYPE_MEDIAN = 33,
  LANE_TYPE_OFFRAMP = 34,
  LANE_TYPE_ONRAMP = 35,
  LANE_TYPE_RAIL = 36,
  LANE_TYPE_ROADWORKS = 37,
  LANE_TYPE_TAXI = 38,
  LANE_TYPE_TRAM = 39,
};

enum LANE_SPEEDLIMIT_SOURCE {
  LANE_SPEEDLIMIT_SOURCE_Onsite = 1,
  LANE_SPEEDLIMIT_SOURCE_Theory = 2,
};

enum ROAD_MATERIAL {
  ROAD_MATERIAL_General = 0,
  ROAD_MATERIAL_Asphalt,
  ROAD_MATERIAL_Concrete,
  ROAD_MATERIAL_Cement,
  ROAD_MATERIAL_Soil,
  ROAD_MATERIAL_Sand,
  ROAD_MATERIAL_Gravel,
  ROAD_MATERIAL_Brick,
  ROAD_MATERIAL_Cobblestone,
  ROAD_MATERIAL_Composite,
};

enum LANE_MARK {
  LANE_MARK_None = 0,
  LANE_MARK_Solid = 0x1,
  LANE_MARK_Broken = 0x2,
  LANE_MARK_Solid2 = 0x10,
  LANE_MARK_Broken2 = 0x20,
  LANE_MARK_SolidSolid = LANE_MARK_Solid | LANE_MARK_Solid2,  // (for double solid line)
  LANE_MARK_SolidBroken =
      LANE_MARK_Solid | LANE_MARK_Broken2,  // (from inside to outside, exception: center lane - from left to right)
  LANE_MARK_BrokenSolid =
      LANE_MARK_Broken | LANE_MARK_Solid2,  // (from inside to outside, exception: center lane - from left to right)
  LANE_MARK_BrokenBroken = LANE_MARK_Broken | LANE_MARK_Broken2,
  // botts dots
  // grass (meaning a grass edge)
  LANE_MARK_Curb = 0x100,
  LANE_MARK_Fence = 0x200,
  LANE_MARK_RoadEdge = 0x400,
  LANE_MARK_Bold = 0x1000,     // bold lines
  LANE_MARK_Yellow = 0x10000,  // yellow lines, default white,blue green red
  LANE_MARK_Yellow2 = 0x20000,
  LANE_MARK_YellowYellow = 0x30000,
  LANE_MARK_OtherColors = 0x40000,
  LANE_MARK_Red = 0x50000,
  LANE_MARK_Green = 0x60000,
  LANE_MARK_Blue = 0x70000,
  LANE_MARK_LDM = 0x100000,
  LANE_MARK_VGL = 0x200000,

  LANE_MARK_Shielded = 0x1000000,  // virture mark
  LANE_MARK_Absent = 0x2000000     // virture mark
};
enum LANE_SURFACE {
  LANE_SURFACE_CEMENT = 0x01,       // 水泥
  LANE_SURFACE_ASPHALT = 0x02,      // 柏油路
  LANE_SURFACE_COBBLESTONE = 0x03,  // 石头路
  LANE_SURFACE_DIRT = 0x04,         // 土路

  LANE_SURFACE_OTHER = 0x99
};

enum LANE_ARROW {
  LANE_ARROW_None = 0x0,
  LANE_ARROW_Straight = 0x1,
  LANE_ARROW_Left = 0x2,
  LANE_ARROW_Right = 0x4,
  LANE_ARROW_Uturn = 0x8,
  LANE_ARROW_SkewLeft = 0x20,
  LANE_ARROW_SkewRight = 0x40,
  LANE_ARROW_StraightLeft = LANE_ARROW_Straight | LANE_ARROW_Left,
  LANE_ARROW_StraightRight = LANE_ARROW_Straight | LANE_ARROW_Right,
  LANE_ARROW_LeftRight = LANE_ARROW_Left | LANE_ARROW_Right,
  LANE_ARROW_LeftUturn = LANE_ARROW_Left | LANE_ARROW_Uturn,
  LANE_ARROW_StraightUturn = LANE_ARROW_Straight | LANE_ARROW_Uturn,
};

enum DIR_TYPE { DIR_None = 0x0, DIR_Straight = 0x1, DIR_Left = 0x2, DIR_Right = 0x4, DIR_Uturn = 0x8 };

enum PRIORITY_TYPE { PRIORITY_NONE = 0, PRIORITY_SAME = 1, PRIORITY_HIGH = 2, PRIORITY_LOW = 3 };

enum CONTACT_POINT { CONTACT_POINT_Start = 0, CONTACT_POINT_End = 1, CONTACT_POINT_RStart = 2, CONTACT_POINT_REnd = 3 };

enum OBJECT_CATE {
  OBJECT_CATE_None = 0,
  OBJECT_CATE_Curb = 1,
  OBJECT_CATE_TrafficBarrier = 2,
  OBJECT_CATE_OverheadCrossing = 3,
  OBJECT_CATE_Tunnel = 4,
  OBJECT_CATE_Gantry = 5,
  OBJECT_CATE_TrafficSign = 6,
  OBJECT_CATE_VariableMessageSign = 7,
  OBJECT_CATE_PermanentDelineator = 8,
  OBJECT_CATE_TollBooth = 9,
  OBJECT_CATE_EmergencyCallbox = 10,
  OBJECT_CATE_Ditch = 11,
  OBJECT_CATE_Pole = 12,
  OBJECT_CATE_Building = 13,
  OBJECT_CATE_RoadSign = 14,
  OBJECT_CATE_CrossWalk = 15,
  OBJECT_CATE_TrafficLights = 16,
  OBJECT_CATE_TunnelWall = 17,
  OBJECT_CATE_Arrow = 18,
  OBJECT_CATE_Text = 19,
  OBJECT_CATE_RoadSign_Text = 20,
  OBJECT_CATE_Stop = 21,
  OBJECT_CATE_LaneBoundary = 22,
  OBJECT_CATE_ParkingSpace = 23,
  OBJECT_CATE_DrivingArea = 24,
  OBJECT_CATE_FeaturePoint = 25,
};

enum OBJECT_TYPE {
  OBJECT_TYPE_None = 0,
  OBJECT_TYPE_Curb = 1,
  OBJECT_TYPE_TrafficBarrier = 2,
  OBJECT_TYPE_OverheadCrossing = 3,
  OBJECT_TYPE_Tunnel = 4,
  OBJECT_TYPE_Gantry = 5,
  OBJECT_TYPE_TrafficSign = 6,
  OBJECT_TYPE_VariableMessageSign = 7,
  OBJECT_TYPE_PermanentDelineator = 8,
  OBJECT_TYPE_TollBooth = 9,
  OBJECT_TYPE_EmergencyCallbox = 10,
  OBJECT_TYPE_Ditch = 11,
  OBJECT_TYPE_Pole = 12,  // 杆
  OBJECT_TYPE_Building = 13,
  OBJECT_TYPE_RoadSign = 14,
  OBJECT_TYPE_CrossWalk = 15,
  OBJECT_TYPE_TrafficLights = 16,
  OBJECT_TYPE_TunnelWall = 17,
  OBJECT_TYPE_Arrow = 18,
  OBJECT_TYPE_Text = 19,
  OBJECT_TYPE_RoadSign_Text = 20,
  OBJECT_TYPE_Stop = 21,
  OBJECT_TYPE_LaneBoundary = 22,
  OBJECT_TYPE_ParkingSpace = 23,
  OBJECT_TYPE_DrivingArea = 24,
  OBJECT_TYPE_FeaturePoint = 25,
  OBJECT_TYPE_LineSegment_PL = 26,
  OBJECT_TYPE_TrafficLight = 27,  // the light
  OBJECT_TYPE_BikeRoadArea = 28,
  OBJECT_TYPE_NoStopArea = 29,
  OBJECT_TYPE_SpeedBump = 30,
  OBJECT_TYPE_VerticalWall = 31,
  OBJECT_TYPE_BusStation = 32,
  OBJECT_TYPE_Wall = 33,
  OBJECT_TYPE_StationArea = 34,
  OBJECT_TYPE_ChargingPile = 35,
  OBJECT_TYPE_Surface = 36,
  OBJECT_TYPE_Obstacle = 37,
  OBJECT_TYPE_Tree = 38,
  OBJECT_TYPE_PedestrianBridge = 39,
  OBJECT_TYPE_RR_Parking = 1001,  // roadrunning parking
  OBJECT_TYPE_V2X_Sensor = 1002,  // v2x
  OBJECT_TYPE_Custom = 9999,      // custom
};

enum OBJECT_SUB_TYPE {
  OtherSubtype = 0,
  // Tunnel
  Tunnel_001 = 401,
  // pole
  POLE_VERTICAL = 1201,
  POLE_CROSS = 1202,
  POLE_Cantilever = 1203,
  POLE_Pillar_6m = 1204,
  POLE_Pillar_3m = 1205,
  // building
  Building_001 = 1301,
  // RoadSign
  RoadSign_Turn_Left_Waiting = 1401,
  RoadSign_Crosswalk_Warning_Line = 1402,
  RoadSign_White_Broken_Line = 1403,
  RoadSign_White_Semicircle_Line = 1404,
  RoadSign_100_120 = 1405,
  RoadSign_80_100 = 1406,
  RoadSign_Non_Motor_Vehicle = 1407,
  RoadSign_Disable_Parking = 1408,
  RoadSign_Lateral_Dec = 1409,
  RoadSign_Longitudinal_Dec = 1410,
  RoadSign_Circular_Center = 1411,
  RoadSign_Rhombus_Center = 1412,
  RoadSign_Mesh_Line = 1413,
  RoadSign_BusOnly = 1414,
  RoadSign_SmallCarOnly = 1415,
  RoadSign_BigCarOnly = 1416,
  RoadSign_Non_Motor_Vehicle_Line = 1417,
  RoadSign_Turning_Forbidden = 1418,
  RoadSign_Crosswalk_with_Left_and_Right_Side = 1420,
  RoadSign_Road_Guide_Lane_Line = 1421,
  RoadSign_Variable_Direction_Lane_Line = 1422,
  RoadSign_Intersection_Guide_Line = 1423,
  RoadSign_U_Turning_Forbidden = 1426,
  RoadSign_Non_Motor_Vehicle_Area = 1427,
  // cross walk
  CrossWalk_001 = 1501,
  // TrafficLights
  LIGHT_VERTICAL_ALLDIRECT = 1601,
  LIGHT_VERTICAL_STRAIGHTROUND = 1602,
  LIGHT_VERTICAL_LEFTTURN = 1603,
  LIGHT_VERTICAL_STRAIGHT = 1604,
  LIGHT_VERTICAL_RIGHTTURN = 1605,
  LIGHT_VERTICAL_UTURN = 1606,
  LIGHT_HORIZON_PEDESTRIAN = 1607,
  LIGHT_HORIZON_ALLDIRECT = 1608,
  LIGHT_HORIZON_STRAIGHTROUND = 1609,
  LIGHT_HORIZON_LEFTTURN = 1610,
  LIGHT_HORIZON_STRAIGHT = 1611,
  LIGHT_HORIZON_RIGHTTURN = 1612,
  LIGHT_HORIZON_UTURN = 1613,
  LIGHT_BICYCLELIGHT = 1614,
  LIGHT_TWOCOLOR = 1615,

  // Arrow
  Arrow_Straight = 1801,
  Arrow_StraightLeft = 1802,
  Arrow_Left = 1803,
  Arrow_Right = 1804,
  Arrow_StraightRight = 1805,
  Arrow_Uturn = 1806,
  Arrow_StraightUturn = 1807,
  Arrow_LeftUturn = 1808,
  Arrow_LeftRight = 1809,
  Arrow_TurnStraight = 1810,
  Arrow_SkewLeft = 1811,
  Arrow_SkewRight = 1812,  // 向右合流
  // stop
  Stop_Line = 2101,
  Stop_Line_GiveWay = 2102,
  Stop_Slow_Line_GiveWay = 2103,

  // pacespace
  Parking_Space_001 = 2301,
  Parking_Space_002 = 2302,
  Parking_Space_003 = 2303,  // 6m
  Parking_Space_004 = 2304,  // 5m
  Parking_Space_005 = 2305,  // 45 deg
  Parking_Space_006 = 2306,  // 60 deg

  // speeddump
  SpeedBump_001 = 3001,

  // busStation
  BusStation_001 = 3201,

  // ChargingPile
  ChargingPile_001 = 3501,

  // roadsurface
  Surface_Pothole = 3601,
  Surface_Patch = 3602,
  Surface_Crack = 3603,
  Surface_Asphalt_Line = 3604,
  Surface_Rut_Track = 3605,
  Surface_Stagnant_Water = 3606,
  Surface_Protrusion = 3607,
  Surface_Well_Cover = 3608,

  // obstacle
  Obstacle_Reflective_Road_Sign = 3701,
  Obstacle_Parking_Hole = 3702,
  Obstacle_Parking_Lot = 3703,
  Obstacle_Ground_Lock = 3704,
  Obstacle_Plastic_Vehicle_Stopper = 3705,
  Obstacle_Parking_Limit_Position_Pole_2m = 3706,
  Obstacle_Support_Vehicle_Stopper = 3707,
  Obstacle_Traffic_Barrier = 3708,
  Obstacle_Road_Curb = 3709,
  Obstacle_Lamp = 3710,
  Obstacle_Traffic_Cone = 3711,
  Obstacle_Traffic_Horse = 3712,
  Obstacle_GarbageCan = 3713,
  Obstacle_Obstacle = 3714,

  // tree
  Tree_001 = 3801,
  Shrub_001 = 3802,
  Grass_001 = 3803,

  // traffic sign (warning sign)
  SIGN_WARNING_SLOWDOWN = 6001,
  SIGN_WARNING_TURNLEFT,
  SIGN_WARNING_TURNRIGHT,
  SIGN_WARNING_T_RIGHT,
  SIGN_WARNING_T_DOWN,
  SIGN_WARNING_CROSS,
  SIGN_WARNING_CHILD,
  SIGN_WARNING_UP,
  SIGN_WARNING_BUILDING,
  SIGN_WARNING_NARROW_LEFT,
  SIGN_WARNING_NARROW_BOTH,
  SIGN_WARNING_RAILWAY,
  SIGN_WARNING_T_LEFT,
  SIGN_WARNING_STEEP_LEFT,
  SIGN_WARNING_STEEP_RIGHT,
  SIGN_WARNING_VILLAGE,
  SIGN_WARNING_DIKE_LEFT,
  SIGN_WARNING_DIKE_RIGHT,
  SIGN_WARAING_T_CROSSED,
  SIGN_WARAING_FERRY,
  SIGN_WARAING_FALL_ROCK,
  SIGN_WARAING_REVERSE_CURVE_LEFT,
  SIGN_WARAING_REVERSE_CURVE_RIGHT,
  SIGN_WARAING_WATER_PAVEMENT,
  SIGN_WARNING_T_BOTH,
  SIGN_WARNING_JOIN_LEFT,
  SIGN_WARNING_JOIN_RIGHT,
  SIGN_WARNING_Y_LEFT,
  SIGN_WARNING_CIRCLE_CROSS,
  SIGN_WARNING_Y_RIGHT,
  SIGN_WARNING_CURVE_AHEAD,
  SIGN_WARNING_LONG_DESCENT,
  SIGN_WARNING_ROUGH_ROAD,
  SIGN_WARNING_SNOW,
  SIGN_WARNING_DISABLE,
  SIGN_WARNING_ANIMALS,
  SIGN_WARNING_ACCIDENT,
  SIGN_WARNING_TIDALBU_LANE,
  SIGN_WARNING_BAD_WEATHER,
  SIGN_WARNING_LOWLYING,
  SIGN_WARNING_HIGHLYING,
  SIGN_WARNING_DOWNHILL,
  SIGN_WARNING_QUEUESLIKELY,
  SIGN_WARNING_CROSS_PLANE,
  SIGN_WARNING_TUNNEL,
  SIGN_WARNING_TUNNEL_LIGHT,
  SIGN_WARNING_HUMPBACK_BRIDGE,
  SIGN_WARNING_NARROW_RIGHT,
  SIGN_WARNING_NON_MOTOR,
  SIGN_WARNING_SLIPPERY,
  SIGN_WARNING_TRIFFICLIGHT,
  SIGN_WARNING_DETOUR_RIGHT,
  SIGN_WARNING_NARROW_BRIDGE,
  SIGN_WARNING_KEEP_DISTANCE,
  SIGN_WARNING_MERGE_LEFT,
  SIGN_WARNING_MERGE_RIGHT,
  SIGN_WARNING_CROSSWIND,
  SIGN_WARNING_ICY_ROAD,
  SIGN_WARNING_ROCKFALL,
  SIGN_WARNING_CAUTION,
  SIGN_WARNING_FOGGY,
  SIGN_WARNING_LIVESTOCK,
  SIGN_WARNING_DETOUR_LEFT,
  SIGN_WARNING_DETOUR_BOTH,
  SIGN_WARNING_BOTHWAY,
  SIGN_BAN_STRAIGHT,
  SIGN_BAN_VEHICLE,
  SIGN_BAN_SPPED_120,
  SIGN_BAN_SPPED_100,
  SIGN_BAN_SPPED_80,
  SIGN_BAN_SPPED_70,
  SIGN_BAN_SPPED_60,
  SIGN_BAN_SPPED_50,
  SIGN_BAN_SPPED_40,
  SIGN_BAN_SPPED_30,
  SIGN_BAN_STOP_YIELD,
  SIGN_BAN_HEIGHT_5,
  SIGN_BAN_SPPED_20,
  SIGN_BAN_SPPED_05,
  SIGN_BAN_DIVERINTO,
  SIGN_BAN_MOTOR_BIKE,
  SIGN_BAN_WEIGHT_50,
  SIGN_BAN_WEIGHT_20,
  SIGN_BAN_HONKING,
  SIGN_BAN_TRUCK,
  SIGN_BAN_WEIGHT_30,
  SIGN_BAN_WEIGHT_10,
  SIGN_BAN_TEMP_PARKING,
  SIGN_BAN_AXLE_WEIGHT_14,
  SIGN_BAN_AXLE_WEIGHT_13,
  SIGN_BAN_WEIGHT_40,
  SIGN_BAN_SLOW,
  SIGN_BAN_TURN_LEFT,
  SIGN_BAN_DANGEROUS_GOODS,
  SIGN_BAN_TRACTORS,
  SIGN_BAN_TRICYCLE,
  SIGN_BAN_MINIBUS,
  SIGN_BAN_STRAIGHT_AND_LEFT,
  SIGN_BAN_VEHICLE_BY_HUMAN,
  SIGN_BAN_TRACYCLE01_BY_HUMAN,
  SIGN_BAN_TRACYCLE02_BY_HUMAN,
  SIGN_BAN_TURN_RIGHT,
  SIGN_BAN_LEFT_AND_RIGHT,
  SIGN_BAN_STRAIGHT_AND_RIGHT,
  SIGN_BAN_GO,
  SIGN_BAN_GIVE_WAY,
  SIGN_BAN_BUS_TURN_RIGHT,
  SIGN_BAN_TRUCK_TURN_RIGHT,
  SIGN_BAN_BYCICLE_DOWN,
  SIGN_BAN_BYCICLE_UP,
  SIGN_BAN_NO_OVERTAKING,
  SIGN_BAN_BUS_TURN_LEFT,
  SIGN_BAN_OVERTAKING,
  SIGN_BAN_ANIMALS,
  SIGN_BAN_BUS,
  SIGN_BAN_ELECTRO_TRICYCLE,
  SIGN_BAN_NO_MOTOR,
  SIGN_BAN_TRUCK_TURN_LEFT,
  SIGN_BAN_TRAILER,
  SIGN_BAN_HUMAN,
  SIGN_BAN_THE_TWO_TYPES,
  SIGN_BAN_HEIGHT_3_5,
  SIGN_BAN_HEIGHT_3,
  SIGN_BAN_AXLE_WEIGHT_10,
  SIGN_BAN_CUSTOMS_MARK,
  SIGN_BAN_STOP,
  SIGN_BAN_LONG_PARKING,
  SIGN_BAN_REMOVE_LIMIT_40,
  SIGN_INDOCATION_STRAIGHT,
  SIGN_INDOCATION_LOWEST_SPEED_60,
  SIGN_INDOCATION_LOWEST_SPEED_40,
  SIGN_INDOCATION_ALONG_RIGHT,
  SIGN_INDOCATION_PEDESTRIAN_CROSSING,
  SIGN_INDOCATION_TURN_RIGHT,
  SIGN_INDOCATION_ROUNDABOUT,
  SIGN_INDOCATION_TURN_LEFT,
  SIGN_INDOCATION_STRAIGHT_RIGHT,
  SIGN_INDOCATION_STRAIGHT_LEFT,
  SIGN_INDOCATION_LOWEST_SPEED_50,
  SIGN_INDOCATION_WALK,
  SIGN_INDOCATION_NO_MOTOR,
  SIGN_INDOCATION_MOTOR,
  SIGN_INDOCATION_ALONG_LEFT,
  SIGN_INDOCATION_PASS_STAIGHT_001,
  SIGN_INDOCATION_PASS_STAIGHT_002,
  SIGN_INDOCATION_WHISTLE,
  SIGN_INDOCATION_LEFT_AND_RIGHT,
  // ADD
  SIGN_BAN_UTURN,
  SIGN_WARNING_PED,
  SIGN_INDOCATION_PARKING,
  // 人行天桥
  PedestrianBridge = 3901,
  // v2x
  Sensors_Camera = 100201,
  Sensors_Radar = 100202,
  Sensors_RSU = 100203,
  Sensors_Lidar = 100204,
  // custom type
  CustomSubType = 999999,
};

enum OBJECT_STYLE {
  OBJECT_STYLE_None = 0,
  OBJECT_STYLE_Rectangle = 1,
  OBJECT_STYLE_Triangle = 2,
  OBJECT_STYLE_Circle = 3,
  OBJECT_STYLE_Diamond = 4,
  OBJECT_STYLE_RTriangle = 5,
  OBJECT_STYLE_Arrow = 6,
  OBJECT_STYLE_Bicycle = 7,
  OBJECT_STYLE_WheelChair = 8,
  OBJECT_STYLE_Cross = 9,
  OBJECT_STYLE_Polyline = 10,
  OBJECT_STYLE_Polygon = 11
};

enum OBJECT_GEOMETRY_TYPE {
  OBJECT_GEOMETRY_TYPE_None = 0,
  OBJECT_GEOMETRY_TYPE_Point = 1,
  OBJECT_GEOMETRY_TYPE_Polyline = 2,
  OBJECT_GEOMETRY_TYPE_Polygon = 3
};

enum OBJECT_COLOR {
  OBJECT_COLOR_Unknown = 0x00,
  OBJECT_COLOR_White = 0x01,
  OBJECT_COLOR_Red = 0x02,
  OBJECT_COLOR_Yellow = 0x04,
  OBJECT_COLOR_Blue = 0x08,
  OBJECT_COLOR_Green = 0x10,
  OBJECT_COLOR_Black = 0x20,
};

enum OBJECT_SOURCE {
  OBJECT_SOURCE_OBJECT = 0,
  OBJECT_SOURCE_SIGNAL,
  OBJECT_SOURCE_SIGNAL_DYNAMIC,
  OBJECT_SOURCE_PARKING,
};
enum FEATURE_FLAG {
  FEATURE_FLAG_Road = 1,
  FEATURE_FLAG_Lane = 2,
  FEATURE_FLAG_Boundary = 3,
};

enum FEATURE_TYPE {
  FEATURE_TYPE_Color = (1 << 3),
  FEATURE_TYPE_Mark = (1 << 4),
  FEATURE_TYPE_Material = (1 << 5),
  FEATURE_TYPE_RightBreak = (1 << 6),
  FEATURE_TYPE_LeftBreak = (1 << 7),
  FEATURE_TYPE_RightMerge = (1 << 8),
  FEATURE_TYPE_LeftMerge = (1 << 9),
  FEATURE_TYPE_LaneNumber = (1 << 10),
  FEATURE_TYPE_RightExit = (1 << 11),
  FEATURE_TYPE_RightEntry = (1 << 12),
  FEATURE_TYPE_LeftExit = (1 << 13),
  FEATURE_TYPE_LeftEntry = (1 << 14),
  FEATURE_TYPE_TunnelStart = (1 << 15),
  FEATURE_TYPE_TunnelEnd = (1 << 16),
  FEATURE_TYPE_TollboothStart = (1 << 17),
  FEATURE_TYPE_TollboothEnd = (1 << 18),
  FEATURE_TYPE_OverheadCrossing = (1 << 19),
  FEATURE_TYPE_Roundabout = (1 << 20),
  FEATURE_TYPE_Intersection = (1 << 21),
  FEATURE_TYPE_LeftServiceArea = (1 << 22),
  FEATURE_TYPE_RightServiceArea = (1 << 23),
  FEATURE_TYPE_Other = (1 << 31),
};
}  // namespace hadmap
