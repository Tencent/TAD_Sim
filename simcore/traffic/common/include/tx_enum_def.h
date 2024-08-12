// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"

#define _plus_(v) (+v)
#define BETTER_ENUMS_DEFAULT_CONSTRUCTOR(Enum) \
 public:                                       \
  Enum() = default;
#define BETTER_ENUMS_MACRO_FILE <enum_macros.h>
#include "enum.h"

#define DECLARE_ENUMS_TYPE_SIZE(_TypeName_) constexpr size_t sz##_TypeName_ = _TypeName_::_values().size();
#define DECLARE_ENUMS_STREAM(_TypeName_)                                               \
  inline std::ostream &operator<<(std::ostream &os, const _TypeName_ &v) TX_NOEXCEPT { \
    os << v._to_string();                                                              \
    return os;                                                                         \
  }

#define __enum2idx__(_e_) ((_e_)._to_index())
#define __idx2enum__(_type_, _idx_) (_type_::_from_index(_idx_))
#define __int2enum__(_type_, _int_) (Base::Enums::_type_::_from_integral(_int_))
#define __enum2lpsz__(_type_, _e_) ((_e_)._to_string())
#define __enum2int__(_type_, _e_) ((_e_)._to_integral())

#define __strcat__(_str_, _type_) (_str_ + (_type_)._to_string())

template <typename T>
T String2Enum(Base::txLpsz _lpsz_) {
  auto opValue = T::_from_string_nocase_nothrow(_lpsz_);
  if (opValue) {
    return *opValue;
  } else {
    return T::_from_index(0);
  }
}
#if 1
#  define __lpsz2enum__(_type_, _lpsz_) (String2Enum<Base::Enums::_type_>(_lpsz_))
#else
#  define __lpsz2enum__(_type_, _lpsz_) (Base::Enums::_type_::_from_string(_lpsz_))
#endif

TX_NAMESPACE_OPEN(Base)
TX_NAMESPACE_OPEN(Enums)

BETTER_ENUM(Planning_Activate, txInt, eOff = 0, eLateral = 1, eLongitudinal = 2, eAutopilot = 3)
DECLARE_ENUMS_STREAM(Planning_Activate)

BETTER_ENUM(DITW_Status, txInt, eLog = 1, eWorld = 2)
DECLARE_ENUMS_STREAM(DITW_Status)

BETTER_ENUM(ElementType, txInt, TAD_Vehicle = 1, TAD_Ego, TAD_Pedestrian, Bicycle, TAD_Immovability,
            TAD_Map_Immovability, Unknown_Movability, Unknown_Immovability, TAD_SignalLight, PS_SignalLight, PS_Vehicle,
            PS_Ego, Filter_Vehicle, Replay_Ego, Replay_Vehicle, TAD_Planning, TAD_DummyDriver, TAD_Ignore, Unknown)
DECLARE_ENUMS_STREAM(ElementType)

BETTER_ENUM(VehicleLaneChangeType, txInt, eLeft = 0, eRight = 1)
DECLARE_ENUMS_STREAM(VehicleLaneChangeType)
#if 0
enum VEHICLE_LANE_CHANGE_TYPE {
  VEHICLE_LANE_CHANGE_LEFT = 0,
  VEHICLE_LANE_CHANGE_RIGHT,
  VEHICLE_LANE_CHANGE_TYPE_COUNT
};
#endif

BETTER_ENUM(NearestElementDirection, txInt, eFront = 0, eBack = 1, eLeft = 2, eLeftFront = 3, eLeftBack = 4, eRight = 5,
            eRightFront = 6, eRightBack = 7)
DECLARE_ENUMS_STREAM(NearestElementDirection)
#if 0
enum OBJECT_TARGET_DIRECTION_TYPE {
  OBJECT_TARGET_FRONT_DIRECTION = 0,   /*FrontVeh_CurrentLane*/
  OBJECT_TARGET_BACK_DIRECTION,        /*RearVeh_CurrentLane*/
  OBJECT_TARGET_LEFT_DIRECTION,        /*LeftVeh_TargetLane*/
  OBJECT_TARGET_LEFT_FRONT_DIRECTION,  /**/
  OBJECT_TARGET_LEFT_BACK_DIRECTION,   /**/
  OBJECT_TARGET_RIGHT_DIRECTION,       /*RightVeh_TargetLane*/
  OBJECT_TARGET_RIGHT_FRONT_DIRECTION, /*FrontVeh_TargetLane*/
  OBJECT_TARGET_RIGHT_BACK_DIRECTION,  /*RearVeh_TargetLane*/
  OBJECT_TARGET_DIRECTION_COUNT
};
#endif

BETTER_ENUM(PedestrianTargetDirectionType, txInt, eNone = -1, eCrossLeft = 0, eCrossRight = 1, eFrontLeft = 2,
            eFrontCenter = 3, eFrontRight = 4, eLeft = 5, eRight = 6, eFrontLeftFar = 7, eFrontRightFar = 8)
DECLARE_ENUMS_STREAM(PedestrianTargetDirectionType)
#if 0
enum DYNAMIC_TARGET_DIRECTION_TYPE {
  DYNAMIC_TARGET_CROSS_LEFT_DIRECTION = 0,
  DYNAMIC_TARGET_CROSS_RIGHT_DIRECTION,
  DYNAMIC_TARGET_FRONT_LEFT_DIRECTION,
  DYNAMIC_TARGET_FRONT_CENTER_DIRECTION,
  DYNAMIC_TARGET_FRONT_RIGHT_DIRECTION,
  DYNAMIC_TARGET_LEFT_DIRECTION,
  DYNAMIC_TARGET_RIGHT_DIRECTION,
  DYNAMIC_TARGET_FRONT_LEFT_FAR_DIRECTION,
  DYNAMIC_TARGET_FRONT_RIGHT_FAR_DIRECTION,
  DYNAMIC_TARGET_DIRECTION_COUNT
};
#endif
BETTER_ENUM(Vehicle2Vehicle_RelativeDirection, txInt, eOpposite = 0, eLeftSide = 1, eRightSide = 2, eSameSide = 3)
DECLARE_ENUMS_STREAM(Vehicle2Vehicle_RelativeDirection)

BETTER_ENUM(VehicleInJunction_SelfDrivingDirection, txInt, eStraight = 0, eLeftSide = 1, eRightSide = 2, eUTurnSide = 3)

DECLARE_ENUMS_STREAM(VehicleInJunction_SelfDrivingDirection)
#if 0
enum TRAFFIC_VEHICLE_CROSS_ROAD_DIRECTION {
  TRAFFIC_VEHICLE_CROSS_ROAD_STRAIGHT = 0,
  TRAFFIC_VEHICLE_CROSS_ROAD_LEFT_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_RIGHT_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_SAME_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_STRAIGHT_EXIT_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_RIGHT_EXIT_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_SAME_EXIT_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_RANDOW_SIDE,
  TRAFFIC_VEHICLE_CROSS_ROAD_DIRECTION_COUNT
};
#endif

BETTER_ENUM(EStatus, txInt, eNone, eLoading, eReady, eError)

DECLARE_ENUMS_STREAM(EStatus)
#if 0
enum class EStatus : txInt { None, Loading, Ready, Error };
#endif
BETTER_ENUM(DistanceProjectionType, txInt, Euclidean, Lane, Frenet)

DECLARE_ENUMS_STREAM(DistanceProjectionType)

BETTER_ENUM(Element_Spatial_Relationship, txInt,
            eFront = -1 /*Historical reasons, vehicle is behind of ego, target is in front of source*/,
            eRear = 1 /*vehicle is front of ego, target is behind of source vehicle*/, eOverlap = 0)

DECLARE_ENUMS_STREAM(Element_Spatial_Relationship)

#if 0
enum class DistanceProjectionType : Base::txInt { Euclidean = 0, Lane = 1 };
#endif
BETTER_ENUM(EventTriggerType, txInt, eTime, TTC, EgoDistance)

DECLARE_ENUMS_STREAM(EventTriggerType)
#if 0
enum class EventTriggerType : Base::txInt { Time = 0, TTC = 1, EgoDistance = 3 };
#endif
BETTER_ENUM(acc_invalid_type, txInt, eNone, eTime, eVelocity)

DECLARE_ENUMS_STREAM(acc_invalid_type)
#if 0
enum class acc_invalid_type : Base::txInt { None = 0, Time = 1, Velocity = 3 };
#endif
BETTER_ENUM(EgoType, txInt, eTruck, eVehicle, eCloud)

DECLARE_ENUMS_STREAM(EgoType)
#if 0
enum class EgoType : txInt { Truck, Vehicle };
#endif
BETTER_ENUM(ESceneType, txInt, eOSC, eTAD, eSimrec, eRAW, eCloud, eTAD_Ego, eTAD_DummyDriver, eUNDEF)

DECLARE_ENUMS_STREAM(ESceneType)
#if 0
enum class ESceneType : txInt { OSC = 0, TAD, RAW, UNDEF };
#endif
BETTER_ENUM(ROUTE_TYPE, txInt, ePos, eRoad)

DECLARE_ENUMS_STREAM(ROUTE_TYPE)
#if 0
enum class ROUTE_TYPE : txInt {
  ROUTE_START_TYPE = 0,
  ROUTE_POS_TYPE,
  ROUTE_ROAD_TYPE,
};

#endif
BETTER_ENUM(VEHICLE_BEHAVIOR, txInt, undefined, eUserDefined, eTadAI, eTadAI_Arterial, e3rd, eL2W, eRaw, ePlanning,
            eLog, eTrajectoryFollow, eRelativeTrajectoryFollow, eInjectMode)

DECLARE_ENUMS_STREAM(VEHICLE_BEHAVIOR)
#if 0
enum class VEHICLE_BEHAVIOR : txInt {
  TAD_PLANNING_VEHICLE_BEHAVIOR = 0,
  TAD_USER_DEFINED_VEHICLE_BEHAVIOR,
  TAD_TRAFFIC_VEHICLE_BEHAVIOR,
  L2W_VEHICLE_BEHAVIOR,
  TAD_IBEO_VEHICLE_BEHAVIOR TXSTMARK("Raw-Data")
};
#endif
BETTER_ENUM(PEDESTRIAN_TYPE, txInt, human = 0, child = 1, oldman = 2, woman = 3, girl = 4, Woman002 = 5, Woman003 = 6,
            Woman004 = 7, Woman005 = 8, Woman006 = 9, Woman007 = 10, Girl002 = 11, Man002 = 12, Man003 = 13,
            Man004 = 14, Man005 = 15, Man006 = 16, Oldman002 = 17, Oldman003 = 18,

            cat = 100, dog = 101, bike_001 = 201, elecBike_001 = 202, tricycle_001 = 203, tricycle_002 = 204,
            tricycle_003 = 205, Mobike_Classic2 = 206,

            moto_001 = 301, moto_002 = 302, Honda_CreaScoopy_AF55 = 303, HarleyDavidson_VRod_2003 = 304,
            Aprilia_SR50R_Euro4 = 305, Suzuki_GSXR1000_2010 = 306, Honda_Monkey125_2017 = 307,
            Giant_EscapeR3_2018 = 308, Giant_SCR2_2017 = 309, Maruishi_Bicycle_26Inch = 310, Xidesheng_AD350_2020 = 311,

            vendingCar_001 = 401, Port_Crane_001 = 601, Port_Crane_002 = 602, Port_Crane_002_0_0 = 603,
            Port_Crane_002_0_2 = 604, Port_Crane_002_0_5 = 605, Port_Crane_002_1_0 = 606, Port_Crane_002_5_0 = 607,
            Port_Crane_003 = 608, Port_Crane_004 = 609)

TX_NAMESPACE_OPEN(DynamicObstacle)

static Base::txBool IsHuman(const PEDESTRIAN_TYPE &_ped_enum) TX_NOEXCEPT {
  const txInt ped_int = __enum2int__(PEDESTRIAN_TYPE, _ped_enum);
  return ((ped_int >= 0) && (ped_int < 100));
}

static Base::txBool IsAnimal(const PEDESTRIAN_TYPE &_ped_enum) TX_NOEXCEPT {
  const txInt ped_int = __enum2int__(PEDESTRIAN_TYPE, _ped_enum);
  return ((ped_int >= 100) && (ped_int < 200));
}

static Base::txBool IsBike(const PEDESTRIAN_TYPE &_ped_enum) TX_NOEXCEPT {
  const txInt ped_int = __enum2int__(PEDESTRIAN_TYPE, _ped_enum);
  return ((ped_int >= 200) && (ped_int < 400));
}

static Base::txBool IsDynamicObs(const PEDESTRIAN_TYPE &_ped_enum) TX_NOEXCEPT {
  const txInt ped_int = __enum2int__(PEDESTRIAN_TYPE, _ped_enum);
  return ((ped_int >= 400) && (ped_int < 700));
}

TX_NAMESPACE_CLOSE(DynamicObstacle)
TX_MARK("tricycle_001 --->  sf");
TX_MARK("tricycle_002 --->  electro-tricycle, blue");
TX_MARK("tricycle_002 --->  cycle rickshaw");

DECLARE_ENUMS_STREAM(PEDESTRIAN_TYPE)
#if 0
enum class PEDESTRIAN_TYPE : txInt {
  human = 0,
  child = 1,
  oldman = 2,
  cat = 100,
  dog = 101,
  bike_001 = 201,
  moto_001 = 301,
  moto_002 = 302
};
#endif
// BETTER_ENUM(VEHICLE_TYPE, txInt, undefined = 0, sedan, suv, bus, truck, truck_1, sedan_001, sedan_002, sedan_003,
// sedan_004, suv_001, suv_002, suv_003, suv_004, truck_002, truck_003, bus_001, Semi_Trailer_Truck_001)
BETTER_ENUM(VEHICLE_TYPE, txInt, undefined = 0, Sedan = 1, SUV = 2, Bus = 3, Truck = 4, Truck_1 = 5, Sedan_001 = 6,
            Sedan_002 = 7, Sedan_003 = 8, Sedan_004 = 9, SUV_001 = 10, SUV_002 = 11, SUV_003 = 12, SUV_004 = 13,
            Truck_002 = 14, Truck_003 = 15, Bus_001 = 16, Semi_Trailer_Truck_001 = 17, SUV_005 = 18, SUV_006 = 19,
            Bus_004 = 20, Ambulance_001 = 21, HongqiHS5 = 22, Bus_003 = 23, Sedan_005 = 24, Bus_005 = 25,
            LandRover_RangeRover_2010 = 27, BMW_MINI_CooperS_2010 = 28, Opel_Corsa_2009 = 29,
            Lexus_IS_2009 = 30, Suzuki_Celerio_2011 = 31, Lancia_DeltaIntegrale_Evo1 = 32,
            Chevrolet_ExpressVan1500_2001 = 33, MercedesBenz_SL_2005 = 34, Dodge_Charger_2006 = 35,
            Porsche_911_TurboS_2020 = 36, Nissan_GTR_2009 = 37, Opel_Ampera_2009 = 38,
            Dodge_ViperMambaEdition_RCH680 = 39, Mitsubishi_iMiEV_2018 = 40, Audi_A8_2015 = 41, Toyota_Tacoma_2017 = 42,
            Lamborghini_Gallardo_2011 = 43, MercedesBenz_SLSAMG_2014 = 44, Scania_R620_2012 = 45,
            MAN_LionsCoach_2003 = 46, Cadillac_XTSLimousine_2018 = 47, MercedesBenz_Arocs_3240 = 48, BMW_X6_2014 = 49,
            Truck_004 = 52, Ambulance = 501, Ambulance_002 = 502,
            FireEngine_01 = 530, AdminVehicle_01 = 601, AdminVehicle_02 = 602, RelativeObstacle = 650,
            AIV_FullLoad_001 = 503, AIV_Empty_001 = 504, MPV_001 = 505, AIV_FullLoad_002 = 506, AIV_Empty_002 = 507,
            MIFA_01 = 508, Truck_with_cargobox = 509, Truck_without_cargobox = 510, MPV_002 = 511,
            MiningDumpTruck_001 = 512, AIV_V5_FullLoad_001 = 513, AIV_V5_Empty_001 = 514, Unloaded_Trailer = 515,
            Half_loaded_Trailer = 516, Full_loaded_Trailer = 517, Truck_jdmk = 518, Port_Crane_001 = 701,
            Port_Crane_002 = 702, Port_Crane_002_0_0 = 703, Port_Crane_002_0_2 = 704, Port_Crane_002_0_5 = 705,
            Port_Crane_002_1_0 = 706, Port_Crane_002_5_0 = 707, Port_Crane_003 = 708, Port_Crane_004 = 709,
            MainSUV = 1001, MainTruck = 1002)

DECLARE_ENUMS_STREAM(VEHICLE_TYPE)
#if 0
enum class VEHICLE_TYPE : txInt {
  VEHICLE_TYPE_UNDEFINED = 0,
  VEHICLE_TYPE_SEDAN = 1,
  VEHICLE_TYPE_SUV = 2,
  VEHICLE_TYPE_BUS = 3,
  VEHICLE_TYPE_TRUCK = 4,
  VEHICLE_TYPE_TRUCK_1 = 5,
  VEHICLE_TYPE_Sedan_001 = 6,
  VEHICLE_TYPE_Sedan_002 = 7,
  VEHICLE_TYPE_Sedan_003 = 8,
  VEHICLE_TYPE_Sedan_004 = 9,
  VEHICLE_TYPE_SUV_001 = 10,
  VEHICLE_TYPE_SUV_002 = 11,
  VEHICLE_TYPE_SUV_003 = 12,
  VEHICLE_TYPE_SUV_004 = 13,
  VEHICLE_TYPE_Truck_002 = 14,
  VEHICLE_TYPE_Truck_003 = 15,
  VEHICLE_TYPE_Bus_001 = 16, /*16*/
  VEHICLE_TYPE_Semi_Trailer_Truck_001 = 17,
  VEHICLE_TYPE_COUNT
};
#endif
BETTER_ENUM(STATIC_ELEMENT_TYPE, txInt, Sedan = 0, Box = 1, Person = 2, Cone = 3, Sand = 4, Box_001 = 5, Vertex = 6,
            VertexList = 7, Stob_001 = 101, Stob_002 = 102, Stob_003 = 103, Stob_004 = 104, Stob_005 = 105,
            Stob_006 = 106, Stob_tran_001 = 201, Trash_001 = 301, Trash_002 = 302, Cone002 = 311, Wheel_001 = 401,
            Wheel_002 = 402, Stone_001 = 403, Stone_002 = 404, Warning_001 = 501, Warning_002 = 502,
            Port_Crane_001 = 601, Port_Crane_002 = 602, Port_Container_001 = 603, Port_Container_002 = 604, J_001 = 701,
            J_002 = 702, J_003 = 703, J_004 = 704, J_005 = 705, J_006 = 706, J_007 = 707, J_008 = 708, J_009 = 709,
            J_010 = 710, J_011 = 711, J_012 = 712, J_013 = 713, J_014 = 714, J_015 = 715, ChargingStation_001 = 801)

DECLARE_ENUMS_STREAM(STATIC_ELEMENT_TYPE)

TX_NAMESPACE_OPEN(StaticObstacle)

// TODO(yangguo): USE_CustomModelImport
static Base::txBool IsObstacle(const STATIC_ELEMENT_TYPE &_obs_enum) TX_NOEXCEPT {
  const txInt obs_int = __enum2int__(STATIC_ELEMENT_TYPE, _obs_enum);
  return ((obs_int >= 0) && (obs_int < 500));
}

TX_NAMESPACE_CLOSE(StaticObstacle)

#if 0
enum class STATIC_ELEMENT_TYPE : txInt { Sedan = 0, Box = 1, Person = 2, Cone = 3, Sand = 4 };

#endif
BETTER_ENUM(EventActionType, txInt, TTC_EgoDist_With_DefaultProjection, TTC_EgoDist_With_SpecialProjection,
            TIME_TRIGGER, ActionTypeUndefined, ActionTypeCount, TTC_EgoDist_With_SpecialProjection_With_TriggerIndex)

DECLARE_ENUMS_STREAM(EventActionType)
#if 0
enum class EventActionType : Base::txInt {
  TTC_EgoDist_With_DefaultProjection = 0,
  TTC_EgoDist_With_SpecialProjection = 1,
  TIME_TRIGGER = 2,
  ActionTypeUndefined = 3,
  ActionTypeCount = 4,
  TTC_EgoDist_With_SpecialProjection_With_TriggerIndex = 5
};
#endif
BETTER_ENUM(NearPedestrianSearchType, txInt, eFront, eLeft, eRight)

DECLARE_ENUMS_STREAM(NearPedestrianSearchType)
#if 0
enum NEAR_DYNAMIC_SEARCH_TYPE { NEAR_DYNAMIC_SEARCH_FRONT = 0, NEAR_DYNAMIC_SEARCH_LEFT, NEAR_DYNAMIC_SEARCH_RIGHT };
#endif
BETTER_ENUM(Driving_Location_Type, txInt, Normal, Arterial)

DECLARE_ENUMS_STREAM(Driving_Location_Type)
#if 0
enum Driving_Location_Type { Driving_Location_Normal = 0, Driving_Location_Arterial, Driving_Location_Count };
#endif
BETTER_ENUM(Object_Type, txInt, eVehicle, ePedestrian, EgoSUV, EgoTruckLeader, EgoTruckFollower, eObstacle, Unknown)

DECLARE_ENUMS_STREAM(Object_Type)
#if 0
enum class Object_Type : txInt { eVehicle, ePedestrian, EgoSUV, EgoTruckLeader, EgoTruckFollower, Unknown };
#endif
BETTER_ENUM(VehicleMoveLaneState, txInt, eRightInLane = -2, eRight = -1, eStraight = 0, eLeft = 1, eLeftInLane = 2)

DECLARE_ENUMS_STREAM(VehicleMoveLaneState)
#if 0
enum class VEHICLE_MOVE_LANE_STATE : Base::txInt {
    VEHICLE_MOVE_LANE_RIGHT_IN_LANE = -2,
    VEHICLE_MOVE_LANE_RIGHT = -1,
    VEHICLE_MOVE_LANE_STRAIGHT = 0,
    VEHICLE_MOVE_LANE_LEFT = 1,
    VEHICLE_MOVE_LANE_LEFT_IN_LANE = 2,
    VEHICLE_MOVE_LANE_GOAL = 3
};
#endif

BETTER_ENUM(EventHandlerType, txInt, hVehicle = 0, hPedestrian, eUndef)
DECLARE_ENUMS_STREAM(EventHandlerType)
#if 0
enum class EventHandlerType : txInt { hVehicle, hPedestrian, eUndef };
#endif
BETTER_ENUM(TAD_VehicleState_UserDefined, txInt, eIdle = 0, eStart, eStop, eKill, eLaneKeep, eTurnLeft_Start,
            eTurnLeft_Ing, eTurnLeft_Finish, eTurnLeft_Abort, eTurnLeft_InLane_Start, eTurnLeft_InLane_Ing,
            eTurnLeft_InLane_Finish, eTurnLeft_InLane_Abort, eTurnRight_Start, eTurnRight_Ing, eTurnRight_Finish,
            eTurnRight_Abort, eTurnRight_InLane_Start, eTurnRight_InLane_Ing, eTurnRight_InLane_Finish,
            eTurnRight_InLane_Abort, eLateral_Action)
DECLARE_ENUMS_STREAM(TAD_VehicleState_UserDefined)

BETTER_ENUM(TAD_VehicleState_AI, txInt, eIdle = 0, eStart, eStop, eKill, eLaneKeep, eTurnLeft_Start, eTurnLeft_Ing,
            eTurnLeft_Finish, eTurnLeft_Abort, eTurnRight_Start, eTurnRight_Ing, eTurnRight_Finish, eTurnRight_Abort)
DECLARE_ENUMS_STREAM(TAD_VehicleState_AI)

BETTER_ENUM(EgoSubType, txInt, eLeader, eFollower)
DECLARE_ENUMS_STREAM(EgoSubType)

#if 0
enum class EgoSubType : Base::txInt { Leader = 0, Follower = 1, nCount = 2 };
#endif

BETTER_ENUM(SIGN_LIGHT_COLOR_TYPE, txInt, eGreen = 0, eYellow = 1, eRed = 2, eGrey = 3, eBlack = 4)
DECLARE_ENUMS_STREAM(SIGN_LIGHT_COLOR_TYPE)

#if 0
enum SIGN_LIGHT_COLOR_TYPE {
  SIGN_LIGHT_COLOR_GREEN = 0,
  SIGN_LIGHT_COLOR_YELLOW,
  SIGN_LIGHT_COLOR_RED,
  SIGN_LIGHT_COLOR_GREY,
  SIGN_LIGHT_COLOR_COUNT
};
#endif

BETTER_ENUM(WayPointType, txInt, Point = 0, Road_Section = 1, LaneLink = 2)
DECLARE_ENUMS_STREAM(WayPointType)
#if 0
enum class WayPointType { Point, Road_Section, LaneLink };
#endif

BETTER_ENUM(SceneEventType, txInt, none = 0, velocity_trigger = 1, reach_abs_position = 2, reach_abs_lane = 3,
            time_trigger = 4, ttc_trigger = 5, distance_trigger = 6, ego_attach_laneid_custom = 7, dummy_speed = 8,
            dummy_relative_speed = 9, dummy_reach_position = 10, dummy_ttc_trigger = 11, dummy_distance_trigger = 12,
            dummy_ego_attach_laneid_custom = 13, dummy_time_trigger = 14, conditionGroup = 15, timeheadway_trigger = 16,
            element_state = 17, dummy_time_head_way = 18, dummy_element_state = 19)
DECLARE_ENUMS_STREAM(SceneEventType)

BETTER_ENUM(SceneEventElementStatusType, txInt, none = 0, pendingState = 1, runningState = 2, completeState = 3)
DECLARE_ENUMS_STREAM(SceneEventElementStatusType)

BETTER_ENUM(EndConditionKeyType, txInt, none = 0, time = 1, velocity = 2)
DECLARE_ENUMS_STREAM(EndConditionKeyType)

BETTER_ENUM(SceneEventActionType, txInt, acc = 0, merge = 1, velocity = 2, crash_stop_renew = 3, lateralDistance = 4)
DECLARE_ENUMS_STREAM(SceneEventActionType)

BETTER_ENUM(ConditionSpeedType, txInt, absolute = 0, relative = 1)
DECLARE_ENUMS_STREAM(ConditionSpeedType)

BETTER_ENUM(ConditionEquationOp, txInt, eq = 0, gt = 1, gte = 2, lt = 3, lte = 4)
DECLARE_ENUMS_STREAM(ConditionEquationOp)

BETTER_ENUM(ConditionDistanceType, txInt, euclideandistance = 0, laneprojection = 1)
DECLARE_ENUMS_STREAM(ConditionDistanceType)

BETTER_ENUM(ConditionBoundaryType, txInt, none = 0, rising = 1, falling = 2, both = 3)
DECLARE_ENUMS_STREAM(ConditionBoundaryType)

BETTER_ENUM(ConditionPositionType, txInt, absolute_position = 0, relative_position = 1)
DECLARE_ENUMS_STREAM(ConditionPositionType)

BETTER_ENUM(ConditionLaneType, txInt, absolute_lane = 0, relative_lane = 1)
DECLARE_ENUMS_STREAM(ConditionLaneType)

BETTER_ENUM(ConditionDirDimension, txInt, eLONGITUDINAL = 0, eLATERAL = 1, eVERTICAL = 2)
DECLARE_ENUMS_STREAM(ConditionDirDimension)

BETTER_ENUM(ControlPathGear, txInt, drive = 0, reverse = 1)
DECLARE_ENUMS_STREAM(ControlPathGear)

BETTER_ENUM(DrivingStatus, txInt, unknown = 0, normal = 1, stop_crash = 2, junction_yield = 3)
DECLARE_ENUMS_STREAM(DrivingStatus)

BETTER_ENUM(SketchEnumType, txInt, default_scene = 0, weather_rain = 1, roadsurface_water = 2, traffic_signs = 3,
            surrounding_cutin = 4, ego_cutin = 5, surrounding_cutout = 6, surrounding_speedup = 7,
            surrounding_speedreduction = 8, surrounding_stop = 9, surrounding_cross = 10, ego_3_2_lanechange = 11,
            ego_front_rear_lanechange = 12, ego_front_front_lanechange = 13,
            surrounding_front_rear_frontlanechange = 14, ped_front_cross = 15, bicycle_front_cross = 16,
            obs_front_egolanechange = 17, multi_obs_front_egolanechange = 18, junction_opposite_turnleft = 19,
            junction_opposite_turnright = 20, junction_opposite_straight = 21, junction_opposite_turnU = 22,
            ego_parking = 23)
DECLARE_ENUMS_STREAM(SketchEnumType)
#if 1
DECLARE_ENUMS_TYPE_SIZE(ElementType)
DECLARE_ENUMS_TYPE_SIZE(VehicleLaneChangeType)
DECLARE_ENUMS_TYPE_SIZE(NearestElementDirection)
DECLARE_ENUMS_TYPE_SIZE(PedestrianTargetDirectionType)
DECLARE_ENUMS_TYPE_SIZE(Vehicle2Vehicle_RelativeDirection)
DECLARE_ENUMS_TYPE_SIZE(VehicleInJunction_SelfDrivingDirection)
DECLARE_ENUMS_TYPE_SIZE(EStatus)
DECLARE_ENUMS_TYPE_SIZE(DistanceProjectionType)
DECLARE_ENUMS_TYPE_SIZE(EventTriggerType)
DECLARE_ENUMS_TYPE_SIZE(acc_invalid_type)
DECLARE_ENUMS_TYPE_SIZE(EgoType)
DECLARE_ENUMS_TYPE_SIZE(ESceneType)
DECLARE_ENUMS_TYPE_SIZE(ROUTE_TYPE)
DECLARE_ENUMS_TYPE_SIZE(VEHICLE_BEHAVIOR)
DECLARE_ENUMS_TYPE_SIZE(PEDESTRIAN_TYPE)
DECLARE_ENUMS_TYPE_SIZE(VEHICLE_TYPE)
DECLARE_ENUMS_TYPE_SIZE(STATIC_ELEMENT_TYPE)
DECLARE_ENUMS_TYPE_SIZE(EventActionType)
DECLARE_ENUMS_TYPE_SIZE(NearPedestrianSearchType)
DECLARE_ENUMS_TYPE_SIZE(Driving_Location_Type)
DECLARE_ENUMS_TYPE_SIZE(Object_Type)
DECLARE_ENUMS_TYPE_SIZE(VehicleMoveLaneState)
DECLARE_ENUMS_TYPE_SIZE(TAD_VehicleState_UserDefined)
DECLARE_ENUMS_TYPE_SIZE(EgoSubType)
DECLARE_ENUMS_TYPE_SIZE(SIGN_LIGHT_COLOR_TYPE)
DECLARE_ENUMS_TYPE_SIZE(WayPointType)
DECLARE_ENUMS_TYPE_SIZE(SceneEventType)
DECLARE_ENUMS_TYPE_SIZE(SceneEventElementStatusType)
DECLARE_ENUMS_TYPE_SIZE(EndConditionKeyType)
DECLARE_ENUMS_TYPE_SIZE(SceneEventActionType)
DECLARE_ENUMS_TYPE_SIZE(ConditionSpeedType)
DECLARE_ENUMS_TYPE_SIZE(ConditionEquationOp)
DECLARE_ENUMS_TYPE_SIZE(ConditionDistanceType)
DECLARE_ENUMS_TYPE_SIZE(ConditionBoundaryType)
DECLARE_ENUMS_TYPE_SIZE(ConditionPositionType)
DECLARE_ENUMS_TYPE_SIZE(ConditionLaneType)
DECLARE_ENUMS_TYPE_SIZE(ConditionDirDimension)
DECLARE_ENUMS_TYPE_SIZE(ControlPathGear)
DECLARE_ENUMS_TYPE_SIZE(DrivingStatus)
DECLARE_ENUMS_TYPE_SIZE(SketchEnumType)
DECLARE_ENUMS_TYPE_SIZE(Planning_Activate)
#endif

TX_NAMESPACE_CLOSE(Enums)
TX_NAMESPACE_CLOSE(Base)

#undef DECLARE_ENUMS_TYPE_SIZE
#undef DECLARE_ENUMS_STREAM

#define DECLARE_ENUMS_TYPE_SAVE_LOAD(_TypeName_)                                       \
  namespace cereal {                                                                   \
  template <class Archive>                                                             \
  inline std::string save_minimal(Archive const &, _TypeName_ const &t) {              \
    return std::string(t._to_string());                                                \
  }                                                                                    \
  template <class Archive>                                                             \
  inline void load_minimal(Archive const &, _TypeName_ &t, const std::string &value) { \
    t = _TypeName_::_from_string(value.c_str());                                       \
  }                                                                                    \
  }

DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ElementType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::VehicleLaneChangeType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::NearestElementDirection)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::PedestrianTargetDirectionType)

DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::Vehicle2Vehicle_RelativeDirection)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::VehicleInJunction_SelfDrivingDirection)

DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::EStatus)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::DistanceProjectionType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::EventTriggerType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::acc_invalid_type)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::EgoType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ESceneType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ROUTE_TYPE)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::VEHICLE_BEHAVIOR)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::PEDESTRIAN_TYPE)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::VEHICLE_TYPE)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::STATIC_ELEMENT_TYPE)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::EventActionType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::NearPedestrianSearchType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::Driving_Location_Type)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::Object_Type)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::VehicleMoveLaneState)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::TAD_VehicleState_UserDefined)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::EgoSubType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::SIGN_LIGHT_COLOR_TYPE)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::WayPointType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::SceneEventType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::SceneEventElementStatusType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::EndConditionKeyType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::SceneEventActionType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionSpeedType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionEquationOp)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionDistanceType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionBoundaryType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionPositionType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionLaneType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ConditionDirDimension)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::ControlPathGear)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::DrivingStatus)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::SketchEnumType)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::TAD_VehicleState_AI)
DECLARE_ENUMS_TYPE_SAVE_LOAD(Base::Enums::Planning_Activate)

#undef DECLARE_ENUMS_TYPE_SAVE_LOAD
