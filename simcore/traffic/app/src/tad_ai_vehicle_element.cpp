// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_ai_vehicle_element.h"
#include "../element_impl/vehicle_element_inject_mode.h"
#include "HdMap/tx_hashed_road.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_collision_detection2d.h"
#include "tx_frame_utils.h"
#include "tx_geometry_element_line.h"
#include "tx_locate_info.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#include "tx_trajectory_sampling_node.h"
#include "tx_units.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_AI_Traffic)
#define LogWarn LOG(WARNING)
#define RouteLogInfo LOG_IF(INFO, FLAGS_LogLevel_AI_Routing)
#define ChangeLaneLogInfo LOG_IF(INFO, FLAGS_LogLevel_LaneChange)
#define EnvPerceptionInfo LOG_IF(INFO, FLAGS_LogLevel_AI_EnvPercption)
#define VehicleTrajInfo LOG_IF(INFO, FLAGS_LogLevel_Vehicle_Traj)
#define VehMeetingInfo LOG_IF(INFO, FLAGS_LogLevel_VehMeeting) << "[meeting_debug]" << TX_VARS(Id())
// #define VehDebugInfo LOG_IF(INFO, FLAGS_DebugVehId == Id()) << TX_VARS(Id())
#define JunctionInfo LOG_IF(INFO, FLAGS_LogLevel_Junction) << "[Junction]" << TX_VARS(Id())
#define LogL2WEgoInfo LOG_IF(INFO, FLAGS_LogLevel_L2W_EgoInfo)
#define USE_MAPSDK_XY2SL (0)

// #define __Trace_PushBack_(__str__) m_vecTrace.push_back(__str__)
#define __Trace_PushBack_(__str__)

#if __TX_Mark__(txVehicleBehavior)
TX_NAMESPACE_OPEN(TrafficFlow)
txVehicleBehavior::txVehicleBehavior() {
  mFriction_Factor = Utils::VehicleBehaviorCfg::Friction_Factor();
  mLaneKeep = Utils::VehicleBehaviorCfg::LaneKeep();
  mMAX_SPEED = Utils::VehicleBehaviorCfg::MAX_SPEED();
  mChangeTopologyThreshold = Utils::VehicleBehaviorCfg::ChangeTopologyThreshold();
  mNoCheckLCAbortThreshold = Utils::VehicleBehaviorCfg::NoCheckLCAbortThreshold();
  mEmptyTargetLaneGapThreshold = Utils::VehicleBehaviorCfg::EmptyTargetLaneGapThreshold();
  mSpeedDenominatorOffset = Utils::VehicleBehaviorCfg::SpeedDenominatorOffset();
  mSafetyGap_Rear = Utils::VehicleBehaviorCfg::SafetyGap_Rear();
  mSafetyGap_Front = Utils::VehicleBehaviorCfg::SafetyGap_Front();
  mSafetyGap_Side = Utils::VehicleBehaviorCfg::SafetyGap_Side();
  mComfortGap = Utils::VehicleBehaviorCfg::ComfortGap();
  mMax_Ped_Reaction_Distance = Utils::VehicleBehaviorCfg::Max_Ped_Reaction_Distance();
  mMAX_REACTION_GAP = Utils::VehicleBehaviorCfg::MAX_REACTION_GAP();
  mLaneChanging_Aggressive_Threshold = Utils::VehicleBehaviorCfg::LaneChanging_Aggressive_Threshold();
  mTolerance_Threshold = Utils::VehicleBehaviorCfg::Tolerance_Threshold();
  mMax_Deceleration = Utils::VehicleBehaviorCfg::Max_Deceleration();
  mSignal_Reaction_Gap = Utils::VehicleBehaviorCfg::Signal_Reaction_Gap();
  mMax_Comfort_Deceleration = Utils::VehicleBehaviorCfg::Max_Comfort_Deceleration();
  mYielding_Aggressive_Threshold = Utils::VehicleBehaviorCfg::Yielding_Aggressive_Threshold();
  mW74_DEFAULT_ax = Utils::VehicleBehaviorCfg::W74_DEFAULT_ax();
  mW74_DEFAULT_bx_add = Utils::VehicleBehaviorCfg::W74_DEFAULT_bx_add();
  mW74_DEFAULT_bx_mult = Utils::VehicleBehaviorCfg::W74_DEFAULT_bx_mult();
  mW74_DEFAULT_cx = Utils::VehicleBehaviorCfg::W74_DEFAULT_cx();
  mW74_DEFAULT_ex_add = Utils::VehicleBehaviorCfg::W74_DEFAULT_ex_add();
  mW74_DEFAULT_ex_mult = Utils::VehicleBehaviorCfg::W74_DEFAULT_ex_mult();
  mW74_DEFAULT_opdv_mult = Utils::VehicleBehaviorCfg::W74_DEFAULT_opdv_mult();
  mW74_DEFAULT_bnull_mult = Utils::VehicleBehaviorCfg::W74_DEFAULT_bnull_mult();
  mW74_DEFAULT_v2cav = Utils::VehicleBehaviorCfg::W74_DEFAULT_v2cav();
  mDriving_Parameters[0] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(0);
  mDriving_Parameters[1] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(1);
  mDriving_Parameters[2] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(2);
  mDriving_Parameters[3] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(3);
  mDriving_Parameters[4] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(4);
  mDriving_Parameters[5] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(5);
  mDriving_Parameters[6] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(6);
  mDriving_Parameters[7] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(7);
  mDriving_Parameters[8] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(8);
  mDriving_Parameters[9] = Utils::VehicleBehaviorCfg::g_Driving_Parameters(9);
}
TX_NAMESPACE_CLOSE(TrafficFlow)
#endif /*__TX_Mark__(txVehicleBehavior)*/

TX_NAMESPACE_OPEN(TrafficFlow)

TAD_AI_VehicleElement::~TAD_AI_VehicleElement() { /*LOG(WARNING) << "[~]TAD_AI_VehicleElement" << TX_VARS(Id());*/
}

void TAD_AI_VehicleElement::Initialize_SUDOKU_GRID() TX_NOEXCEPT {
  mVehicleScanRegion2D.Initialize(mGeometryData.Length(), mGeometryData.Width());
  InitLocalCoord_SUDOKU_GRID(mGeometryData.Length(), mGeometryData.Width());
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::Initialize(ISceneLoader::IViewerPtr _absAttrView,
                                                                ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (NonNull_Pointer(_absAttrView) &&       // 检查 _absAttrView 指针是否为非空
      NonNull_Pointer(_sceneLoader) &&       // 检查 _sceneLoader 指针是否为非空
      CallSucc(_absAttrView->IsInited())) {  // 调用 _absAttrView 的 IsInited 方法检查是否已被初始化
    // 初始化行为
    InitializeBehavior(nullptr);
    // 设置车辆的来源为场景配置
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(_absAttrView);
    if (NonNull_Pointer(_elemAttrViewPtr) && CallSucc(IsSupportBehavior(_elemAttrViewPtr->behaviorEnum()))) {
      LogInfo << TX_VARS(_elemAttrViewPtr->Str());
      Base::ISceneLoader::IRouteViewerPtr _elemRouteViewPtr = _sceneLoader->GetRouteData(_elemAttrViewPtr->routeID());
      LogInfo << "Vehicle Attr :" << (_elemAttrViewPtr->Str());
      if (NonNull_Pointer(_elemRouteViewPtr)) {
        LogInfo << "Vehicle Id : " << (_elemAttrViewPtr->id()) << ", Route : " << (_elemRouteViewPtr->Str());

        m_vehicle_type = __lpsz2enum__(
            VEHICLE_TYPE,
            (_elemAttrViewPtr->vehicleType()).c_str()); /*Utils::String2ElementType((_elemAttrViewPtr->vehicleType()),
                                                           _plus_(VEHICLE_TYPE::Sedan)); */
#if USE_CustomModelImport
        m_vehicle_type_str = _elemAttrViewPtr->vehicleType().c_str();
        m_vehicle_type_id = Base::CatalogCache::VEHICLE_TYPE(m_vehicle_type_str);
#endif /*#USE_CustomModelImport*/
        if (CallSucc(IsOnlyTrajectoryFollow(m_vehicle_type))) {
          LogWarn << TX_VARS(Id()) << TX_VARS_NAME(type, (_elemAttrViewPtr->vehicleType()))
                  << " IsOnlyTrajectoryFollow";
          return false;
        }
        const auto refRoutePtr = (_elemRouteViewPtr);
        const auto refVehiclePtr = (_elemAttrViewPtr);
        mIdentity.Id() = refVehiclePtr->id();
        mIdentity.SysId() = CreateSysId(mIdentity.Id());
#if 1
#  if USE_CustomModelImport
        if (CallFail(InitVehicleCatalog(m_vehicle_type_str))) {
#  else  /*#USE_CustomModelImport*/
        if (CallFail(InitCatalog(m_vehicle_type))) {
#  endif /*#USE_CustomModelImport*/
          mGeometryData.Length() = refVehiclePtr->length();
          mGeometryData.Width() = refVehiclePtr->width();
          mGeometryData.Height() = refVehiclePtr->height();
          LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][failure] " << TX_VARS(Id())
                                               << TX_VARS_NAME(VehicleType, (_elemAttrViewPtr->vehicleType()))
                                               << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        } else {
          mGeometryData.Length() = Catalog_Length();
          mGeometryData.Width() = Catalog_Width();
          mGeometryData.Height() = Catalog_Height();
          LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success] " << TX_VARS(Id())
                                               << TX_VARS_NAME(VehicleType, (_elemAttrViewPtr->vehicleType()))
                                               << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        }
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;
#endif
        SM::txAITrafficState::Initialize(mIdentity.Id());
        mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
        // 云端 id 分成10组
        ConfigurePerceptionPeriod(mPRandom.GetRandomInt() % FLAGS_AiVehEnvPercptionPeriod);
        mLifeCycle.StartTime() = refVehiclePtr->start_t();

        txFloat initCompliance = mPRandom.GetRandomValue();
        mRuleCompliance = (initCompliance < _sceneLoader->GetRuleComplianceProportion());
        LogInfo << TX_VARS(Id()) << TX_VARS(mRuleCompliance) << TX_VARS(initCompliance)
                << TX_VARS(_sceneLoader->GetRuleComplianceProportion());

        m_aggress = refVehiclePtr->aggress();
        TX_MARK("For lanekeep");
        mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());
        LogInfo << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.RndFactor());
        mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
        Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
        ConfigureFollowStrategy();
        if (CallFail(Initialize_Kinetics(_elemAttrViewPtr TX_MARK("visualizer")))) {
          LogWarn << "initialize kinetics error.";
          return false;
        }

        const Base::txLaneID startLaneId = refVehiclePtr->laneID();
        mLocation.NextLaneIndex() = TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(startLaneId);  // Deprecated

        mLocation.LaneOffset() = refVehiclePtr->l_offset();

        const txBool bLocateOnLaneLink = (0 <= startLaneId);
        LogInfo << "[vehicle_init_location] " << TX_VARS_NAME(VehicleId, mIdentity.Id())
                << (bLocateOnLaneLink ? " [LaneLink]" : " [Lane]");

        if (CallFail(bLocateOnLaneLink)) {
          TX_MARK("on lane");
          txFloat distanceCurveOnInit_Lane = 0.0;
          txFloat distancePedal_lane = 0.0;
          hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetLaneForInit(
              refRoutePtr->startLon(), refRoutePtr->startLat(), distanceCurveOnInit_Lane, distancePedal_lane);

          if (NonNull_Pointer(initLane_roadId_sectionId)) {
            LogInfo << TX_VARS(Id()) << TX_VARS_NAME(LaneUid, initLane_roadId_sectionId->getTxLaneId())
                    << TX_VARS_NAME(lane_length, initLane_roadId_sectionId->getLength())
                    << TX_VARS(distanceCurveOnInit_Lane) << TX_VARS(refVehiclePtr->start_s());
            LogInfo << TX_VARS_NAME(initLaneUid, Utils::ToString(initLane_roadId_sectionId->getTxLaneId()));
            Base::txLaneUId _laneUid = initLane_roadId_sectionId->getTxLaneId();
            _laneUid.laneId = startLaneId;
            hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(_laneUid);
            if (Null_Pointer(initLane)) {
              LogWarn << ", GetLane Failure." << TX_VARS_NAME(_laneUid, Utils::ToString(_laneUid));
              return false;
            }

            initLane_roadId_sectionId = initLane;
            // Get starting point and laneId ... (PosOnLaneCenterLinePos).
            if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(
                    initLane->getTxLaneId(), GenerateST(refVehiclePtr->start_s(), distanceCurveOnInit_Lane),
                    mLocation.PosOnLaneCenterLinePos())) {
              mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
              mLocation.DistanceAlongCurve() = GenerateST(refVehiclePtr->start_s(), distanceCurveOnInit_Lane);
              mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, mLocation.DistanceAlongCurve());
              /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
              RawVehicleCoord() =
                  ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
              SyncPosition(0.0);
              mLocation.InitTracker(mIdentity.Id());
              RelocateTracker(initLane, 0.0);
#if USE_SUDOKU_GRID
              /*heading_for_front_region_on_ENU = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());*/
              Compute_Region_Clockwise_Close(GeomCenter().ToENU(), mLocation.vLaneDir(),
                                             mLocation.heading_for_front_region_on_ENU());
#endif /*USE_SUDOKU_GRID*/
              mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
              FillingSpatialQuery();
              SaveStableState();

              // if route have mid/end (midGPS endGPS is gps point).
              if (CallSucc(mRouteAI.Initialize(refRoutePtr->id(), 0 /*SubRouteId*/, mIdentity.Id(),
                                               mLocation.PosOnLaneCenterLinePos().WGS84(), refRoutePtr->midGPS(),
                                               refRoutePtr->endGPS()))) {
                mRouteAI.ComputeRoute(mPRandom);
              } else {
                LogInfo << TX_VARS_NAME(VehicleId, mIdentity.Id()) << TX_VARS_NAME(RouteId, refRoutePtr->id())
                        << " mRouteAI.Initialize failure.";
              }
              LogInfo << TX_VARS(Id()) << TX_VARS(GetCurrentLaneInfo());
              return true;
            } else {
              LogWarn << ", Get_LAL_Lane_By_S Failure.";
              return false;
            }
          } else {
            LogWarn << ", Get Init Lane Error.";
            return false;
          }
        } else if (CallSucc(bLocateOnLaneLink)) {
          TX_MARK("on link");
          Base::txFloat distancePedal_lanelink = 0.0;
          Base::txFloat distanceCurveOnInit_LaneLink = 0.0;
          hadmap::txLaneLinkPtr initLaneLinkptr = HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(
              refRoutePtr->startLon(), refRoutePtr->startLat(), distanceCurveOnInit_LaneLink, distancePedal_lanelink);
          distanceCurveOnInit_LaneLink = refVehiclePtr->start_s();
          if (NonNull_Pointer(initLaneLinkptr)) {
            if (HdMap::HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(
                    Base::Info_Lane_t(initLaneLinkptr->getId(), initLaneLinkptr->fromTxLaneId(),
                                      initLaneLinkptr->toTxLaneId()),
                    distanceCurveOnInit_LaneLink, mLocation.PosOnLaneCenterLinePos())) {
              mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
              mLocation.DistanceAlongCurve() = distanceCurveOnInit_LaneLink;
              mLocation.vLaneDir() =
                  HdMap::HadmapCacheConCurrent::GetLaneLinkDir(initLaneLinkptr, mLocation.DistanceAlongCurve());
              /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
              RawVehicleCoord() =
                  ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
              SyncPosition(0.0);
              mLocation.InitTracker(mIdentity.Id());
              RelocateTracker(initLaneLinkptr, 0.0);
#if USE_SUDOKU_GRID
              /*heading_for_front_region_on_ENU = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());*/
              Compute_Region_Clockwise_Close(GeomCenter().ToENU(), mLocation.vLaneDir(),
                                             mLocation.heading_for_front_region_on_ENU());
#endif /*USE_SUDOKU_GRID*/
              mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
              FillingSpatialQuery();
              SaveStableState();

              if (CallSucc(mRouteAI.Initialize(refRoutePtr->id(), 0 /*SubRouteId*/, mIdentity.Id(),
                                               mLocation.PosOnLaneCenterLinePos().WGS84(), refRoutePtr->midGPS(),
                                               refRoutePtr->endGPS()))) {
                mRouteAI.ComputeRoute(mPRandom);
              } else {
                LogWarn << TX_VARS_NAME(VehicleId, mIdentity.Id()) << TX_VARS_NAME(RouteId, refRoutePtr->id())
                        << " mRouteAI.Initialize failure.";
              }
              LogInfo << Str() << TX_VARS(GetCurrentLaneInfo());
              return true;
            } else {
              LogWarn << "GetLonLatFromSTonLinkWithoutOffset Failure.";
              return false;
            }
          } else {
            LogWarn << ", Get Init LaneLink Error.";
            return false;
          }
        } else {
          LogWarn << "start lane id error. " << TX_VARS(startLaneId);
          return false;
        }
      } else {
        LogWarn << "Can not find Route Info.";
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr))
              << TX_VARS(_elemAttrViewPtr->behaviorEnum())
              << TX_COND_NAME(IsSupportBehavior, IsSupportBehavior(_elemAttrViewPtr->behaviorEnum()));
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

Base::txBool TAD_AI_VehicleElement::IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT {
  return ((_plus_(VEHICLE_BEHAVIOR::eTadAI) == behavior) || (_plus_(VEHICLE_BEHAVIOR::eTadAI_Arterial) == behavior));
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::Initialize_Kinetics(
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_elemAttrViewPtr)) {
#ifdef ON_LOCAL
    // mKinetics.m_velocity = (Base::g_TT_V) ? (*Base::g_TT_V) : (_elemAttrViewPtr->start_v());
    mKinetics.m_velocity = _elemAttrViewPtr->start_v();
#else
    mKinetics.m_velocity = _elemAttrViewPtr->start_v();
#endif

    mKinetics.LastVelocity() = mKinetics.m_velocity;
    mKinetics.m_velocity_max = _elemAttrViewPtr->max_v();
    mKinetics.velocity_desired = mKinetics.m_velocity_max;
    mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
    return true;
  } else {
    LogWarn << "Initialize_Kinetics Error. " << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
    return false;
  }
}

#if __TX_Mark__("nearest object wrap")

/*
BETTER_ENUM(NearestElementDirection, txInt, eFront = 0, eBack = 1, eLeft = 2, eLeftFront = 3, eLeftBack = 4, eRight = 5,
eRightFront = 6, eRightBack = 7)
*/
std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::FrontVeh_CurrentLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eFront));
  return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::RearVeh_CurrentLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eBack));
  return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::FrontVeh_TargetLane() const
    TX_NOEXCEPT {
  Base::txInt dir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eFront));
  if (IsInTurnLeft()) {
    dir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftFront));
  } else if (IsInTurnRight()) {
    dir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightFront));
  }
  return std::make_tuple(m_NearestObject.elemPtr(dir), m_NearestObject.dist(dir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::FrontVeh_TargetLane(
    const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT {
  if ((_plus_(Base::Enums::VehicleMoveLaneState::eLeft)) == (dest_move_state)) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftFront));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  } else if ((_plus_(Base::Enums::VehicleMoveLaneState::eRight)) == (dest_move_state)) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightFront));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  }
  LOG(FATAL) << "FrontVeh_TargetLane only support left/right";
  static const Base::txInt cFrontDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eFront));
  return std::make_tuple(m_NearestObject.elemPtr(cFrontDir), m_NearestObject.dist(cFrontDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::RearVeh_TargetLane() const
    TX_NOEXCEPT {
  Base::txInt dir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eBack));
  if (IsInTurnLeft()) {
    dir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftBack));
  } else if (IsInTurnRight()) {
    dir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightBack));
  }
  return std::make_tuple(m_NearestObject.elemPtr(dir), m_NearestObject.dist(dir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::RearVeh_TargetLane(
    const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT {
  if ((_plus_(Base::Enums::VehicleMoveLaneState::eLeft)) == (dest_move_state)) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftBack));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  } else if ((_plus_(Base::Enums::VehicleMoveLaneState::eRight)) == (dest_move_state)) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightBack));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  }
  LOG(FATAL) << "RearVeh_TargetLane only support left/right";
  static const Base::txInt cBackDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eBack));
  return std::make_tuple(m_NearestObject.elemPtr(cBackDir), m_NearestObject.dist(cBackDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::Any_SideVeh_TargetLane() const
    TX_NOEXCEPT {
  if (NonNull_Pointer(m_NearestObject.elemPtr(Info_NearestObject::nRight))) {
    return std::make_tuple(m_NearestObject.elemPtr(Info_NearestObject::nRight), 5.0);
  }
  if (NonNull_Pointer(m_NearestObject.elemPtr(Info_NearestObject::nLeft))) {
    return std::make_tuple(m_NearestObject.elemPtr(Info_NearestObject::nLeft), 5.0);
  }
  return std::make_tuple(nullptr, 999.9);
}

// 获取当前车辆需要切换到的目标车道，如果正在改变车道，返回该车道的车辆指针和距离
// 如果不需要改变车道，则返回一个空指针和一个很大的距离值
std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::SideVeh_TargetLane() const
    TX_NOEXCEPT {
  // 如果当前车辆处于左转向
  if (IsInTurnLeft()) {
    // 获取左侧车道的方向枚举值
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeft));
    // 返回左侧车道的车辆指针和距离
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  } else if (IsInTurnRight()) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRight));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  }
  return std::make_tuple(nullptr, 999.9);
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::SideVeh_TargetLane(
    const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT {
  if ((_plus_(Base::Enums::VehicleMoveLaneState::eLeft)) == (dest_move_state)) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeft));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  } else if ((_plus_(Base::Enums::VehicleMoveLaneState::eRight)) == (dest_move_state)) {
    static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRight));
    return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
  }
  LOG(FATAL) << "RearVeh_TargetLane only support left/right";
  return std::make_tuple(nullptr, 999.9);
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::LeftVeh_TargetLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeft));
  return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> TAD_AI_VehicleElement::RightVeh_TargetLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRight));
  return std::make_tuple(m_NearestObject.elemPtr(cDir), m_NearestObject.dist(cDir));
}

#endif /*__TX_Mark__("nearest object wrap")*/

#if __TX_Mark__("nearest cross vehicle wrap")
std::tuple<Base::IVehicleElementPtr, Base::txFloat> TAD_AI_VehicleElement::OpposeSideVeh_NextCross() const TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::Vehicle2Vehicle_RelativeDirection::eOpposite));
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

std::tuple<Base::IVehicleElementPtr, Base::txFloat> TAD_AI_VehicleElement::SameSideVeh_NextCross() const TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::Vehicle2Vehicle_RelativeDirection::eSameSide));
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

std::tuple<Base::IVehicleElementPtr, Base::txFloat> TAD_AI_VehicleElement::LeftSideVeh_NextCross() const TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::Vehicle2Vehicle_RelativeDirection::eLeftSide));
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

std::tuple<Base::IVehicleElementPtr, Base::txFloat> TAD_AI_VehicleElement::RightSideVeh_NextCross() const TX_NOEXCEPT {
  static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::Vehicle2Vehicle_RelativeDirection::eRightSide));
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

#endif /*__TX_Mark__("nearest cross vehicle wrap")*/
Base::txFloat TAD_AI_VehicleElement::Random(const Base::txString &strMark) TX_NOEXCEPT {
  __Trace_PushBack_(mPRandom.CurrentStateStr(strMark));
  // txMsg("Random()");
  return mPRandom.GetRandomValue();
}

Base::txFloat TAD_AI_VehicleElement::GetRandomValue_NegOne2PosOne(Base::txString strMark) TX_NOEXCEPT {
  // txMsg("GetRandomValue_NegOne2PosOne()");
  __Trace_PushBack_(mPRandom.CurrentStateStr(strMark));
  return mPRandom.GetRandomValue_NegOne2PosOne();
}

#if __TX_Mark__("DrivingFollow")

Base::txFloat TAD_AI_VehicleElement::DrivingFollow(
    const Base::TimeParamManager &timeMgr /*, Base::IVehicleElementPtr frontElementPtr, const txFloat frontDist*/)
    TX_NOEXCEPT {
  switch (mDrivingFollowStrategy) {
    case TrafficFlow::TAD_AI_VehicleElement::DrivingFollowType::e99: {
      Base::SimulationConsistencyPtr frontElementPtr = FollowFrontPtr();
      Base::txFloat return_acc = DrivingFollowNormal(
          timeMgr,
          frontElementPtr, /*std::dynamic_pointer_cast<Base::IVehicleElement>(std::get<_ElementIdx_>(frontVehicleInfo)),
                            */
          FollowFrontDistance());
#  if 1 /* Ego_Front_Specia */
      LOG_IF(INFO, FLAGS_LogLevel_IDM_Front || FLAGS_LogLevel_AI_EnvPercption)
#  else
      LogWarn
#  endif
          << TX_VARS(timeMgr) << TX_VARS(Id())
          << TX_VARS_NAME(FrontId, (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->ConsistencyId()) : (-1)))
          << TX_VARS(FollowFrontDistance()) << TX_VARS(return_acc) << TX_VARS(StableVelocity())
          << TX_VARS(mKinetics.velocity_desired) << TX_VARS(StableGeomCenter().ToWGS84().StrWGS84())
          << TX_VARS_NAME(Front_gps,
                          (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->StableGeomCenter().ToWGS84().StrWGS84())
                                                            : (Base::txString("-1"))))
          << TX_VARS(mKinetics.raw_velocity_max) << TX_VARS(StableLaneInfo()) << TX_VARS(GetOnLaneLinkTimeStamp());
#  if __Consistency__
      std::ostringstream oss;
      oss << "[W99]"
          << TX_VARS_NAME(FrontId, (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->ConsistencyId()) : (-1)))
          << TX_VARS(FollowFrontDistance()) << TX_VARS(return_acc) << TX_VARS(StableVelocity())
          << TX_VARS(StableGeomCenter().ToWGS84().StrWGS84())
          << TX_VARS_NAME(Front_gps,
                          (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->StableGeomCenter().ToWGS84().StrWGS84())
                                                            : (Base::txString("-1"))))
          << TX_VARS(mKinetics.raw_velocity_max) << TX_VARS(StableLaneInfo()) << TX_VARS(GetOnLaneLinkTimeStamp());
      ConsistencyAppend(oss.str());
#  endif /*__Consistency__*/
      return return_acc;
      break;
    }
    case TrafficFlow::TAD_AI_VehicleElement::DrivingFollowType::e74: {
      Base::SimulationConsistencyPtr frontElementPtr = FollowFrontPtr();
      Base::txFloat return_acc = DrivingFollowArterial(timeMgr, frontElementPtr, FollowFrontDistance());
      LOG_IF(INFO, FLAGS_LogLevel_IDM_Front || FLAGS_LogLevel_AI_EnvPercption)
          << TX_VARS(timeMgr) << TX_VARS(Id())
          << TX_VARS_NAME(FrontId, (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->ConsistencyId()) : (-1)))
          << TX_VARS(FollowFrontDistance()) << TX_VARS(return_acc) << TX_VARS(StableVelocity())
          << TX_VARS(StableGeomCenter().ToWGS84().StrWGS84())
          << TX_VARS_NAME(Front_gps,
                          (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->StableGeomCenter().ToWGS84().StrWGS84())
                                                            : (Base::txString("-1"))))
          << TX_VARS(mKinetics.raw_velocity_max) << TX_VARS(StableLaneInfo()) << TX_VARS(GetOnLaneLinkTimeStamp());
#  if __Consistency__
      std::ostringstream oss;
      oss << "[W74]"
          << TX_VARS_NAME(FrontId, (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->ConsistencyId()) : (-1)))
          << TX_VARS(FollowFrontDistance()) << TX_VARS(return_acc) << TX_VARS(StableVelocity())
          << TX_VARS(StableGeomCenter().ToWGS84().StrWGS84())
          << TX_VARS_NAME(Front_gps,
                          (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->StableGeomCenter().ToWGS84().StrWGS84())
                                                            : (Base::txString("-1"))))
          << TX_VARS(mKinetics.raw_velocity_max) << TX_VARS(StableLaneInfo()) << TX_VARS(GetOnLaneLinkTimeStamp());
      ConsistencyAppend(oss.str());
#  endif /*__Consistency__*/
      return return_acc;
      break;
    }
    default:
      break;
  }
  return 0.0;
}

Base::txString follower_status_2_string(const TAD_AI_VehicleElement::DRIVING_FOLLOW_STATE e) TX_NOEXCEPT {
  /*enum class DRIVING_FOLLOW_STATE : txInt { N = 0, A = 1, B = 2, F = 3, W = 4 };*/
  switch (e) {
    case TAD_AI_VehicleElement::DRIVING_FOLLOW_STATE::A:
      return "A";
    case TAD_AI_VehicleElement::DRIVING_FOLLOW_STATE::N:
      return "N";
    case TAD_AI_VehicleElement::DRIVING_FOLLOW_STATE::B:
      return "B";
    case TAD_AI_VehicleElement::DRIVING_FOLLOW_STATE::F:
      return "F";
    case TAD_AI_VehicleElement::DRIVING_FOLLOW_STATE::W:
      return "W";
    default:
      return "unknown";
      break;
  }
}

Base::txFloat TAD_AI_VehicleElement::DrivingFollowNormal(const Base::TimeParamManager &timeMgr,
                                                         Base::SimulationConsistencyPtr frontVehicleElementPtr,
                                                         const txFloat frontDist,
                                                         const Base::txBool bHurryToExitState) TX_NOEXCEPT {
#  if __Consistency__
  std::ostringstream oss;
#  endif /*__Consistency__*/
  txFloat acceleration = 0.0;
#  if 1
  txFloat fLeaderVelocity = (NonNull_Pointer(frontVehicleElementPtr)) ? (frontVehicleElementPtr->StableVelocity())
                                                                      : (mKinetics.m_velocity_max + 10.0);
  txFloat fLeaderAcceleration = (NonNull_Pointer(frontVehicleElementPtr)) ? (frontVehicleElementPtr->StableAcc())
                                                                          : (mKinetics.m_velocity_max + 10.0);
#    if __JunctionYield__
  if (CheckDrivingStatus(_plus_(Base::IDrivingStatus::DrivingStatus::junction_yield))) {
    fLeaderVelocity = 0.0;
    fLeaderAcceleration = 0.0;
  }
#    endif /*__JunctionYield__*/
#  else
  txFloat fLeaderVelocity = (NonNull_Pointer(frontVehicleElementPtr))
                                ? ((StableLaneInfo().isOnLaneLink) ? (0.0) : (frontVehicleElementPtr->StableVelocity()))
                                : (mKinetics.m_velocity_max + 10.0);

  txFloat fLeaderAcceleration = (NonNull_Pointer(frontVehicleElementPtr))
                                    ? ((StableLaneInfo().isOnLaneLink) ? (0.0) : (frontVehicleElementPtr->StableAcc()))
                                    : (mKinetics.m_velocity_max + 10.0);
#  endif
#  if __Consistency__
  oss << TX_VARS(fLeaderVelocity) << TX_VARS(fLeaderAcceleration) << TX_VARS(fLeaderVelocity);
#  endif /*__Consistency__*/
  /*NearestObjectWrap nearest_object(m_NearestObject, changeLaneState, moveState);*/
  Base::txFloat dx = frontDist;

  // Base::txFloat dx = m_NearestObject.aroudDistance[Base::OBJECT_TARGET_FRONT_DIRECTION];
  Base::txFloat dv = fLeaderVelocity - StableVelocity();  // 速度差, 前车比本车快, 则为正, 反正为负
  Base::txFloat sdxc = 0.0;
  // 0205 HD加了摩擦系数////////////////////////////////////////////////////
  Base::txFloat Min_Gap = Driving_Parameters(0);  // 0206 HD
#  if 1
  Base::SimulationConsistencyPtr sideVehiclePtr = nullptr;
  Base::txFloat SideGap_TargetLane = 999.0;
  std::tie(sideVehiclePtr, SideGap_TargetLane) = Any_SideVeh_TargetLane();
#  endif
  const Base::txFloat SideVehicle_Velocity =
      (NonNull_Pointer(sideVehiclePtr) ? (sideVehiclePtr->StableVelocity()) : (MAX_SPEED()));
  Base::txFloat Max_Speed = mKinetics.velocity_desired;

  if (SideVehicle_Velocity < SideGapMinVelocity()) {
    TX_MARK("旁边有车，但速度为0");
    Max_Speed = mKinetics.velocity_desired * Friction_Factor();
    Min_Gap = Min_Gap * (2 - Friction_Factor());  // 0206 HD
    __Trace_PushBack_("2");
  }
#  if __Consistency__
  oss << TX_VARS(dx) << TX_VARS(dv) << TX_VARS(sdxc) << TX_VARS(Min_Gap) << TX_VARS(SideGap_TargetLane)
      << TX_VARS(SideVehicle_Velocity) << TX_VARS(Max_Speed);
#  endif /*__Consistency__*/
  // 0205 HD加了摩擦系数////////////////////////////////////////////////////
  if (bHurryToExitState) {
    static const txInt front_idx = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eFront));
    static const txInt back_idx = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eBack));
    /* 前方和后前方的最小距离*/
    dx = fmin(m_NearestObject.dist(front_idx), m_NearestObject.dist(back_idx));
    __Trace_PushBack_("3");
  }

  if (fLeaderVelocity <= 0) {
    /*  前车停了： sdxc = 1.50*/
    // sdxc = Driving_Parameters(0); /*停车间距 - 米*/
    sdxc = Min_Gap; /*停车间距 - 米 0206 HD*/
    __Trace_PushBack_("4");
  } else {
    Base::txFloat v_slower = ((dv >= 0) || (fLeaderAcceleration < -1))
                                 ? (StableVelocity())
                                 : (fLeaderVelocity + dv * (Random("Driving_Follow_1") - 0.5));
    // sdxc = Driving_Parameters(0) + Driving_Parameters(1) * v_slower; //给定一个速度v下,
    // 跟车状态下期望保持的最小安全距离
    sdxc = Min_Gap + Driving_Parameters(1) * v_slower;  // 给定一个速度v下,);//0206 HD
    __Trace_PushBack_("5");
  }

  Base::txFloat sdxo = sdxc + Driving_Parameters(2);  // 从自由驾驶到跟车行为开始启动时的车距上限(阈值)
  Base::txFloat sdxv = sdxo + Driving_Parameters(3) * (dv - Driving_Parameters(4));

  Base::txFloat sdv = Driving_Parameters(6) * dx *
                      dx;  // 在这个速度差之内(同时在SDX距离内), 本车意识到正在接近前面的慢车, 所以慢慢减速接近
  Base::txFloat sdvc =
      (fLeaderVelocity > 0) ? (Driving_Parameters(4) - sdv) : 0;  // CLDV(closing)本车意识到比前车速度快
  Base::txFloat sdvo = (StableVelocity() > Driving_Parameters(5)) ? (sdv + Driving_Parameters(5))
                                                                  : sdv;  // OPDV(opening), 本车意识到比前车速度慢

  m_follower_status.dx = dx;
  m_follower_status.dv = dv;
  m_follower_status.sdxc = sdxc;
  m_follower_status.sdxv = sdxv;
  m_follower_status.sdxo = sdxo;
  m_follower_status.sdvc = sdvc;
  m_follower_status.sdvo = sdvo;

  /*std::ostringstream oss;
  oss << TX_VARS(frontDist) << TX_VARS(m_follower_status);
  __Trace_PushBack_(oss.str());*/
  if (dx < 0) {
    /*TX_MARK(" 距离已经小于0了，则按最大减速度减速 ");*/
#  if _0206_log_switch_
    LOG(INFO) << "[20210414]" << TXST_TRACE_VARIABLES(m_id) << " (dx < 0) " << TXST_TRACE_VARIABLES(dx);
#  endif /*_0206_log_switch_*/
#  if __Consistency__
    oss << "[F1]";
    ConsistencyAppend(oss.str());
#  endif /*__Consistency__*/
    return Max_Deceleration();
  }
  if ((dv < sdvo) && (dx <= sdxc)) {
    __Trace_PushBack_("6");
    // 减速 - 增加间距, 太近, 减速
    m_follower_status.code = DRIVING_FOLLOW_STATE::A;
    if (StableVelocity() > 0) {
      __Trace_PushBack_("7");
      if (dv < 0) {
        __Trace_PushBack_("8");
        // if (dx > Driving_Parameters(0))
        if (dx > Min_Gap) {  // 0206 HD
          // acceleration = fmin(fLeaderAcceleration + dv * dv / (Driving_Parameters(0) - dx), m_acceleration);
          acceleration = fmin(fLeaderAcceleration + dv * dv / (Min_Gap - dx), StableAcc());  // 0206 HD
          __Trace_PushBack_("9");
        } else {
          acceleration = fmin(fLeaderAcceleration + 0.5 * (dv - sdvo), StableAcc());
          __Trace_PushBack_("10");
        }
      }

      if (acceleration > (-1.0 * Driving_Parameters(7))) {
        acceleration = (-1.0 * Driving_Parameters(7));
        __Trace_PushBack_("11");
      } else {
        acceleration = fmax(acceleration, -10 + 0.5 * sqrt(StableVelocity()));
        __Trace_PushBack_("12");
      }
    }
  } else if ((dv < sdvc) && (dx < sdxv)) {
    // 减速 - 减少间距, 太近, 减速
    m_follower_status.code = DRIVING_FOLLOW_STATE::B;
    acceleration = fmax(0.5 * dv * dv / (-dx + sdxc - 0.1), Max_Deceleration());  // 不超过重力加速度
    __Trace_PushBack_("13");
  } else if ((dv < sdvo) && (dx < sdxo)) {
    __Trace_PushBack_("14");
    // 加速/减速 - 保持车距, 保持车距, 跟车
    m_follower_status.code = DRIVING_FOLLOW_STATE::F;
    if (StableAcc() <= 0) {
      __Trace_PushBack_("15");
      acceleration = fmin(StableAcc(), (-1.0 * Driving_Parameters(7)));
    } else {
      __Trace_PushBack_("16");
      acceleration = fmax(StableAcc(), (-1.0 * Driving_Parameters(7)));
      acceleration = fmin(acceleration, Max_Speed - StableVelocity());  // 0205 HD加了摩擦系数
      acceleration = Math::Clamp(acceleration, -5.0 - m_aggress, 5.0 + m_aggress);
    }
    // 0203 HD////////////////跟车时加入一定随机性
    acceleration =
        acceleration + m_aggress * GetRandomValue_NegOne2PosOne("Driving_Follow_3"); /* Math::RandomFloat(-1.0, 1.0);*/
                                                                                     // 0203 HD////////////////
  } else {
    __Trace_PushBack_("17");
    // 加速/放松 - 加快/保持速度, 自由流, 加速
    m_follower_status.code = DRIVING_FOLLOW_STATE::W;
    if (dx > sdxc) {
      __Trace_PushBack_("18");
      if (DRIVING_FOLLOW_STATE::W != m_follower_status.status) {
        __Trace_PushBack_("19");
        acceleration = Driving_Parameters(7);
      } else {
        __Trace_PushBack_("20");
        Base::txFloat a_max = Driving_Parameters(8) +
                              Driving_Parameters(9) * fmin(StableVelocity(), mKinetics.m_velocity_max) +
                              Random("Driving_Follow_2");  // 祥閉徹癹厒MAX_Speed km/h
        if (dx < sdxo) {
          __Trace_PushBack_("21");
          acceleration = fmin(dv * dv / (sdxo - dx), a_max);
        } else {
          __Trace_PushBack_("22");
          acceleration = a_max;
        }
      }

      // 将自由流下的最大速度限制在3.5m/s2的曲线下
      Base::txFloat a_Cap = 0.0;
      if (StableVelocity() <= 20) {
        __Trace_PushBack_("23");
        a_Cap = 3.5;
      } else {
        // 0.004035057 - 0.224349145 4.613966138
        a_Cap = 0.004035 * StableVelocity() * StableVelocity() - 0.22435 * StableVelocity() + 4.614;
        __Trace_PushBack_("24");
      }

      acceleration = fmin(acceleration, Max_Speed - StableVelocity());  // 0205 HD加了摩擦系数
      acceleration = fmin(acceleration, a_Cap);
      acceleration = Math::Clamp(acceleration, -5.0 - m_aggress, 5.0 + m_aggress);
      // if (fabs(follower.v_desired - follower.v) < 0.1) {
      //  follower_status.message_action = '在限速下行驶';
    }
  }
  __Trace_PushBack_("25");
  m_follower_status.status = m_follower_status.code;
#  if __Consistency__
  oss << mPRandom.CurrentStateStr("[25]");
  oss << m_follower_status << TX_VARS_NAME(ret_acc, acceleration);
  ConsistencyAppend(oss.str());
#  endif /*__Consistency__*/
  LOG_IF(INFO, FLAGS_LogLevel_AI_EnvPercption)
      << TX_VARS(Id()) << TX_VARS_NAME(m_follower_status.code, follower_status_2_string(m_follower_status.code));
  return acceleration;
}

#  define ArterialInfo                       \
    LOG_IF(INFO, FLAGS_LogLevel_AI_Arterial) \
        << "[74_Model] " << TX_VARS(Id()) << TX_VARS_NAME(passTime, timeMgr.PassTime())
Base::txFloat TAD_AI_VehicleElement::DrivingFollowArterial(const Base::TimeParamManager &timeMgr,
                                                           Base::SimulationConsistencyPtr frontElementPtr,
                                                           const txFloat idm_dist) TX_NOEXCEPT {
  Base::txFloat acceleration = 0.0;
  const Base::Info_Lane_t last_laneInfo = StableLaneInfo();
#  if __TX_Mark__("ID90268533")
  // comment (20230926)
  // Base::txFloat fLeaderVelocity = NonNull_Pointer(frontElementPtr) ? ((last_laneInfo.IsOnLaneLink()) ? (0.0) :
  // (frontElementPtr->StableVelocity()))
  //                                                                  : (mKinetics.m_velocity_max + 10);
  Base::txFloat fLeaderVelocity =
      (NonNull_Pointer(frontElementPtr)) ? (frontElementPtr->StableVelocity()) : (mKinetics.m_velocity_max + 10.0);
#  else
  Base::txFloat fLeaderVelocity = (pLeaderData) ? (pLeaderData->GetLastVelocity()) : (m_velocity_max + 10);
#  endif /*__TX_Mark__("ID90268533")*/
  Base::txFloat fLeaderAcceleration =
      NonNull_Pointer(frontElementPtr) ? (frontElementPtr->StableAcc()) : (mKinetics.m_velocity_max + 10);
#  if __JunctionYield__
  if (CheckDrivingStatus(_plus_(Base::IDrivingStatus::DrivingStatus::junction_yield))) {
    fLeaderVelocity = 0.0;
    fLeaderAcceleration = 0.0;
  }
#  endif /*__JunctionYield__*/

  Base::txFloat dx = idm_dist;
  // Base::txFloat dv = fLeaderVelocity - m_objectData.velocity; //速度差, 前车比本车快, 则为正, 反正为负
  Base::txFloat dv =
      StableVelocity() - fLeaderVelocity;  // 速度差, 本车比前车快, 则为正, 反正为负 DV>0说明后车(本车)比前车快
  Base::txFloat sdxc = 0.0;
  Base::txFloat AX = W74_DEFAULT_ax();  // //////*****************//////这三个参数需要校准，其他用缺省
  Base::txFloat bx_add = W74_DEFAULT_bx_add();  // //////*****************//////这三个参数需要校准，其他用缺省
  Base::txFloat bx_mult = W74_DEFAULT_bx_mult();  // //////*****************//////这三个参数需要校准，其他用缺省

  Base::txFloat CX =
      20 * (1 + Random("Arterial") + m_aggress) + .5 * StableVelocity();  // //////*****************//////
  Base::txFloat bnull_mult = W74_DEFAULT_bnull_mult();  // //////*****************//////这个用上面的缺省值就行了
  Base::txFloat BX = 0;

  /*if (bHurryToExitState && ((Distance_To_Go / FLAGS_Mandatory_Aware_Distance) < 0.2)) {
      dx = fmin(m_NearestObject.dist(Base::OBJECT_TARGET_FRONT_DIRECTION),
          m_NearestObject.dist(Base::OBJECT_TARGET_RIGHT_FRONT_DIRECTION));
  }*/
  if (fLeaderVelocity <= 0) {
    BX = 0;
  } else {
    const Base::txFloat v_slower = (dv >= 0) ? (fLeaderVelocity) : (StableVelocity());  // Vslower是慢的那辆车的速度
    BX = (bx_add + m_aggress * bx_mult) * sqrt(v_slower);
  }
  if (fLeaderVelocity <= 3) {
    BX = 3.5 * sqrt(StableVelocity());
  }

  Base::txFloat ABX = AX + BX;
  // Base::txFloat EX = 2.5 - m_aggress;
  // Base::txFloat EX = 3.5 - m_aggress; // 2023/1012 HDU
  // Base::txFloat EX = 1.5 - m_aggress; // 2023/1017 HDUs
  Base::txFloat EX = 3.2 - m_aggress;  // 2023/1025 HDU
  Base::txFloat SDX = AX + fmax(EX * BX, 2);
  // Base::txFloat SDV = pow((dv - AX) / CX, 2);
  Base::txFloat SDV = pow((dx - AX) / CX, 2);  // 2023/1016 HDU
  Base::txFloat CLDV = SDV * EX * EX;          // 2023/0621 HDU
  Base::txFloat OPDV = -2 * SDV;
  Base::txFloat BNULL = bnull_mult * (m_aggress + Random("BNULL"));
  Base::txFloat bmax = fmax(3.5 - .0875 * StableVelocity(), 0);
  Base::txFloat bmin = fmin(-3.5 + .025 * StableVelocity(), 0);
  ArterialInfo << TX_VARS(fLeaderVelocity) << TX_VARS(fLeaderAcceleration) << TX_VARS(dx) << TX_VARS(dv)
               << TX_VARS(sdxc) << TX_VARS(AX) << TX_VARS(bx_add) << TX_VARS(bx_mult) << TX_VARS(CX)
               << TX_VARS(bnull_mult) << TX_VARS(BX) << TX_VARS(ABX) << TX_VARS(EX) << TX_VARS(SDX) << TX_VARS(SDV)
               << TX_VARS(CLDV) << TX_VARS(OPDV) << TX_VARS(BNULL) << TX_VARS(bmax) << TX_VARS(bmin);
  m_follower_status.dx = dx;
  m_follower_status.dv = dv;

  if (dv > -2 && dx <= ABX) {  // Emergency Brake
    m_follower_status.code = DRIVING_FOLLOW_STATE::A;
    acceleration = 0;
    if (StableVelocity() > 0) {
      if (dv > 0) {  // DV>0说明后车e比前车t快
        if (dx > AX) {
          acceleration = fmin(.5 * dv * dv / (AX - dx) + fLeaderAcceleration + bmin * (ABX - dx) / BX, StableAcc());
        } else {
          acceleration = fmin(.5 * (OPDV - dv) + fLeaderAcceleration + bmin * (ABX - dx) / BX, StableAcc());
        }
      }
      if (acceleration > -BNULL) {
        acceleration = -BNULL;
      } else {
        acceleration = fmax(acceleration, -10 + .5 * sqrt(StableVelocity()));
      }
    }
    ArterialInfo << "[DRIVING_FOLLOW_STATE::A]" << TX_VARS(StableVelocity()) << TX_VARS(acceleration);
  } else if (dv > CLDV && dx <= SDX) {
    // E.description = "Decelerate - Decrease Distance",  //Closing In
    m_follower_status.code = DRIVING_FOLLOW_STATE::B;
    const txFloat param1 = FLAGS__74_param_1;
    const txFloat param2 = FLAGS__74_param_2;
    const txFloat param3 = FLAGS__74_param_3;
    // acceleration = fmax(.5 * dv * dv / (ABX - dx - .1), -10 + sqrt(StableVelocity()));
    acceleration = fmax(.5 * dv * dv / (ABX - dx + param1), -10 + sqrt(StableVelocity()));  // 添加了param123
    // float Dec_Cap = -4.0 + Random("Arterial") / 5;                                         // 2023/0828 HDU
    float Dec_Cap = -3.0 + Random("Arterial") / 5;                       // 2023/1016 HDU
    acceleration = fmax(Dec_Cap, acceleration);                          // 2023/0828 HDU
    ArterialInfo << "[1]" << TX_VARS(acceleration) << TX_VARS(Dec_Cap);  // 2023/0828 HDU
#  if 0
        if (acceleration > param2) {
            ArterialInfo << TX_VARS(acceleration) << TX_VARS(param2) << TX_VARS(param3);
            acceleration = acceleration * param3;  // 添加了param123//2023/0621 HDU
            ArterialInfo << "[2]" << TX_VARS(acceleration);
        }
#  else
    // if ((StableVelocity() < 2) && (fLeaderVelocity < 1) && (dx < 10))
    if ((StableVelocity() < 2) && (fLeaderVelocity < 1) && (dx < AX * 3)) {
      // 2023/1013 HDU
      ArterialInfo << TX_VARS(acceleration) << TX_VARS(param2) << TX_VARS(param3);
      acceleration = m_aggress - 1.0 - Random("Arterial") / 2;  // 2023/0828 HDU
      ArterialInfo << "[2]" << TX_VARS(acceleration);
    }
#  endif
    ArterialInfo << "[Arterial]" << TX_VARS(param1) << TX_VARS(param2) << TX_VARS(param3)
                 << R"(EQ : acceleration = fmax(.5 * dv * dv / (ABX - dx + param), -10 + sqrt(StableVelocity())))";
    ArterialInfo << "[DRIVING_FOLLOW_STATE::B]" << TX_VARS(StableVelocity()) << TX_VARS(acceleration);
  } else if (dv > OPDV && dx < SDX) {
    // E.description = "Accelerate/Decelerate - Keep Distance",  //Following
    m_follower_status.code = DRIVING_FOLLOW_STATE::F;
    if (acceleration <= 0) {
      acceleration = fmin(StableAcc(), -1 * BNULL);
      ArterialInfo << "[DRIVING_FOLLOW_STATE::F]" << TX_VARS(StableAcc()) << TX_VARS(acceleration);
    } else {
      acceleration = fmax(StableAcc(), BNULL);
      acceleration = fmin(acceleration, mKinetics.velocity_desired - StableVelocity());
      acceleration = Math::Clamp(acceleration, -5.0 - m_aggress, 5.0 + m_aggress);
      ArterialInfo << "[DRIVING_FOLLOW_STATE::F]" << TX_VARS(StableAcc()) << TX_VARS(acceleration)
                   << TX_VARS(mKinetics.velocity_desired - StableVelocity());
    }
  } else {
    m_follower_status.code = DRIVING_FOLLOW_STATE::W;
    /*if (dx > ABX)*/
    ArterialInfo << TX_VARS(dx) << TX_VARS(ABX) << TX_VARS(StableVelocity()) << TX_VARS(dv);
    if ((dx < ABX * 2) && (StableVelocity() < 0.2) && (dv <= 0.2) && (dv > -0.0001)) {
      acceleration = m_aggress - 1.0 - Random("Arterial") / 2;  // 2023/1016 HDU
    }
    // if ((dx > ABX) && (!((StableVelocity() < 0.001) && (dv <= 0) && (dv > -0.0001))))                       //
    // 2023/0625 HDU if ((dx > ABX) && (!((dx < ABX * 1.5) && (StableVelocity() < 0.001) && (dv <= 0) && (dv >
    // -0.0001)))) // 2023/1012 HDU
    if ((dx > ABX) &&
        (!((dx < ABX * 2) && (StableVelocity() < 0.2) && (dv <= 0.2) && (dv > -0.0001)))) {  // 2023/1016 HDU
      ArterialInfo << "[w]";
      if (m_follower_status.status != DRIVING_FOLLOW_STATE::W) {
        acceleration = BNULL;
      } else {
        Base::txFloat T = bmax;
        acceleration = dx < SDX ? fmin(dv * dv / (SDX - dx), T) : T;
      }
      acceleration = fmin(acceleration, mKinetics.velocity_desired - StableVelocity());
      acceleration = Math::Clamp(acceleration, -5.0 - m_aggress, 5.0 + m_aggress);
      if ((StableVelocity() < 0.2) && (fLeaderVelocity < 0.1) && (dx < 8)) {
        acceleration = m_aggress - 1.0 - Random("Arterial") / 2;  // 2023/0828 HDU
        // acceleration = m_aggress + Random("Arterial") / 2; // 2023/1013 HDU
        ArterialInfo << "[9]" << TX_VARS(acceleration);
      }
      /*if (acceleration > 0) {
          m_follower_status.message_action = follower_message_Accelerate;
      } else {
          m_follower_status.message_action = follower_message_Decelerate;
      }
      if (fabs(mKinetics.velocity_desired - StableVelocity()) < .1) {
          m_follower_status.message_action = follower_message_AtTopSpeed;
      }*/

      // 将自由流下的最大速度限制在3.5m/s2的曲线下
      txFloat a_Cap;
      if (StableVelocity() <= 20) {
        a_Cap = 3.5;
      } else {  // 0.004035057 - 0.224349145 4.613966138
        a_Cap = 0.004035 * StableVelocity() * StableVelocity() - 0.22435 * StableVelocity() + 4.614;
      }
      acceleration = fmin(acceleration, a_Cap);
    }
    ArterialInfo << "[DRIVING_FOLLOW_STATE::W]"
                 << TX_VARS_NAME(m_follower_status.status, toString(m_follower_status.status)) << TX_COND((dx > ABX))
                 << TX_VARS(acceleration);
  }
  m_follower_status.status = m_follower_status.code;
  ArterialInfo << TX_VARS_NAME(m_follower_status.status, toString(m_follower_status.status)) << TX_VARS(acceleration);
  return acceleration;
}
#  undef ArterialInfo
#endif /*__TX_Mark__("DrivingFollow")*/

#if __TX_Mark__("DrivingChangeLane")
Base::txFloat TAD_AI_VehicleElement::DrivingChangeLane(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base = std::get<0>(FrontVeh_CurrentLane());
  Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base = std::get<0>(RearVeh_CurrentLane());
  Base::SimulationConsistencyPtr LeftVeh_TargetLane_Base = std::get<0>(LeftVeh_TargetLane());
  Base::SimulationConsistencyPtr RightVeh_TargetLane_Base = std::get<0>(RightVeh_TargetLane());
  return Driving_SwitchLaneOrg(timeMgr, FrontVeh_CurrentLane_Base, RearVeh_CurrentLane_Base, LeftVeh_TargetLane_Base,
                               RightVeh_TargetLane_Base);
}

Base::txFloat TAD_AI_VehicleElement::Get_MoveBack_Sideway(const txFloat exper_time) TX_NOEXCEPT {
#  if USE_MAPSDK_XY2SL
  if (m_LaneAbortingTimeMgr.Procedure() > 0.0 && NonNull_Pointer(mLocation.tracker()->getCurrentLane())) {
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    const hadmap::txLineCurve *currentLaneCurve =
        dynamic_cast<const hadmap::txLineCurve *>(mLocation.tracker()->getCurrentLane()->getGeometry());

    if (NonNull_Pointer(currentLaneCurve) && (hadmap::COORD_WGS84 == (currentLaneCurve->getCoordType()))) {
      const Coord::txWGS84 &m_PosWithoutOffset = mLocation.PosWithLateralWithoutOffset();
      currentLaneCurve->xy2sl(m_PosWithoutOffset.Lon(), m_PosWithoutOffset.Lat(), s, l, yaw);
    } else {
      LOG(WARNING) << "Warning : hadmap::COORD_WGS84 == (currentLaneCurve->getCoordType())";
    }
    l = std::fabs(l);
    return l / m_LaneAbortingTimeMgr.Procedure();
  } else {
    return 0.0;
  }
#  else  /*USE_MAPSDK_XY2SL*/
  if (m_LaneAbortingTimeMgr.Procedure() > 0.0 && NonNull_Pointer(mLocation.tracker()->getLaneGeomInfo())) {
    txFloat s = 0.0, l = 0.0;
    mLocation.tracker()->getLaneGeomInfo()->xy2sl(mLocation.PosWithLateralWithoutOffset().ToENU().ENU2D(), s, l);
    return std::fabs(l) / m_LaneAbortingTimeMgr.Procedure();
  } else {
    return 0.0;
  }
#  endif /*USE_MAPSDK_XY2SL*/
}

Base::txFloat TAD_AI_VehicleElement::MoveBack_Sideway(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  const txFloat exper_time = m_LaneAbortingTimeMgr.Experience();
  const txFloat lateral_velocity = Get_MoveBack_Sideway(exper_time);
  m_LaneAbortingTimeMgr.Step(timeMgr.RelativeTime());
  return lateral_velocity;
}

Base::txFloat TAD_AI_VehicleElement::Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT {
  if (IsInLaneChange_StartIng() && m_LaneChangeTimeMgr.Procedure() > 0.0) {
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    if (IsInTurnLeft()) {
#  if USE_MAPSDK_XY2SL
      if (NonNull_Pointer(mLocation.tracker()->getLeftLane())) {
        const hadmap::txLineCurve *targetLaneCurve =
            dynamic_cast<const hadmap::txLineCurve *>(mLocation.tracker()->getLeftLane()->getGeometry());
        if (NonNull_Pointer(targetLaneCurve)) {
          if (hadmap::COORD_WGS84 == (targetLaneCurve->getCoordType())) {
            const Coord::txWGS84 &m_PosWithoutOffset = mLocation.PosWithLateralWithoutOffset();
            targetLaneCurve->xy2sl(m_PosWithoutOffset.Lon(), m_PosWithoutOffset.Lat(), s, l, yaw);
          } else {
            LOG(FATAL) << "Error : hadmap::COORD_WGS84 == (targetLaneCurve->getCoordType())";
          }
        }
      }
#  else  /*USE_MAPSDK_XY2SL*/
      if (NonNull_Pointer(mLocation.tracker()->get_Left_LaneGeomInfo())) {
        mLocation.tracker()->get_Left_LaneGeomInfo()->xy2sl(mLocation.PosWithLateralWithoutOffset().ToENU().ENU2D(), s,
                                                            l);
      }
#  endif /*USE_MAPSDK_XY2SL*/
    } else if (IsInTurnRight()) {
#  if USE_MAPSDK_XY2SL
      if (NonNull_Pointer(mLocation.tracker()->getRightLane())) {
        const hadmap::txLineCurve *targetLaneCurve =
            dynamic_cast<const hadmap::txLineCurve *>(mLocation.tracker()->getRightLane()->getGeometry());
        if (NonNull_Pointer(targetLaneCurve)) {
          if (hadmap::COORD_WGS84 == (targetLaneCurve->getCoordType())) {
            const Coord::txWGS84 &m_PosWithoutOffset = mLocation.PosWithLateralWithoutOffset();
            targetLaneCurve->xy2sl(m_PosWithoutOffset.Lon(), m_PosWithoutOffset.Lat(), s, l, yaw);
          } else {
            LOG(FATAL) << "Error : hadmap::COORD_WGS84 == (targetLaneCurve->getCoordType())";
          }
        }
      }
#  else  /*USE_MAPSDK_XY2SL*/
      if (NonNull_Pointer(mLocation.tracker()->get_Right_LaneGeomInfo())) {
        mLocation.tracker()->get_Right_LaneGeomInfo()->xy2sl(mLocation.PosWithLateralWithoutOffset().ToENU().ENU2D(), s,
                                                             l);
      }
#  endif /*USE_MAPSDK_XY2SL*/
    }

    return std::fabs(l) / m_LaneChangeTimeMgr.Procedure();
  } else {
    return 0.0;
  }
}

Base::txFloat TAD_AI_VehicleElement::MoveTo_Sideway(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  const txFloat exper_time = m_LaneChangeTimeMgr.Experience();
  const txFloat lateral_velocity = Get_MoveTo_Sideway(exper_time);
  m_LaneChangeTimeMgr.Step(timeMgr.RelativeTime());
  return lateral_velocity;
}

Base::txFloat TAD_AI_VehicleElement::FrontPedWalkingInSight(const PedestrianSearchType targetType,
                                                            const Base::txFloat distance) TX_NOEXCEPT {
  return 0.0;
}

// Lane change 1: Determine whether to change lanes based on regulations.
Base::txBool TAD_AI_VehicleElement::CheckCanSwitchRightLaneState(const Base::TimeParamManager &) TX_NOEXCEPT {
  if (mKinetics.m_velocity_max <=
      FLAGS_RefuseLaneChangeVelocity) { /* TODO:consider distinguish between the intersection queue scene and others*/
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false max velocity too slow."
                      << TX_VARS(mKinetics.m_velocity_max) << TX_VARS(FLAGS_RefuseLaneChangeVelocity);
    return false;
  }
  if (GetVelocity() < FLAGS_RefuseLaneChangeVelocity) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false velocity too slow."
                      << TX_VARS(GetVelocity()) << TX_VARS(FLAGS_RefuseLaneChangeVelocity);
    return false;
  }
  if (mLocation.IsOnLaneLink()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false isOnLaneLink.";
    return false;
  } else if (mLocation.IsOnRightestLane()) {
    TX_MARK("on the right boundary, could not right switch.");
    ChangeLaneLogInfo << TX_VARS(Id()) << " on the right boundary, could not right switch.";
    return false;
  } else if (mRuleCompliance && mLocation.tracker()->IsRightBoundarySolid()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " bInRightSolidLane.";
    return false;
  } else if (CallFail(Utils::IsLaneValidDriving(mLocation.tracker()->getRightLane()))) {
    return false;
    // __Meeting__ 上下匝道
#  if __Meeting__
  } else if (CallSucc(HdMap::HashedRoadCacheConCurrent::IsInMeetingRegion(m_curHashedLaneInfo))) {
    ChangeLaneLogInfo << TX_VARS(Id()) << m_curHashedLaneInfo << " IsInMeetingRegion, refuse turn right.";
    return false;
#  endif /*__Meeting__*/
  } else {
    const Base::txLaneUId &onLaneUid = mLocation.LaneLocInfo().onLaneUid;
#  if 1
    if (HdMap::HadmapCacheConCurrent::IsSpecialRefuseChangeLane_Right(onLaneUid, DistanceAlongCurve())) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][special][right]" << Utils::ToString(onLaneUid)
                        << TX_VARS(DistanceAlongCurve());
      return false;
    }
#  endif
    if (HdMap::HadmapCacheConCurrent::IsRefuseChangeLane_Right(onLaneUid)) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][right]" << Utils::ToString(onLaneUid);
      return false;
    }

#  if 1
    const auto rightLaneUid = Utils::RightLaneUid(onLaneUid);
    if (HdMap::HadmapCacheConCurrent::IsDstRefuseLaneChange_Right(rightLaneUid)) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][right][DstRefuseLaneChange]"
                        << Utils::ToString(rightLaneUid);
      return false;
    }
    if (CallSucc(mRouteAI.IsValid())) {
      // RouteAI_EdgeData 两点之间拓扑连线
      TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
      if (CallSucc(mRouteAI.IsReachable(onLaneUid, routeHint))) {
        if (CallFail(routeHint.NeedTurnRight())) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " IsReachable, but not NeedTurnRight." << TX_VARS(onLaneUid);
          return false;
        }
      } else {
        TX_MARK("current lane donot reach dest.");
        TX_MARK("avoid ");
        const auto leftLaneUid = Utils::LeftLaneUid(onLaneUid);
        const txBool leftReachable = mRouteAI.CheckReachable(leftLaneUid);
        const txBool rightReachable = mRouteAI.CheckReachable(rightLaneUid);
        if (CallSucc(leftReachable) && CallFail(rightReachable)) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " do not turn right. " << TX_COND(rightReachable)
                            << TX_COND(leftReachable) << TX_VARS(onLaneUid);
          return false;
        }
      }
    } else {
      ChangeLaneLogInfo << TX_VARS(Id()) << " do not have route, under free lane change. [right]";
    }
#  endif
    Base::Info_Lane_t rightLaneInfo;
    rightLaneInfo.FromLane(rightLaneUid);
    const auto geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(rightLaneUid);
    if (NonNull_Pointer(geom_ptr)) {
      const auto rightHashedPt =
          Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, DistanceAlongCurve(), rightLaneInfo);

      if (HdMap::HashedRoadCacheConCurrent::IsCloseLane(rightHashedPt)) {
        ChangeLaneLogInfo << TX_VARS(Id()) << " CheckCanSwitchRightLaneState return false. Close Lane.";
        return false;
      } else {
        ChangeLaneLogInfo << TX_VARS(Id()) << " [_close_lane_] CheckCanSwitchRightLaneState return true. Close Lane."
                          << TX_VARS(rightHashedPt);
      }
    } else {
      return false;
    }
  }
  return true;
}

Base::txBool TAD_AI_VehicleElement::CheckCanSwitchLeftLaneState(const Base::TimeParamManager &) TX_NOEXCEPT {
  if (mKinetics.m_velocity_max <=
      FLAGS_RefuseLaneChangeVelocity) { /* TODO:consider distinguish between the intersection queue scene and others*/
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchLeftLaneState false max velocity too slow."
                      << TX_VARS(mKinetics.m_velocity_max) << TX_VARS(FLAGS_RefuseLaneChangeVelocity);
    return false;
  }
  if (GetVelocity() < FLAGS_RefuseLaneChangeVelocity) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchLeftLaneState false velocity too slow."
                      << TX_VARS(GetVelocity()) << TX_VARS(FLAGS_RefuseLaneChangeVelocity);
    return false;
  }
  if (mLocation.IsOnLaneLink()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchLeftLaneState false isOnLaneLink.";
    return false;
  } else if (mLocation.IsOnLeftestLane()) {
    TX_MARK("on the left boundary, could not left switch.");
    ChangeLaneLogInfo << TX_VARS(Id()) << " on the left boundary, could not left switch.";
    return false;
  } else if (mRuleCompliance && mLocation.tracker()->IsLeftBoundarySolid()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " bInLeftSolidLane.";
    return false;
  } else if (CallFail(Utils::IsLaneValidDriving(mLocation.tracker()->getLeftLane()))) {
    return false;
#  if __Meeting__
  } else if (CallSucc(HdMap::HashedRoadCacheConCurrent::IsInMeetingRegion(m_curHashedLaneInfo))) {
    ChangeLaneLogInfo << TX_VARS(Id()) << m_curHashedLaneInfo << " IsInMeetingRegion, refuse turn left.";
    return false;
#  endif /*__Meeting__*/
  } else {
    const Base::txLaneUId &onLaneUid = mLocation.LaneLocInfo().onLaneUid;
    if (HdMap::HadmapCacheConCurrent::IsRefuseChangeLane_Left(onLaneUid)) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][left]" << Utils::ToString(onLaneUid);
      return false;
    }

    if (HdMap::HadmapCacheConCurrent::IsSpecialRefuseChangeLane_Left(onLaneUid, DistanceAlongCurve())) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][special][left]" << Utils::ToString(onLaneUid)
                        << TX_VARS(DistanceAlongCurve());
      return false;
    }

    const auto leftLaneUid = Utils::LeftLaneUid(onLaneUid);
    if (HdMap::HadmapCacheConCurrent::IsDstRefuseLaneChange_Left(leftLaneUid)) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][left][DstRefuseLaneChange]"
                        << Utils::ToString(leftLaneUid);
      return false;
    }
#  if 1
    if (CallSucc(mRouteAI.IsValid())) {
      TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
      if (CallSucc(mRouteAI.IsReachable(onLaneUid, routeHint))) {
        if (CallFail(routeHint.NeedTurnLeft())) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " IsReachable, but not NeedTurnLeft." << TX_VARS(onLaneUid);
          return false;
        }
      } else {
        TX_MARK("current lane donot reach dest.");
        TX_MARK("avoid ");
        const auto rightLaneUid = Utils::RightLaneUid(onLaneUid);

        const txBool leftReachable = mRouteAI.CheckReachable(leftLaneUid);
        const txBool rightReachable = mRouteAI.CheckReachable(rightLaneUid);
        if (CallSucc(rightReachable) && CallFail(leftReachable)) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " do not turn left. " << TX_COND(leftReachable)
                            << TX_COND(rightReachable) << TX_VARS(onLaneUid);
          return false;
        }
      }
    } else {
      ChangeLaneLogInfo << TX_VARS(Id()) << " do not have route, under free lane change. [left]";
    }
#  endif

    Base::Info_Lane_t leftLaneInfo;
    leftLaneInfo.FromLane(leftLaneUid);
    const auto geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(leftLaneUid);
    if (NonNull_Pointer(geom_ptr)) {
      const auto leftHashedPt =
          Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, DistanceAlongCurve(), leftLaneInfo);

      if (HdMap::HashedRoadCacheConCurrent::IsCloseLane(leftHashedPt)) {
        ChangeLaneLogInfo << TX_VARS(Id()) << " CheckCanSwitchLeftLaneState return false. Close Lane.";
        return false;
      } else {
        ChangeLaneLogInfo << TX_VARS(Id()) << " [_close_lane_] CheckCanSwitchLeftLaneState return true. Close Lane."
                          << TX_VARS(leftHashedPt);
      }
    } else {
      return false;
    }
  }
  return true;
}

Base::txFloat TAD_AI_VehicleElement::ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT {
  Base::txFloat retTime = 0.0;
  if (Math::isNotZero(StableVelocity())) {
    TX_MARK("The road is too short to change lanes. JIRA-284");
    Base::txFloat distToRoadEnd = 100;
    if (mLocation.tracker()->isReachedRoadLastSection() &&
        HdMap::HadmapCacheConCurrent::IsRoadNearJunction(mLocation.LaneLocInfo().onLaneUid)) {
      const Coord::txWGS84 curRoadEndPt =
          HdMap::HadmapCacheConCurrent::GetRoadEnd(mLocation.LaneLocInfo().onLaneUid.roadId);
      distToRoadEnd =
          Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(mLocation.PosOnLaneCenterLinePos(), curRoadEndPt) - 10.0;
    }

    if (distToRoadEnd < 0.0) {
      return -1.0;
    }
    retTime = distToRoadEnd / StableVelocity();
    if (retTime < laneChangeTime) {
      if (retTime > 2.0) {
        return retTime;
      } else {
        return -1.0;
      }
    } else {
      return laneChangeTime;
    }
  } else {
    return laneChangeTime;
  }
}

// Lane change 2: Determine whether to change lanes based on lane dynamic information
TAD_AI_VehicleElement::LaneChangeIntention TAD_AI_VehicleElement::LaneChanging(
    Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base, Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base,
    Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base, Base::SimulationConsistencyPtr RearVeh_TargetLane_Base,
    const txFloat FrontGap_Current, const txFloat RearGap_Current, txFloat FrontGap_TargetLane,
    const txFloat RearGap_TargetLane, const txFloat SideGap_TargetLane,
    const PedestrianSearchType targetType) TX_NOEXCEPT {
  LaneChangeIntention LaneChange_Intention = LaneChangeIntention::eNo;  // 标记是否有换车道的意愿

  const txFloat FrontVeh_CurrentLaneVelocity =
      (NonNull_Pointer(FrontVeh_CurrentLane_Base)) ? (FrontVeh_CurrentLane_Base->StableVelocity()) : MAX_SPEED();
#  if __TX_Mark__("crash stop lane change")
  const txFloat FrontVeh_TargetLaneVelocity = (NonNull_Pointer(FrontVeh_TargetLane_Base))
                                                  ? ((_plus_(Base::ISimulationConsistency::DrivingStatus::stop_crash) ==
                                                      FrontVeh_TargetLane_Base->StableDriving_Status())
                                                         ? (5.0)
                                                         : (FrontVeh_TargetLane_Base->StableVelocity()))
                                                  : MAX_SPEED();
#  else
  const txFloat FrontVeh_TargetLaneVelocity =
      (NonNull_Pointer(FrontVeh_TargetLane_Base)) ? (FrontVeh_TargetLane_Base->StableVelocity()) : MAX_SPEED;
#  endif /*__TX_Mark__("crash stop lane change")*/
  const txFloat RearVeh_TargetLaneVelocity =
      (NonNull_Pointer(RearVeh_TargetLane_Base)) ? (RearVeh_TargetLane_Base->StableVelocity()) : MAX_SPEED();
  const txFloat RearVeh_CurrentLanePushVelocity =
      (NonNull_Pointer(RearVeh_CurrentLane_Base)) ? RearVeh_CurrentLane_Base->StableVelocity() : 0.0;

#  if __TX_Mark__("crash stop lane change")
  if (NonNull_Pointer(FrontVeh_TargetLane_Base) && (_plus_(Base::ISimulationConsistency::DrivingStatus::stop_crash) ==
                                                    FrontVeh_TargetLane_Base->StableDriving_Status())) {
    FrontGap_TargetLane = SafetyGap_Front() + 1.0;
    // LOG(WARNING) << TX_VARS(Id()) << TX_VARS(FrontVeh_TargetLaneVelocity) << TX_VARS(FrontGap_TargetLane);
  }
#  endif /*__TX_Mark__("crash stop lane change")*/

  ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(FrontGap_Current) << TX_VARS(RearGap_Current)
                    << TX_VARS(FrontGap_TargetLane) << TX_VARS(RearGap_TargetLane) << TX_VARS(SideGap_TargetLane)
                    << TX_VARS(FrontVeh_CurrentLaneVelocity) << TX_VARS(FrontVeh_TargetLaneVelocity)
                    << TX_VARS(RearVeh_TargetLaneVelocity) << TX_VARS(SideGap_TargetLane)
                    << TX_VARS(RearVeh_CurrentLanePushVelocity) << TX_VARS(ComfortGap())
                    << TX_VARS(mKinetics.velocity_comfort);

  // 自由换道：
  // 1.当旁边车道的前车的gap比本车道前车的gap大，而且开得更快,而且本车车速慢于本车的期望速度
  // 2.本车道后面的车跟得太紧了，超出本车司机的comfort zone（距离和速度）了
  // if ((RearGap_Current < ComfortGap) && RearVeh_CurrentLanePushVelocity > velocity_comfort)
  // //本车道后面的车跟得太紧了，超出本车司机的comfort zone（距离和速度）了,则必然(不进行LaneChangeProb的判断)换道
  if (((RearGap_Current < ComfortGap()) && RearVeh_CurrentLanePushVelocity > mKinetics.velocity_comfort) ||
      ((FrontGap_TargetLane > EmptyTargetLaneGapThreshold()) &&
       (m_aggress >
        LaneChanging_Aggressive_Threshold()))) {  // 2020/0203 HD
                                                  // ：旁边车道前面的gap如果大于EmptyTargetLaneGap=50，而且是比较激进的车，则有换道意愿
    if (FLAGS_EnableRearComfortGap) {
      LaneChange_Intention = LaneChangeIntention::eYes;
      ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = yes [1]";
    }
    // 当旁边车道的前车的gap比本车道前车的gap大，而且开得更快,而且本车车速慢于本车的期望速度
    // else if ((FrontGap_Current< MAX_REACTION_GAP) && (FrontVeh_CurrentLaneVelocity < FrontVeh_TargetLaneVelocity) &&
    // (m_objectData.velocity < m_Data.velocity_desired) && ((FrontGap_Current< FrontGap_TargetLane *
    // pow(FrontVeh_TargetLaneVelocity / FrontVeh_CurrentLaneVelocity, 2.0)) || (FrontGap_Current<FrontGap_TargetLane)))
    /* else if (FrontGap_Current < MAX_REACTION_GAP &&
        ( (FrontVeh_CurrentLaneVelocity < FrontVeh_TargetLaneVelocity) &&
          (m_velocity < velocity_desired) &&
            ( (FrontGap_Current < FrontGap_TargetLane * pow(FrontVeh_TargetLaneVelocity /
       FrontVeh_CurrentLaneVelocity, 2.0)) || (FrontGap_Current < FrontGap_TargetLane)
            )
        ))*/
  } else if (FrontGap_Current < MAX_REACTION_GAP() &&
             ((FrontVeh_CurrentLaneVelocity < FrontVeh_TargetLaneVelocity) &&
              (StableVelocity() < mKinetics.velocity_desired) &&
              ((FrontGap_Current <
                FrontGap_TargetLane *
                    pow(FrontVeh_TargetLaneVelocity / (FrontVeh_CurrentLaneVelocity + SpeedDenominatorOffset()),
                        2.0)) ||
               (FrontGap_Current < FrontGap_TargetLane)))) {  // 1229 HD
    // if (Random() > m_Data.aggress) return 0;//彆呴儂杅苤衾隅砱腔遙耋衙薹ㄛ寀殿隙ㄗ祥遙耋ㄘ
    // else LaneChange_Intention = 1;
    // if ((Random() < m_Data.aggress) || (m_objectData.velocity < m_Data.velocity_desired * Tolerance_Threshold))
    if (m_aggress > LaneChanging_Aggressive_Threshold() ||
        (StableVelocity() < (mKinetics.velocity_desired * Tolerance_Threshold()))) {
      LaneChange_Intention = LaneChangeIntention::eYes;
      ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [2]";
    } else {
      ChangeLaneLogInfo << TX_VARS(Id()) << " lanechanging refuse lane change.  lanechange return 0";
      return LaneChangeIntention::eNo;
    }
    /*else if (FrontPedWalkingInSight(NEAR_DYNAMIC_SEARCH_FRONT, MAX_REACTION_GAP) > 0)
      LaneChange_Intention = 1;*/
  } else if (mRouteAI.IsValid() && mLocation.IsOnLane()) {
    Base::txLaneUId curLaneUid = mLocation.LaneLocInfo().onLaneUid;
    if (CallFail(mRouteAI.CheckReachable(mLocation.LaneLocInfo()))) {
      if (PedestrianSearchType::eLeft == targetType) {
        Base::txLaneID dstLaneId = curLaneUid.laneId + 1;
        while (dstLaneId < 0) {
          const Base::txLaneUId dstLaneUid(curLaneUid.roadId, curLaneUid.sectionId, dstLaneId);
          if (mRouteAI.CheckReachable(dstLaneUid)) {
            LaneChange_Intention = LaneChangeIntention::eYes;
            ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [3][left_route_reachable]";
          } else {
            ChangeLaneLogInfo << TX_VARS_NAME(dst_laneUid, Utils::ToString(dstLaneUid)) << " do not reachable [left].";
          }
          ++dstLaneId;
        }
      } else if (PedestrianSearchType::eRight == targetType) {
        Base::txLaneID dstLaneId = curLaneUid.laneId - 1;
        const Base::txLaneID LaneCount = mLocation.tracker()->GetCurrentLaneCount();
        /*LOG(WARNING) << "PedestrianSearchType::eRight == targetType" << TX_VARS(curLaneUid.laneId)
            << TX_VARS(dstLaneId)
            << TX_VARS((LaneCount + dstLaneId));*/

        while ((LaneCount + dstLaneId) >= 0) {
          /*LOG(WARNING) << "PedestrianSearchType::eRight == targetType" << TX_VARS(curLaneUid.laneId)
              << TX_VARS(dstLaneId)
              << TX_VARS(LaneCount)
              << TX_VARS((LaneCount + dstLaneId));*/
          const Base::txLaneUId dstLaneUid(curLaneUid.roadId, curLaneUid.sectionId, dstLaneId);
          if (mRouteAI.CheckReachable(dstLaneUid)) {
            LaneChange_Intention = LaneChangeIntention::eYes;
            ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [4][right_route_reachable]";
          } else {
            ChangeLaneLogInfo << TX_VARS_NAME(dst_laneUid, Utils::ToString(dstLaneUid)) << " do not reachable [right].";
          }
          --dstLaneId;
        }
      }
    } else {
      TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
      mRouteAI.IsReachable(mLocation.LaneLocInfo(), routeHint);
      if (routeHint.NeedTurnLeft() && PedestrianSearchType::eLeft == targetType) {
        LaneChange_Intention = LaneChangeIntention::eYes;
        ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [5][NeedTurnLeft]";
      } else if (routeHint.NeedTurnRight() && PedestrianSearchType::eRight == targetType) {
        LaneChange_Intention = LaneChangeIntention::eYes;
        ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [6][NeedTurnRight]";
      }

      // LOG(WARNING) << TX_VARS(targetType) << TX_VARS_NAME(lanechange_action,
      // routeHint.m_lanechange_action._to_string());
    }
  } else if (CallFail(mRouteAI.IsValid())) {
    const Base::txFloat rnd = mPRandom.GetRandomValue_NegOne2PosOne();
    if (rnd > FLAGS_AI_NonRoute_LaneChangeThreshold) {
      LaneChange_Intention = LaneChangeIntention::eYes;
      ChangeLaneLogInfo << TX_VARS(Id()) << " mRouteAI.IsValid() = false";
    }
  } else {
    ChangeLaneLogInfo
        << TX_VARS(Id()) << " lanechanging refuse lane change. lanechange return 0  [1]" << TX_VARS(RearGap_Current)
        << TX_VARS(ComfortGap()) << TX_VARS(RearVeh_CurrentLanePushVelocity) << TX_VARS(mKinetics.velocity_comfort)
        << TX_VARS(FrontGap_TargetLane) << TX_VARS(EmptyTargetLaneGapThreshold()) << TX_VARS(m_aggress)
        << TX_VARS(LaneChanging_Aggressive_Threshold()) << TX_VARS(FrontGap_Current) << TX_VARS(MAX_REACTION_GAP())
        << TX_VARS(FrontVeh_CurrentLaneVelocity) << TX_VARS(FrontVeh_TargetLaneVelocity) << TX_VARS(StableVelocity())
        << TX_VARS(mKinetics.velocity_desired) << TX_VARS(FrontGap_Current)
        << TX_VARS((FrontGap_TargetLane *
                    pow(FrontVeh_TargetLaneVelocity / (FrontVeh_CurrentLaneVelocity + SpeedDenominatorOffset()), 2.0)))
        << TX_VARS(FrontGap_Current) << TX_VARS(FrontGap_TargetLane);
    return LaneChangeIntention::eNo;
  }

  if (LaneChangeIntention::eNo == LaneChange_Intention) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " lanechanging refuse lane change. lanechange return 0  [2]";
    return LaneChangeIntention::eNo;  // 如果没有换道意愿，则返回（不换道）
  }

#  if _refuse_lane_change_before_signal_
  if (approach_signal()) {
#    if _20210103_debug_
    if (mShowLog) {
      LOG(INFO) << TXST_TRACE_VARIABLES(m_id) << " lanechanging refuse lane change.";
    }
#    endif /*_20210103_debug_*/
    return 0;
  }
#  endif
  /*_refuse_lane_change_before_signal_
  此换道安全的条件以后可以替换,后面安全条件加了一个系数，即在旁边车道的后车速度较大的情况下需要留出更大的后车距
  /*if ((FrontGap_TargetLane > SafetyGap_Front) &&
      ((RearGap_TargetLane > SafetyGap_Rear * pow(RearVeh_TargetLaneVelocity / m_velocity, 1.5))) &&
      (FrontPedWalkingInSight(targetType, MAX_REACTION_GAP) == 0))*/
  const Base::txFloat SafetyGap_V_AdjWeight = 0.5;                                             // 1229 HD
  if ((FrontGap_TargetLane > SafetyGap_Front()) && (SideGap_TargetLane > SafetyGap_Side()) &&  // 0103 HD
      /*if ((FrontGap_TargetLane > SafetyGap_Front) &&*/
      ((RearGap_TargetLane >
        SafetyGap_Rear() *
            pow(RearVeh_TargetLaneVelocity / (StableVelocity() + SpeedDenominatorOffset()), SafetyGap_V_AdjWeight))) &&
      (StableVelocity() >
       ((NonNull_Pointer(RearVeh_TargetLane_Base)) ? (RearVeh_TargetLane_Base->StableVelocity()) : -1.0)) &&
      (Math::isZero(FrontPedWalkingInSight(targetType, MAX_REACTION_GAP())))) {  // 1229 HD
    ChangeLaneLogInfo << TX_VARS(Id()) << " lanechanging enable lane change. ";
    return LaneChangeIntention::eYes;  // 0103 YC
  } else {
    ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(FrontGap_TargetLane) << TX_VARS(SafetyGap_Front())
                      << TX_VARS(SideGap_TargetLane) << TX_VARS(SafetyGap_Side()) << TX_VARS(RearGap_TargetLane)
                      << TX_VARS((SafetyGap_Rear() *
                                  pow(RearVeh_TargetLaneVelocity / (StableVelocity() + SpeedDenominatorOffset()),
                                      SafetyGap_V_AdjWeight)))
                      << TX_VARS(SafetyGap_Rear()) << TX_VARS(RearVeh_TargetLaneVelocity) << TX_VARS(StableVelocity())
                      << TX_VARS(SpeedDenominatorOffset()) << TX_VARS(SafetyGap_V_AdjWeight)
                      << " lanechanging refuse lane change. return 0.0";
    return LaneChangeIntention::eNo;  // 换道不安全，返回
  }
}

Base::txBool TAD_AI_VehicleElement::ValidSampling(const Base::TimeParamManager &timeMgr) const TX_NOEXCEPT {
  return m_NearestObject.IsValid();
}

// Lane change 3: check again, regulations. Userdefined veh based on this func.
Base::txBool TAD_AI_VehicleElement::SwitchLane(const PedestrianSearchType state) TX_NOEXCEPT {
  TODO("TAD_VehicleMergeEvent::SwitchLane");
  txAssert(IsAlive());
  if (PedestrianSearchType::eFront == state) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " set state is VEHICLE_MOVE_LANE_STRAIGHT. do not need switch lane.";
    return false;
  }

  if (IsInLaneChange()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " under lane change.";
    return false;
  }

#  if 1
  if (mLocation.IsOnLaneLink()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " in lane link, cannot lane change.";
    return false;
  }
#    if 0 /*TADSIM-84*/
    // if (HdMap::HadmapCacheConCurrent::IsRoadNearJunction(mLocation.LaneLocInfo().onLaneUid) && !BlockAhead())
    // {
        if (mLocation.tracker()->isReachedRoadLastSection()) {
            const Base::txFloat distanceToRoadEnd =
                (mLocation.tracker()->getCurrentLane()->getGeometry(/*need change getLaneGeomInfo()*/)
                  ->getLength() - mLocation.DistanceAlongCurve());
            if (distanceToRoadEnd < FLAGS_RefuseLaneChangeDistance) {
                ChangeLaneLogInfo << TX_VARS(Id()) << " near to junction , cannot lane change."
                                  << TX_VARS(distanceToRoadEnd) << TX_VARS(FLAGS_RefuseLaneChangeDistance);
                return false;
            } else {
                ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(distanceToRoadEnd);
            }
        } else {
            TX_MARK("maybe section is too short.");
            Coord::txWGS84 roadEnd_GPS =
                            HdMap::HadmapCacheConCurrent::GetRoadEnd(mLocation.LaneLocInfo().onLaneUid.roadId);
            const Base::txFloat distanceToRoadEnd = Coord::txENU::EuclideanDistanceLocal_2D(
                                                                          roadEnd_GPS.ToENU(), StableGeomCenter());
            if (distanceToRoadEnd < FLAGS_RefuseLaneChangeDistance) {
                ChangeLaneLogInfo << TX_VARS(Id()) << " near to junction , cannot lane change."
                                  << TX_VARS(distanceToRoadEnd) << TX_VARS(FLAGS_RefuseLaneChangeDistance);
                return false;
            }
        }
    // }
#    endif

#  endif

  if (PedestrianSearchType::eLeft == state) {
    TX_MARK("向左变道");
    if (mLocation.IsOnLeftestLane()) {
      TX_MARK("已经最内侧车道");
      ChangeLaneLogInfo << TX_VARS(Id()) << " IsOnLeftestLane.";
      return false;
    }
    if (mLocation.tracker()->IsLeftBoundarySolid()) {
      TX_MARK("在边道上，不换道");
      ChangeLaneLogInfo << TX_VARS(Id()) << " IsLeftBoundarySolid.";
      return false;
    }
  } else if (PedestrianSearchType::eRight == state) {
    TX_MARK("向右变道");
    if (mLocation.IsOnFinalLane()) {
      /*TX_MARK("在road的最后一个lane，或者说是road的末端");*/
      ChangeLaneLogInfo << TX_VARS(Id()) << " IsOnFinalLane.";
      return false;
    }

    if (mLocation.IsOnRightestLane()) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " IsOnRightestLane.";
      return false;
    }

    if (mLocation.tracker()->IsRightBoundarySolid()) {
      TX_MARK("在边道上，不换道");
      ChangeLaneLogInfo << TX_VARS(Id()) << " IsRightBoundarySolid.";
      return false;
    }
  }

  const txFloat laneChangeTime = ComputeMinLaneChangeTime(LcDuration());
#  if 1
  if (laneChangeTime <= 0.0) {
    ChangeLaneLogInfo << TX_VARS(laneChangeTime) << " invalid.";
    return false;
  }
#  endif
  m_LaneChangeTimeMgr.SetDuration(laneChangeTime);
  m_LaneChangeTimeMgr.SetProcedure(m_LaneChangeTimeMgr.Duration());
  m_LaneChangeTimeMgr.SetMinCancelDuration(NoCheckLCAbortThreshold() * m_LaneChangeTimeMgr.Duration());  // 1229 HD

  if (PedestrianSearchType::eLeft == state) {
    SetTurnLeft_Start();
  } else if (PedestrianSearchType::eRight == state) {
    SetTurnRight_Start();
  }
  return true;
}

void TAD_AI_VehicleElement::UpdateLaneKeep(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  // Variance 2,3 ; Mean 5 (about)
  if (CallFail(mLaneKeepMgr.Expired())) {
    ChangeLaneLogInfo << "[LaneKeep]" << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.Duration())
                      << TX_VARS(timeMgr.AbsTime());
    mLaneKeepMgr.Decrement(timeMgr.RelativeTime());
  }
}

// Lane change func entrance, handle the various situations.
Base::txFloat TAD_AI_VehicleElement::Driving_SwitchLaneOrg(
    const Base::TimeParamManager &timeMgr, Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base,
    Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base, Base::SimulationConsistencyPtr LeftVeh_TargetLane_Base,
    Base::SimulationConsistencyPtr RightVeh_TargetLane_Base) TX_NOEXCEPT {
  txFloat resLateralVelocity = 0.0;
  if (IsInTurn_Abort() /* m_LaneAbortingTimeMgr.Duration() > 0.0*/) {
    TX_MARK("under abort lane change.");
    ChangeLaneLogInfo << TX_VARS_NAME(" under abort lane change, ", Id());
    if (m_LaneAbortingTimeMgr.Procedure() <= 0.0) {
      m_LaneAbortingTimeMgr.SetDuration(0.0);  /*LaneAborting_Duration = 0;*/
      m_LaneAbortingTimeMgr.SetProcedure(0.0); /*LaneAborting_Procedure = 0;*/

      if (SetLaneKeep()) {
        mLaneKeepMgr.SetRndDuration(mPRandom.GetRandomValue());
        ChangeLaneLogInfo << TX_VARS(Id()) << " finish abort lane change."
                          << TX_VARS_NAME(LaneKeepTime, mLaneKeepMgr.Duration());
      } else {
        LogWarn << TX_VARS_NAME("change state from laneAbort to LaneKeep failure, ", Id());
      }
    } else {
      resLateralVelocity = MoveBack_Sideway(timeMgr);
      ChangeLaneLogInfo << TX_VARS(Id()) << " under abort lane change. MoveBack_Sideway"
                        << TX_VARS(m_LaneAbortingTimeMgr);
    }
  } else {
    if (IsInLaneChange_StartIng()) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " under lane change.";
      Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
      Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
      Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
      Base::txFloat FrontGap_TargetLane = 999.0;
      Base::txFloat RearGap_TargetLane = 999.0;
      Base::txFloat SideGap_TargetLane = 999.0;

      std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) = FrontVeh_TargetLane();
#  if __TX_Mark__("crash stop lane change")
      if (NonNull_Pointer(FrontVeh_TargetLane_Base) &&
          (_plus_(Base::ISimulationConsistency::DrivingStatus::stop_crash) ==
           FrontVeh_TargetLane_Base->StableDriving_Status())) {
        FrontGap_TargetLane = SafetyGap_Front() + 1.0;
      }
#  endif /*__TX_Mark__("crash stop lane change")*/
      std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) = RearVeh_TargetLane();
      // 0205 HD 变量名称改变MAX_SPEED
      const Base::txFloat RearVeh_TargetLaneVelocity =
          (NonNull_Pointer(RearVeh_TargetLane_Base)) ? (RearVeh_TargetLane_Base->StableVelocity()) : MAX_SPEED();
      // float FrontGap_TargetLane = (FrontVeh_TargetLane) ? (VectorSub(FrontVeh_TargetLane->m_vPos, m_vPos).Magnitude()
      // - m_Data.length) : 100.0; const Base::txFloat FrontGap_TargetLane =
      // m_NearestObject.aroudDistance[Base::OBJECT_TARGET_FRONT_DIRECTION];

      // if ((RearGap_TargetLane < SafetyGap_Rear) || (FrontGap_TargetLane < SafetyGap_Front)/* ||
      // (RearVeh_TargetLan_Velocity > m_Data.velocity_comfort)*/) if (((RearGap_TargetLane < SafetyGap_Rear) ||
      // (FrontGap_TargetLane < SafetyGap_Front)) && (LaneChanging_Procedure > LaneChanging_Min_Cancel_Duration)) 0203
      // HD/////////////////////////////////////////////////////
      std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) = SideVeh_TargetLane();
      ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(FrontGap_TargetLane) << TX_VARS(RearGap_TargetLane)
                        << TX_VARS(m_LaneChangeTimeMgr.Duration()) << TX_VARS(m_LaneChangeTimeMgr.Procedure())
                        << TX_VARS(m_LaneChangeTimeMgr.Min_Cancel_Duration()) << TX_VARS(SideGap_TargetLane);
      // 0203 HD/////////////////////////////////////////////////////
      const PedestrianSearchType searchDirectionDir =
          (IsInTurnLeft()) ? (PedestrianSearchType::eLeft) : (PedestrianSearchType::eRight);
      const Base::txFloat SafetyGap_V_AdjWeight = 0.5;
      const Base::txFloat SafetyGap_Rear_Special =
          SafetyGap_Rear() *
          pow(RearVeh_TargetLaneVelocity / (StableVelocity() + SpeedDenominatorOffset()), SafetyGap_V_AdjWeight);
      if (((RearGap_TargetLane < SafetyGap_Rear_Special) || (FrontGap_TargetLane < SafetyGap_Front()) ||
           (SideGap_TargetLane < SafetyGap_Side()) ||
           (FrontPedWalkingInSight(searchDirectionDir, Max_Ped_Reaction_Distance()) > 0)) &&
          (IsInTurn_Start())) {
        ChangeLaneLogInfo << TX_VARS(Id()) << "abort_lane_change  " << TX_VARS(RearGap_TargetLane)
                          << TX_VARS(SafetyGap_Rear_Special) << TX_VARS(FrontGap_TargetLane)
                          << TX_VARS(SafetyGap_Front()) << TX_VARS(SideGap_TargetLane) << TX_VARS(SafetyGap_Side())
                          << TX_VARS(__enum2lpsz__(TAD_VehicleState_AI, State()));
        m_LaneAbortingTimeMgr.SetDuration(m_LaneChangeTimeMgr.Experience());
        m_LaneAbortingTimeMgr.SetProcedure(m_LaneAbortingTimeMgr.Duration());
        m_LaneChangeTimeMgr.SetProcedure(0.0);
        m_LaneChangeTimeMgr.SetDuration(0.0);
        SetTurnAbort();
        mLaneKeepMgr.SetRndDuration(mPRandom.GetRandomValue());
        ChangeLaneLogInfo << TX_VARS(Id()) << " GetLaneKeepDuration " << TX_VARS(mLaneKeepMgr.Duration());
      } else {
        ChangeLaneLogInfo << TX_VARS(Id()) << "abort_lane_change_error  " << TX_VARS(RearGap_TargetLane)
                          << TX_VARS(SafetyGap_Rear_Special) << TX_VARS(FrontGap_TargetLane)
                          << TX_VARS(SafetyGap_Front()) << TX_VARS(SideGap_TargetLane) << TX_VARS(SafetyGap_Side())
                          << TX_VARS(__enum2lpsz__(TAD_VehicleState_AI, State()));
      }

      if (IsInLaneChange_StartIng()) {
        resLateralVelocity = MoveTo_Sideway(timeMgr);
      }
    } else {
      Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base = nullptr;
      Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base_4_lanechange = nullptr;
      Base::SimulationConsistencyPtr FrontVeh_CurrentLane_idm = nullptr;
      Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base = nullptr;
      Base::txFloat FrontGap_CurrentLane = 999.0;
      Base::txFloat FrontGap_CurrentLane_4_lanechange = 999.0;
      Base::txFloat FrontGap_CurrentLane_idm = 999.0;
      Base::txFloat RearGap_CurrentLane = 999.0;
      std::tie(FrontVeh_CurrentLane_Base, FrontGap_CurrentLane) = FrontVeh_CurrentLane();
      FrontVeh_CurrentLane_Base_4_lanechange = FrontVeh_CurrentLane_Base;
      FrontGap_CurrentLane_4_lanechange = FrontGap_CurrentLane;
#  if 0
            if (NonNull_Pointer(FollowFrontPtr()) && (FollowFrontDistance() < FrontGap_CurrentLane)) {
                ChangeLaneLogInfo << TX_VARS(Id()) << " Employ IDM Front Info for LaneChange : "
                    << TX_VARS_NAME(SUDOKU_GRID_FrontDist, FrontGap_CurrentLane)
                    << TX_VARS_NAME(IDM_FrontDist, FollowFrontDistance());
                FrontVeh_CurrentLane_Base = FollowFrontPtr();
                FrontGap_CurrentLane = FollowFrontDistance();
            }
#  endif
      std::tie(RearVeh_CurrentLane_Base, RearGap_CurrentLane) = RearVeh_CurrentLane();
      const Base::txBool bCanSwitchToRightLane =
          ValidSampling(timeMgr) && CheckCanSwitchRightLaneState(timeMgr) &&
          (Null_Pointer(FrontVeh_CurrentLane_Base) || (NonNull_Pointer(FrontVeh_CurrentLane_Base) &&
                                                       CallSucc(FrontVeh_CurrentLane_Base->IsNotStableTurnRight()))) &&
          (Null_Pointer(RearVeh_CurrentLane_Base) ||
           (NonNull_Pointer(RearVeh_CurrentLane_Base) && CallSucc(RearVeh_CurrentLane_Base->IsNotStableTurnRight())));
      const Base::txBool bCanSwitchToLeftLane =
          ValidSampling(timeMgr) && CheckCanSwitchLeftLaneState(timeMgr) &&
          (Null_Pointer(FrontVeh_CurrentLane_Base) || (NonNull_Pointer(FrontVeh_CurrentLane_Base) &&
                                                       CallSucc(FrontVeh_CurrentLane_Base->IsNotStableTurnLeft()))) &&
          (Null_Pointer(RearVeh_CurrentLane_Base) ||
           (NonNull_Pointer(RearVeh_CurrentLane_Base) && CallSucc(RearVeh_CurrentLane_Base->IsNotStableTurnLeft())));

      ChangeLaneLogInfo << "[lane_change] change can switch " << TX_VARS(Id()) << TX_VARS(ValidSampling(timeMgr))
                        << TX_VARS(bCanSwitchToRightLane) << TX_VARS(bCanSwitchToRightLane)
                        << TX_VARS(CheckCanSwitchRightLaneState(timeMgr))
                        << TX_VARS(CheckCanSwitchLeftLaneState(timeMgr))
                        << TX_VARS((Null_Pointer(FrontVeh_CurrentLane_Base) ||
                                    (NonNull_Pointer(FrontVeh_CurrentLane_Base) &&
                                     CallSucc(FrontVeh_CurrentLane_Base->IsNotStableTurnRight()))))
                        << TX_VARS((Null_Pointer(RearVeh_CurrentLane_Base) ||
                                    (NonNull_Pointer(RearVeh_CurrentLane_Base) &&
                                     CallSucc(RearVeh_CurrentLane_Base->IsNotStableTurnRight()))));

#  if 1
      const Base::txFloat RearVeh_CurrentLanePushVelocity =
          (NonNull_Pointer(RearVeh_CurrentLane_Base)) ? (RearVeh_CurrentLane_Base->StableVelocity()) : (0.0);
      if ((RearGap_CurrentLane < ComfortGap()) && (RearVeh_CurrentLanePushVelocity > mKinetics.velocity_comfort)) {
        if (FLAGS_EnableRearComfortGap) {
          ChangeLaneLogInfo << "mLaneKeepMgr.Clear() : " << TX_VARS(Id()) << TX_VARS(RearGap_CurrentLane)
                            << TX_VARS(ComfortGap()) << TX_VARS(RearVeh_CurrentLanePushVelocity)
                            << TX_VARS(mKinetics.velocity_comfort);
          mLaneKeepMgr.Clear();
        }
      }
#  endif

      if (CallSucc(mLaneKeepMgr.Expired())) {
        std::vector<PedestrianSearchType> cmd_vec;
        cmd_vec.reserve(2);
        if (CallSucc(bCanSwitchToRightLane) && CallSucc(bCanSwitchToLeftLane)) {
          const Base::txBool left_first = (1 == mPRandom.GetRandomInt() % 2);
          if (left_first) {
            cmd_vec.push_back(PedestrianSearchType::eLeft);
            cmd_vec.push_back(PedestrianSearchType::eRight);
          } else {
            cmd_vec.push_back(PedestrianSearchType::eRight);
            cmd_vec.push_back(PedestrianSearchType::eLeft);
          }
        } else if (CallSucc(bCanSwitchToRightLane) && CallFail(bCanSwitchToLeftLane)) {
          cmd_vec.push_back(PedestrianSearchType::eRight);
        } else if (CallFail(bCanSwitchToRightLane) && CallSucc(bCanSwitchToLeftLane)) {
          cmd_vec.push_back(PedestrianSearchType::eLeft);
        }
        for (const auto dir : cmd_vec) {
          if (PedestrianSearchType::eLeft == dir) {
            ChangeLaneLogInfo << TX_VARS(Id()) << " bCanSwitchToLeftLane true.";
            Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
            Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
            Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
            Base::txFloat FrontGap_TargetLane = 999.0;
            Base::txFloat RearGap_TargetLane = 999.0;
            Base::txFloat SideGap_TargetLane = 999.0;

            std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
                FrontVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eLeft));
            std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
                RearVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eLeft));
            std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
                SideVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eLeft));
            ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(timeMgr.AbsTime())
                              << TX_VARS_NAME(VehicleMoveLaneState, Base::Enums::VehicleMoveLaneState::eLeft)
                              << TX_VARS_NAME(FrontVeh_TargetLane_BaseId,
                                              (NonNull_Pointer(FrontVeh_TargetLane_Base)
                                                   ? FrontVeh_TargetLane_Base->ConsistencyId()
                                                   : -1))
                              << TX_VARS(FrontGap_TargetLane)
                              << TX_VARS_NAME(RearVeh_TargetLane_BaseId, (NonNull_Pointer(RearVeh_TargetLane_Base)
                                                                              ? RearVeh_TargetLane_Base->ConsistencyId()
                                                                              : -1))
                              << TX_VARS(RearGap_TargetLane);

            if (Null_Pointer(LeftVeh_TargetLane_Base) &&
                LaneChangeIntention::eYes ==
                    LaneChanging(FrontVeh_CurrentLane_Base_4_lanechange /*FrontVeh_CurrentLane_Base*/,
                                 RearVeh_CurrentLane_Base, FrontVeh_TargetLane_Base, RearVeh_TargetLane_Base,
                                 FrontGap_CurrentLane_4_lanechange /*FrontGap_CurrentLane*/, RearGap_CurrentLane,
                                 FrontGap_TargetLane, RearGap_TargetLane, SideGap_TargetLane,
                                 PedestrianSearchType::eLeft)) {
              if (SwitchLane(PedestrianSearchType::eLeft)) {
                ChangeLaneLogInfo << "[20221114]" << TX_VARS(timeMgr.AbsTime()) << TX_VARS(Id())
                                  << TX_VARS(timeMgr.AbsTime()) << "VEHICLE_MOVE_LANE_LEFT SwitchLane true.";
                resLateralVelocity = MoveTo_Sideway(timeMgr);
              } else {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_LEFT SwitchLane failure.";
              }
            }
          } else {
            ChangeLaneLogInfo << TX_VARS(Id()) << " bCanSwitchToRightLane true.";
            Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
            Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
            Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
            Base::txFloat FrontGap_TargetLane = 999.0;
            Base::txFloat RearGap_TargetLane = 999.0;
            Base::txFloat SideGap_TargetLane = 999.0;

            std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
                FrontVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));
            std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
                RearVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));
            std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
                SideVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));

            ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(timeMgr.AbsTime())
                              << TX_VARS_NAME(VehicleMoveLaneState, Base::Enums::VehicleMoveLaneState::eRight)
                              << TX_VARS_NAME(FrontVeh_TargetLane_BaseId,
                                              (NonNull_Pointer(FrontVeh_TargetLane_Base)
                                                   ? FrontVeh_TargetLane_Base->ConsistencyId()
                                                   : -1))
                              << TX_VARS(FrontGap_TargetLane)
                              << TX_VARS_NAME(RearVeh_TargetLane_BaseId, (NonNull_Pointer(RearVeh_TargetLane_Base)
                                                                              ? RearVeh_TargetLane_Base->ConsistencyId()
                                                                              : -1))
                              << TX_VARS(RearGap_TargetLane);

            if (LaneChangeIntention::eYes ==
                LaneChanging(FrontVeh_CurrentLane_Base_4_lanechange /*FrontVeh_CurrentLane_Base*/,
                             RearVeh_CurrentLane_Base, FrontVeh_TargetLane_Base, RearVeh_TargetLane_Base,
                             FrontGap_CurrentLane_4_lanechange /*FrontGap_CurrentLane*/, RearGap_CurrentLane,
                             FrontGap_TargetLane, RearGap_TargetLane, SideGap_TargetLane,
                             PedestrianSearchType::eRight)) {
              if (SwitchLane(PedestrianSearchType::eRight)) {
                resLateralVelocity = MoveTo_Sideway(timeMgr);
                ChangeLaneLogInfo << "[20221114]" << TX_VARS(Id()) << TX_VARS(timeMgr.AbsTime())
                                  << "VEHICLE_MOVE_LANE_RIGHT SwitchLane true.";
              } else {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane false.";
              }
            }
          }
        }
#  if 0
                if (bCanSwitchToLeftLane) {  // 如果左边有车道，先考虑往左边换道
                    // LOG_IF(WARNING, 13 == Id()) /*ChangeLaneLogInfo*/ << TX_VARS(Id())
                                                      << " CheckCanSwitchLeftLaneState true.";

                    Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
                    Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
                    Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
                    Base::txFloat FrontGap_TargetLane = 999.0;
                    Base::txFloat RearGap_TargetLane = 999.0;
                    Base::txFloat SideGap_TargetLane = 999.0;

                    std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
                                FrontVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eLeft));
                    std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
                                RearVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eLeft));
                    std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
                                SideVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eLeft));
                    if (Null_Pointer(LeftVeh_TargetLane_Base) &&
                        LaneChangeIntention::eYes == LaneChanging(
                                  FrontVeh_CurrentLane_Base_4_lanechange/*FrontVeh_CurrentLane_Base*/,
                                  RearVeh_CurrentLane_Base,
                                  FrontVeh_TargetLane_Base, RearVeh_TargetLane_Base,
                                  FrontGap_CurrentLane_4_lanechange/*FrontGap_CurrentLane*/,
                                  RearGap_CurrentLane,
                                  FrontGap_TargetLane,
                                  RearGap_TargetLane,
                                  SideGap_TargetLane,
                                  PedestrianSearchType::eLeft)) {
                        if (SwitchLane(PedestrianSearchType::eLeft)) {
                            ChangeLaneLogInfo << "[20221114]" << TX_VARS(timeMgr.AbsTime()) << TX_VARS(Id())
                                          << TX_VARS(timeMgr.AbsTime()) << "VEHICLE_MOVE_LANE_LEFT SwitchLane true.";
                            resLateralVelocity = MoveTo_Sideway(timeMgr);
                        } else {
                            ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_LEFT SwitchLane failure.";
                        }
                    } else if (bCanSwitchToRightLane) {
                        ChangeLaneLogInfo << TX_VARS(Id()) << " bCanSwitchToRightLane true.";
                        std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
                                                FrontVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));
                        std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
                                                RearVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));
                        std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
                                                SideVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));


                        if (LaneChangeIntention::eYes == LaneChanging(
                                      FrontVeh_CurrentLane_Base_4_lanechange/*FrontVeh_CurrentLane_Base*/,
                                      RearVeh_CurrentLane_Base,
                                      FrontVeh_TargetLane_Base,
                                      RearVeh_TargetLane_Base,
                                      FrontGap_CurrentLane_4_lanechange/*FrontGap_CurrentLane*/,
                                      RearGap_CurrentLane,
                                      FrontGap_TargetLane,
                                      RearGap_TargetLane,
                                      SideGap_TargetLane,
                                      PedestrianSearchType::eRight)) {
                            if (SwitchLane(PedestrianSearchType::eRight)) {
                                ChangeLaneLogInfo << "[20221114]" << TX_VARS(Id()) << TX_VARS(timeMgr.AbsTime())
                                                  << "VEHICLE_MOVE_LANE_RIGHT SwitchLane true.";
                                resLateralVelocity = MoveTo_Sideway(timeMgr);
                            } else {
                                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane false.";
                            }
                        }
                    }
                } else if (bCanSwitchToRightLane) {
                    ChangeLaneLogInfo << TX_VARS(Id()) << " bCanSwitchToRightLane true.";
                    Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
                    Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
                    Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
                    Base::txFloat FrontGap_TargetLane = 999.0;
                    Base::txFloat RearGap_TargetLane = 999.0;
                    Base::txFloat SideGap_TargetLane = 999.0;

                    std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
                                    FrontVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));
                    std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
                                    RearVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));
                    std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
                                    SideVeh_TargetLane(_plus_(Base::Enums::VehicleMoveLaneState::eRight));

                    if (LaneChangeIntention::eYes == LaneChanging(
                                  FrontVeh_CurrentLane_Base_4_lanechange/*FrontVeh_CurrentLane_Base*/,
                                  RearVeh_CurrentLane_Base, FrontVeh_TargetLane_Base,
                                  RearVeh_TargetLane_Base,
                                  FrontGap_CurrentLane_4_lanechange/*FrontGap_CurrentLane*/,
                                  RearGap_CurrentLane,
                                  FrontGap_TargetLane,
                                  RearGap_TargetLane,
                                  SideGap_TargetLane,
                                  PedestrianSearchType::eRight)) {
                        if (SwitchLane(PedestrianSearchType::eRight)) {
                            resLateralVelocity = MoveTo_Sideway(timeMgr);
                            ChangeLaneLogInfo << "[20221114]" << TX_VARS(Id()) << TX_VARS(timeMgr.AbsTime())
                                              << "VEHICLE_MOVE_LANE_RIGHT SwitchLane true.";
                        } else {
                            ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane false.";
                        }
                    }
                }
#  endif
        mLaneKeepMgr.SetRndDuration(mPRandom.GetRandomValue());
        ResetLaneKeep();
        ChangeLaneLogInfo << "[lane_change] ResetCounter "
                          << "[20221114]" << TX_VARS(Id()) << TX_VARS(timeMgr.AbsTime())
                          << TX_VARS(mLaneKeepMgr.Duration());
      } /*if (CallSucc(mLaneKeepMgr.Expired()))*/
    }
  }
  return resLateralVelocity;
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::CheckPedestrianWalking(const txFloat relative_time,
                                                                            txFloat &refLateralVelocity) TX_NOEXCEPT {
#  ifdef ON_CLOUD
  return false;
#  endif
  const auto front_center_idx = __enum2idx__(_plus_(Base::Enums::PedestrianTargetDirectionType::eFrontCenter));
  const auto right_front_idx = __enum2idx__(_plus_(Base::Enums::PedestrianTargetDirectionType::eFrontRight));
  const auto left_front_idx = __enum2idx__(_plus_(Base::Enums::PedestrianTargetDirectionType::eFrontLeft));
  Base::txFloat minDist = FLT_MAX;
  Base::ITrafficElementPtr pede_ptr = nullptr;
  if (minDist > m_NearestDynamic.dist(front_center_idx)) {
    minDist = m_NearestDynamic.dist(front_center_idx);
    pede_ptr = m_NearestDynamic.elemPtr(front_center_idx);
  }

  if (minDist > m_NearestDynamic.dist(right_front_idx)) {
    minDist = m_NearestDynamic.dist(right_front_idx);
    pede_ptr = m_NearestDynamic.elemPtr(right_front_idx);
  }

  if (minDist > m_NearestDynamic.dist(left_front_idx)) {
    minDist = m_NearestDynamic.dist(left_front_idx);
    pede_ptr = m_NearestDynamic.elemPtr(left_front_idx);
  }

  if (NonNull_Pointer(pede_ptr)) {
    const Base::txFloat tmpSlowingDownDec = Deceleration(minDist, GetVelocity(), FLAGS_Deceleration_Safe_Distance,
                                                         Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速
    mKinetics.m_acceleration = fmin(tmpSlowingDownDec, mKinetics.m_acceleration);
    LOG_IF(INFO, FLAGS_LogLevel_AI_EnvPercption)
        << TX_VARS(Id()) << TX_VARS(pede_ptr->Id()) << TX_VARS(minDist) << TX_VARS(GetVelocity())
        << TX_VARS(FLAGS_Deceleration_Safe_Distance) << TX_VARS(Max_Deceleration())
        << TX_VARS_NAME(Dec, (GetVelocity() * GetVelocity()) / (-2.0 * (minDist - FLAGS_Deceleration_Safe_Distance)))
        << TX_VARS(tmpSlowingDownDec) << TX_VARS(mKinetics.m_acceleration);
    return true;
  } else {
    LOG_IF(INFO, FLAGS_LogLevel_AI_EnvPercption) << TX_VARS(Id()) << " front clear.";
    return false;
  }
}

#endif /*__TX_Mark__("DrivingChangeLane")*/

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::CheckJunctionYieldConflict() TX_NOEXCEPT {
  auto conflict_veh = Weak2SharedPtr(std::get<_ElementIdx_>(m_YieldConflict));
  if (NonNull_Pointer(conflict_veh)) {
    const Base::txFloat tmpSlowingDownDec =
        Deceleration(std::get<_DistIdx_>(m_YieldConflict), GetVelocity(), FLAGS_Deceleration_Safe_Distance,
                     Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速
    mKinetics.m_acceleration = fmin(tmpSlowingDownDec, mKinetics.m_acceleration);
    return true;
  } else {
    LOG_IF(INFO, FLAGS_LogLevel_AI_EnvPercption) << TX_VARS(Id()) << " junction yield clear.";
    return false;
  }
  return true;
}

#if __TX_Mark__("check signal")
// deprecated
void TAD_AI_VehicleElement::UpdateNextCrossDirection() TX_NOEXCEPT {
  if (mLocation.IsOnLane()) {
    m_op_self_next_driving_direction.clear_value();
    JunctionInfo_SetLastOnLaneInfo(GeomCenter().ToENU(), GetLaneDir());
  } else {
    if (CallFail(m_op_self_next_driving_direction.has_value())) {
      hadmap::txLanePtr pFromLane =
          HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLinkFromLaneUid);
      hadmap::txLanePtr pToLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLinkToLaneUid);

      const Base::txVec3 fromLaneDir =
          HdMap::HadmapCacheConCurrent::GetLaneDir(pFromLane, pFromLane->getLength() - 1.0);
      const Base::txVec3 toLaneDir = HdMap::HadmapCacheConCurrent::GetLaneDir(pToLane, 1.0);

      m_op_self_next_driving_direction = Base::GetSelfDrivingDirection(fromLaneDir, toLaneDir);
    }
  }
}

// doesn't follow this logic
Base::txFloat TAD_AI_VehicleElement::TurnLeft_vs_GoStraight(const Base::IVehicleElementPtr pOpposingVeh,
                                                            const txFloat distance2signal,
                                                            const txFloat acceleration) const TX_NOEXCEPT {
  /*TX_MARK("左转让直行");
  TX_MARK("本车要左转，考虑让直行的情况,Yielding_Reaction_Gap是距离停止线的距离，在这个距离范围之内对对向的直行车进行检查");*/
  txFloat SlowingDownDec = MAX_SPEED();
  if (NonNull_Pointer(pOpposingVeh) && (IsTurnLeftAtNextIntersection()) &&
      ((distance2signal + LeftTurnWaitingBuffer) < LeftYielding_Reaction_Gap) &&
      (distance2signal + LeftTurnWaitingBuffer > 0)) {
    const Base::txFloat OpposingVehDistanceToSignal = pOpposingVeh->GetCurDistanceToTargetSignLight();
    /*float OpposingVehDistanceToSignal = pOpposingVeh->GetData()->nearestSignlightInfo.distance;*/
    /*if (fabs(OpposingVehDistanceToSignal - 999999)<1.0)//此时车已经彻底离开直路部分，进入路口的link部分
    999999是在TrafficVehcicleData.h里面定义的MAX_DISTANCE
    {
        OpposingVehDistanceToSignal = 0.0;//则置0，仅用OpposingBuffer来考虑
    }*/
    // OpposingVehDistanceToSignal这个数值，如果已经跨过了停止线，则是车头到线的负数距离，即，如果是负数，则证明已经过线了！！！
    // 所以这里用这OpposingVehDistanceToSignal +
    // OpposingBuffer>0来判断冲突车是否还需进行判断，OpposingBuffer其实是深入十字路口的距离！！！
    int tempflag = 0;
    /*if ((m_objectData.id == 119) && (OpposingVehDistanceToSignal<5.2)) {
        char szText3[512];
        sprintf(szText3, "OwnID: %ld StraightID: %ld Distance:%lf  OppoDistance:%lf OppoMovingDirection:%1d
    Threshold:%1f", m_objectData.id, pOpposingVeh->GetBasicData()->id, distance, OpposingVehDistanceToSignal,
    pOpposingVeh->m_nextCrossRoadInfo.direction, pOpposingVeh->GetBasicData()->velocity*Critical_Gap_LeftTurn);
        MessageBoxA(nullptr, szText3, "", 0);
        tempflag = 1;
    }*/
    // if ((pOpposingVeh->m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_SAME_SIDE) &&
    // ((OpposingVehDistanceToSignal + OpposingBuffer)<pOpposingVeh->GetBasicData()->velocity*Critical_Gap_LeftTurn))
    // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆

    // if ((pOpposingVeh->m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_SAME_SIDE) &&
    // (OpposingVehDistanceToSignal + LYTOpposingBuffer > 0) && ((OpposingVehDistanceToSignal + LYTOpposingBuffer) <
    // pOpposingVeh->GetBasicData()->velocity*Critical_Gap_LeftTurn))
    // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆
    // 不止要看对向车的速度，还要看对向车的加速度，以防对向车速度慢但正在加速的情形
    // if ((pOpposingVeh->m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_SAME_SIDE) &&
    // (OpposingVehDistanceToSignal + LYTOpposingBuffer > 0) && ((OpposingVehDistanceToSignal + LYTOpposingBuffer) <
    // (pOpposingVeh->GetBasicData()->velocity+ pOpposingVeh->GetBasicData()->acceleration)*Critical_Gap_LeftTurn))
    // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆
    if ((pOpposingVeh->IsGoStraightAtNextIntersection()) && (OpposingVehDistanceToSignal + LYTOpposingBuffer > 0) &&
        ((OpposingVehDistanceToSignal + LYTOpposingBuffer) <
         (pOpposingVeh->StableVelocity() + pOpposingVeh->StableAcc()) *
             Critical_Gap_LeftTurn)) {  // 如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积
                                        // ；OpposingVeh是对向来车中距离停车线最近的一辆
      // SlowingDownDec = Deceleration(distance + WaitingBuffer_LeftTurn, m_objectData.velocity, 0);
      // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance+WaitingBuffer_LeftTurn距离
      // 在信号灯停止线后WaitingBuffer_LeftTurn米处停下来
      // SlowingDownDec = Deceleration(distance+0.5*m_objectData.length, m_objectData.velocity, 0);
      // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance距离，在信号灯停止线处停下来
      // SlowingDownDec = Deceleration(distance, m_objectData.velocity, 0);
      // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance距离，在信号灯停止线处停下来
      SlowingDownDec =
          Deceleration(distance2signal + LeftTurnWaitingBuffer, StableVelocity(), 0,
                       Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速,即本车在距离左转待转区线Distance+
                                             // LeftTurnWaitingBuffer时开始减速，通过Distance+
                                             // LeftTurnWaitingBuffer距离，在信号灯后LeftTurnWaitingBuffer处停下来
      /*if ((m_objectData.id == 119) && (OpposingVehDistanceToSignal<5.2)) {  char szText1[512];
              sprintf(szText1, "OwnDistance: %lf  OwnSpeed: %lf  OwnAcc: %lf", distance, m_objectData.velocity,
      SlowingDownDec); MessageBoxA(nullptr, szText1, "", 0);
      }*/
    }
  }
  return fmin(acceleration, SlowingDownDec);
}

// doesn't follow this logic
Base::txFloat TAD_AI_VehicleElement::TurnRight_vs_TurnLeft_GoStraight(
    const Base::IVehicleElementPtr pOpposingVeh, const Base::IVehicleElementPtr pLeftApproachingVeh,
    const txFloat distance2signal, const txFloat acceleration) const TX_NOEXCEPT {
  txFloat SlowingDownDec = MAX_SPEED();
  if ((IsTurnRightAtNextIntersection()) && (distance2signal + RightTurnWaitingBuffer < RightYielding_Reaction_Gap) &&
      (distance2signal + RightTurnWaitingBuffer > 0)) {  // 考虑了深入十字路口的右转待转区
    // if ((m_nextCrossRoadInfo.direction == TRAFFIC_VEHICLE_CROSS_ROAD_RIGHT_SIDE) && (distance<Yielding_Reaction_Gap))
    // if ((m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_RIGHT_SIDE) &&
    // (distance<Yielding_Reaction_Gap))
    // //本车要左转，考虑让直行的情况,Yielding_Reaction_Gap是距离停止线的距离，在这个距离范围之内对对向的直行车进行检查
    // if
    // ((m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_LEFT_SIDE) && (distance<Yielding_Reaction_Gap))
    // //本车要左转，考虑让直行的情况,Yielding_Reaction_Gap是距离停止线的距离，在这个距离范围之内对对向的直行车进行检查
    // 右转让左转
    if (NonNull_Pointer(pOpposingVeh)) {
      const Base::txFloat OpposingVehDistanceToSignal = pOpposingVeh->GetCurDistanceToTargetSignLight();
      /*float OpposingVehDistanceToSignal = pOpposingVeh->GetData()->nearestSignlightInfo.distance;*/
      /*if (fabs(OpposingVehDistanceToSignal - 999999)<1.0)//此时车已经彻底离开直路部分，进入路口的link部分
      999999是在TrafficVehcicleData.h里面定义的MAX_DISTANCE
      {
          OpposingVehDistanceToSignal = 0.0;//则置0，仅用OpposingBuffer来考虑
      }*/
      // ////////////////////////////////////////////
      // char szText1[512];
      // sprintf(szText1, "OpposingVehDistanceToSignal: %lf, OwnDistance: %lf, Calculated_Distance: %lf, Threshold:
      // %lf", OpposingVehDistanceToSignal,distance, OpposingVehDistanceToSignal +
      // OpposingBuffer,pOpposingVeh->GetBasicData()->velocity*Critical_Gap_LeftTurn); MessageBoxA(nullptr, szText1, "",
      // 0);
      // ////////////////////////////////////////////
      // if (pOpposingVeh && (pOpposingVeh->m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_LEFT_SIDE)
      // && ((OpposingVehDistanceToSignal + OpposingBuffer) <
      // pOpposingVeh->GetBasicData()->velocity*Critical_Gap_LeftTurn))
      // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆
      if ((pOpposingVeh->IsTurnLeftAtNextIntersection()) && ((OpposingVehDistanceToSignal + RYLOpposingBuffer) > 0) &&
          ((OpposingVehDistanceToSignal + RYLOpposingBuffer) <
           (pOpposingVeh->StableVelocity() + pOpposingVeh->StableAcc()) *
               Critical_Gap_LeftTurn)) {  // 如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；
                                          // OpposingVeh是对向来车中距离停车线最近的一辆
        // if (pOpposingVeh && (pOpposingVeh->m_nextCrossRoadPathPlaneDirection == TRAFFIC_VEHICLE_CROSS_ROAD_LEFT_SIDE)
        // && ((OpposingVehDistanceToSignal + RYLOpposingBuffer)>0) && ((OpposingVehDistanceToSignal +
        // RYLOpposingBuffer) < pOpposingVeh->GetBasicData()->velocity*Critical_Gap_LeftTurn))
        // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆

        // SlowingDownDec = Deceleration(distance + WaitingBuffer_RightTurn, m_objectData.velocity, 0);
        // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance+WaitingBuffer_RightTurn距离，
        // //在信号灯停止线后WaitingBuffer_RightTurn米处停下来
        // SlowingDownDec = Deceleration(distance + 0.5*m_objectData.length, m_objectData.velocity, 0);
        // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance距离，在信号灯停止线处停下来
        // SlowingDownDec = Deceleration(distance , m_objectData.velocity, 0);
        // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance距离，在信号灯停止线处停下来
        SlowingDownDec = Deceleration(distance2signal + RightTurnWaitingBuffer, StableVelocity(), 0,
                                      Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速,
                                                            // 即本车在距离右转待转区Distance+RightTurnWaitingBuffer
                                                            // 时开始减速，通过Distancee+RightTurnWaitingBuffer距离，
                                                            // 在信号灯后RightTurnWaitingBuffer处停下来
      }
    }
    // 右转让从左侧道路驶来的直行车
    if (NonNull_Pointer(pLeftApproachingVeh)) {
      const Base::txFloat LeftApproachingVehDistanceToSignal = pLeftApproachingVeh->GetCurDistanceToTargetSignLight();
      /*float LeftApproachingVehDistanceToSignal = pLeftApproachingVeh->GetData()->nearestSignlightInfo.distance;*/
      /*if (fabs(LeftApproachingVehDistanceToSignal- 999999)<1.0)//此时车已经彻底离开直路部分，进入路口的link部分
      999999是在TrafficVehcicleData.h里面定义的MAX_DISTANCE
      {
          LeftApproachingVehDistanceToSignal = 0.0;//则置0，仅用ApproachingBuffer来考虑
      }*/
      // ////////////////////////////////////////////
      // char szText2[512];
      // sprintf(szText2, "LeftApproachingVehDistanceToSignal: %lf, OwnDistance: %lf",
      // LeftApproachingVehDistanceToSignal, distance); MessageBoxA(nullptr, szText2, "", 0);
      // ////////////////////////////////////////////
      // if (pLeftApproachingVeh && (pLeftApproachingVeh->m_nextCrossRoadPathPlaneDirection ==
      // TRAFFIC_VEHICLE_CROSS_ROAD_SAME_SIDE) && ((LeftApproachingVehDistanceToSignal +
      // ApproachingBuffer)<pLeftApproachingVeh->GetBasicData()->velocity * Critical_Gap_LeftTurn))
      // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆
      // if (pLeftApproachingVeh && (pLeftApproachingVeh->m_nextCrossRoadPathPlaneDirection ==
      // TRAFFIC_VEHICLE_CROSS_ROAD_SAME_SIDE) && (LeftApproachingVehDistanceToSignal +
      // ApproachingBuffer>0)&&((LeftApproachingVehDistanceToSignal +
      // ApproachingBuffer)<pLeftApproachingVeh->GetBasicData()->velocity * Critical_Gap_LeftTurn))
      // //如果对向车为直行，而且到停止线的距离小于它速度和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆
      if ((pLeftApproachingVeh->IsGoStraightAtNextIntersection()) &&
          (LeftApproachingVehDistanceToSignal + ApproachingBuffer > 0) &&
          ((LeftApproachingVehDistanceToSignal + ApproachingBuffer) <
           (pLeftApproachingVeh->StableVelocity() + pLeftApproachingVeh->StableAcc()) *
               Critical_Gap_LeftTurn)) {  // 如果对向车为直行，而且到停止线的距离小于它速度
                                          // 和CriticalGap的乘积；OpposingVeh是对向来车中距离停车线最近的一辆
        // SlowingDownDec = Deceleration(distance + WaitingBuffer_RightTurn, m_objectData.velocity, 0);
        // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance+WaitingBuffer_RightTurn距离
        // 在信号灯停止线后WaitingBuffer_RightTurn米处停下来
        // SlowingDownDec = Deceleration(distance + 0.5*m_objectData.length, m_objectData.velocity, 0);
        // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance距离，在信号灯停止线处停下来
        // SlowingDownDec = Deceleration(distance , m_objectData.velocity, 0);
        // //逐渐减速到0,以固定的减速度减速,即本车在距离停止线Distance时开始减速，通过Distance距离，在信号灯停止线处停下来
        SlowingDownDec = Deceleration(
            distance2signal + RightTurnWaitingBuffer, StableVelocity(), 0,
            Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速,即本车在距离右转待转区
                                  // // Distance+RightTurnWaitingBuffer时开始减速，通过
                                  // // Distancee+RightTurnWaitingBuffer距离，在信号灯后RightTurnWaitingBuffer处停下来
      }
    }
  }
  return fmin(acceleration, SlowingDownDec);
}

TAD_AI_VehicleElement::txFloat TAD_AI_VehicleElement::CheckSignLight(
    const txSysId signalId, const txFloat distance, const SIGN_LIGHT_COLOR_TYPE lightType) TX_NOEXCEPT {
  /*TX_MARK("distance这个数值，如果已经跨过了停止线，则是车头到线的负数距离，即，如果是负数，则证明已经过线了"); */
  txFloat acceleration = mKinetics.m_acceleration;
  txFloat SlowingDownDec = MAX_SPEED();
  const txBool b20201212_trick = true;

  if ((distance >= 0.0) && (distance < SignalReactionGap()) &&  // 1230 HD
      (_plus_(SIGN_LIGHT_COLOR_TYPE::eRed) == lightType) &&
      (b20201212_trick || CallFail(IsTurnRightAtNextIntersection()))) {
    SlowingDownDec = Deceleration(distance, StableVelocity(), 0.5,
                                  Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速,在信号灯停止线前0.5米处停下来
    LogInfo << TX_VARS(Id()) << TX_VARS(distance) << TX_VARS(lightType) << TX_VARS(SlowingDownDec)
            << TX_VARS(acceleration) << TX_VARS(signalId);
  }
  // if ((distance < SignalReactionGap) && (lightType == SIGN_LIGHT_COLOR_YELLOW) && (m_nextCrossRoadPathPlaneDirection
  // != TRAFFIC_VEHICLE_CROSS_ROAD_RIGHT_SIDE))//黄灯时候的反应
  if ((distance < SignalReactionGap()) && (_plus_(SIGN_LIGHT_COLOR_TYPE::eYellow) == lightType) &&
      (b20201212_trick || CallFail(IsTurnRightAtNextIntersection()))) {  // 黄灯时候的反应
    if ((StableVelocity() * 2) <
        distance) {  // 如果车辆以当前速度在两秒钟内行驶的距离比这个当前与信号灯之间的距离要短，则减速
      SlowingDownDec =
          Deceleration(distance, StableVelocity(), 0.5, Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速
    } else if (((-0.5 * StableVelocity() * StableVelocity()) / Max_Comfort_Deceleration()) > distance) {
      TX_MARK("//v1t+(1/2)at2//t=(v2-v1)/MaxA//以最大舒适加速度仍然无法停住，则选择不减速 ");
      SlowingDownDec = mKinetics.m_acceleration + 0.5;  // 稍微加速闯过去,只是借用SlowingDownDec这个变量名
    } else if (m_aggress < Yielding_Aggressive_Threshold()) {
      /*TX_MARK("不属于上述两种情形，则取决于车辆的aggressiveness, aggressive越大，越不愿意减速停车");*/
      SlowingDownDec =
          Deceleration(distance, StableVelocity(), 0.5, Max_Deceleration());  // 逐渐减速到0,以固定的减速度减速
    }
  }

  acceleration = fmin(acceleration, SlowingDownDec);  // 取跟车计算的加速度和上面加速度的最小值来进行减速

  // ////////////////////1230 HD/////////////////////////////////////////////////////

  Base::IVehicleElementPtr pOpposingVeh = std::get<0>(OpposeSideVeh_NextCross());
  Base::IVehicleElementPtr pLeftApproachingVeh = std::get<0>(LeftSideVeh_NextCross());
  // ////////////////////////////////////////////
  // char szText1[512];
  // sprintf(szText1, "OwnDistance: %lf", distance);
  // MessageBoxA(nullptr, szText1, "", 0);
  // ////////////////////////////////////////////
  // if (fabs(distance - 999999)<1.0)//此时车已经彻底离开直路部分，进入路口的link部分
  // 999999是在TrafficVehcicleData.h里面定义的MAX_DISTANCE
  // {
  // distance = 0.0;//则置0，仅用WaitingBuffer_LeftTurn和WaitingBuffer_RightTurn来考虑
  // }

  acceleration = TurnLeft_vs_GoStraight(pOpposingVeh, distance, acceleration); /*TX_MARK("左转让直行");*/

  acceleration = TurnRight_vs_TurnLeft_GoStraight(pOpposingVeh, pLeftApproachingVeh, distance,
                                                  acceleration); /*TX_MARK("右转让左转和从左侧道路驶来的直行车")*/

  acceleration = fmin(acceleration, SlowingDownDec);

  return acceleration;
}
#endif /*__TX_Mark__("check signal")*/

Base::txFloat TAD_AI_VehicleElement::Curvature2decc(const Base::txFloat curv,
                                                    const Base::txFloat cur_acc) const TX_NOEXCEPT {
  if (curv < 1e-3) {
    LOG_IF(INFO, FLAGS_LogLevel_Curve2Acc)
        << "[Curve2Acc][straight]" << TX_VARS(Id()) << TX_VARS(GetCurrentLaneInfo()) << TX_VARS(cur_acc);
    return cur_acc;
  } else {
    // const Base::txFloat retV = -1.0 * (FLAGS_curve2acc_a * curv + FLAGS_curve2acc_b) + (cur_acc > 0 ? 0 : cur_acc);
    const Base::txFloat retV = -1.0 * (FLAGS_curve2acc_a * curv + FLAGS_curve2acc_b) +
                               ((curv > 1e-2 && GetVelocity() > FLAGS_Speed_Max_OnCurve && cur_acc > 0) ? 0 : cur_acc);
    LOG_IF(INFO, FLAGS_LogLevel_Curve2Acc)
        << "[Curve2Acc][curve]" << TX_VARS(Id()) << TX_VARS(GetCurrentLaneInfo()) << TX_VARS(curv) << TX_VARS(retV);
    return retV;
  }
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::Update(TimeParamManager const &timeMgr) TX_NOEXCEPT {
  if (CallSucc(IsAlive()) && CallFail(IsStop())) {
    mKinetics.m_lateral_acceleration = 0.0;
#if __Consistency__
    std::ostringstream oss;
#endif /*__Consistency__*/
    TX_MARK("Stop forcibly based on the abnormal time");
    if (Math::isZero(StableVelocity()) && (mKinetics.m_velocity_max > 0)) {
      m_stop_second += timeMgr.RelativeTime();
      m_stop_on_junction_second =
          StableLaneInfo().isOnLaneLink ? (m_stop_on_junction_second + timeMgr.RelativeTime()) : 0.0;
      if ((m_stop_on_junction_second >= FLAGS_stop_on_junction_duration) || (m_stop_second >= FLAGS_stop_duration)) {
        LogWarn << "[KillByStop]" << TX_VARS(Id())
                << TX_VARS_NAME(front_id,
                                ((NonNull_Pointer(FollowFrontPtr()) ? (FollowFrontPtr()->ConsistencyId()) : (-1))));
        Kill();
      }
    } else {
      m_stop_on_junction_second = 0.0;
      m_stop_second = 0.0;
    }
#if __Consistency__
    oss << TX_VARS_NAME(stop_second, m_stop_second) << TX_VARS_NAME(junction_second, m_stop_on_junction_second);
#endif /*__Consistency__*/
    if (FLAGS_LogLevel_AI_EnvPercption || FLAGS_LogLevel_Surrounding_Info) {
      LOG(INFO) << "[Surrounding]" << timeMgr << TX_VARS(Id()) << TX_VARS_NAME(IsOnLaneLink, IsOnLaneLink())
                << StableGeomCenter().ToWGS84() << m_NearestObject.Str() << TX_VARS(StableLaneInfo())
                << TX_VARS(DistanceAlongCurve()) << TX_VARS(mLocation.LaneOffset())
                << TX_VARS_NAME(front_id,
                                ((NonNull_Pointer(FollowFrontPtr()) ? (FollowFrontPtr()->ConsistencyId()) : (-1))));
    }
#if __Consistency__
    oss << m_NearestObject.Str() << TX_VARS(StableLaneInfo()) << TX_VARS(DistanceAlongCurve())
        << TX_VARS(mLocation.LaneOffset())
        << TX_VARS_NAME(front_id, ((NonNull_Pointer(FollowFrontPtr()) ? (FollowFrontPtr()->ConsistencyId()) : (-1))));
    ConsistencyAppend(oss.str());
    oss.str("");
#endif /*__Consistency__*/

    CheckWayPoint();
    UpdateNextCrossDirection();
    mKinetics.m_acceleration = DrivingFollow(timeMgr);

    // if (CallFail(CheckPedestrianWalking(timeMgr.RelativeTime(), mKinetics.m_LateralVelocity)))
    txFloat last_lateral_velocity = mKinetics.m_LateralVelocity;
    CheckPedestrianWalking(timeMgr.RelativeTime(), mKinetics.m_LateralVelocity);
    CheckJunctionYieldConflict();
    {
      mKinetics.m_LateralVelocity = DrivingChangeLane(timeMgr);
#if __Consistency__
      oss << TX_VARS_NAME(LateralVelocity, mKinetics.m_LateralVelocity);
#endif /*__Consistency__*/
      ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(mKinetics.m_LateralVelocity);
      if (ExitHighWayType::eDec == m_Need_To_Exit) {
        const txFloat Dec_a =
            -1 * Max_Deceleration() * pow(m_DistanceToExit / FLAGS_Mandatory_Aware_Distance, 0.6) + Max_Deceleration();
        mKinetics.m_acceleration = fmin(Dec_a, mKinetics.m_acceleration);
      } else if (ExitHighWayType::eAcc == m_Need_To_Exit) {
        mKinetics.m_acceleration += 1.0;
      }
#if ___Consistency__
      oss << TX_VARS_NAME(acceleration, mKinetics.m_acceleration);
#endif /*__Consistency__*/
       // 静态障碍物太近不换道，速度为0时换道，配置一个小的加速度
#if __TX_Mark__("chengdu2")
      if (IsInTurnRight() && NonNull_Pointer(FollowFrontPtr()) &&
          _plus_(Base::Enums::DrivingStatus::stop_crash) == FollowFrontPtr()->StableDriving_Status()) {
        mKinetics.m_acceleration = 3.0;
        // LOG(INFO) << "[acc+5][success]" << TX_VARS(Id());
      } else if (IsInTurnRight() && NonNull_Pointer(FollowFrontPtr()) && (FollowFrontPtr()->StableVelocity() > 0.0) &&
                 (-2 == StableLaneInfo().onLaneUid.laneId) && StableVelocity() < 3.0) {
        mKinetics.m_acceleration = 2.0;
        // LOG(INFO) << "[acc+5][success]" << TX_VARS(Id());
      } else {
        // LOG(INFO) << "[acc+5][failure]" << TX_VARS(Id()) << TX_COND(NonNull_Pointer(FollowFrontPtr()));
      }
#endif
#if __Consistency__
      oss << TX_VARS_NAME(acceleration, mKinetics.m_acceleration);
#endif                                /*__Consistency__*/
#if 1                                 /* Curvature2decc on LaneLink*/
      if (CallSucc(IsOnLaneLink())) { /*TADSIM-137*/
        const auto &curLocInfo = mLocation.LaneLocInfo();
        auto linkPtr =
            HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(curLocInfo.onLinkFromLaneUid, curLocInfo.onLinkToLaneUid);
        if (NonNull_Pointer(linkPtr)) {
          const Base::txFloat dist_s = DistanceAlongCurve();
          const auto &CurvatureVec = linkPtr->getCurvature();
          for (const auto &Curvature : CurvatureVec) {
            if (dist_s < (Curvature.m_starts + Curvature.m_length)) {
              if (mKinetics.m_velocity > (FLAGS_curve2acc_active_factor * mKinetics.raw_velocity_max)) {
                LOG_IF(INFO, FLAGS_LogLevel_Curve2Acc)
                    << "[Curve2Acc][active] " << TX_VARS(Id()) << TX_VARS(mKinetics.m_velocity)
                    << TX_VARS(mKinetics.raw_velocity_max);
                mKinetics.m_acceleration = Curvature2decc(std::fabs(Curvature.m_curvature), mKinetics.m_acceleration);
              } else {
                LOG_IF(INFO, FLAGS_LogLevel_Curve2Acc)
                    << "[Curve2Acc][un-active] " << TX_VARS(Id()) << TX_VARS(mKinetics.m_velocity)
                    << TX_VARS(mKinetics.raw_velocity_max);
              }
              break;
            }
          }
        }
      }
#endif
    }
    if (NonNull_Pointer(GetCurTargetSignLightPtr())) {
      const txFloat signal_dist = GetCurDistanceToTargetSignLight();
      const auto signal_color = GetCurTargetSignLightPtr()->GetCurrentSignLightColorType();
      const txSysId signalId = GetCurTargetSignLightPtr()->Id();
      LogInfo << TX_VARS(Id()) << "[signal] " << TX_VARS(signalId) << TX_VARS(Id()) << "[signal] " << TX_VARS(signalId)
              << TX_VARS(signal_dist) << TX_VARS_NAME(Color, __enum2lpsz__(SIGN_LIGHT_COLOR_TYPE, signal_color));
#if __Consistency__
      oss << TX_VARS(signal_dist) << TX_VARS_NAME(follow_dist, FollowFrontDistance()) << TX_VARS(signalId)
          << TX_VARS_NAME(Color, __enum2lpsz__(SIGN_LIGHT_COLOR_TYPE, signal_color));
#endif /*__Consistency__*/
      // if ( (NonNull_Pointer(FollowFrontPtr()) && signal_dist < FollowFrontDistance()) || signal_dist <
      // Signal_Reaction_Gap())
      if (signal_dist < FollowFrontDistance()) {
        mKinetics.m_acceleration = CheckSignLight(signalId, signal_dist, signal_color);
#if __Consistency__
        oss << TX_VARS_NAME(SignalReactionGap, SignalReactionGap())
            << TX_VARS_NAME(acceleration, mKinetics.m_acceleration);
#endif /*__Consistency__*/
        LOG_IF(WARNING, FLAGS_LogLevel_Surrounding_Info || FLAGS_LogLevel_Signal)
            << TX_VARS(Id()) << "[signal] " << TX_VARS(mKinetics.m_acceleration) << TX_VARS(signal_dist)
            << TX_VARS(FollowFrontDistance()) << TX_VARS(GetLength());
      }
    }
    mKinetics.m_lateral_acceleration = (mKinetics.m_LateralVelocity - last_lateral_velocity) / timeMgr.RelativeTime();
    LogInfo << TX_VARS(mKinetics.m_LateralVelocity) << TX_VARS(last_lateral_velocity)
            << TX_VARS(mKinetics.m_lateral_acceleration);

    TX_MARK("Low-speed drift");
    if (Math::isZero(GetVelocity(), 1.31) && IsInTurn_Start()) {
      mKinetics.m_acceleration = FLAGS_StaticLaneChange_Acc;
    }

    mKinetics.m_velocity = StableVelocity() + mKinetics.m_acceleration * timeMgr.RelativeTime();
    LogInfo << TX_VARS(Id()) << "[signal] " << TX_VARS(mKinetics.m_velocity);
#if __Consistency__
    oss << TX_VARS_NAME(velocity, mKinetics.m_velocity);
    ConsistencyAppend(oss.str());
#endif /*__Consistency__*/
#if 0
        if (GetVelocity() <= 0.0) {
            mKinetics.m_velocity = 0.0;
            Compute_Displacement(timeMgr);
            mLocation.vPos() = ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(),
                                                   mLocation.vLaneDir(), mLocation.LaneOffset());
        }  // else
#endif
    {
      mKinetics.m_velocity = GetVelocity() < 0.0 ? 0.0 : mKinetics.m_velocity;
      // mKinetics.LimitVelocity();
      if (MoveStraightOnS(timeMgr)) {
        // mKinetics.LimitVelocity();
        ComputeRealAcc(timeMgr);
        TX_MARK("move to Component::Location, pImpl->NormalTargetRotateAngle();");
        LaneChangePostOp(timeMgr);
        mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
        return true;
      } else {
        StopVehicle();
        LogWarn << TX_VARS_NAME(vehicle_id, Id()) << " MoveStraightOnS failure. stop!";
        return false;
      }
    }
    return true;
  } else {
    /*LogWarn << "vehicle is not update. " << TX_VARS(Id())
        << TX_COND_NAME(IsAlive, IsAlive())
        << TX_COND_NAME(IsStop, IsStop());*/
    return (IsStop());
  }
}

Base::txBool TAD_AI_VehicleElement::IsUnderLaneKeep() const TX_NOEXCEPT {
  if (_plus_(VehicleMoveLaneState::eStraight) == GetSwitchLaneState() && CallFail(mLaneKeepMgr.Expired())) {
    return true;
  } else {
    return false;
  }
}

// Determine whether the current position has reached the end
void TAD_AI_VehicleElement::CheckWayPoint() TX_NOEXCEPT {
  if (CallSucc(mRouteAI.CheckReachable(mLocation.LaneLocInfo()))) {
    RouteLogInfo << "The vehicle can reach the destination." << TX_VARS_NAME(VehicleId, Id());
    mRouteAI.SetTrendOff();
  } else {
    RouteLogInfo << "The vehicle cannot reach the destination." << TX_VARS_NAME(VehicleId, Id());
    if (_plus_(VehicleMoveLaneState::eStraight) == GetSwitchLaneState()) {
      if (mLaneKeepMgr.Expired()) {
        RouteLogInfo << "lane keep is expired." << TX_VARS_NAME(VehicleId, Id());
        mRouteAI.SetTrendOn();
      } else {
        RouteLogInfo << "vehicle need lane keep." << TX_VARS_NAME(VehicleId, Id());
        mRouteAI.SetTrendOff();
      }
    } else {
      RouteLogInfo << ("The vehicle is in lane change.") << TX_VARS_NAME(VehicleId, Id());
      mRouteAI.SetTrendOff();
    }
  }
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::Release() TX_NOEXCEPT {
  mLifeCycle.SetEnd();
  return true;
}

Base::txBool TAD_AI_VehicleElement::FillingSpatialQuery() TX_NOEXCEPT {
  ParentClass::FillingSpatialQuery();
  return true;
}

sim_msg::Car *TAD_AI_VehicleElement::FillingElement(txFloat const timeStamp, sim_msg::Car *pSimVehicle) TX_NOEXCEPT {
  pSimVehicle = ParentClass::FillingElement(timeStamp, pSimVehicle);
#if USE_VehicleKinectInfo
  if (NonNull_Pointer(pSimVehicle)) {
    pSimVehicle->set_show_abs_acc(GetAcc());
    pSimVehicle->set_show_abs_velocity(GetVelocity());
#  if 1
    pSimVehicle->set_show_relative_velocity(show_relative_velocity_vertical());
    pSimVehicle->set_show_relative_velocity_horizontal(show_relative_velocity_horizontal());
    pSimVehicle->set_show_relative_dist_vertical(show_relative_dist_vertical());
    pSimVehicle->set_show_relative_dist_horizontal(show_relative_dist_horizontal());
    pSimVehicle->set_show_relative_acc(show_relative_acc());
    pSimVehicle->set_show_relative_acc_horizontal(show_relative_acc_horizontal());
#  else
    {
      if (HasRoute()) {
        pSimVehicle->set_show_relative_velocity(GetCurRouteGroupId());
        pSimVehicle->set_show_relative_velocity_horizontal(SubRouteId());
      } else {
        pSimVehicle->set_show_relative_velocity(-1);
        pSimVehicle->set_show_relative_velocity_horizontal(-1);
      }
    }
    {
      if (NonNull_Pointer(FollowFrontPtr())) {
        pSimVehicle->set_show_relative_dist_vertical(FollowFrontPtr()->Id());
        pSimVehicle->set_show_relative_dist_horizontal(FollowFrontDistance());
      } else {
        pSimVehicle->set_show_relative_dist_vertical(-1);
        pSimVehicle->set_show_relative_dist_horizontal(-1);
      }
    }
#  endif
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << TX_VARS(Id()) << TX_VARS(show_abs_acc()) << TX_VARS(show_abs_velocity())
        << TX_VARS(show_relative_velocity_horizontal()) << TX_VARS(show_relative_velocity_vertical())
        << TX_VARS(show_relative_dist_horizontal()) << TX_VARS(show_relative_dist_vertical());
  }
#endif /*USE_VehicleKinectInfo*/
  return pSimVehicle;
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::FillingElement(Base::TimeParamManager const &timeMgr,
                                                                    sim_msg::Traffic &trafficData) TX_NOEXCEPT {
  return ParentClass::FillingElement(timeMgr, trafficData);
}

// The veh can send trajectory
TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::FillingTrajectory(Base::TimeParamManager const &timeMgr,
                                                                       sim_msg::Trajectory &refTraj) TX_NOEXCEPT {
  refTraj.Clear();
  mPlanningTrajectoryPtr = nullptr;
  SaveStableState();
  if (IsAlive()) {
    HashedLaneInfoOrthogonalListPtr start_hashed_node_ptr = Weak2SharedPtr(std::get<1>(m_HashedRoadInfoCache));
    if (IsInTurnLeft()) {
      const HashedLaneInfo leftHashedLaneInfo = mLocation.GetLeftHashedLaneInfo(Id());
      start_hashed_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(leftHashedLaneInfo);
    } else if (IsInTurnRight()) {
      const HashedLaneInfo rightHashedLaneInfo = mLocation.GetRightHashedLaneInfo(Id());
      start_hashed_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(rightHashedLaneInfo);
    }

    if (NonNull_Pointer(start_hashed_node_ptr)) {
      const Base::txFloat reverse_base = DistanceAlongCurve() - start_hashed_node_ptr->GetSelfHashedInfo().StartScope();
      Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
      const Base::txFloat traj_length = cur_velocity * FLAGS_vehicle_trajectory_duration_s + 10.0;
      const Base::txFloat real_search_dist = reverse_base + traj_length;

      std::list<HashedLaneInfoOrthogonalListPtr> search_result_by_level;
      static const Base::txBool bCheckReachable = true;
      static const Base::txBool bDoNotCheckReachable = false;
      if (CallFail(SearchOrthogonalListFrontByLevel(mRouteAI.IsValid(), start_hashed_node_ptr, real_search_dist, 0,
                                                    search_result_by_level))) {
        search_result_by_level.clear();
        SearchOrthogonalListFrontByLevel(bDoNotCheckReachable, start_hashed_node_ptr, real_search_dist, 0,
                                         search_result_by_level);
        VehicleTrajInfo << "[trajectory][using_random_select]";
      } else {
        VehicleTrajInfo << "[trajectory][using_check_reachable]";
      }

      VehicleTrajInfo << "[trajectory]" << TX_VARS(reverse_base) << TX_VARS(traj_length) << TX_VARS(real_search_dist)
                      << TX_VARS(search_result_by_level.size());
      if (FLAGS_LogLevel_Vehicle_Traj) {
        Base::txInt idx = 0;
        for (const auto &ref : search_result_by_level) {
          // LOG(INFO) << "[trajectory]" << TX_VARS(idx) << (ref->GetSelfHashedInfo());
          ++idx;
        }
      }

      std::vector<Base::TrajectorySamplingNode> vec_sampling_traj_pts;
      // vec_sampling_traj_pts.emplace_back(
      //     TrajectorySamplingNode(GetCurrentLaneInfo() /*current lane info*/, DistanceAlongCurve()));
      vec_sampling_traj_pts.emplace_back(
          Base::TrajectorySamplingNode(start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo() /*current lane info*/,
                                       DistanceAlongCurve(), LaneOffset()));

      if (CallSucc(IsInLaneChange())) {
        for (const auto &refTrajOrthogonalListPtr : search_result_by_level) {
          const auto &cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();
          vec_sampling_traj_pts.emplace_back(
              Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                           cur_level_HashNodeInfo.EndScope(), LaneOffset()));
        }
      } else {
        const auto &start_lane_info = start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo();

        for (const auto &refTrajOrthogonalListPtr : search_result_by_level) {
          const auto &cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();

          if (CallSucc(start_lane_info == cur_level_HashNodeInfo.LaneInfo())) {
            if (DistanceAlongCurve() < cur_level_HashNodeInfo.EndScope() &&
                cur_level_HashNodeInfo.StartScope() < DistanceAlongCurve()) {
              const Base::txFloat real_length = cur_level_HashNodeInfo.EndScope() - DistanceAlongCurve();

              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               DistanceAlongCurve() + 0.3 * real_length, LaneOffset()));

              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               DistanceAlongCurve() + 0.6 * real_length, LaneOffset()));

              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
            } else {
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
            }
          } else {
            const Base::txFloat real_length = cur_level_HashNodeInfo.RealLength();
            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.StartScope() + 0.35 * real_length, LaneOffset()));

            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.StartScope() + 0.7 * real_length, LaneOffset()));

            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.EndScope(), LaneOffset()));
          }
        }
      }

      for (auto itr = std::begin(vec_sampling_traj_pts); itr != vec_sampling_traj_pts.end();) {
        if (itr->ComputeSamplingPoint()) {
          ++itr;
        } else {
          itr = vec_sampling_traj_pts.erase(itr);
        }
      }
#if 1
      // if (IsOnLane() /*&& CallSucc(IsInLaneChange())*/)
      {
        for (auto itr = (std::begin(vec_sampling_traj_pts) + 1); itr != vec_sampling_traj_pts.end();) {
          const Base::txFloat dist_with_first_pt =
              Coord::txENU::EuclideanDistanceLocal_2D(vec_sampling_traj_pts[0].SamplingPt(), itr->SamplingPt());

          if (dist_with_first_pt < 5.0) {
            itr = vec_sampling_traj_pts.erase(itr);
          } else {
            break;
          }
        }
      }
#endif
      if (FLAGS_LogLevel_Vehicle_Traj) {
        for (const auto &ref : vec_sampling_traj_pts) {
          LOG(INFO) << "[trajectory]" << TX_VARS_NAME(SamplingNode, ref.Str());
        }
      }
      const Base::txFloat ego_alt = Altitude();
      if (vec_sampling_traj_pts.size() > 1) {
        TrafficFlow::CentripetalCatMull::control_path_node_vec ref_control_path;
        {
          /*first node real position*/
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = GeomCenter().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        for (Base::txSize idx = 1; idx < vec_sampling_traj_pts.size(); ++idx) {
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = vec_sampling_traj_pts[idx].SamplingPt().ToWGS84().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        mPlanningTrajectoryPtr = std::make_shared<TrafficFlow::CentripetalCatMull>(ref_control_path);
        if (NonNull_Pointer(mPlanningTrajectoryPtr)) {
          const Base::txFloat traj_real_length = mPlanningTrajectoryPtr->GetLength();
          Coord::txWGS84 end_wgs84;
          end_wgs84.FromENU(mPlanningTrajectoryPtr->GetEndPt());
#if 1
          refTraj.set_a(GetAcc());
          VehicleTrajInfo << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());
          Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
          const Base::txFloat cur_acc = GetAcc();
          Base::txFloat cur_s = 0.0;
          for (Base::txFloat start_s = 0.0; start_s <= FLAGS_vehicle_trajectory_duration_s;
               start_s += timeMgr.RelativeTime()) {
            if (cur_s < traj_real_length && cur_s < (cur_velocity * FLAGS_vehicle_trajectory_duration_s)) {
              sim_msg::TrajectoryPoint *new_traj_point_ptr = refTraj.add_point();
              new_traj_point_ptr->set_z(ego_alt);
              new_traj_point_ptr->set_t(start_s + timeMgr.AbsTime());
              new_traj_point_ptr->set_v(cur_velocity);
              new_traj_point_ptr->set_a(GetAcc());
              new_traj_point_ptr->set_s(cur_s);

              /*1. update velocity*/
              cur_velocity += cur_acc * timeMgr.RelativeTime();
              if (cur_velocity >= mKinetics.m_velocity_max) {
                cur_velocity = mKinetics.m_velocity_max;
                // cur_acc = 0.0;
              }
              if (cur_velocity < 0.0) {
                cur_velocity = 0.0;
                // cur_acc = 0.0;
              }

              Coord::txWGS84 cur_pt_wgs84;
              cur_pt_wgs84.FromENU(mPlanningTrajectoryPtr->GetLocalPos(new_traj_point_ptr->s()));
              new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
              new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(mPlanningTrajectoryPtr->GetLaneDir(new_traj_point_ptr->s()))
                      .GetRadian();
              new_traj_point_ptr->set_theta(_headingRadian);

              /*2. update s*/
              cur_s += cur_velocity * timeMgr.RelativeTime();
            } else {
              break;
            }
          }
          VehicleTrajInfo << TX_VARS(cur_s) << TX_VARS(cur_velocity) << TX_VARS(cur_acc)
                          << TX_VARS(timeMgr.RelativeTime());
#else
          for (Base::txSize idx = 0; idx < vec_ms_time_traj_point.size(); ++idx) {
            auto &ref_traj_sample = vec_ms_time_traj_point[idx];
            auto point_ptr = std::get<1>(ref_traj_sample);
            if ((point_ptr->s()) < traj_real_length) {
              Coord::txWGS84 cur_pt_wgs84;
              cur_pt_wgs84.FromENU(laneGeomPtr->GetLocalPos(point_ptr->s()));
              point_ptr->set_x(cur_pt_wgs84.Lon());
              point_ptr->set_y(cur_pt_wgs84.Lat());

              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(point_ptr->s())).GetRadian();
              point_ptr->set_theta(_headingRadian);
            } else {
              point_ptr->set_a(0.0);
              point_ptr->set_v(0.0);
              point_ptr->set_s(traj_real_length);
              point_ptr->set_x(end_wgs84.Lon());
              point_ptr->set_y(end_wgs84.Lat());

              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(traj_real_length)).GetRadian();
              point_ptr->set_theta(_headingRadian);
              // LogWarn << TX_VARS(_headingRadian);
            }
          }
#endif
        } else {
          LOG(FATAL) << "mPlanningTrajectoryPtr is nullptr.";
          return false;
        }
      } else if (1 == vec_sampling_traj_pts.size()) {
        sim_msg::TrajectoryPoint *new_traj_point_ptr = refTraj.add_point();
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(timeMgr.AbsTime());
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(0.0);
        new_traj_point_ptr->set_s(0.0);

        Coord::txWGS84 cur_pt_wgs84 = vec_sampling_traj_pts.front().SamplingPt().ToWGS84();
        new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
        new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
        const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
        new_traj_point_ptr->set_theta(_headingRadian);

        sim_msg::TrajectoryPoint *new_clone_traj_point_ptr = refTraj.add_point();
        new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
      } else {
#if 0
        const auto& refPos = GetPosition();
        for (Base::txSize idx = 0; idx < vec_ms_time_traj_point.size(); ++idx) {
          auto point_ptr = std::get<1>(vec_ms_time_traj_point[idx]);
          point_ptr->set_a(0.0);
          point_ptr->set_v(0.0);
          point_ptr->set_s(0.0);
          point_ptr->set_x(refPos.Lon());
          point_ptr->set_y(refPos.Lat());
          const Base::txFloat _headingRadian = GetHeadingWithAngle().GetRadian();
          point_ptr->set_theta(_headingRadian);
        }
#endif
      }
    }
    return true;
  } else {
    return false;
  }
}

TAD_AI_VehicleElement::txBool TAD_AI_VehicleElement::CheckStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  if (CallFail(IsStart())) {
    if (/*timeMgr.RelativeTime() > 0.0 &&*/
        timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      mLifeCycle.SetStart();
      SM::txAITrafficState::SetStart();
      SM::txAITrafficState::SetLaneKeep();
      OnStart(timeMgr);
    }
  }
  return IsStart();
}

TAD_AI_VehicleElement::VehicleMoveLaneState TAD_AI_VehicleElement::GetSwitchLaneState() const TX_NOEXCEPT {
  if (IsInLaneChange()) {
    if (IsInTurnLeft()) {
      return VehicleMoveLaneState::eLeft;
    } else if (IsInTurnRight()) {
      return VehicleMoveLaneState::eRight;
    } else {
      LOG(FATAL) << "State machine error.";
      return VehicleMoveLaneState::eStraight;
    }
  } else {
    return VehicleMoveLaneState::eStraight;
  }
}

#if USE_VehicleKinectInfo

TAD_AI_VehicleElement::KineticsUtilInfo_t TAD_AI_VehicleElement::GetKineticsUtilInfo() const TX_NOEXCEPT {
  return m_self_KineticsUtilInfo;
}

Base::txBool TAD_AI_VehicleElement::ComputeKinetics(Base::TimeParamManager const &timeMgr,
                                                    const KineticsUtilInfo_t &_objInfo) TX_NOEXCEPT {
  m_show_abs_velocity = GetVelocity();
  m_show_abs_acc = GetAcc();
  if (timeMgr.RelativeTime() > 0.0 && CallSucc(_objInfo.m_isValid) /*&&
                                      CallFail(_objInfo._vecPolygon.empty()) &&
                                      CallFail(GetPolygon().empty())*/
  ) {
    const Base::txVec2 _adc_center = _objInfo._ElementGeomCenter.ENU2D();
    /*const Base::txVec2 _last_adc_center = _objInfo._LastElementPos.GetLocalPos_2D();*/
    const Base::txVec2 _obs_center = GeomCenter().ToENU().ENU2D();
    /*const Base::txVec2 _last_obs_center = pImpl->m_elemRoadInfo.lastPos.GetLocalPos_2D();*/
    const Base::txMat2 &tranMatInv = _objInfo._TransMatInv;

    const Base::txVec2 local_adc_center(0.0, 0.0);
    /*const Base::txVec2 local_last_adc_center = (_last_adc_center - _adc_center).transpose() * tranMatInv;*/
    const Base::txVec2 &local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
    /*const Base::txVec2 local_last_obs_center = (_last_obs_center - _adc_center).transpose() * tranMatInv;*/

    m_show_relative_dist_horizontal = local_obs_center.x() /** -1.0*/;
    m_show_relative_dist_vertical = local_obs_center.y();
    m_show_abs_dist_elevation = Altitude();
    m_show_relative_dist_elevation = show_abs_dist_elevation() - _objInfo._ElementGeomCenter.GetWGS84().Alt();

    // const Base::txVec2 local_adc_velocity = (local_adc_center - local_last_adc_center) / timeMgr.relativeTime;
    Base::txVec2 local_relative_velocity;
    if (Math::isNotZero(m_local_last_obs_center.x()) && Math::isNotZero(m_local_last_obs_center.y())) {
      local_relative_velocity = (local_obs_center - m_local_last_obs_center) / timeMgr.RelativeTime();
    } else {
      local_relative_velocity.setZero();
    }

    // const Base::txVec2 local_relative_velocity = (local_obs_velocity - local_adc_velocity);
    m_show_relative_velocity_horizontal = local_relative_velocity.x() * -1.0;
    m_show_relative_velocity_vertical = local_relative_velocity.y();

    Base::txVec2 local_relative_acc = (local_relative_velocity - m_local_last_obs_velocity) / timeMgr.RelativeTime();
    m_show_relative_acc = local_relative_acc.x();
    m_show_relative_acc_horizontal = local_relative_acc.y();

    // update cache
    m_local_last_obs_center = local_obs_center;
    m_local_last_obs_velocity = local_relative_velocity;
  } else {
    m_show_relative_dist_horizontal = 0.0;
    m_show_relative_dist_vertical = 0.0;
    m_show_relative_velocity_horizontal = 0.0;
    m_show_relative_velocity_vertical = 0.0;
    m_show_relative_acc = 0.0;
    m_show_relative_acc_horizontal = 0.0;
  }
  return true;
}

#endif /*USE_VehicleKinectInfo*/

#if __TX_Mark__("IEnvPerception")

void TAD_AI_VehicleElement::ConfigureFocusTypes() TX_NOEXCEPT {
  m_vehicle_type_filter_set.insert(ElementType::TAD_Vehicle);
  m_vehicle_type_filter_set.insert(ElementType::TAD_Ego);
  m_pedestrian_type_filter_set.insert(ElementType::TAD_Pedestrian);
}

void TAD_AI_VehicleElement::ExecuteEnvPerception(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
// compute SUDOKU_GRID. The veh is decorated with SUDOKU_GRID.
#  if USE_SUDOKU_GRID
  Compute_Region_Clockwise_Close(StableGeomCenter(), StableLaneDir(), GetHeadingWithAngle());
#  endif /*USE_SUDOKU_GRID*/
  SearchSurroundingVehicle(timeMgr);
  Search_Follow_Front(timeMgr);
  SearchSurroundingPedestrian(timeMgr);
  SearchSurroundingObstacle(timeMgr);
  SearchNextCrossVehicle(timeMgr);
}

Base::txInt SearchScopeX(const std::array<Base::txFloat, 4> &axis, const Base::txFloat val) TX_NOEXCEPT {
  Base::txInt idx = -1;
  for (Base::txInt i = 0; i < 4 && (val >= axis[i]); ++i, ++idx) {
  }
  return idx;
}

Base::txInt SearchScopeY(const std::array<Base::txFloat, 4> &axis, const Base::txFloat val) TX_NOEXCEPT {
  Base::txInt idx = -1;
  for (Base::txInt i = 0; i < 4 && (val < axis[i]); ++i, ++idx) {
  }
  return idx;
}
#  if USE_HashedRoadNetwork
#    if 0
Base::txBool TAD_AI_VehicleElement::SearchOrthogonalListFront(
    const Base::txSysId elemSysId, const Base::txSysId elemId, const txFloat elemLength,
    const IEnvPerception_TAD_AI_Hashed::OrthogonalListSearchCmd& cmd,
    Base::txSurroundVehiclefo& res_gap_vehPtr) TX_NOEXCEPT {
    Base::txFloat& minDist = std::get<_DistIdx_>(res_gap_vehPtr);
    Base::IVehicleElementPtr& front_nearest_veh_ptr = std::get<_ElementIdx_>(res_gap_vehPtr);
    VehicleContainer register_vehicles;
    cmd.curNodePtr->GetRegisterVehicles(register_vehicles);
    for (const auto& pair_id_vehPtr : register_vehicles) {
        const auto& obj_veh_ptr = pair_id_vehPtr.second;
        if (elemSysId == obj_veh_ptr->SysId()) {
            continue;
        }
        const auto& refHashNode = obj_veh_ptr->StableHashedLaneInfo();
        const Base::txFloat dist_s = refHashNode.S_in_Node() - cmd.original_s_in_node;
        const Base::txFloat real_dist = FLAGS_DETECT_OBJECT_DISTANCE - (cmd.valid_dist_along_original - dist_s);
        /*LOG_IF(INFO, 54 == elemId) << TX_VARS(elemId) << TX_VARS(obj_veh_ptr->Id())
            << TX_VARS(refHashNode.LaneInfo()) << TX_VARS(dist_s) << TX_VARS(real_dist)
            << TX_VARS(cmd.valid_dist_along_original);*/
        if (dist_s >= 0.0 &&
            dist_s < cmd.valid_dist_along_original &&
            real_dist < minDist) {
            minDist = real_dist - (obj_veh_ptr->GetLength() + elemLength) * 0.5;
            front_nearest_veh_ptr = obj_veh_ptr;
        }
    }
    return NonNull_Pointer(front_nearest_veh_ptr);
}
#    endif

Base::txBool TAD_AI_VehicleElement::SearchOrthogonalListBack(
    const IEnvPerception_TAD_AI_Hashed::OrthogonalListSearchCmd &cmd,
    txSurroundVehiclefo &res_gap_vehPtr) const TX_NOEXCEPT {
  Base::txFloat &minDist = std::get<0>(res_gap_vehPtr);
  auto &back_nearest_veh_weak_ptr = std::get<_ElementIdx_>(res_gap_vehPtr);
  minDist = 1000.0;
  back_nearest_veh_weak_ptr.reset();
  using VehicleContainer = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::VehicleContainer;
  VehicleContainer register_vehicles;
  cmd.curNodePtr->GetRegisterVehicles(register_vehicles);
  for (const auto &pair_id_vehPtr : register_vehicles) {
    const auto &obj_veh_ptr = pair_id_vehPtr.second;
    const auto &refHashNode = obj_veh_ptr->StableHashedLaneInfo();
    const Base::txFloat dist_s = cmd.original_s_in_node - refHashNode.S_in_Node();
    TX_MARK("back direction");
    if (dist_s >= 0.0 && dist_s <= cmd.valid_dist_along_original && dist_s < minDist) {
      minDist = dist_s;
      back_nearest_veh_weak_ptr = obj_veh_ptr;
    }
  }
  return WeakIsValid(back_nearest_veh_weak_ptr); /*NonNull_Pointer(back_nearest_veh_ptr);*/
}

Base::Info_NearestObject TAD_AI_VehicleElement::SearchSurroundingVehicle_HLane(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  Base::Info_NearestObject ret_NearestObject = Info_NearestObject(TX_MARK("must clear last frame result."));
  const Base::txVec3 &vehicleLaneDir = StableLaneDir();
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txSysId pVehicleId = Id();
  const Base::txVec3 &vPos = StableGeomCenter().ENU();
  const Base::txVec2 vPos2d = StableGeomCenter().ENU2D();
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t &selfLaneInfo = StableLaneInfo();

  const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();
  UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
  VehicleContainer surrounding_vehicles;
  HdMap::HashedRoadCacheConCurrent::QuerySurroundingVehicles(selfStableHashedLaneInfo, surrounding_vehicles);
#    if __FocusVehicle__
  FocusVehicleContainer focus_vehicles_map = GetFocusVehicles();
  if (_NonEmpty_(focus_vehicles_map)) {
    for (auto pair_itr : focus_vehicles_map) {
      const Base::txSysId focusSysId = pair_itr.first;
      Base::ITrafficElementPtr elemPtr = Weak2SharedPtr(pair_itr.second);
      Base::IVehicleElementPtr focus_elem_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(elemPtr);
      if (NonNull_Pointer(focus_elem_ptr)) {
        /*
        void RegisterVehicle(Base::IVehicleElementPtr _veh_ptr) TX_NOEXCEPT
        {
            tbb::mutex::scoped_lock lock(tbbMutex_vehicles);
            m_map_vehicles[_veh_ptr->SysId()] = _veh_ptr;
        }
        */
        surrounding_vehicles[/* it is not id */ focus_elem_ptr->SysId()] = focus_elem_ptr;
        EnvPerceptionInfo << "[vehilce_rsv_focus]" << TX_VARS(timeMgr.PassTime()) << TX_VARS(Id())
                          << TX_VARS(focus_elem_ptr->SysId());
      }
    }
  }
#    endif /*__FocusVehicle__*/
  if (surrounding_vehicles.size() > 1) {
    const std::array<Base::txVec3, Info_NearestObject::N_Dir> &vTargetPos = ComputeTargetPos(vPos, vehicleLaneDir);

    /*std::ostringstream oss, oss_valid;
    oss << TX_VARS(Id()) << " surround : ";
    oss_valid << TX_VARS(Id()) << " surround : ";*/
    for (const auto &pair_sysId_vehiclePtr : surrounding_vehicles) {
      // oss << (pair_sysId_vehiclePtr.second->Id()) << ", ";
      if (NonNull_Pointer(pair_sysId_vehiclePtr.second) && pair_sysId_vehiclePtr.first != pVehicleSysId) {
        const Base::IVehicleElementPtr &other_vehicle_ptr = pair_sysId_vehiclePtr.second;
        const Base::txVec3 &otherVehicleLaneDir = other_vehicle_ptr->StableLaneDir();
        if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(vehicleLaneDir), Utils::Vec3_Vec2(otherVehicleLaneDir)))) {
          const Base::txVec2 other_vpos_2d = other_vehicle_ptr->StableGeomCenter().ENU2D();
          const Base::txVec2 local_coord_other = RelativePosition(vPos2d, other_vpos_2d, TransMatInv());

          const Base::txInt x_idx = SearchScopeX(m_local_coord_x_axis, local_coord_other.x());
          const Base::txInt y_idx = SearchScopeY(m_local_coord_y_axis, local_coord_other.y());

          if (x_idx >= 0 && x_idx <= 2 && y_idx >= 0 && y_idx <= 2 && (1 != x_idx && 1 != y_idx)) {
            const auto &dir_name = m_xy2jdir_without_serialize[x_idx][y_idx];
            const txInt j_dir = dir_name._to_integral();
            // oss_valid << "[ " << (pair_sysId_vehiclePtr.second->Id()) << ": " << dir_name._to_string() << "], ";
            const Base::txVec3 &vTraget = vTargetPos[j_dir];
            const Base::txFloat curGap = (other_vehicle_ptr->GetLength() + selfVehicleLength) * 0.5;
            const Base::txFloat curDistance =
                ((Base::txVec2(vTraget.x(), vTraget.y())) - (other_vpos_2d)).norm() - curGap;

            if (curDistance < ret_NearestObject.dist(j_dir) && curDistance > 0.0) {
              ret_NearestObject.dist(j_dir) = curDistance;
              ret_NearestObject.setElemPtr(j_dir, other_vehicle_ptr);
              if (_plus_(Base::Enums::NearestElementDirection::eFront) == dir_name && curDistance < 0.0) {
                LOG(INFO) << "[FRONT_FAILURE] " << TX_VARS(Id()) << TX_VARS(other_vehicle_ptr->Id())
                          << TX_VARS(Utils::ToString(vPos2d))
                          << TX_VARS_NAME(self_headingAngle, Utils::GetLaneAngleFromVectorOnENU(vehicleLaneDir))
                          << TX_VARS(Utils::ToString(other_vpos_2d)) << TX_VARS(Utils::ToString(local_coord_other))
                          << TX_VARS(x_idx) << TX_VARS(y_idx);
              }
            }
          }
        }
      }
    }
    // LOG(WARNING) << oss.str() << std::endl << oss_valid.str();
  } else {
    if (1 == surrounding_vehicles.size() && NonNull_Pointer(surrounding_vehicles.begin()->second)) {
      if (pVehicleSysId != (surrounding_vehicles.begin()->second)->SysId()) {
        LOG(WARNING) << "[surrounding_vehicles] do not contain self." << TX_VARS(selfStableHashedLaneInfo)
                     << TX_VARS(Id()) << TX_VARS_NAME(otherId, (surrounding_vehicles.begin()->second)->Id());
      } else {
        /*normal, no cars around*/
      }
    } else {
      /*std::list< HashedLaneInfo> refRelativeHashedLanes;
      HdMap::HashedRoadCacheConCurrent::QueryRelatedHashedPt(selfStableHashedLaneInfo, refRelativeHashedLanes);
      std::ostringstream oss;
      for (const auto& refHashLaneInfo : refRelativeHashedLanes) {
          oss << "{" << refHashLaneInfo << "},";
      }*/

      LOG(WARNING) << "[surrounding_vehicles] do not find self." << TX_VARS(Id())
                   << TX_VARS(selfStableHashedLaneInfo) /*<< ", surround : " << oss.str()*/;
    }
  }
  return ret_NearestObject;
}
#  endif /*USE_HashedRoadNetwork*/

#  if USE_SUDOKU_GRID

Base::Info_NearestObject TAD_AI_VehicleElement::SearchSurroundingVehicle_RTree(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  Base::Info_NearestObject ret_NearestObject = Info_NearestObject(TX_MARK("must clear last frame result."));
  const Base::txVec3 &vehicleLaneDir = StableLaneDir();
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txSysId pVehicleId = Id();
  const Base::txVec3 &vPos = StableGeomCenter().ENU();
  const Base::txVec2 vPos2d = Utils::Vec3_Vec2(vPos);
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t &selfLaneInfo = StableLaneInfo();

  const std::array<Base::txVec3, Info_NearestObject::N_Dir> &vTargetPos = ComputeTargetPos(vPos, vehicleLaneDir);
  for (Base::txSize j_dir = 0; j_dir < Info_NearestObject::N_Dir; ++j_dir) {
    std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
    resultElementPtrVec.reserve(20);
    const Base::txVec3 &vTraget = vTargetPos[j_dir];
#    if USE_SUDOKU_GRID
    std::vector<Base::txVec2> localPts_clockwise_close;
    Get_Region_Clockwise_Close_By_Enum(__idx2enum__(Base::Enums::NearestElementDirection, j_dir),
                                       localPts_clockwise_close);
    const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInAreaByType(
        pVehicleSysId, localPts_clockwise_close, m_vehicle_type_filter_set, resultElementPtrVec);
#    endif /*USE_SUDOKU_GRID*/

    if (bFind) {
      for (auto resElemBasePtr : resultElementPtrVec) {
        Base::IVehicleElementPtr resElemPtr = std::dynamic_pointer_cast<Base::IVehicleElement>(resElemBasePtr);
        const Base::txVec3 &otherVehicleLaneDir = resElemPtr->StableLaneDir();
        const Base::Info_Lane_t &otherLaneInfo = resElemPtr->StableLaneInfo();
        /*LOG_IF(WARNING, ((700070 == pVehicleId))) << TX_VARS(j_dir) <<
           Base::Enums::NearestElementDirection::_from_index(j_dir)._to_string()
                << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id());*/
        if (CallSucc(_plus_(VEHICLE_TYPE::undefined) == resElemPtr->VehicleType())) {
          Base::txFloat curDistance = 0.0;
          if (Geometry::CD2D::GJK::HitPolygons(resElemPtr->GetPolygon(), GetPolygon())) {
            curDistance = 0.0;
          } else {
            Geometry::CD2D::cdVec2 nouse_intersectPt1, nouse_intersectPt2;
            curDistance = Geometry::CD2D::smallest_dist_between_ego_and_obsVehicle(
                resElemPtr->GetPolygon(), GetPolygon(), nouse_intersectPt1, nouse_intersectPt2);
          }

          if (curDistance < ret_NearestObject.dist(j_dir)) {
            ret_NearestObject.dist(j_dir) = curDistance;
            ret_NearestObject.setElemPtr(j_dir, resElemPtr);
          }
        } else if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(vehicleLaneDir),
                                                  Utils::Vec3_Vec2(otherVehicleLaneDir)))) {
          if (Info_NearestObject::nFront == j_dir) {
            const Base::txVec2 nearestElementCenter2d = resElemPtr->StableGeomCenter().ENU2D();
            const Base::txVec2 local_coord_other = RelativePosition(vPos2d, nearestElementCenter2d, TransMatInv());
            if (local_coord_other.y() > 0.0) {
              const Base::txFloat curGap = (resElemPtr->GetLength() + selfVehicleLength) * 0.5;
              const Base::txFloat curDistance =
                  ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
              /*LOG_IF(WARNING, ((1 == pVehicleId) || (17 == pVehicleId)) && 0 == j_dir)
                  << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                  << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id()) << TX_VARS(curDistance)
                  << TX_VARS_NAME(self_vehicle_sysid, pVehicleSysId)
                  << TX_VARS_NAME(front_vehicle_sysid, resElemPtr->SysId());*/
              if (curDistance < ret_NearestObject.dist(j_dir)) {
                ret_NearestObject.dist(j_dir) = curDistance;
                ret_NearestObject.setElemPtr(j_dir, resElemPtr);
              }
#    if 0 /*20230516*/
  LogWarn << "[20230515]" << TX_VARS(Id()) << TX_VARS_NAME(vPos2d, Utils::ToString(vPos2d))
          << TX_VARS(resElemPtr->Id()) << TX_VARS_NAME(nearestElementCenter2d, Utils::ToString(nearestElementCenter2d))
          << TX_VARS_NAME(local_coord_other, Utils::ToString(local_coord_other));
#    endif
            }
          } else if (Info_NearestObject::nBack == j_dir) {
            const Base::txVec2 nearestElementCenter2d = resElemPtr->StableGeomCenter().ENU2D();
            const Base::txVec2 local_coord_other = RelativePosition(vPos2d, nearestElementCenter2d, TransMatInv());
            if (local_coord_other.y() < 0.0) {
              const Base::txFloat curGap = (resElemPtr->GetLength() + selfVehicleLength) * 0.5;
              const Base::txFloat curDistance =
                  ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
              /*LOG_IF(WARNING, ((1 == pVehicleId) || (17 == pVehicleId)) && 0 == j_dir)
                  << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                  << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id()) << TX_VARS(curDistance)
                  << TX_VARS_NAME(self_vehicle_sysid, pVehicleSysId)
                  << TX_VARS_NAME(front_vehicle_sysid, resElemPtr->SysId());*/
              if (curDistance < ret_NearestObject.dist(j_dir)) {
                ret_NearestObject.dist(j_dir) = curDistance;
                ret_NearestObject.setElemPtr(j_dir, resElemPtr);
              }
            }
          } else {
            const Base::txVec2 nearestElementCenter2d = resElemPtr->StableGeomCenter().ENU2D();
            const Base::txFloat curGap = (resElemPtr->GetLength() + selfVehicleLength) * 0.5;
            const Base::txFloat curDistance =
                ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
            /*LOG_IF(WARNING, ((1 == pVehicleId) || (17 == pVehicleId)) && 0 == j_dir)
                << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id()) << TX_VARS(curDistance)
                << TX_VARS_NAME(self_vehicle_sysid, pVehicleSysId)
                << TX_VARS_NAME(front_vehicle_sysid, resElemPtr->SysId());*/
            if (curDistance < ret_NearestObject.dist(j_dir)) {
              ret_NearestObject.dist(j_dir) = curDistance;
              ret_NearestObject.setElemPtr(j_dir, resElemPtr);
            }
          }
        }
      }
    } /*FindElementsInAreaByType*/
  }   /*for (Base::txSize j_dir = 0; j_dir < Base::OBJECT_TARGET_DIRECTION_COUNT; ++j_dir)*/
  return ret_NearestObject;
}

#  endif
void TAD_AI_VehicleElement::SearchSurroundingVehicle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
#  ifdef ON_CLOUD
  static constexpr Base::txBool is_on_cloud = true;
#  else
  static constexpr Base::txBool is_on_cloud = false;
#  endif
  const Base::txBool hitSampling = true || CheckSamplingPeriod();
  m_NearestObject.setValid(false);
  if (IsAlive() && (timeMgr.PassTime() > 0.0) && CallFail(IsUnderLaneKeep()) && IsOnLane() || CallFail(is_on_cloud)) {
    // last state : on_lanelink or lanekeep  or local
    if (m_last_is_on_lanelink || m_last_lanekeep || hitSampling || CallFail(is_on_cloud)) {
      EnvPerceptionInfo << "[EnvSamplingPeriod]" << timeMgr << TX_VARS(Id()) << TX_VARS(GetPerceptionPeriod())
                        << TX_COND(hitSampling) << TX_COND(m_last_is_on_lanelink) << TX_COND(m_last_lanekeep);
#  if ON_CLOUD
      m_NearestObject = SearchSurroundingVehicle_RTree(timeMgr);
#  elif ON_PREVIEW /*ON_CLOUD*/
      m_NearestObject = SearchSurroundingVehicle_HLane(timeMgr);
#  else
#    if _EraseRtree2d_
      // m_NearestObject = SearchSurroundingVehicle_HLane(timeMgr);
      m_NearestObject = SearchSurroundingVehicle_RTree(timeMgr);
#    else  /*_EraseRtree2d_*/
      m_NearestObject = SearchSurroundingVehicle_HLane(timeMgr);
#    endif /*_EraseRtree2d_*/
#  endif   /*ON_CLOUD*/
      m_NearestObject.setValid(true);
      m_NearestObject.SamplingTime() = timeMgr.PassTime();
#  if 1
      {
        static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftFront));
        Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = m_NearestObject.elemPtr(cDir);
        Base::txFloat FrontGap_TargetLane = m_NearestObject.dist(cDir);
        if (NonNull_Pointer(FrontVeh_TargetLane_Base) &&
            (_plus_(Base::ISimulationConsistency::DrivingStatus::stop_crash) ==
             FrontVeh_TargetLane_Base->StableDriving_Status())) {
          m_NearestObject.dist(cDir) = SafetyGap_Front() + 1.0;
        }
      }
      {
        static const Base::txInt cDir = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightFront));
        Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = m_NearestObject.elemPtr(cDir);
        Base::txFloat FrontGap_TargetLane = m_NearestObject.dist(cDir);
        if (NonNull_Pointer(FrontVeh_TargetLane_Base) &&
            (_plus_(Base::ISimulationConsistency::DrivingStatus::stop_crash) ==
             FrontVeh_TargetLane_Base->StableDriving_Status())) {
          m_NearestObject.dist(cDir) = SafetyGap_Front() + 1.0;
        }
      }
#  endif
    } else {
      EnvPerceptionInfo << "[EnvSamplingPeriod]" << timeMgr << TX_VARS(Id()) << TX_VARS(GetPerceptionPeriod())
                        << TX_COND(hitSampling) << TX_COND(m_last_is_on_lanelink) << TX_COND(m_last_lanekeep)
                        << TX_COND(m_last_is_on_lanelink) << TX_COND(m_last_lanekeep);
    }
  } else {
    EnvPerceptionInfo << "[EnvSamplingPeriod]" << timeMgr << TX_VARS(Id()) << TX_COND(IsAlive())
                      << TX_COND((timeMgr.PassTime() > 0.0)) << TX_COND(CallFail(IsUnderLaneKeep()))
                      << TX_COND(IsOnLane());
  }
  m_last_is_on_lanelink = IsOnLaneLink();
  m_last_lanekeep = IsUnderLaneKeep();
  ChangeLaneLogInfo << TX_VARS(Id()) << TX_COND(ValidSampling(timeMgr));
}

void TAD_AI_VehicleElement::SearchNextCrossVehicle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT { return; }

void TAD_AI_VehicleElement::SearchSurroundingPedestrian(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  m_NearestDynamic = Base::Info_NearestDynamic();
#  if ON_CLOUD
  return;
#  endif /*ON_CLOUD*/

#  if ON_LOCAL
  const Base::txVec3 &vehicleLaneDir = StableLaneDir();
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txInt pVehicleId = Id();
  const Base::txVec3 &vPos = StableGeomCenter().ENU();
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t &selfLaneInfo = StableLaneInfo();

  static std::map<Base::Enums::PedestrianTargetDirectionType, Base::Enums::NearestElementDirection> map_pede2vehicle{
      {(_plus_(Base::Enums::PedestrianTargetDirectionType::eFrontCenter)),
       (_plus_(Base::Enums::NearestElementDirection::eFront))},
      {(_plus_(Base::Enums::PedestrianTargetDirectionType::eFrontLeft)),
       (_plus_(Base::Enums::NearestElementDirection::eLeftFront))},
      {(_plus_(Base::Enums::PedestrianTargetDirectionType::eFrontRight)),
       (_plus_(Base::Enums::NearestElementDirection::eRightFront))}};
  for (Base::txSize j_dir = 2; j_dir < 5; ++j_dir) {
    const auto curPedeDirType = __int2enum__(
        PedestrianTargetDirectionType, j_dir); /*Base::Enums::PedestrianTargetDirectionType::_from_integral(j_dir);*/
    std::vector<Base::ITrafficElementPtr> result_pede_ptr_vec;
    result_pede_ptr_vec.reserve(20);
    const auto curVehicleDirType = map_pede2vehicle.at(curPedeDirType);
    /*const auto curVehicleDirInteger = curPedeDirType._to_integral();
    const Base::txVec3& vTraget = vTargetPos_by_vehicle[curVehicleDirInteger];*/
#    if USE_SUDOKU_GRID
    std::vector<Base::txVec2> localPts_clockwise_close;
    Get_Region_Clockwise_Close_By_Enum(curVehicleDirType, localPts_clockwise_close);
    const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInAreaByType(
        pVehicleSysId, localPts_clockwise_close, m_pedestrian_type_filter_set, result_pede_ptr_vec);
#    endif /*USE_SUDOKU_GRID*/

    if (bFind) {
      for (auto resElemBasePtr : result_pede_ptr_vec) {
        {
          const Base::txVec2 nearestElementCenter2d = resElemBasePtr->StableGeomCenter().ENU2D();
          const Base::txFloat curGap = (resElemBasePtr->GetLength() + selfVehicleLength) * 0.5;
          const Base::txFloat curDistance =
              ((Base::txVec2(vPos.x(), vPos.y())) - (nearestElementCenter2d)).norm() - curGap;

          if (curDistance < m_NearestDynamic.dist(__enum2idx__(curPedeDirType))) {
            m_NearestDynamic.dist(__enum2idx__(curPedeDirType)) = curDistance;
            m_NearestDynamic.setElemPtr(__enum2idx__(curPedeDirType), resElemBasePtr);
          }
        }
      }
    } /*FindElementsInAreaByType*/
  }   /*for (Base::txSize j_dir = 0; j_dir < Base::OBJECT_TARGET_DIRECTION_COUNT; ++j_dir)*/
  LOG_IF(INFO, FLAGS_LogLevel_IDM_Front) << TX_VARS(Id()) << m_NearestDynamic.Str();
#  endif /*ON_LOCAL*/
}

void CheckEgoCorrect() {
#  if 0 /* Ego_Front_Special ego is searched repeated and is incorrect */
    {
        const Base::txVec3 &vehicleLaneDir = StableLaneDir();
        const Base::txSysId pVehicleSysId = SysId();
        const Base::txSysId pVehicleId = Id();
        const Base::txVec3 &vPos = StableGeomCenter().ENU();
        const Base::txVec2 vPos2d = Utils::Vec3_Vec2(vPos);
        const Base::txFloat selfVehicleLength = GetLength();
        const Base::txFloat selfVehicleWidth = GetWidth();

        std::vector<Base::ITrafficElementPtr> resultEgoPtrVec;
        const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInCircleByType(
            vPos2d, pVehicleSysId, FLAGS_EgoCircleSearchRadius,
            {ElementType::TAD_Ego}, resultEgoPtrVec);

        for (auto ego_ptr : resultEgoPtrVec) {
            if (NonNull_Pointer(ego_ptr)) {
                LOG_IF(INFO, FLAGS_LogLevel_Surrounding_Info) << "[EgoCircleSearchRadius][check]"
                                                              << TX_VARS_NAME(VehId, Id())
                                                              << TX_VARS_NAME(EgoId, ego_ptr->Id());
                const Base::txVec2 other_vpos_2d = ego_ptr->StableGeomCenter().ENU2D();
                const Base::txVec2 local_coord_other = RelativePosition(vPos2d, other_vpos_2d, TransMatInv());
                /*const Base::txInt x_idx = SearchScopeX(m_local_coord_x_axis, local_coord_other.x());
                const Base::txInt y_idx = SearchScopeY(m_local_coord_y_axis, local_coord_other.y());
                Base::txBool isFront = false;*/

                const Base::txBool isHalfPlaneFront = (local_coord_other.y() >= 0.0);
                if (isHalfPlaneFront && Math::isNotZero(local_coord_other.x())) {
                    Unit::txDegree retDegree;
                    retDegree.FromRadian(Math::Atan2(local_coord_other.y(), local_coord_other.x()));
                    if (retDegree.GetDegree() > 45.0 && retDegree.GetDegree() < 135.0) {
                        std::get<_DistIdx_>(DrivingFollow_Front()) = 0.0;
                        std::get<_ElementIdx_>(DrivingFollow_Front()) = ego_ptr;
                        LOG_IF(INFO, FLAGS_LogLevel_Surrounding_Info) << "[EgoCircleSearchRadius][hit]"
                                                                      << TX_VARS_NAME(VehId, Id())
                                                                      << TX_VARS_NAME(EgoId, ego_ptr->Id())
                                                                      << TX_VARS(retDegree.GetDegree())
                                                                      << (ego_ptr->StableGeomCenter())
                                                                      << (ego_ptr->GetCurrentLaneInfo());
                        break;
                    }
                }
            }
        }
    }
#  endif
}

void TAD_AI_VehicleElement::Search_Follow_Front(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  Search_Follow_Front_HLane(timeMgr);
  if (NonNull_Pointer(FollowFrontPtr()) &&
      _plus_(Base::Enums::ElementType::TAD_Ego) == FollowFrontPtr()->ConsistencyElementType()) {
    LogL2WEgoInfo << " [Ego Info] " << TX_VARS(Id()) << TX_VARS(timeMgr.PassTime())
                  << TX_VARS_NAME(frontId,
                                  ((NonNull_Pointer(FollowFrontPtr())) ? (FollowFrontPtr()->ConsistencyId()) : (-1)))
                  << TX_VARS(FollowFrontDistance()) << TX_VARS_NAME(front_v, FollowFrontPtr()->StableVelocity())
                  << TX_VARS_NAME(front_acc, FollowFrontPtr()->StableAcc());
  }
  // Search_Follow_Front_RTree(timeMgr);
}

void TAD_AI_VehicleElement::Search_Follow_Front_HLane_Meeting(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
#  if __Meeting__
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txSysId pVehicleId = Id();
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::txFloat selfVehicleWidth = GetWidth();
  const Base::txVec3 &vPos = StableGeomCenter().ENU();
  const Base::txVec2 vPos2d = Utils::Vec3_Vec2(vPos);
  const Base::Info_Lane_t &selfLaneInfo = StableLaneInfo();
  Coord::txENU meetingPt;

  if (CallSucc(HdMap::HashedRoadCacheConCurrent::IsInMeetingRegion(m_curHashedLaneInfo, meetingPt)) &&
      CallFail(IsSignalLightAhead(_plus_(Base::Enums::SIGN_LIGHT_COLOR_TYPE::eRed))) &&
      CallFail(IsSignalLightAhead(Base::Enums::SIGN_LIGHT_COLOR_TYPE::eYellow)) &&
      CallFail(Math::isZero(StableVelocity()))) {
    // LOG(WARNING) << TX_VARS(Id()) << TX_VARS(meetingPt.ENU2D().x()) << TX_VARS(meetingPt.ENU2D().y());
    const Base::txFloat self_inv_dist_to_meeting_square = (vPos2d - meetingPt.ENU2D()).squaredNorm();
    VehMeetingInfo << TX_VARS(Id()) << TX_VARS(m_curHashedLaneInfo) << ", IsInMeetingRegion.";
    TX_MARK("IsInMeetingRegion");
    Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::VehicleContainer meetingVehicles;
    HdMap::HashedRoadCacheConCurrent::QueryMeetingVehicles(m_curHashedLaneInfo, meetingVehicles);
    Base::txSurroundVehiclefo nearestOnMeeting;
    std::get<_DistIdx_>(nearestOnMeeting) = 999.0;
    if (meetingVehicles.size() > 1 /*self*/) {
      // std::ostringstream oss;
      for (const auto &pair_sysId_vehiclePtr : meetingVehicles) {
        if (NonNull_Pointer(pair_sysId_vehiclePtr.second) && (pair_sysId_vehiclePtr.first != pVehicleSysId) &&
            CallFail((pair_sysId_vehiclePtr.second->StableLaneInfo()) == selfLaneInfo)) {
          const Base::IVehicleElementPtr &other_vehicle_ptr = pair_sysId_vehiclePtr.second;
          if (Math::isZero(other_vehicle_ptr->StableVelocity()) ||
              (other_vehicle_ptr->StableFollowFrontId() == ConsistencyId()) &&
                  other_vehicle_ptr->ConsistencyId() > ConsistencyId()) {
            continue;
          }
          const Base::txVec2 other_vpos_2d = other_vehicle_ptr->StableGeomCenter().ENU2D();
          // const Base::txVec2 local_coord_other = RelativePosition(vPos2d, other_vpos_2d, _TransMatInv);
          // const Base::txInt x_idx = SearchScopeX(m_local_coord_x_axis, local_coord_other.x());
          // const Base::txInt y_idx = SearchScopeY(m_local_coord_y_axis, local_coord_other.y());
          //
          // if (x_idx >= 0 && x_idx <= 2 && y_idx >= 0 && y_idx <= 1/*ignore rear*/)
          {
            const Base::txFloat curGap = (other_vehicle_ptr->GetLength() + selfVehicleLength) * 0.5 * 2;
            const Base::txFloat curDistance = ((vPos2d) - (other_vpos_2d)).norm() - curGap;
            const Base::txFloat other_inv_dist_to_meeting_square = (other_vpos_2d - meetingPt.ENU2D()).squaredNorm();
            /*LOG(INFO) << "[meeting_crash_debug]" << TX_VARS(pVehicleId) << TX_VARS_NAME(Id,
               pair_sysId_vehiclePtr.second->ConsistencyId())
                << TX_VARS(StablePosition()) << TX_VARS(other_vehicle_ptr->StablePosition())
                << TX_VARS(meetingPt) << TX_VARS(self_inv_dist_to_meeting_square) <<
               TX_VARS(other_inv_dist_to_meeting_square);*/
            // oss << "{" << TX_VARS_NAME(Id,
            // pair_sysId_vehiclePtr.second->ConsistencyId()) << TX_VARS(curDistance);
            if (curDistance < std::get<_DistIdx_>(nearestOnMeeting) &&
                /*(curDistance < (2.0 * FLAGS_default_lane_width)) &&*/
                (/*1*/
                 /*2*/ (other_inv_dist_to_meeting_square < self_inv_dist_to_meeting_square) ||
                 /*3*/ (((other_vehicle_ptr->Id()) < pVehicleId) &&
                        Math::isEqual(other_inv_dist_to_meeting_square, self_inv_dist_to_meeting_square)))) {
              /*LOG(INFO) << "[meeting_crash_debug]" << TX_VARS(pVehicleId) << TX_VARS_NAME(pOtherId,
                 other_vehicle_ptr->Id())
                  << TX_VARS(curDistance) << TX_VARS(std::get<_DistIdx_>(nearestOnMeeting))
                  << TX_VARS(other_inv_dist_to_meeting_square) << TX_VARS(self_inv_dist_to_meeting_square)
                  << TX_VARS(mLocation.tracker()->getLaneGeomInfo()->LaneLocInfo());*/
              Base::txFloat yaw = 0.0, distancePedal = 0.0, distanceCurve = 0.0;
              const auto &other_enu2d = other_vehicle_ptr->StableGeomCenter().ENU2D();
              mLocation.tracker()->getLaneGeomInfo()->xy2sl(other_enu2d, distanceCurve, distancePedal);
              const txFloat nudgeWidth =
                  0.5 * selfVehicleWidth + 0.5 * (other_vehicle_ptr->GetWidth()) + FLAGS_default_lane_width;
              // oss << TX_VARS(distancePedal) << TX_VARS(nudgeWidth)
              // << TX_VARS(selfVehicleWidth) << TX_VARS((other_vehicle_ptr->GetWidth()));
              // if (fabs(distancePedal) <= nudgeWidth)
              // LOG(WARNING) << TX_VARS(Id()) << TX_VARS_NAME(ignore, pOtherVehicle->Id()) <<
              // TX_VARS(fabs(distancePedal));
              std::get<_DistIdx_>(nearestOnMeeting) = curDistance;
              std::get<_ElementIdx_>(nearestOnMeeting) = other_vehicle_ptr;
            } else {
              /*LOG(INFO) << "[meeting_crash_debug][failure]" << TX_VARS(pVehicleId) << TX_VARS_NAME(pOtherId,
                 other_vehicle_ptr->Id())
                  << TX_VARS(curDistance) << TX_VARS(std::get<_DistIdx_>(nearestOnMeeting))
                  << TX_VARS(other_inv_dist_to_meeting_square) << TX_VARS(self_inv_dist_to_meeting_square);*/
            }
            // oss << "},";
          }
        }
      }
      // LOG(INFO) << "[meeting_crash_debug]" << TX_VARS(pVehicleId) << " surrounding : "
      // << oss.str();
    } else {
      VehMeetingInfo << " surrounding empty.";
    }
    if (WeakIsValid(std::get<_ElementIdx_>(nearestOnMeeting))) {
      /*find meeting vehicle*/
      if (NonNull_Pointer(FollowFrontPtr())) {
        if (std::get<_DistIdx_>(nearestOnMeeting) < FollowFrontDistance()) {
          auto log_ptr = Weak2SharedPtr(std::get<_ElementIdx_>(nearestOnMeeting));
          VehMeetingInfo << ", hit meeting " << TX_VARS_NAME(Id, (log_ptr->ConsistencyId()))
                         << TX_VARS_NAME(dist, std::get<_DistIdx_>(nearestOnMeeting));
          // txPAUSE;
          DrivingFollow_Front() = nearestOnMeeting;
        }
      } else {
        auto log_ptr = Weak2SharedPtr(std::get<_ElementIdx_>(nearestOnMeeting));
        VehMeetingInfo << ", hit meeting " << TX_VARS_NAME(Id, (log_ptr->ConsistencyId()))
                       << TX_VARS_NAME(dist, std::get<_DistIdx_>(nearestOnMeeting));
        // txPAUSE;
        DrivingFollow_Front() = nearestOnMeeting;
      }
    }
  }
#  endif /*__Meeting__*/
}

void TAD_AI_VehicleElement::Search_Follow_Front_HLane(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
#  define _DistToIntersectPt_ (0)
#  define _InvertSOnLink_ (1)
#  define _LinkUId_ (2)
#  define _ElemPtr_ (3)
  std::map<Base::txSysId, std::tuple<Base::txFloat /*dist between intersection pt to straight vehicle*/,
                                     Base::txFloat /*invert dist on link*/, Base::Info_Lane_t /*link uid*/,
                                     Base::IVehicleElementPtr /*other vehicle ptr*/>>
      map_id2straightVehicleInfo;
  SetDrivingStatus(_plus_(Base::IDrivingStatus::DrivingStatus::unknown));
#  if __Consistency__
  std::ostringstream oss;
#  endif /*__Consistency__*/
  if (IsAlive() && timeMgr.PassTime() > 0.0) {
    /*m_follow_info*/
    Clear_Follow_Info();
    Base::SimulationConsistencyPtr ptr = nullptr;
    std::get<_ElementIdx_>(m_YieldConflict) = ptr;
    std::get<_DistIdx_>(m_YieldConflict) = 999.0;
    const Base::txVec3 &vehicleLaneDir = StableLaneDir();
    const Base::txSysId pVehicleSysId = SysId();
    const Base::txSysId pVehicleId = Id();
    const Base::txVec3 &vPos = StableGeomCenter().ENU();
    const Base::txVec2 vPos2d = Utils::Vec3_Vec2(vPos);
    const Base::txFloat selfVehicleLength = GetLength();
    const Base::txFloat yieldLength = 1.5 * selfVehicleLength;
    const Base::txFloat selfVehicleWidth = GetWidth();
    const Base::Info_Lane_t &selfLaneInfo = StableLaneInfo();
    const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();
    const Base::txFloat selfDegree = Utils::GetLaneAngleFromVectorOnENU(vehicleLaneDir).GetDegree();
    const Base::txFloat _zeroRadian = Utils::GetLaneAngleFromVectorOnENU(vehicleLaneDir).GetRadian();
    Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::Half_PI(), _zeroRadian + Unit::CircleRadian::Half_PI(),
                               _zeroRadian);
    const Base::txFloat selfInvertS = StableInvertDistanceAlongCurve();
#  if __Consistency__
    oss << TX_VARS(selfStableHashedLaneInfo) << TX_VARS(selfDegree) << TX_VARS(_zeroRadian) << TX_VARS(selfInvertS);
#  endif /*__Consistency__*/
    UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);

    /*FLAGS_DETECT_OBJECT_DISTANCE*/
    HashedLaneInfoOrthogonalListPtr center_node_ptr = Weak2SharedPtr(std::get<1>(m_HashedRoadInfoCache));
    if (NonNull_Pointer(center_node_ptr)) {
      HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), selfStableHashedLaneInfo,
                                                              center_node_ptr, DrivingFollow_Front(),
                                                              FLAGS_DETECT_OBJECT_DISTANCE);
#  if __Consistency__
      oss << TX_VARS_NAME(front_dist_1, FollowFrontDistance());
#  endif /*__Consistency__*/
      /*on lanelink */
      if (selfLaneInfo.IsOnLaneLink()) {
        HdMap::HadmapCacheConCurrent::lanelinkExpandVisionType::mapped_type res_expand_vision;
        HdMap::HadmapCacheConCurrent::CheckExpandVision(selfLaneInfo, res_expand_vision);
        VehicleContainer resultElementPtrMap;
        const HdMap::HashedRoadCacheConCurrent::HashedLaneInfoSet res_toRoad_locInfoSet =
            HdMap::HashedRoadCacheConCurrent::GetLaneLinksToRoad(selfLaneInfo.onLinkToLaneUid.roadId);
        // get lanelink toRoad veh id
        HdMap::HashedRoadCacheConCurrent::QueryRegisterVehicles(res_toRoad_locInfoSet, resultElementPtrMap);
#  if _EraseRtree2d_
        // get 20m area veh ptr
        std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
        const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInCircleByType(
            vPos2d, pVehicleSysId, 20.0, m_vehicle_type_filter_set, resultElementPtrVec);
        for (auto ptr : resultElementPtrVec) {
          Base::IVehicleElementPtr pOtherVehicle = std::dynamic_pointer_cast<Base::IVehicleElement>(ptr);
          if (NonNull_Pointer(pOtherVehicle)) {
            resultElementPtrMap[pOtherVehicle->SysId()] = pOtherVehicle;
          }
        }
#  endif
        if (NonNull_Pointer(mLocation.tracker()) && NonNull_Pointer(mLocation.tracker()->getLaneGeomInfo())) {
          auto self_polyline_traj_ptr = StableTrajectory();
          const HdMap::txLaneInfoInterface::LaneShape self_shape = mLocation.tracker()->getLaneGeomInfo()->GetShape();
          Base::txSurroundVehiclefo nearestOnLink;
          std::get<_DistIdx_>(nearestOnLink) = 999.0;
          for (auto &pair_id_OtherTrafficBase : resultElementPtrMap) {
            Base::IVehicleElementPtr pOtherVehicle = (pair_id_OtherTrafficBase).second;
            if (NonNull_Pointer(pOtherVehicle) && (pOtherVehicle->SysId() != pVehicleSysId) &&
                CallSucc(pOtherVehicle->IsOnLaneLink()) &&
                CallFail((pOtherVehicle->StableLaneInfo()) == selfLaneInfo)) {
              const Base::txSysId otherId = pOtherVehicle->Id();
              const Base::txFloat otherDegree =
                  Utils::GetLaneAngleFromVectorOnENU(pOtherVehicle->StableLaneDir()).GetDegree();
              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(pOtherVehicle->StableLaneDir()).GetRadian();
              const Base::txFloat wrap_headingRadian = circRad.Wrap(_headingRadian);
              const Base::txBool isSynchronicity = std::fabs(_zeroRadian - wrap_headingRadian) < (Math::HALF_PI / 3.0);
              const Base::txVec2 other_vpos_2d = pOtherVehicle->StableGeomCenter().ENU2D();
              const Base::txVec2 local_coord_other = RelativePosition(vPos2d, other_vpos_2d, TransMatInv());
              const Base::txInt x_idx = SearchScopeX(m_local_coord_x_axis, local_coord_other.x());
              const Base::txInt y_idx = SearchScopeY(m_local_coord_y_axis, local_coord_other.y());
              Base::txBool isFront = false;
              const Base::txBool isHalfPlaneFront = (local_coord_other.y() >= 0.0);
              if (x_idx >= 0 && x_idx <= 2 && y_idx >= 0 && y_idx <= 2) {
                const auto &dir_name = m_xy2jdir_without_serialize[x_idx][y_idx];
                isFront = (_plus_(Base::Enums::NearestElementDirection::eFront) == dir_name) ||
                          (_plus_(Base::Enums::NearestElementDirection::eLeftFront) == dir_name) ||
                          (_plus_(Base::Enums::NearestElementDirection::eRightFront) == dir_name);
              }
              const auto &otherLaneInfo = pOtherVehicle->StableLaneInfo();
              const txBool bSameDst = (otherLaneInfo.onLinkToLaneUid == selfLaneInfo.onLinkToLaneUid);
              const txBool bSameSrc = (otherLaneInfo.onLinkFromLaneUid == selfLaneInfo.onLinkFromLaneUid);
              const txBool bfromDiffRoad =
                  (otherLaneInfo.onLinkFromLaneUid.roadId != selfLaneInfo.onLinkFromLaneUid.roadId);
              const txBool bToSameRoad = (otherLaneInfo.onLinkToLaneUid.roadId == selfLaneInfo.onLinkToLaneUid.roadId);
              const txBool bExpandVision = _Contain_(res_expand_vision, (pOtherVehicle->StableLaneInfo()));
              auto obj_polyline_traj_ptr = pOtherVehicle->StableTrajectory();
              const txFloat otherInvertS = pOtherVehicle->StableInvertDistanceAlongCurve();
              JunctionInfo << TX_VARS_NAME(otherId, otherId) << TX_COND(isSynchronicity) << TX_COND(isFront)
                           << TX_COND(isHalfPlaneFront) << TX_VARS(std::get<_DistIdx_>(nearestOnLink))
                           << TX_COND(bSameDst) << TX_COND(bSameSrc) << TX_COND(bfromDiffRoad) << TX_COND(bToSameRoad)
                           << TX_COND(bExpandVision) << TX_VARS(selfInvertS) << TX_VARS(otherInvertS)
                           << TX_COND(NonNull_Pointer(self_polyline_traj_ptr))
                           << TX_COND(NonNull_Pointer(obj_polyline_traj_ptr)) << TX_VARS(x_idx) << TX_VARS(y_idx)
                           << TX_VARS(local_coord_other.x()) << TX_VARS(local_coord_other.y()) << TX_VARS(selfDegree)
                           << TX_VARS(otherDegree) << TX_VARS(_zeroRadian) << TX_VARS(wrap_headingRadian);
#  if __JunctionYield__
              if (/*isFront &&*/ /*isHalfPlaneFront &&*/ bfromDiffRoad &&
#    if 0
                                bToSameRoad &&
#    endif
                  NonNull_Pointer(self_polyline_traj_ptr) && NonNull_Pointer(obj_polyline_traj_ptr)) {
                JunctionInfo << TX_VARS_NAME(otherId, otherId) << " func 1";
                Geometry::Element::txGeomElementBase::txVertex intersectionPt;
                txFloat dist_on_s = 0.0;
                txFloat dist_on_obj = 0.0;
                if (CallFail(isSynchronicity)) {
                  if ((HdMap::txLaneInfoInterface::LaneShape::sLine != self_shape)) {
                    TX_MARK("check yield");
                    JunctionInfo << TX_VARS_NAME(otherId, otherId) << " check yield.";
                    if (CallSucc(self_polyline_traj_ptr->CheckIntersection(obj_polyline_traj_ptr, intersectionPt,
                                                                           dist_on_s, dist_on_obj))) {
                      map_id2straightVehicleInfo[otherId] =
                          std::make_tuple(dist_on_s - yieldLength, otherInvertS, otherLaneInfo, pOtherVehicle);
                      double cur_pass_t = dist_on_s / StableVelocity();
                      double other_pass_t = dist_on_obj / pOtherVehicle->StableVelocity();
                      Base::txFloat yaw = 0.0, distancePedal = 0.0, distanceCurve = 0.0;
                      // {
                      //     auto geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(otherLaneInfo);
                      //     if (NonNull_Pointer(geom_ptr))
                      //     {

                      //         const auto &other_gps = StableGeomCenter().ENU2D();
                      //         geom_ptr->xy2sl(other_gps, distanceCurve, distancePedal);
                      //         distancePedal = std::fabs(distancePedal);
                      //     }
                      // }
                      JunctionInfo << TX_VARS_NAME(otherId, otherId) << " traj intersection" << TX_VARS(dist_on_s)
                                   << TX_VARS((dist_on_s - yieldLength)) << TX_VARS(distancePedal)
                                   << TX_VARS(selfVehicleLength);
                      if (StableVelocity() < 1.0 && other_pass_t > 2.0 &&
                          dist_on_obj > pOtherVehicle->GetLength() * 1.5) {
                      } else if (dist_on_obj < pOtherVehicle->GetLength() * 1.5 ||
                                 (Math::isNotZero(pOtherVehicle->StableVelocity()) &&
                                  Math::isNotZero(StableVelocity()) && other_pass_t < cur_pass_t)) {
                        dist_on_s -= yieldLength;
                        if (dist_on_s < std::get<_DistIdx_>(nearestOnLink) /* && distancePedal > selfVehicleLength*/) {
                          // SetDrivingStatus(_plus_(Base::IDrivingStatus::DrivingStatus::junction_yield));
                          std::get<_DistIdx_>(nearestOnLink) = (dist_on_s);
                          std::get<_ElementIdx_>(nearestOnLink) = pOtherVehicle;

                          JunctionInfo << "[yield]" << TX_VARS_NAME(otherId, otherId)
                                       << TX_VARS_NAME(intersection_dist, dist_on_s);
                        }
                      }
                    }
                  } else {
                    TX_MARK("check front-yield-stop");
                    JunctionInfo << TX_VARS_NAME(otherId, otherId) << " check front-yield-stop.";
                    if ((_plus_(Base::IDrivingStatus::DrivingStatus::junction_yield) ==
                         pOtherVehicle->StableDriving_Status()) &&
                        CallSucc(self_polyline_traj_ptr->CheckIntersection(pOtherVehicle->StableLongitudinalSkeleton(),
                                                                           intersectionPt, dist_on_s, dist_on_obj))) {
                      JunctionInfo << TX_VARS_NAME(otherId, otherId) << " traj front-yield-stop" << TX_VARS(dist_on_s)
                                   << TX_VARS((dist_on_s - yieldLength));
                      dist_on_s -= yieldLength;
                      if (dist_on_s < std::get<_DistIdx_>(nearestOnLink)) {
                        std::get<_DistIdx_>(nearestOnLink) = (dist_on_s);
                        std::get<_ElementIdx_>(nearestOnLink) = pOtherVehicle;
                        JunctionInfo << "[invert_yield]" << TX_VARS_NAME(otherId, otherId)
                                     << TX_VARS_NAME(intersection_dist, dist_on_s);
                      }
                    }
                  }
                }
                if ((HdMap::txLaneInfoInterface::LaneShape::sLine == self_shape)) {
                  TX_MARK("check straight-yield-stop")
                  if (CallSucc(self_polyline_traj_ptr->CheckIntersection(obj_polyline_traj_ptr, intersectionPt,
                                                                         dist_on_s, dist_on_obj))) {
                    double cur_pass_t = dist_on_s / StableVelocity();
                    double other_pass_t = dist_on_obj / pOtherVehicle->StableVelocity();
                    if (StableVelocity() < 1.0 && other_pass_t > 2.0 &&
                        dist_on_obj > pOtherVehicle->GetLength() * 1.5) {
                    } else if ((dist_on_obj < pOtherVehicle->GetLength() * 1.5) ||
                               (Math::isNotZero(pOtherVehicle->StableVelocity()) && Math::isNotZero(StableVelocity()) &&
                                other_pass_t < cur_pass_t)) {
                      dist_on_s -= yieldLength;
                      if (dist_on_s < std::get<_DistIdx_>(m_YieldConflict)) {
                        std::get<_ElementIdx_>(m_YieldConflict) = pOtherVehicle;
                        std::get<_DistIdx_>(m_YieldConflict) = dist_on_s;
                        // LOG(ERROR) << TX_VARS(pVehicleId) << " yield: " << TX_VARS(otherId) << TX_VARS(dist_on_s) <<
                        // TX_VARS(dist_on_obj);
                      }
                    }
                  }
                }
                if (bSameDst && isHalfPlaneFront /*&& CallSucc(isSynchronicity)*/ && (otherInvertS) < selfInvertS) {
                  Base::txFloat yaw = 0.0, distancePedal = 0.0, distanceCurve = 0.0;
                  const auto &other_gps = pOtherVehicle->StableGeomCenter().ToWGS84();
                  mLocation.tracker()->getCurrentCurve()->xy2sl(other_gps.Lon(), other_gps.Lat(), distanceCurve,
                                                                distancePedal, yaw);
                  JunctionInfo << TX_VARS_NAME(otherId, otherId) << TX_VARS(DistanceAlongCurve())
                               << TX_VARS(distanceCurve);
                  if (distanceCurve > DistanceAlongCurve() &&
                      (pOtherVehicle->StableDriving_Status() !=
                       (_plus_(Base::IDrivingStatus::DrivingStatus::junction_yield)))) {
                    JunctionInfo << TX_VARS_NAME(otherId, otherId) << " func 3";
                    dist_on_s =
                        distanceCurve - DistanceAlongCurve() - (selfVehicleLength + (pOtherVehicle->GetLength()) * 0.5);
                    if (dist_on_s <
                        std::get<_DistIdx_>(
                            nearestOnLink) /* && (HdMap::txLaneInfoInterface::LaneShape::sLine != self_shape)*/) {
                      std::get<_DistIdx_>(nearestOnLink) = (dist_on_s);
                      std::get<_ElementIdx_>(nearestOnLink) = pOtherVehicle;

                      JunctionInfo << TX_VARS_NAME(otherId, otherId) << TX_VARS_NAME(intersection_dist, dist_on_s);
                    }
                  }
                }
              }
              if (CallSucc(bSameDst) || CallSucc(bSameSrc TX_MARK("from same link start, but different link end")) ||
                  CallSucc(bExpandVision)) {
                const txFloat distance_thresold =
                    (1.0 * FLAGS_default_lane_width); /* (bExpandVision) ? (3.0 * FLAGS_default_lane_width) :
                                                         (FLAGS_default_lane_width); */
                const Base::txVec2 nearestElementCenter2d = pOtherVehicle->StableGeomCenter().ENU2D();
                const Base::txFloat curGap = (pOtherVehicle->GetLength() + GetLength()) * 0.5;
                TX_MARK("SIM-6412");
                Base::txFloat curDistance = (((StableGeomCenter().ENU2D()) - (nearestElementCenter2d)).norm() - curGap);

                JunctionInfo << TX_VARS_NAME(otherId, otherId) << TX_VARS(curDistance) << TX_VARS(distance_thresold)
                             << TX_VARS(selfInvertS) << TX_VARS(otherInvertS) << TX_VARS(GetOnLaneLinkTimeStamp())
                             << TX_VARS(pOtherVehicle->GetOnLaneLinkTimeStamp());
                if (curDistance < std::get<_DistIdx_>(nearestOnLink) && isFront &&
                    ((curDistance < distance_thresold) || (otherInvertS < selfInvertS))) {
                  if ((curDistance < distance_thresold) &&
                      (pOtherVehicle->GetOnLaneLinkTimeStamp()) > GetOnLaneLinkTimeStamp()) {
                    TX_MARK("The current vehicle spends more time at the intersection than other vehicles");
                    TX_MARK("Ignore other vehicle, or other vehicle avoid current vehicle.");
                    JunctionInfo << " continue 1.";
                    continue;
                  } else if ((curDistance < distance_thresold) &&
                             Math::isEqual(pOtherVehicle->GetOnLaneLinkTimeStamp(), GetOnLaneLinkTimeStamp()) &&
                             (otherId) < Id()) {
                    JunctionInfo << " continue 2.";
                    continue;
                  } else {
                    TX_MARK("Put it last because of the large amount of calculation");
                    Base::txFloat yaw = 0.0, distancePedal = 0.0, distanceCurve = 0.0;
                    const auto &other_gps = pOtherVehicle->StableGeomCenter().ToWGS84();
                    mLocation.tracker()->getCurrentCurve()->xy2sl(other_gps.Lon(), other_gps.Lat(), distanceCurve,
                                                                  distancePedal, yaw);
                    const txFloat nudgeWidth = FLAGS_lanelink_nudge_delta + GetWidth(/*JIRA TADSIM-25*/);
                    if (fabs(distancePedal) > nudgeWidth) {
                      JunctionInfo << " continue 3." << TX_VARS(distancePedal) << TX_VARS(nudgeWidth);
                      continue;
                    }
                  }
                  std::get<_DistIdx_>(nearestOnLink) =
                      (/*bExpandVision && */ curDistance < FLAGS_default_lane_width * 1.5) ? (-1.0) : (curDistance);
                  std::get<_ElementIdx_>(nearestOnLink) = pOtherVehicle;
                }
              }
#  endif /*__JunctionYield__*/
            }
          }
          if (WeakIsValid(
                  std::get<_ElementIdx_>(nearestOnLink)) /*NonNull_Pointer(std::get<_ElementIdx_>(nearestOnLink))*/) {
            if (NonNull_Pointer(FollowFrontPtr())) {
              if (std::get<_DistIdx_>(nearestOnLink) < FollowFrontDistance()) {
                DrivingFollow_Front() = nearestOnLink;
              }
            } else {
              DrivingFollow_Front() = nearestOnLink;
            }
          }
        }
      } else if (selfLaneInfo.IsOnLane()) {
#  if __TX_Mark__("virtual city crash")
        if (CallSucc(IsInLaneChange())) {
          if (NonNull_Pointer(FollowFrontPtr()) &&
              ((_plus_(Base::Enums::DrivingStatus::stop_crash) == FollowFrontPtr()->StableDriving_Status()) ||
               (Math::isZero(FollowFrontPtr()->StableVelocity())))) {
            Clear_Follow_Info();
          }
        }
#  endif /*__TX_Mark__("virtual city crash")*/
        Search_Follow_Front_HLane_Meeting(timeMgr);

#  if __Consistency__
        oss << TX_VARS_NAME(front_dist_2, FollowFrontDistance());
#  endif /*__Consistency__*/
        if (IsInTurnLeft() && CallSucc(mLocation.tracker()->hasLeftLane())) {
          /*if (CallFail(mLocation.tracker()->hasLeftLane())) {
              LOG(WARNING) << "[cloud_debug][lane_change]" << TX_VARS(Id()) << TX_VARS(GetPosition()) <<
          TX_VARS(mLocation.LaneLocInfo())
                  << TX_VARS(m_LaneChangeTimeMgr) << TX_VARS(m_LaneAbortingTimeMgr);
          }*/
          if (IsInTurn_Ing()) {
            Clear_Follow_Info();
          }
          const HashedLaneInfo leftHashedLaneInfo = mLocation.GetLeftHashedLaneInfo(Id());
          const HashedLaneInfoOrthogonalListPtr &left_node_ptr =
              HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(leftHashedLaneInfo);
          Base::txSurroundVehiclefo leftFollowFront;
          std::get<_DistIdx_>(leftFollowFront) = 999.0;
          HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), leftHashedLaneInfo,
                                                                  left_node_ptr, leftFollowFront,
                                                                  FLAGS_DETECT_OBJECT_DISTANCE);
          txInt leftFollowFront_id = WeakIsValid(std::get<_ElementIdx_>(leftFollowFront))
                                         ? (Weak2SharedPtr(std::get<_ElementIdx_>(leftFollowFront)))->ConsistencyId()
                                         : -1;
          ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS_NAME(PassTime, timeMgr.PassTime())
                            << TX_VARS(leftHashedLaneInfo) << TX_VARS_NAME(leftFollowFront_id, leftFollowFront_id);

          if (WeakIsValid(std::get<_ElementIdx_>(leftFollowFront))
              /*NonNull_Pointer(std::get<_ElementIdx_>(leftFollowFront))*/) {
            if (NonNull_Pointer(FollowFrontPtr())) {
              if (std::get<_DistIdx_>(leftFollowFront) < FollowFrontDistance()) {
                DrivingFollow_Front() = leftFollowFront;
              }
            } else {
              DrivingFollow_Front() = leftFollowFront;
            }
          } else if (IsInTurn_Ing()) {
            DrivingFollow_Front() = leftFollowFront;
          }

#  if __Consistency__
          oss << TX_VARS_NAME(front_dist_3, FollowFrontDistance());
#  endif /*__Consistency__*/
        } else if (IsInTurnRight() && CallSucc(mLocation.tracker()->hasRightLane())) {
          /*if (CallFail(mLocation.tracker()->hasRightLane())) {
              LOG(WARNING) << "[cloud_debug][lane_change]" << TX_VARS(Id()) << TX_VARS(GetPosition()) <<
          TX_VARS(mLocation.LaneLocInfo())
                  << TX_VARS(m_LaneChangeTimeMgr) << TX_VARS(m_LaneAbortingTimeMgr);
          }*/
          if (IsInTurn_Ing()) {
            Clear_Follow_Info();
          }
          const HashedLaneInfo rightHashedLaneInfo = mLocation.GetRightHashedLaneInfo(Id());
          const HashedLaneInfoOrthogonalListPtr &right_node_ptr =
              HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(rightHashedLaneInfo);

          Base::txSurroundVehiclefo rightFollowFront;
          std::get<_DistIdx_>(rightFollowFront) = 999.0;
          HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), rightHashedLaneInfo,
                                                                  right_node_ptr, rightFollowFront,
                                                                  FLAGS_DETECT_OBJECT_DISTANCE);
          txInt rightFollowFront_id = WeakIsValid(std::get<_ElementIdx_>(rightFollowFront))
                                          ? (Weak2SharedPtr(std::get<_ElementIdx_>(rightFollowFront)))->ConsistencyId()
                                          : -1;
          ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS_NAME(PassTime, timeMgr.PassTime())
                            << TX_VARS(rightHashedLaneInfo) << TX_VARS_NAME(rightFollowFront_id, rightFollowFront_id);
          if (WeakIsValid(std::get<_ElementIdx_>(
                  rightFollowFront)) /*NonNull_Pointer(std::get<_ElementIdx_>(rightFollowFront))*/) {
            if (NonNull_Pointer(FollowFrontPtr())) {
              if (std::get<_DistIdx_>(rightFollowFront) < FollowFrontDistance()) {
                DrivingFollow_Front() = rightFollowFront;
              }
            } else {
              DrivingFollow_Front() = rightFollowFront;
            }
          } else if (IsInTurn_Ing()) {
            DrivingFollow_Front() = rightFollowFront;
          }
#  if __Consistency__
          oss << TX_VARS_NAME(front_dist_4, FollowFrontDistance());
#  endif /*__Consistency__*/
        } else if (CallSucc(IsLaneKeep())) {
          const auto matInv = TransMatInv();
#  if __Consistency__
          oss << TX_VARS_NAME(matInv00, matInv.coeffRef(0, 0)) << TX_VARS_NAME(matInv00, matInv.coeffRef(0, 1))
              << TX_VARS_NAME(matInv00, matInv.coeffRef(1, 0)) << TX_VARS_NAME(matInv00, matInv.coeffRef(1, 1));
#  endif /*__Consistency__*/
          if (m_NearestObject.IsValid()) {
            auto left_front_elem_ptr = m_NearestObject.elemPtr(Base::Info_NearestObject::nLeftFront);
            auto right_front_elem_ptr = m_NearestObject.elemPtr(Base::Info_NearestObject::nRightFront);
            auto front_elem_ptr = m_NearestObject.elemPtr(Base::Info_NearestObject::nFront);
            std::vector<Base::SimulationConsistencyPtr> front_elem_vec;
            front_elem_vec.reserve(3);
            if (NonNull_Pointer(left_front_elem_ptr)) {
              front_elem_vec.emplace_back(left_front_elem_ptr);
            }
            if (NonNull_Pointer(right_front_elem_ptr)) {
              front_elem_vec.emplace_back(right_front_elem_ptr);
            }
            if (NonNull_Pointer(front_elem_ptr)) {
              front_elem_vec.emplace_back(front_elem_ptr);
            }
            if (_NonEmpty_(front_elem_vec)) {
              for (auto elem_ptr : front_elem_vec) {
                const Base::txVec2 other_vpos_2d = elem_ptr->StableGeomCenter().ENU2D();
                const Base::txVec2 local_coord_other = RelativePosition(vPos2d, other_vpos_2d, TransMatInv());

                const Base::txInt x_idx = SearchScopeX(m_local_coord_x_axis, local_coord_other.x());
                const Base::txInt y_idx = SearchScopeY(m_local_coord_y_axis, local_coord_other.y());
#  if __Consistency__
                oss << TX_VARS_NAME(otherId, elem_ptr->ConsistencyId())
                    << TX_VARS_NAME(otherPos, Utils::ToString(other_vpos_2d)) << TX_VARS(x_idx) << TX_VARS(y_idx);
#  endif /*__Consistency__*/
                if (x_idx >= 0 && x_idx <= 2 && y_idx >= 0 && y_idx <= 2) {
                  const auto &dir_name = m_xy2jdir_without_serialize[x_idx][y_idx];
#  if __Consistency__
                  oss << TX_VARS_NAME(dir, dir_name._to_string());
#  endif /*__Consistency__*/
                  if (_plus_(Base::Enums::NearestElementDirection::eFront) == dir_name) {
                    Base::IVehicleElementPtr other_vehicle_ptr =
                        std::dynamic_pointer_cast<Base::IVehicleElement>(elem_ptr);
                    if (NonNull_Pointer(other_vehicle_ptr)) {
                      const Base::txFloat curGap = (other_vehicle_ptr->GetLength() + selfVehicleLength) * 0.5;
                      const Base::txFloat curDistance = ((vPos2d) - (other_vpos_2d)).norm() - curGap;
#  if __Consistency__
                      oss << TX_VARS(curDistance) << TX_VARS(FollowFrontDistance());
#  endif /*__Consistency__*/

                      if (curDistance < FollowFrontDistance()) {
                        /*LogWarn << "[20230515] " << TX_VARS(timeMgr.PassTime()) << TX_VARS(Id())<<
                         * TX_VARS_NAME(set_front_id, other_vehicle_ptr->Id()) << TX_VARS(curDistance);*/
                        DrivingFollow_Front() = std::make_tuple(curDistance, other_vehicle_ptr);
                      }
                    } /*if (NonNull_Pointer(other_vehicle_ptr))*/
                  }   /*if (_plus_(Base::Enums::NearestElementDirection::eFront) == dir_name)*/
                }     /*if (x_idx >= 0 && x_idx <= 2 && y_idx >= 0 && y_idx <= 2)*/
              }       /*for (auto elem_ptr : front_elem_vec)*/
            }         /*if (_NonEmpty_(front_elem_vec))*/
#  if __Consistency__
            oss << TX_VARS_NAME(front_dist_5, FollowFrontDistance());
#  endif    /*__Consistency__*/
          } /*if (m_NearestObject.IsValid())*/
        }
      }
    } else {
      LOG(WARNING) << "[Query_OrthogonalList][null] " << TX_VARS(Id()) << TX_VARS_NAME(PassTime, timeMgr.PassTime())
                   << TX_VARS(selfStableHashedLaneInfo);
    }
  }
#  if __TX_Mark__("20221213 vr")
  if (CallSucc(m_NearestObject.IsValid()) && NonNull_Pointer(std::get<0>(FrontVeh_CurrentLane())) &&
      CallFail(IsInTurn_Ing() && (IsInTurnLeft() && CallSucc(mLocation.tracker()->hasLeftLane())) ||
               (IsInTurnRight() && CallSucc(mLocation.tracker()->hasRightLane())))) {
    const Base::txFloat rtree_front_elem_dist = std::get<1>(FrontVeh_CurrentLane());
    if (rtree_front_elem_dist < FollowFrontDistance()) {
      Base::txSurroundVehiclefo rtree_front;
      std::get<_DistIdx_>(rtree_front) = rtree_front_elem_dist;
      std::get<_ElementIdx_>(rtree_front) = std::get<0>(FrontVeh_CurrentLane());
      Base::txSysId srcId = 999;
      Base::txFloat srcDist = -1.0;
      if (NonNull_Pointer(FollowFrontPtr())) {
        srcId = FollowFrontPtr()->ConsistencyId();
        srcDist = FollowFrontDistance();
      }
      DrivingFollow_Front() = rtree_front;
      LOG_IF(INFO, FLAGS_LogLevel_Surrounding_Info || ((Id() == FLAGS_DebugVehId)))
          << "[20221213_vr]" << TX_VARS(Id()) << "replace idm front by rtree front "
          << TX_VARS_NAME(rtree_front_id, std::get<0>(FrontVeh_CurrentLane())->ConsistencyId()) << TX_VARS(srcId)
          << TX_VARS(rtree_front_elem_dist) << TX_VARS(srcDist);
    }
#    if __Consistency__
    oss << TX_VARS_NAME(front_dist_6, FollowFrontDistance());
#    endif /*__Consistency__*/
  }
#  endif /*__TX_Mark__("20221213 vr")*/

#  if __JunctionYield__
  if (_NonEmpty_(map_id2straightVehicleInfo) && NonNull_Pointer(FollowFrontPtr()) &&
      _Contain_(map_id2straightVehicleInfo, FollowFrontPtr()->ConsistencyId())) {
    const Base::txSysId otherId = FollowFrontPtr()->ConsistencyId();
    const auto &curVehicleInfo = map_id2straightVehicleInfo.at(otherId);
    const Base::Info_Lane_t linkUid = std::get<_LinkUId_>(curVehicleInfo);
    Base::txFloat invert_s = std::get<_InvertSOnLink_>(curVehicleInfo);
    Base::IVehicleElementPtr elemPtr = std::get<_ElemPtr_>(curVehicleInfo);
    Base::txSysId newOtherId = otherId;
    Base::txFloat dist_on_s = std::get<_DistToIntersectPt_>(curVehicleInfo);
    for (const auto &ref_id2info : map_id2straightVehicleInfo) {
      if (std::get<_LinkUId_>(ref_id2info.second) == linkUid &&
          std::get<_InvertSOnLink_>(ref_id2info.second) < invert_s) {
        newOtherId = ref_id2info.first;
        invert_s = std::get<_InvertSOnLink_>(ref_id2info.second);
        elemPtr = std::get<_ElemPtr_>(ref_id2info.second);
        dist_on_s = std::get<_DistToIntersectPt_>(ref_id2info.second);
      }
    }
    if (otherId != newOtherId) {
      Base::txSurroundVehiclefo nearestOnLink;
      std::get<_DistIdx_>(nearestOnLink) = 999.0;
      std::get<_DistIdx_>(DrivingFollow_Front()) = dist_on_s;
      std::get<_ElementIdx_>(DrivingFollow_Front()) = elemPtr;
      JunctionInfo << TX_VARS_NAME(replaceIdOnSameLink_from, otherId)
                   << TX_VARS_NAME(replaceIdOnSameLink_to, newOtherId);
    }
    SetDrivingStatus(_plus_(Base::IDrivingStatus::DrivingStatus::junction_yield));
#    if __Consistency__
    oss << TX_VARS_NAME(front_dist_7, FollowFrontDistance());
#    endif /*__Consistency__*/
  }
  JunctionInfo << TX_VARS(Id()) << TX_VARS(timeMgr.PassTime())
               << TX_VARS_NAME(frontId,
                               ((NonNull_Pointer(FollowFrontPtr())) ? (FollowFrontPtr()->ConsistencyId()) : (-1)))
               << TX_VARS(FollowFrontDistance());
#  endif /*__JunctionYield__*/
#  if __Consistency__
  ConsistencyAppend(oss.str());
#  endif /*__Consistency__*/
  CheckEgoCorrect();
#  undef _DistToIntersectPt_
#  undef _InvertSOnLink_
#  undef _LinkUId_
#  undef _ElemPtr_
}

Base::txBool TAD_AI_VehicleElement::Pre_Update(
    const TimeParamManager &_t, std::map<Elem_Id_Type, KineticsInfo_t> &map_elemId2Kinetics) TX_NOEXCEPT {
  return ParentClass::Pre_Update(_t, map_elemId2Kinetics);
}

void TAD_AI_VehicleElement::Search_Follow_Front_RTree(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  if (IsAlive() && timeMgr.PassTime() > 0.0) {
    /*m_follow_info*/
    Clear_Follow_Info();
    const Base::txVec3 &vehicleLaneDir = StableLaneDir();
    const Base::txSysId pVehicleSysId = SysId();
    const Base::txSysId pVehicleId = Id();
    const Base::txVec3 &vPos = StableGeomCenter().ENU();
    const Base::txVec2 vPos2d = Utils::Vec3_Vec2(vPos);
    const Base::txFloat selfVehicleLength = GetLength();
    const Base::txFloat selfVehicleWidth = GetWidth();
    const Base::Info_Lane_t &selfLaneInfo = StableLaneInfo();
    const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();

    UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);

    /*FLAGS_DETECT_OBJECT_DISTANCE*/
    HashedLaneInfoOrthogonalListPtr center_node_ptr = Weak2SharedPtr(std::get<1>(m_HashedRoadInfoCache));
    if (NonNull_Pointer(center_node_ptr)) {
#  if 1
      HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), selfStableHashedLaneInfo,
                                                              center_node_ptr, DrivingFollow_Front(),
                                                              FLAGS_DETECT_OBJECT_DISTANCE);

      /*on lanelink */

      if (IsOnLaneLink()) {
        LogInfo << "[Follow_Front][on link]." << TX_VARS(Id());
        HdMap::HadmapCacheConCurrent::lanelinkExpandVisionType::mapped_type res_expand_vision;
        if (HdMap::HadmapCacheConCurrent::CheckExpandVision(selfLaneInfo, res_expand_vision)) {
          std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
          const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInCircleByType(
              vPos2d, pVehicleSysId, 100.0, m_vehicle_type_filter_set, resultElementPtrVec);

          for (auto ref_ElementPtr : resultElementPtrVec) {
            Base::IVehicleElementPtr vehiclePtr = std::dynamic_pointer_cast<Base::IVehicleElement>(ref_ElementPtr);

            if (NonNull_Pointer(vehiclePtr) && (vehiclePtr->StableLaneInfo().isOnLaneLink)) {
              if (_Contain_(res_expand_vision, (vehiclePtr->StableLaneInfo()))) {
                const auto &expandDist_lanelinkLength = res_expand_vision.at((vehiclePtr->StableLaneInfo()));
                if (vehiclePtr->DistanceAlongCurve() + std::get<0>(expandDist_lanelinkLength) >
                    std::get<1>(expandDist_lanelinkLength)) {
                  DrivingFollow_Front() = std::make_tuple(0.0, vehiclePtr);
                  return;
                } else {
                  LOG(WARNING) << TX_VARS(Id()) << TX_VARS(vehiclePtr->Id())
                               << TX_VARS(vehiclePtr->DistanceAlongCurve())
                               << TX_VARS(std::get<0>(expandDist_lanelinkLength))
                               << TX_VARS(std::get<1>(expandDist_lanelinkLength));
                }
              }
            }
          }
        }

        std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
        const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInCircleByType(
            vPos2d, pVehicleSysId, FLAGS_OnLink_SameTo_QueryRadius, m_vehicle_type_filter_set, resultElementPtrVec);

        Base::txSurroundVehiclefo nearestOnLink;
        std::get<_DistIdx_>(nearestOnLink) = 999.0;
        for (const auto pOtherTrafficBase : resultElementPtrVec) {
          if (NonNull_Pointer(pOtherTrafficBase) && CallSucc(pOtherTrafficBase->IsOnLaneLink()) &&
              CallFail((pOtherTrafficBase->StableLaneInfo()) == selfLaneInfo)) {
            Base::IVehicleElementPtr pOtherVehicle =
                std::dynamic_pointer_cast<Base::IVehicleElement>(pOtherTrafficBase);
            if (NonNull_Pointer(pOtherVehicle)) {
              const auto &otherLaneInfo = pOtherVehicle->StableLaneInfo();
              const txBool bSameDst = (otherLaneInfo.onLinkToLaneUid == selfLaneInfo.onLinkToLaneUid);
              const txBool bSameSrc = (otherLaneInfo.onLinkFromLaneUid == selfLaneInfo.onLinkFromLaneUid);
              if (CallSucc(bSameDst) || CallSucc(bSameSrc TX_MARK("from same link start, but different link end"))) {
                const Base::txVec2 nearestElementCenter2d = pOtherVehicle->StableGeomCenter().ENU2D();
                const Base::txFloat curGap = (pOtherVehicle->GetLength() + GetLength()) * 0.5;
                TX_MARK("SIM-6412");
                const Base::txFloat curDistance =
                    ((StableGeomCenter().ENU2D()) - (nearestElementCenter2d)).norm() - curGap;

                if (curDistance < std::get<_DistIdx_>(nearestOnLink) &&
                    ((curDistance < FLAGS_default_lane_width) ||
                     (pOtherVehicle->InvertDistanceAlongCurve() < InvertDistanceAlongCurve()))) {
                  if ((pOtherVehicle->GetOnLaneLinkTimeStamp()) > GetOnLaneLinkTimeStamp()) {
                    TX_MARK("The current vehicle spends more time at the intersection than other vehicles");
                    TX_MARK("Ignore other vehicle, or other vehicle avoid current vehicle.");
                    continue;
                  } else if (Math::isEqual(pOtherVehicle->GetOnLaneLinkTimeStamp(), GetOnLaneLinkTimeStamp()) &&
                             (pOtherVehicle->Id()) < Id()) {
                    continue;
                  } else {
                    TX_MARK("Put it last because of the large amount of calculation");
                    Base::txFloat yaw = 0.0, distancePedal = 0.0, distanceCurve = 0.0;
                    const auto &other_gps = pOtherVehicle->StableGeomCenter().ToWGS84();
                    mLocation.tracker()->getCurrentCurve()->xy2sl(other_gps.Lon(), other_gps.Lat(), distanceCurve,
                                                                  distancePedal, yaw);
                    const txFloat nudgeWidth =
                        0.5 * selfVehicleWidth + 0.5 * (pOtherVehicle->GetWidth()) + FLAGS_lanelink_nudge_delta;
                    if (fabs(distancePedal) > nudgeWidth) {
                      // LOG(WARNING) << TX_VARS(Id()) << TX_VARS_NAME(ignore, pOtherVehicle->Id()) <<
                      // TX_VARS(fabs(distancePedal));
                      continue;
                    }
                    /*else {
                        const auto gps = mLocation.tracker()->getCurrentCurve()->getPoint(distanceCurve);
                        Coord::txENU enu; enu.FromWGS84(gps);
                        LOG(WARNING) << TX_VARS(Id()) << TX_VARS_NAME(notice, pOtherVehicle->Id()) <<
                    TX_VARS(fabs(distancePedal))
                            << TX_VARS(DistanceAlongCurve()) << TX_VARS(distanceCurve)
                            << TX_VARS(Coord::txENU::EuclideanDistanceLocal_2D(pOtherVehicle->StablePosition(), enu))
                            << TX_VARS(nudgeWidth);
                    }*/
                  }
                  std::get<_DistIdx_>(nearestOnLink) = curDistance;
                  std::get<_ElementIdx_>(nearestOnLink) = pOtherVehicle;
                }
              }
            }
          }
        }

        if (WeakIsValid(
                std::get<_ElementIdx_>(nearestOnLink)) /*NonNull_Pointer(std::get<_ElementIdx_>(nearestOnLink))*/) {
          if (NonNull_Pointer(FollowFrontPtr())) {
            if (std::get<_DistIdx_>(nearestOnLink) < FollowFrontDistance()) {
              DrivingFollow_Front() = nearestOnLink;
            }
          } else {
            DrivingFollow_Front() = nearestOnLink;
          }
        }
      } else if (IsOnLane()) {
        if (IsInTurnLeft()) {
          const HashedLaneInfo leftHashedLaneInfo = mLocation.GetLeftHashedLaneInfo(Id());
          const HashedLaneInfoOrthogonalListPtr &left_node_ptr =
              HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(leftHashedLaneInfo);

          Base::txSurroundVehiclefo leftFollowFront;
          std::get<_DistIdx_>(leftFollowFront) = 999.0;
          HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), leftHashedLaneInfo,
                                                                  left_node_ptr, leftFollowFront,
                                                                  FLAGS_DETECT_OBJECT_DISTANCE);
          if (WeakIsValid(std::get<_ElementIdx_>(leftFollowFront))
              /*NonNull_Pointer(std::get<_ElementIdx_>(leftFollowFront))*/) {
            if (NonNull_Pointer(FollowFrontPtr())) {
              if (std::get<_DistIdx_>(leftFollowFront) < FollowFrontDistance()) {
                DrivingFollow_Front() = leftFollowFront;
              }
            } else {
              DrivingFollow_Front() = leftFollowFront;
            }
          }
          /*LOG_IF(WARNING, 7 == Id()) << TX_VARS(Id())
              << TX_VARS(((NonNull_Pointer(std::get<_ElementIdx_>(leftFollowFront))) ?
             (std::get<_ElementIdx_>(leftFollowFront)->Id()) : (-1)))
              << TX_VARS(std::get<_DistIdx_>(leftFollowFront));*/
        } else if (IsInTurnRight()) {
          const HashedLaneInfo rightHashedLaneInfo = mLocation.GetRightHashedLaneInfo(Id());
          const HashedLaneInfoOrthogonalListPtr &right_node_ptr =
              HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(rightHashedLaneInfo);

          Base::txSurroundVehiclefo rightFollowFront;
          std::get<_DistIdx_>(rightFollowFront) = 999.0;
          HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), rightHashedLaneInfo,
                                                                  right_node_ptr, rightFollowFront,
                                                                  FLAGS_DETECT_OBJECT_DISTANCE);
          if (WeakIsValid(std::get<_ElementIdx_>(
                  rightFollowFront)) /*NonNull_Pointer(std::get<_ElementIdx_>(rightFollowFront))*/) {
            if (NonNull_Pointer(FollowFrontPtr())) {
              if (std::get<_DistIdx_>(rightFollowFront) < FollowFrontDistance()) {
                DrivingFollow_Front() = rightFollowFront;
              }
            } else {
              DrivingFollow_Front() = rightFollowFront;
            }
          }
        }
      }
#  endif
    } else {
      LOG(WARNING) << "[Query_OrthogonalList][null] " << TX_VARS(Id()) << TX_VARS_NAME(PassTime, timeMgr.PassTime())
                   << TX_VARS(selfStableHashedLaneInfo);
    }
#  if __TX_Mark__("20221213 vr")
    if (NonNull_Pointer(std::get<0>(FrontVeh_CurrentLane()))) {
      const Base::txFloat rtree_front_elem_dist = std::get<1>(FrontVeh_CurrentLane());
      if (rtree_front_elem_dist < FollowFrontDistance()) {
        Base::txSurroundVehiclefo rtree_front;
        std::get<_DistIdx_>(rtree_front) = rtree_front_elem_dist;
        std::get<_ElementIdx_>(rtree_front) = std::get<0>(FrontVeh_CurrentLane());
        DrivingFollow_Front() = rtree_front;
        // LogWarn << "[20221213_vr]" << TX_VARS(Id()) << "replace idm front by rtree front " <<
        // TX_VARS_NAME(rtree_front_id, std::get<0>(FrontVeh_CurrentLane())->ConsistencyId());
      }
    }
    // if (NonNull_Pointer(FollowFrontPtr()) && _plus_(Base::Enums::ElementType::TAD_Ego) ==
    // FollowFrontPtr()->ConsistencyElementType())
    // {
    //     // LogWarn << "[20221213_vr]" << TX_VARS(Id()) << " front is ego.";
    // }
#  endif /*__TX_Mark__("20221213 vr")*/
  }
}

void TAD_AI_VehicleElement::SearchSurroundingObstacle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {}
#endif /*__TX_Mark__("IEnvPerception")*/

#if USE_SUDOKU_GRID

void TAD_AI_VehicleElement::Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                                               std::vector<Base::txVec2> &retRegion) const TX_NOEXCEPT {
  mVehicleScanRegion2D.Get_Region_Clockwise_Close_By_Enum(eValue, retRegion);
}

void TAD_AI_VehicleElement::Compute_Region_Clockwise_Close(const Coord::txENU &vehPos, const Base::txVec3 &laneDir,
                                                           const Unit::txDegree &headingDegree) TX_NOEXCEPT {
  const auto vehicleOnLaneDegree = Utils::GetLaneAngleFromVectorOnENU(laneDir);
  const Base::txFloat v_cos = vehicleOnLaneDegree.Cos();
  const Base::txFloat v_sin = vehicleOnLaneDegree.Sin();

  mVehicleScanRegion2D.Rigid_Translate_Rotation(vehPos, v_cos, v_sin, headingDegree.Cos(), headingDegree.Sin());
}
#endif /*USE_SUDOKU_GRID*/

Base::txBool TAD_AI_VehicleElement::MoveStraightOnS(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
#if __Consistency__
  std::ostringstream oss;
#endif /*__Consistency__*/
  const txFloat moving_distance = Compute_Displacement(timeMgr);
  const auto curLaneInfo = mLocation.LaneLocInfo();

  const txFloat curveLength = mLocation.tracker()->getLaneGeomInfo()->GetLength();
#if __Consistency__
  oss << TX_VARS(moving_distance) << TX_VARS(curLaneInfo) << TX_VARS(curveLength);
#endif /*__Consistency__*/
  if ((mLocation.DistanceAlongCurve() + moving_distance) < curveLength) {
    mLocation.DistanceAlongCurve() += moving_distance;
#if __Consistency__
    oss << "[M1]";
#endif /*__Consistency__*/
  } else {
    const txFloat offset = (mLocation.DistanceAlongCurve() + moving_distance) - curveLength;
#if __Consistency__
    oss << "[M2]" << TX_VARS(offset);
#endif /*__Consistency__*/
    if (mLocation.IsOnLaneLink()) {
#if __Consistency__
      oss << "[M3]";
#endif /*__Consistency__*/
      hadmap::txLanePtr nextLane =
          HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.tracker()->getCurrentLaneLink()->toTxLaneId());
      if (RelocateTracker(nextLane, timeMgr.TimeStamp())) {
#if __Consistency__
        oss << "[M4]";
#endif /*__Consistency__*/
        mLocation.DistanceAlongCurve() = offset;
      } else {
        StopVehicle(TODO("state machine"));
        LogWarn << "mLocation.tracker()->relocate error. StopVehicle";
        return false;
      }
    } else {
#if __Consistency__
      oss << "[M5]";
#endif /*__Consistency__*/
      if (mLocation.tracker()->isReachedRoadLastSection()) {
#if __Consistency__
        oss << "[M6]" << TX_VARS(mLocation.LaneLocInfo().onLaneUid);
#endif /*__Consistency__*/
        hadmap::txLaneLinks nextLaneLinks;
        if (NonNull_Pointer(GetCurTargetSignLightPtr())) {
          nextLaneLinks = GetCurTargetSignLightPtr()->GetControlLaneLinks(mLocation.LaneLocInfo().onLaneUid);
        }
        if (_Empty_(nextLaneLinks)) {
          nextLaneLinks = HdMap::HadmapCacheConCurrent::GetLaneNextLinks(mLocation.LaneLocInfo().onLaneUid);
        }
        RouteLogInfo << TX_VARS(Id()) << TX_VARS_NAME(from_laneUid, Utils::ToString(mLocation.LaneLocInfo().onLaneUid))
                     << TX_VARS(nextLaneLinks.size());

        if (_NonEmpty_(nextLaneLinks)) {
          hadmap::txLaneLinks reachableLinks;
          reachableLinks.reserve(nextLaneLinks.size());
          TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
#if __Consistency__
          oss << "[M8]" << TX_VARS(nextLaneLinks.size());
#endif /*__Consistency__*/
          for (auto curLaneLinkPtr : nextLaneLinks) {
#if __Consistency__
            oss << TX_VARS_NAME(to, curLaneLinkPtr->toTxLaneId());
#endif /*__Consistency__*/
            if (NonNull_Pointer(curLaneLinkPtr) &&
                CallFail(HdMap::HadmapCacheConCurrent::IsLinkIdInBlackList(Base::Info_Lane_t(
                    curLaneLinkPtr->getId(), curLaneLinkPtr->fromTxLaneId(), curLaneLinkPtr->toTxLaneId()))) &&
                CallSucc(mRouteAI.IsReachable(curLaneLinkPtr->toTxLaneId(), routeHint))) {
              reachableLinks.emplace_back(curLaneLinkPtr);
            } else {
              RouteLogInfo << TX_VARS(Id())
                           << "[cloud_ditw_blacklist], current next lanelinks could not reachable goal."
                           << TX_VARS_NAME(linkId, (curLaneLinkPtr->getId()))
                           << TX_VARS_NAME(link_from, Utils::ToString(curLaneLinkPtr->fromTxLaneId()))
                           << TX_VARS_NAME(link_to, Utils::ToString(curLaneLinkPtr->toTxLaneId()));
            }
          }
          hadmap::txLaneLinkPtr nextLaneLink = nullptr;
          if (_NonEmpty_(reachableLinks)) {
            nextLaneLink = RandomNextLink(reachableLinks);
            RouteLogInfo << TX_VARS(Id()) << TX_VARS(reachableLinks.size()) << TX_VARS(nextLaneLink->toTxLaneId());
#if __Consistency__
            oss << "[M9]" << TX_VARS(reachableLinks.size()) << TX_VARS(nextLaneLink->toTxLaneId());
#endif /*__Consistency__*/
          } else {
            nextLaneLink = RandomNextLink(nextLaneLinks);
            RouteLogInfo << TX_VARS(Id()) << ", current next lanelinks could not reachable goal."
                         << TX_VARS(nextLaneLinks.size());
#if __Consistency__
            oss << "[M10]" << TX_VARS(nextLaneLinks.size()) << TX_VARS(nextLaneLink->toTxLaneId());
#endif /*__Consistency__*/
          }

          if (NonNull_Pointer(nextLaneLink)) {
#if __Consistency__
            oss << "[M11]";
#endif /*__Consistency__*/
            if (NonNull_Pointer(nextLaneLink->getGeometry()) && (nextLaneLink->getGeometry()->getLength()) > offset) {
              /*TX_MARK("新的lane的长度大于offset，可以承载本次move的溢出");*/
              mLocation.DistanceAlongCurve() = offset;
              RelocateTracker(nextLaneLink, timeMgr.TimeStamp());
              mLocation.NextLaneIndex() = nextLaneLink->toLaneId();
              // GetNextCrossRoadFromLaneLink(m_nextCrossRoadInfo, nextLaneLink->fromRoadId(),
              // nextLaneLink->toRoadId());
#if __Consistency__
              oss << "[M12]";
#endif /*__Consistency__*/
            } else {
#if __Consistency__
              oss << "[M13]";
#endif /*__Consistency__*/
              hadmap::txLanePtr nextLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(nextLaneLink->toTxLaneId());
              if (NonNull_Pointer(nextLane)) {
                RelocateTracker(nextLane, timeMgr.TimeStamp());
                const Base::txFloat newOffset =
                    offset -
                    (NonNull_Pointer(nextLaneLink->getGeometry()) ? (nextLaneLink->getGeometry()->getLength()) : (0.0));
#if __Consistency__
                oss << "[M14]" << TX_VARS_NAME(nextLane, nextLane->getTxLaneId()) << TX_VARS(newOffset);
#endif /*__Consistency__*/
                if (newOffset > 0.0) {
                  mLocation.DistanceAlongCurve() = newOffset;
                } else {
                  mLocation.DistanceAlongCurve() = 0.0;
                  LogWarn << "Compute lanelink offset error.";
                  StopVehicle(); /*Kill();*/
                  return false;
                }
              } else {
                LogWarn << "GetTxLanePtr nullptr,"
                        << TX_VARS_NAME(toLaneUid, Utils::ToString(nextLaneLink->toTxLaneId()));
                StopVehicle(); /*Kill();*/
                return false;
              }
            }
          } else {
            LogWarn << "Cannot find nextLaneLink with roadInfoArray.NextRoadId.";
            StopVehicle(); /*Kill();*/
            return false;
          }
        } else {
          LogWarn << "LaneLink Size is Zero."
                  << TX_VARS_NAME(QueryLaneUid, Utils::ToString(mLocation.LaneLocInfo().onLaneUid));
          StopVehicle();
          return false;
        }
      } else /*if (mLocation.tracker()->isReachedRoadLastSection())*/ {
        TX_MARK("本次到达的不是Road的最后一个section，在路内");
        hadmap::txLanes nextLanes =
            HdMap::HadmapCacheConCurrent::GetNextLanesByLaneUid(mLocation.tracker()->getCurrentLane());
#if __Consistency__
        oss << "[M15]" << TX_VARS(nextLanes.size());
#endif /*__Consistency__*/
        if (_NonEmpty_(nextLanes)) {
          hadmap::txLanes reachableLanes;
          reachableLanes.reserve(nextLanes.size());
          TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
          for (auto curLanePtr : nextLanes) {
            if (NonNull_Pointer(curLanePtr) &&
                CallFail(
                    HdMap::HadmapCacheConCurrent::IsLinkIdInBlackList(Base::Info_Lane_t(curLanePtr->getTxLaneId()))) &&
                CallSucc(mRouteAI.IsReachable(curLanePtr->getTxLaneId(), routeHint)) &&
                CallSucc(Utils::IsDrivingLane(curLanePtr->getLaneType()))) {
              reachableLanes.emplace_back(curLanePtr);
            } else {
              RouteLogInfo << TX_VARS(Id()) << "[cloud_ditw_blacklist], current lane could not reachable goal."
                           << TX_VARS_NAME(black_laneuid, Utils::ToString(curLanePtr->getTxLaneId()));
            }
          }
          hadmap::txLanePtr nextLanePtr = nullptr;
#if __Consistency__
          oss << "[M16]" << TX_VARS(reachableLanes.size());
#endif /*__Consistency__*/
          if (_NonEmpty_(reachableLanes)) {
            nextLanePtr = RandomNextLane(reachableLanes);
          } else {
            nextLanePtr = RandomNextLane(nextLanes);
            RouteLogInfo << TX_VARS(Id()) << ", current next lane could not reachable goal.";
          }

          if (NonNull_Pointer(nextLanePtr)) {
#if __Consistency__
            oss << "[M16]" << TX_VARS_NAME(nextLaneId, nextLanePtr->getTxLaneId());
#endif /*__Consistency__*/
            RelocateTracker(nextLanePtr, timeMgr.TimeStamp());
            mLocation.DistanceAlongCurve() = offset;
            // UpdateNextCrossRoadPathPlaneDirection(m_laneInfo.onLaneUid.roadId, m_laneInfo.onLaneUid.sectionId);//
          } else {
            LogWarn << " not empty vector 'nextLanes' first element ptr is null.";
            StopVehicle(); /* Kill(); */
            return false;
          }
        } else {
          LogWarn << " LaneLink Size is Zero. "
                  << TX_VARS_NAME(cur_lane_uid, Utils::ToString(mLocation.tracker()->getCurrentLane()->getTxLaneId()));
          StopVehicle(); /* Kill(); */
          return false;
        }
      }
    }
  }
  mLocation.UpdateNewMovePointWithoutOffset();
#if __Consistency__
  oss << TX_VARS_NAME(LocInfo, mLocation.LaneLocInfo()) << TX_VARS_NAME(s, DistanceAlongCurve())
      << TX_VARS_NAME(OnlanePos, mLocation.PosOnLaneCenterLinePos().StrWGS84())
      << TX_VARS_NAME(dir, Utils::ToString(mLocation.vLaneDir()));
  ConsistencyAppend(oss.str());
#endif /*__Consistency__*/
  return true;
}

Base::txFloat TAD_AI_VehicleElement::Compute_LateralDisplacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  Base::txFloat cur_step_lateral_displacement = 0.0;
#if 1
  cur_step_lateral_displacement = (m_LaneChangeTimeMgr.Duration() > 0.0)
                                      ? (mKinetics.m_LateralVelocity * timeMgr.RelativeTime())
                                      : (-1.0 * mKinetics.m_LateralVelocity * timeMgr.RelativeTime());

#else
  if (GetVelocity() > 0.0) {
    cur_step_lateral_displacement = (m_LaneChangeTimeMgr.Duration() > 0.0)
                                        ? (mKinetics.m_LateralVelocity * timeMgr.RelativeTime())
                                        : (-1.0 * mKinetics.m_LateralVelocity * timeMgr.RelativeTime());
  } else {
    TX_MARK("lateral displacement is zero when longitudinal velocity is zero");
    cur_step_lateral_displacement = 0.0;
  }
#endif
  mKinetics.m_LateralDisplacement += cur_step_lateral_displacement;
  return mKinetics.m_LateralDisplacement;
}
Base::txFloat TAD_AI_VehicleElement::Get_MoveTo_Sideway_Angle(const txFloat exper_time) TX_NOEXCEPT {
  txFloat curLaneWidth = FLAGS_default_lane_width;
  if (mLocation.IsOnLane()) {
    txFloat searchWidth = mLocation.tracker()->getCurrentLane()->getLaneWidth();
    curLaneWidth = searchWidth > 0 ? searchWidth : FLAGS_default_lane_width;
  }

  // return lateral_velocity * (curLaneWidth) / FLAGS_default_lane_width;
  return Get_MoveTo_Sideway_Angle(exper_time, curLaneWidth);
}

Base::txFloat TAD_AI_VehicleElement::Get_MoveTo_Sideway_Angle(const txFloat exper_time,
                                                              const txFloat laneWidth) TX_NOEXCEPT {
  // V_Y=CalculateLateralSpeed(); // 根据Duration(一共要换多久)和Procedure（换到什么程度了），
  // 还有车道宽度（缺省值？）来计算换道的贝塞尔轨迹曲线，进而算出横向的速度
  const txFloat t = (exper_time > 0.0) ? (exper_time) : (0.0);
  const txFloat lateral_velocity = t * (1.037) - (0.2304444) * (t * t);

  return lateral_velocity * (laneWidth) / FLAGS_default_lane_width;
}

Base::txFloat TAD_AI_VehicleElement::MoveBack_Sideway_Angle(const txFloat time) TX_NOEXCEPT {
  txFloat t = (time > 0) ? time : 0.0;

  txFloat V_Y = 0.0;
  txFloat a, b;
  // if (ZeroValueCheck(LaneChanging_Duration- Average_LaneChanging_Duration))
  /*b = 1.74 + 0.5*(3 - LaneAborting_Duration);*/
  b = 3.11 + 1.5 * (2.2 - m_LaneAbortingTimeMgr.Duration());
  // else if (ZeroValueCheck(LaneChanging_Duration - Average_LaneChanging_Duration_Short))
  // b = 3.57 + 1.75*(2.2 - LaneAborting_Duration);
  a = b / m_LaneAbortingTimeMgr.Duration();
  V_Y = t * b - a * t * t;
  return V_Y;
}

// Relocation after lane change
Base::txBool TAD_AI_VehicleElement::UpdateSwitchLaneRunning(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS_NAME(State(), __enum2lpsz__(TAD_VehicleState_AI, State()));
  if (IsInTurn_Start()) {
    if (m_LaneChangeTimeMgr.Procedure() <= (m_LaneChangeTimeMgr.Duration() * (ChangeTopologyThreshold()))) {
      ChaneFromStartToIng();
      ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(timeMgr.PassTime()) << TX_VARS(m_LaneChangeTimeMgr.Duration())
                        << TX_VARS(m_LaneChangeTimeMgr.Procedure()) << TX_VARS(ChangeTopologyThreshold())
                        << TX_VARS(ChaneFromStartToIng());
    }
  } else if (IsInTurn_Ing()) {
    if (m_LaneChangeTimeMgr.Procedure() <= 0.0) {
      m_LaneChangeTimeMgr.SetProcedure(0.0);
      m_LaneChangeTimeMgr.SetDuration(0.0);
      /*mLocation.OnLanePos() = mLocation.PosWithoutOffset();*/
      /*mLocation.CurLaneIndex() = mLocation.NextLaneIndex();*/
      mKinetics.m_LateralDisplacement = 0.0;
      ChaneFromIngToFinish();
      SetLaneKeep();
      hadmap::txLanePtr initLane = nullptr;
      hadmap::txLaneLinkPtr initLaneLink = nullptr;
      Base::txFloat tmpdistoncurve;

      if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
              mLocation.PosWithLateralWithoutOffset(), mLocation.LaneLocInfo(),
              mLocation.DistanceAlongCurve(TX_MARK("20210920")))) {
        if (mLocation.IsOnLaneLink()) {
          initLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                        mLocation.LaneLocInfo().onLinkToLaneUid);
          if (NonNull_Pointer(initLaneLink)) {
            RelocateTracker(initLaneLink, timeMgr.TimeStamp());
          }
        } else {
          initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
          if (NonNull_Pointer(initLane)) {
            RelocateTracker(initLane, timeMgr.TimeStamp());
          }
        }
      }

#if 1
      /*for lane change finish*/
      UpdateHashedLaneInfo(mLocation);
      const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();
      UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
#endif

      return true;
    }
  } else if (IsInTurn_Abort()) {
    /*changeLaneState = CHANGE_LANE_STATE_CANCEL_ING;*/
  }

  return false;
}

// Position related operations during lane change
Base::txBool TAD_AI_VehicleElement::LaneChangePostOp(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  txFloat fAngle = 0.0;

  mLocation.PosWithLateralWithoutOffset() =
      mLocation.PosOnLaneCenterLinePos(TX_MARK("result from mLocation.UpdateNewMovePointWithoutOffset()"));
  if (IsInLaneChange(/*CHANGE_LANE_STATE::CHANGE_LANE_STATE_START <= changeLaneState*/)) {
    Compute_LateralDisplacement(timeMgr);
    Base::txVec3 tmpLaneDir = mLocation.vLaneDir();

    static const Unit::txDegree sRightDir = Unit::txDegree::MakeDegree(RightOnENU);
    static const Unit::txDegree sLeftDir = Unit::txDegree::MakeDegree(LeftOnENU);
    const Base::txVec3 vTargetDir =
        Utils::VetRotVecByDegree(tmpLaneDir, ((IsInTurnRight()) ? (sRightDir) : (sLeftDir)), Utils::Axis_Enu_Up());

    mLocation.PosWithLateralWithoutOffset().TranslateLocalPos(__East__(vTargetDir) * GetLateralDisplacement(),
                                                              __North__(vTargetDir) * GetLateralDisplacement());

    // static const txFloat maxV_V = Get_MoveTo_Sideway_Angle(2.25);
    const txFloat maxV_V = Get_MoveTo_Sideway_Angle(2.25);
    if (Math::isNotZero(maxV_V)) {
#if 1
      if (m_LaneChangeTimeMgr.Duration() > 0.0) {
        const txFloat t = ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()) *
                           m_LaneChangeTimeMgr.Average_LaneChanging_Duration());
        const txFloat tmpV_V = Get_MoveTo_Sideway_Angle(t);
        fAngle = tmpV_V * 15.0 / maxV_V;
      } else if (m_LaneAbortingTimeMgr.Duration() > 0.0) {
        /*const txFloat t = m_LaneAbortingTimeMgr.Experience();*/
        const txFloat t = ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()) *
                           m_LaneChangeTimeMgr.Average_LaneChanging_Duration());
        const txFloat tmpV_V = MoveBack_Sideway_Angle(t);
        fAngle = tmpV_V * 15.0 / maxV_V;
      } else {
        fAngle = 0.0;
      }

      if (IsInTurn_LeftRight()) {
        fAngle *= (IsInTurnLeft()) ? (1.0) : (-1.0);
        fAngle *= (IsInTurn_Abort()) ? (-1.0) : (1.0);
        // fAngle *= (GetVelocity() > 0.0) ? (1.0) : (0.0);
      } else {
        LogWarn << "unimpossible condition.";
      }
#endif
    }
    if (UpdateSwitchLaneRunning(timeMgr)) {
      mLaneKeepMgr.ResetCounter(LaneKeep(), GetAggress());
    }
  } else {
    SetLaneKeep();
    mKinetics.m_LateralDisplacement = 0.0;
    mKinetics.m_LateralVelocity = 0.0;
  }

  mLocation.fAngle().FromDegree(fAngle);
  RawVehicleCoord() =
      ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(), mLocation.LaneOffset());
  SyncPosition(timeMgr.PassTime());
  Compute_CenterLine_Offset();
  if (mLocation.IsOnLane()) {
    CheckDeadLine();
  }
  return true;
}

Base::txBool TAD_AI_VehicleElement::CheckDeadLine() TX_NOEXCEPT {
  if (Base::IVehicleElement::VehicleSource::scene_cfg == Vehicle_ComeFrom() && FLAGS_TADSimRouteEndStop &&
      mRouteAI.ArriveAtDeadlineArea(mLocation.LaneLocInfo().onLaneUid, GeomCenter().ToENU())) {
    StopVehicle();
  }
  return IsAlive();
}

// Randomly initialize the state on cloud.
void TAD_AI_VehicleElement::InitializeRandomKinetics(Base::txFloat mStartV, Base::txFloat mMaxV,
                                                     Base::txFloat variance) TX_NOEXCEPT {
  /*std::ostringstream oss;
  oss << TX_VARS_NAME(veh_id, mIdentity.Id()) << TX_VARS_NAME(Src_mStartV, mStartV) << TX_VARS_NAME(Src_mMaxV, mMaxV);*/
  const auto rnd_05_15_1 = mPRandom.GetRandomValue() + 0.5;
  mStartV = mStartV * rnd_05_15_1;
  const auto rnd_05_15_2 = mPRandom.GetRandomValue() * 2.0 - 0.5;
  mMaxV = mMaxV + 3.0 * rnd_05_15_2;
  // oss << TX_VARS(rnd_05_15_1) << TX_VARS(rnd_05_15_2);
  if (mStartV > mMaxV) {
    mStartV = mMaxV;
  }
  // oss << TX_VARS_NAME(New_mStartV, mStartV) << TX_VARS_NAME(New_mMaxV, mMaxV);
  // LogInfo << oss.str();

  mKinetics.m_velocity = mStartV;
  mKinetics.LastVelocity() = mKinetics.m_velocity;
  mKinetics.m_velocity_max = mMaxV;
  mKinetics.velocity_desired = mKinetics.m_velocity_max;
  mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
}

// Randomly initialize the state on cloud.
void TAD_AI_VehicleElement::InitializeRandomKinetics(VehicleInitParam_t veh_init_param) TX_NOEXCEPT {
  Base::txFloat mStartV = veh_init_param.StartV();
  Base::txFloat mMaxV = veh_init_param.MaxV(TX_MARK("random kinect"));

  if (veh_init_param.mForceStartV.has_value()) {
    mStartV = *(veh_init_param.mForceStartV);
  }

  if (mStartV > mMaxV) {
    mStartV = mMaxV;
  }

  mKinetics.m_velocity = mStartV;
  mKinetics.LastVelocity() = mKinetics.m_velocity;
  mKinetics.m_velocity_max = mMaxV;
  mKinetics.velocity_desired = mKinetics.m_velocity_max;
  mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
}

void TAD_AI_VehicleElement::InitializeBehavior(Base::ISceneLoader::ITrafficFlowViewer::BehPtr _beh_ptr) TX_NOEXCEPT {
  if (NonNull_Pointer(_beh_ptr)) {
    TX_MARK("call on cloud.");
    SetLcDuration(_beh_ptr->LCduration());
    mDriving_Parameters[0] = _beh_ptr->cc0();
    mDriving_Parameters[1] = _beh_ptr->cc1();
    mDriving_Parameters[2] = _beh_ptr->cc2();
    mW74_DEFAULT_ax = _beh_ptr->AX();
    mW74_DEFAULT_bx_add = _beh_ptr->BX_Add();
    mW74_DEFAULT_bx_mult = _beh_ptr->BX_Mult();

    if (Base::ISceneLoader::ITrafficFlowViewer::IBeh::BehType::Arterial == _beh_ptr->type()) {
      mDrivingFollowStrategy = DrivingFollowType::e74;
    } else {
      mDrivingFollowStrategy = DrivingFollowType::e99;
    }
    LOG_IF(INFO, FLAGS_LogLevel_Vehicle) << TX_VARS(Id()) << TX_VARS_NAME(Beh, _beh_ptr->Str());
  } else {
    TX_MARK("call on desktop.");
    SetLcDuration(FLAGS_Average_LaneChanging_Duration_Short);
  }
}

// Initialize the state on cloud.
Base::txBool TAD_AI_VehicleElement::Initialize(const Base::txSysId vehId,
                                               const Base::VehicleInputeAgent::VehicleInitParam_t &veh_init_param,
                                               const Base::txLaneID startLaneId,
                                               Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (Null_Pointer(veh_init_param.mVehTypePtr) || Null_Pointer(veh_init_param.mBehPtr)) {
    LogWarn << TX_VARS(vehId) << TX_COND(Null_Pointer(veh_init_param.mVehTypePtr))
            << TX_COND(Null_Pointer(veh_init_param.mBehPtr));
    return false;
  }

  InitializeBehavior(veh_init_param.mBehPtr);
  m_vehicle_come_from = Base::IVehicleElement::VehicleSource::veh_input;

  m_vehicle_type = __lpsz2enum__(VEHICLE_TYPE, veh_init_param.mVehTypePtr->vehicleType().c_str());
#if USE_CustomModelImport
  m_vehicle_type_str = veh_init_param.mVehTypePtr->vehicleType().c_str();
  m_vehicle_type_id = Base::CatalogCache::VEHICLE_TYPE(m_vehicle_type_str);
#endif /*#USE_CustomModelImport*/

  /*LogWarn << TX_VARS(vehId) << TX_VARS_NAME(vehicleType, m_vehicle_type._to_string());*/
  mIdentity.Id() = vehId;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  SM::txAITrafficState::Initialize(mIdentity.Id());
  mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
  mLifeCycle.StartTime() = 0.0;

  txFloat initCompliance = mPRandom.GetRandomValue();
  mRuleCompliance = (initCompliance < _sceneLoader->GetRuleComplianceProportion());
  LogInfo << TX_VARS(Id()) << TX_VARS(mRuleCompliance) << TX_VARS(initCompliance)
          << TX_VARS(_sceneLoader->GetRuleComplianceProportion());

  ConfigurePerceptionPeriod(mPRandom.GetRandomInt() % 10);

#if 1
#  if USE_CustomModelImport
  if (CallFail(InitVehicleCatalog(m_vehicle_type_str))) {
#  else  /*#USE_CustomModelImport*/
  if (CallFail(InitCatalog(m_vehicle_type))) {
#  endif /*#USE_CustomModelImport*/
    mGeometryData.Length() = veh_init_param.mVehTypePtr->length();
    mGeometryData.Width() = veh_init_param.mVehTypePtr->width();
    mGeometryData.Height() = veh_init_param.mVehTypePtr->height();
    LogWarn << "[Catalog_query][failure] " << TX_VARS(Id())
            << TX_VARS_NAME(VehicleType, (veh_init_param.mVehTypePtr->vehicleType())) << TX_VARS(GetLength())
            << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
  } else {
    mGeometryData.Length() = Catalog_Length();
    mGeometryData.Width() = Catalog_Width();
    mGeometryData.Height() = Catalog_Height();
    LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success] " << TX_VARS(Id())
                                         << TX_VARS_NAME(VehicleType, (veh_init_param.mVehTypePtr->vehicleType()))
                                         << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
  }
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;
#else
  mGeometryData.Length() = veh_init_param.mVehTypePtr->length();
  mGeometryData.Width() = veh_init_param.mVehTypePtr->width();
  mGeometryData.Height() = veh_init_param.mVehTypePtr->height();
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;
#endif
  m_aggress = veh_init_param.mAggress;
  TX_MARK("For lanekeep");
  mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());
  LogInfo << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.RndFactor());
  mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
  Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
  ConfigureFollowStrategy();
  InitializeRandomKinetics(veh_init_param);

  mLocation.NextLaneIndex() = TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(startLaneId);
  mLocation.LaneOffset() = 0.0;

  const Base::Info_Lane_t StartLaneInfo = veh_init_param.mLocationPtr->GetLaneInfo();
  if (CallFail(StartLaneInfo.isOnLaneLink)) {
    hadmap::txLanePtr initLane_roadId_sectionId = nullptr;
    initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetTxLanePtr(
        Base::txLaneUId(StartLaneInfo.onLaneUid.roadId, StartLaneInfo.onLaneUid.sectionId, startLaneId));
    if (Null_Pointer(initLane_roadId_sectionId)) {
      LogWarn << " initLane_roadId_sectionId GetLane Failure."
              << TX_VARS_NAME(laneUid, Utils::ToString(StartLaneInfo.onLaneUid));
      return false;
    }
    const Base::txLaneUId curLaneUid(StartLaneInfo.onLaneUid.roadId, StartLaneInfo.onLaneUid.sectionId, startLaneId);
    if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(curLaneUid, (veh_init_param.mLocationPtr->GetDistanceOnCurve()),
                                                        mLocation.PosOnLaneCenterLinePos())) {
      mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
      mLocation.DistanceAlongCurve() = veh_init_param.mLocationPtr->GetDistanceOnCurve();
      mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(curLaneUid, mLocation.DistanceAlongCurve());
      RawVehicleCoord() =
          ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
      SyncPosition(0.0);
      mLocation.InitTracker(mIdentity.Id());
      RelocateTracker(initLane_roadId_sectionId, 0.0);
#if USE_SUDOKU_GRID
      /*heading_for_front_region_on_ENU = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());*/
      Compute_Region_Clockwise_Close(GeomCenter().ToENU(), mLocation.vLaneDir(),
                                     mLocation.heading_for_front_region_on_ENU());
#endif /*USE_SUDOKU_GRID*/
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
      FillingSpatialQuery();
      SaveStableState();
      LogInfo << Str();
      return true;
    } else {
      LogWarn << ", Get_LAL_Lane_By_S Failure.";
      return false;
    }
  } else {
    LogWarn << "cloud vehicle do not start lanelink.";
    return false;
  }
}

// Simulation consistency
void TAD_AI_VehicleElement::SaveStableState() TX_NOEXCEPT {
  ParentClass::SaveStableState();
  SaveLastFSMStatus();
#if USE_HashedRoadNetwork
  // StableHashedLaneInfo() = mLocation.GetHashedLaneInfo(-1);
#endif /*USE_HashedRoadNetwork*/
  {
    TX_MARK("JIRA-184"); /* Vehs are in front of each other */
    if (NonNull_Pointer(FollowFrontPtr())) {
      SaveStableFollowFrontId(FollowFrontPtr()->ConsistencyId());
    }
  }
}

// Road network positioning for HashedRoadNetwork
void TAD_AI_VehicleElement::UpdateHashedLaneInfo(const Base::Component::Location &_location) TX_NOEXCEPT {
#if __ClearHashLanePerStep__
  ParentClass::UpdateHashedLaneInfo(_location);
  m_op_LaneChangeHashedLaneInfo = boost::none;
#else  /*__ClearHashLanePerStep__*/
  const HashedLaneInfo newHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  if (CallFail(newHashedLaneInfo == m_curHashedLaneInfo)) {
    HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(m_curHashedLaneInfo, SysId());
    HdMap::HashedRoadCacheConCurrent::RegisterVehicle(newHashedLaneInfo, GetVehicleElementPtr());
    LOG_IF(INFO, FLAGS_LogLevel_IDM_Front) << "[cloud_debug][register][update]" << TX_VARS(Id());
  } else {
    LOG_IF(INFO, FLAGS_LogLevel_IDM_Front) << "[cloud_debug][register][remain]" << TX_VARS(Id());
  }
  m_curHashedLaneInfo = newHashedLaneInfo;

  if (m_op_LaneChangeHashedLaneInfo.has_value() && CallFail(m_curHashedLaneInfo == *m_op_LaneChangeHashedLaneInfo)) {
    HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(*m_op_LaneChangeHashedLaneInfo, SysId());
    m_op_LaneChangeHashedLaneInfo = boost::none;
  }
#endif /*__ClearHashLanePerStep__*/

  if (IsOnLane()) {
    if (IsInTurnLeft()) {
      m_op_LaneChangeHashedLaneInfo = _location.GetLeftHashedLaneInfo(Id());
      if ((*m_op_LaneChangeHashedLaneInfo).IsValid()) {
        HdMap::HashedRoadCacheConCurrent::RegisterVehicle(*m_op_LaneChangeHashedLaneInfo, GetVehicleElementPtr());
      }
    } else if (IsInTurnRight()) {
      m_op_LaneChangeHashedLaneInfo = _location.GetRightHashedLaneInfo(Id());
      if ((*m_op_LaneChangeHashedLaneInfo).IsValid()) {
        HdMap::HashedRoadCacheConCurrent::RegisterVehicle(*m_op_LaneChangeHashedLaneInfo, GetVehicleElementPtr());
      }
    }
  }
}

void TAD_AI_VehicleElement::UnRegisterInfoOnKill() TX_NOEXCEPT {
  ParentClass::UnRegisterInfoOnKill();
#if __ClearHashLanePerStep__
#else  /*__ClearHashLanePerStep__*/
  if (m_op_LaneChangeHashedLaneInfo) {
    HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(*m_op_LaneChangeHashedLaneInfo, SysId());
    m_op_LaneChangeHashedLaneInfo = boost::none;
  }
#endif /*__ClearHashLanePerStep__*/
}

void TAD_AI_VehicleElement::ResetLaneKeep() TX_NOEXCEPT { mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress); }

// DynamicChangeScene -> VehicleElementGenerator(ReRoute) -> ReRoute
Base::txBool TAD_AI_VehicleElement::ReRoute(const Base::txSysId routeGroupId, const Base::txInt subRouteId,
                                            const RouteAI &newRouteAI) TX_NOEXCEPT {
  if (VehicleSource::veh_input == m_vehicle_come_from) {
    tbb::mutex::scoped_lock lock(m_tbbMutex);
    mRouteAI = newRouteAI;
    mRouteAI.ResetVehicleId(Id());
    return mRouteAI.ComputeRoute(mPRandom);
  } else {
    return false;
  }
}

Base::txBool TAD_AI_VehicleElement::Post_Update(TimeParamManager const &_t) TX_NOEXCEPT {
  ParentClass::Post_Update(_t);
  SetCurTargetSignLightPtr(nullptr);
  SetCurDistanceToTargetSignLight(FLT_MAX);
  SetCurFrontPedestrianPtr(nullptr);
  SetDistanceToPedestrian(FLT_MAX);
  ClearForceSampling(/*for serialization*/);
  UpdateLaneKeep(_t);
  UpdateTrajectory(_t);
  return true;
}

hadmap::txLaneLinkPtr TAD_AI_VehicleElement::RandomNextLink(const hadmap::txLaneLinks &nextLaneLinks) TX_NOEXCEPT {
  if (_NonEmpty_(nextLaneLinks)) {
#if 0
    /*for (const auto& refLinkPtr : nextLaneLinks) {
      if (NonNull_Pointer(refLinkPtr) &&
        CallSucc(mRouteAI.CheckReachableRoad(refLinkPtr->toRoadId()))) {
        return refLinkPtr;
      }
    }*/
#endif
    const auto nRnd = mPRandom.GetRandomInt();
    return nextLaneLinks.at(Math::RangeInt32(nRnd, nextLaneLinks.size()));
  } else {
    return nullptr;
  }
}

Base::txString TAD_AI_VehicleElement::binary_archive() const TX_NOEXCEPT {
#ifdef ON_CLOUD
  using OutputArchive = cereal::BinaryOutputArchive;
  using InputArchive = cereal::BinaryInputArchive;

  std::stringstream ss;        // any stream can be used
  OutputArchive oarchive(ss);  // Create an output archive
  oarchive(*this);             // Write the data to the archive
  return std::move(ss.str());
#else
  return "";
#endif
}

Base::txString TAD_AI_VehicleElement::xml_archive() const TX_NOEXCEPT { return ""; }

Base::txString TAD_AI_VehicleElement::json_archive() const TX_NOEXCEPT {
#ifdef ON_CLOUD
  using OutputArchive = cereal::JSONOutputArchive;
  using InputArchive = cereal::JSONInputArchive;
  std::stringstream ss;        // any stream can be used
  OutputArchive oarchive(ss);  // Create an output archive
  oarchive(*this);             // Write the data to the archive
  return std::move(ss.str());
#else
  return "";
#endif
}

// Whether the front is blocked
Base::txBool TAD_AI_VehicleElement::BlockAhead() TX_NOEXCEPT {
  if ((FollowFrontDistance() < GetLength()) &&
      ((_plus_(IDrivingStatus::DrivingStatus::stop_crash) == FollowFrontPtr()->StableDriving_Status()) ||
       ((FollowFrontPtr()->StableVelocity()) < StableVelocity()))) {
    return true;
  } else {
    return false;
  }
}

Base::txBool TAD_AI_VehicleElement::IsSignalLightAhead(const Base::Enums::SIGN_LIGHT_COLOR_TYPE _color) const
    TX_NOEXCEPT {
  return (m_weak_TargetSignLightPtr.expired())
             ? (false)
             : ((_color) == ((m_weak_TargetSignLightPtr.lock())->GetCurrentSignLightColorType()));
}

Base::txBool TAD_AI_VehicleElement::RelocateTracker(hadmap::txLanePtr pLane, const txFloat &_timestamp) TX_NOEXCEPT {
  if (CallSucc(ParentClass::RelocateTracker(pLane, _timestamp))) {
    const Base::txFloat roadSpeedLimit = Unit::kmh_ms(pLane->getSpeedLimit());
    if (FLAGS_Enable_Road_Limit && roadSpeedLimit > 0.0 && roadSpeedLimit < mKinetics.raw_velocity_max) {
      LOG_IF(INFO, FLAGS_LogLevel_SpeedLimit)
          << "[SpeedLimit] " << TX_VARS_NAME(Id, Id()) << TX_VARS_NAME(laneUid, Utils::ToString(pLane->getTxLaneId()))
          << TX_VARS_NAME(SpeedLimitFromMap, pLane->getSpeedLimit())
          << TX_VARS_NAME(kmh_ms, Unit::kmh_ms(pLane->getSpeedLimit()));
      mKinetics.m_velocity_max = roadSpeedLimit;
    } else {
      mKinetics.m_velocity_max = mKinetics.raw_velocity_max;
    }
    mKinetics.velocity_desired = mKinetics.m_velocity_max;
    // txFloat curLaneWidth = pLane->getLaneWidth() > 0 ? pLane->getLaneWidth() : FLAGS_default_lane_width;
    // m_lanechange_max_v =  Get_MoveTo_Sideway_Angle(2.25, curLaneWidth);
    return true;
  } else {
    return false;
  }
}

Base::txBool TAD_AI_VehicleElement::RelocateTracker(hadmap::txLaneLinkPtr pLaneLink,
                                                    const txFloat &_timestamp) TX_NOEXCEPT {
  if (CallSucc(ParentClass::RelocateTracker(pLaneLink, _timestamp))) {
    const Base::txFloat roadSpeedLimit = (Unit::kmh_ms(pLaneLink->getSpeedLimit()) * 0.9);
    if (FLAGS_Enable_Road_Limit && roadSpeedLimit > 0.0 && roadSpeedLimit < mKinetics.raw_velocity_max) {
      LOG_IF(INFO, FLAGS_LogLevel_SpeedLimit)
          << "[SpeedLimit] " << TX_VARS_NAME(Id, Id()) << TX_VARS_NAME(lanelinkUid, pLaneLink->getId())
          << TX_VARS_NAME(radius, pLaneLink->getRadius()) << TX_VARS_NAME(SpeedLimitFromMap, pLaneLink->getSpeedLimit())
          << TX_VARS_NAME(kmh_ms, Unit::kmh_ms(pLaneLink->getSpeedLimit()));
      mKinetics.m_velocity_max = roadSpeedLimit;
    } else {
      mKinetics.m_velocity_max = mKinetics.raw_velocity_max;
    }
    mKinetics.velocity_desired = mKinetics.m_velocity_max;
    return true;
  } else {
    return false;
  }
}

Base::txFloat TAD_AI_VehicleElement::SignalReactionGap() const TX_NOEXCEPT {
  const Base::txFloat u = mKinetics.raw_velocity_max * 1.5; /*initial velocity*/
  const Base::txFloat v = 0.0;                              /*final velocity*/
  const Base::txFloat t = (0.0 - mKinetics.raw_velocity_max) / Max_Deceleration();
  const Base::txFloat s = 0.5 * (u + v) * t;

  return (Signal_Reaction_Gap() < 0.0) ? (s) : (Signal_Reaction_Gap());
}

// Yield at the junction.
Base::txBool TAD_AI_VehicleElement::UpdateTrajectory(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  using namespace Geometry::Element;
  ClearGeomCurve();
  if (IsOnLaneLink()) {
    const Base::txVec2 vehiclecenter2d = GeomCenter().ToENU().ENU2D();
    const Base::txVec2 vehicle_half_dir = Utils::Vec3_Vec2(GetLaneDir()) * (0.5 * GetLength());

    mGeomLongSkeletionLinePtr = std::make_shared<txLine>();
    mGeomLongSkeletionLinePtr->Initialize(Utils::Vec2_Vec3(vehiclecenter2d + vehicle_half_dir),
                                          Utils::Vec2_Vec3(vehiclecenter2d - vehicle_half_dir));
    std::vector<Base::txVec3> input_params;
    if (CallSucc(MakeJuntionTrajectory(timeMgr, input_params)) && input_params.size() > 1) {
      txPolylinePtr polyline_ptr = std::make_shared<txPolyline>();
      polyline_ptr->Initialize(input_params);
      mGeomTrajPtr = polyline_ptr;
      return true;
    }
  }
  return false;
}

Base::txBool TAD_AI_VehicleElement::MakeJuntionTrajectory(Base::TimeParamManager const &,
                                                          std::vector<Base::txVec3> &samplingPtVec) TX_NOEXCEPT {
  samplingPtVec.clear();
  if (IsAlive() && IsOnLaneLink()) {
    // txFloat s = DistanceAlongCurve();
    txFloat s = std::max((DistanceAlongCurve() - GetLength() / 2), 0.0);
    /*s = (s < 0.0) ? (0.0) : (s);*/
    auto geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(GetCurrentLaneInfo());
    if (NonNull_Pointer(geom_ptr)) {
      const txFloat curve_len = geom_ptr->GetLength();
      while (s < curve_len) {
        samplingPtVec.emplace_back(geom_ptr->GetLocalPos(s));
        s += 1.0;
      }
      return true;
    }
  }
  return false;
}

#ifdef InjectMode
InjectModeVehicleElement::InjectModeVehicleElement() : pImpl(std::make_shared<InjectImpl>()) {}

Base::txBool InjectModeVehicleElement::IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT {
  return (_plus_(VEHICLE_BEHAVIOR::eInjectMode) == behavior);
}

Base::txFloat InjectModeVehicleElement::DrivingFollow(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  LOG(INFO) << "[InjectModeVehicleElement::DrivingFollow] " << TX_VARS_NAME(time, timeMgr.PassTime()) << " start.";
  txFloat ret = 0.0;
  if (NonNull_Pointer(pImpl)) {
    using ParamKVMap = InjectImpl::ParamKVMap;
    ParamKVMap inParam;
    Base::SimulationConsistencyPtr frontVehicleElementPtr = FollowFrontPtr();
    double fLeaderVelocity = (NonNull_Pointer(frontVehicleElementPtr)) ? (frontVehicleElementPtr->StableVelocity())
                                                                       : (mKinetics.m_velocity_max + 10.0);
    double fLeaderAcceleration = (NonNull_Pointer(frontVehicleElementPtr)) ? (frontVehicleElementPtr->StableAcc())
                                                                           : (mKinetics.m_velocity_max + 10.0);
    double frontDist = FollowFrontDistance();

    Base::SimulationConsistencyPtr sideVehiclePtr = nullptr;
    double SideGap_TargetLane = 999.0;
    std::tie(sideVehiclePtr, SideGap_TargetLane) = Any_SideVeh_TargetLane();
    const double SideVehicle_Velocity =
        (NonNull_Pointer(sideVehiclePtr) ? (sideVehiclePtr->StableVelocity()) : (MAX_SPEED()));
    double Max_Speed = mKinetics.velocity_desired;

    inParam["has_front_vehilce"] = bool2lpsz(NonNull_Pointer(frontVehicleElementPtr));
    inParam["fLeaderVelocity"] = std::to_string(fLeaderVelocity);
    inParam["fLeaderAcceleration"] = std::to_string(fLeaderAcceleration);
    inParam["mKinetics.m_velocity_max"] = std::to_string(mKinetics.m_velocity_max);
    inParam["frontDist"] = std::to_string(frontDist);
    inParam["StableVelocity"] = std::to_string(StableVelocity());
    inParam["StableAcc"] = std::to_string(StableAcc());
    inParam["m_aggress"] = std::to_string(m_aggress);
    inParam["SideVehicle_Velocity"] = std::to_string(SideVehicle_Velocity);
    inParam["mKinetics.velocity_desired"] = std::to_string(mKinetics.velocity_desired);
    inParam["SideGapMinVelocity"] = std::to_string(SideGapMinVelocity());
    inParam["Friction_Factor"] = std::to_string(Friction_Factor());
    inParam["Random"] = std::to_string(Random(""));
    inParam["Max_Deceleration"] = std::to_string(Max_Deceleration());
    inParam["GetRandomValue_NegOne2PosOne"] = std::to_string(GetRandomValue_NegOne2PosOne(""));

    inParam["Driving_Parameters_0"] = std::to_string(Driving_Parameters(0));
    inParam["Driving_Parameters_1"] = std::to_string(Driving_Parameters(1));
    inParam["Driving_Parameters_2"] = std::to_string(Driving_Parameters(2));
    inParam["Driving_Parameters_3"] = std::to_string(Driving_Parameters(3));
    inParam["Driving_Parameters_4"] = std::to_string(Driving_Parameters(4));
    inParam["Driving_Parameters_5"] = std::to_string(Driving_Parameters(5));
    inParam["Driving_Parameters_6"] = std::to_string(Driving_Parameters(6));
    inParam["Driving_Parameters_7"] = std::to_string(Driving_Parameters(7));
    inParam["Driving_Parameters_8"] = std::to_string(Driving_Parameters(8));
    inParam["Driving_Parameters_9"] = std::to_string(Driving_Parameters(9));

    ret = pImpl->DrivingFollow(txMapHdr, inParam);
  }
  LOG(INFO) << "[InjectModeVehicleElement::DrivingFollow] " << TX_VARS_NAME(time, timeMgr.PassTime()) << " end.";
  return ret;
}

Base::txFloat InjectModeVehicleElement::DrivingChangeLane(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  LOG(INFO) << "[InjectModeVehicleElement::DrivingChangeLane] " << TX_VARS_NAME(time, timeMgr.PassTime()) << " start.";
  txFloat ret = 0.0;
  if (NonNull_Pointer(pImpl)) {
    using ParamKVMap = InjectImpl::ParamKVMap;
    pImpl->DrivingChangeLane(txMapHdr, ParamKVMap());
  }
  LOG(INFO) << "[InjectModeVehicleElement::DrivingChangeLane] " << TX_VARS_NAME(time, timeMgr.PassTime()) << " end.";
  return ret;
}

#endif /*InjectModeVehicleElement*/

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
#undef RouteLogInfo
#undef ChangeLaneLogInfo
#undef EnvPerceptionInfo
#undef VehicleTrajInfo
#undef USE_MAPSDK_XY2SL
#undef __Trace_PushBack_

#if 1
#  undef NoCheckLCAbortThreshold
#  undef ChangeTopologyThreshold
#  undef LaneKeep
#  undef Driving_Parameters
#  undef Max_Deceleration
#  undef Max_Ped_Reaction_Distance
#  undef MAX_SPEED
#  undef ComfortGap
#  undef MAX_REACTION_GAP
#  undef SpeedDenominatorOffset
#  undef Tolerance_Threshold
#  undef LaneChanging_Aggressive_Threshold
#  undef Yielding_Aggressive_Threshold
#  undef SafetyGap_Front
#  undef SafetyGap_Rear
#  undef SafetyGap_Side
#  undef EmptyTargetLaneGapThreshold
#  undef Signal_Reaction_Gap
#  undef Max_Comfort_Deceleration
#  undef Friction_Factor
#endif
