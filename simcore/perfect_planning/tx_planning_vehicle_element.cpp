// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_vehicle_element.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <sstream>
#include "HdMap/tx_hashed_lane_info.h"
#include "HdMap/tx_hashed_road.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_collision_detection2d.h"
#include "tx_frame_utils.h"
#include "tx_hadmap_utils.h"
#include "tx_planning_env_vehicle_info.h"
#include "tx_planning_flags.h"
#include "tx_planning_scene_loader.h"
#include "tx_protobuf_utils.h"
#include "tx_route_path_manager.h"
#include "tx_sim_point.h"
#include "tx_spatial_query.h"
#include "tx_time_utils.h"
#include "tx_trajectory_sampling_node.h"
#include "tx_units.h"
#define USE_MAPSDK_XY2SL (0)
// #define __Trace_PushBack_(__str__) m_vecTrace.push_back(__str__)
#define __Trace_PushBack_(__str__)
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Element || FLAGS_LogLevel_Ego_Traj_ClosedLoop)
#define LogWarn LOG(WARNING)
#define RouteLogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Route)
#define ChangeLaneLogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_LaneChange)
#define EnvPerceptionInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_EnvPerception)
#define FrontFollowInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Front)
#define TrajInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Traj)

#if __TX_Mark__("cfg")

#  define NoCheckLCAbortThreshold (Utils::VehicleBehaviorCfg::NoCheckLCAbortThreshold())
#  define ChangeTopologyThreshold (Utils::VehicleBehaviorCfg::ChangeTopologyThreshold())
#  define LaneKeep (Utils::VehicleBehaviorCfg::LaneKeep())
#  define Driving_Parameters(__index__) (Utils::VehicleBehaviorCfg::g_Driving_Parameters(__index__))
#  define Max_Deceleration (mKinetics.max_dec_m_ss())
#  define Max_Ped_Reaction_Distance (Utils::VehicleBehaviorCfg::Max_Ped_Reaction_Distance())
#  define MAX_SPEED (mKinetics.m_velocity_max)
#  define ComfortGap (Utils::VehicleBehaviorCfg::ComfortGap())
#  define MAX_REACTION_GAP (Utils::VehicleBehaviorCfg::MAX_REACTION_GAP())
#  define SpeedDenominatorOffset (Utils::VehicleBehaviorCfg::SpeedDenominatorOffset())
#  define Tolerance_Threshold (Utils::VehicleBehaviorCfg::Tolerance_Threshold())
#  define LaneChanging_Aggressive_Threshold (Utils::VehicleBehaviorCfg::LaneChanging_Aggressive_Threshold())
#  define Yielding_Aggressive_Threshold (Utils::VehicleBehaviorCfg::Yielding_Aggressive_Threshold())
#  define SafetyGap_Front (Utils::VehicleBehaviorCfg::SafetyGap_Front())
#  define SafetyGap_Rear (Utils::VehicleBehaviorCfg::SafetyGap_Rear())
#  define SafetyGap_Side (Utils::VehicleBehaviorCfg::SafetyGap_Side())
#  define EmptyTargetLaneGapThreshold (Utils::VehicleBehaviorCfg::EmptyTargetLaneGapThreshold())
#  define Signal_Reaction_Gap (FLAGS_ego_Signal_Reaction_Gap)
#  define Max_Comfort_Deceleration (Utils::VehicleBehaviorCfg::Max_Comfort_Deceleration())

#  define Friction_Factor (Utils::VehicleBehaviorCfg::Friction_Factor())
// 0205 HD 旁边车道的摩擦系数
#endif /*__TX_Mark__("cfg")*/

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txFloat SimPlanningVehicleElement::Random(const Base::txString& strMark) TX_NOEXCEPT {
  __Trace_PushBack_(mPRandom.CurrentStateStr(strMark));
  // txMsg("Random()");
  return mPRandom.GetRandomValue();
}

Base::txFloat SimPlanningVehicleElement::GetRandomValue_NegOne2PosOne(Base::txString strMark) TX_NOEXCEPT {
  // txMsg("GetRandomValue_NegOne2PosOne()");
  __Trace_PushBack_(mPRandom.CurrentStateStr(strMark));
  return mPRandom.GetRandomValue_NegOne2PosOne();
}

Base::txBool SimPlanningVehicleElement::Initialize_Kinetics(const txFloat start_v, const txFloat max_v,
                                                            const txFloat max_acce,
                                                            const txFloat max_dece) TX_NOEXCEPT {
  LogInfo << TX_VARS(start_v) << TX_VARS(max_v);
  SetPlanningVelocity(start_v, true);
  mKinetics.LastVelocity() = mKinetics.m_velocity;
  mKinetics.m_velocity_max = max_v;
  mKinetics.velocity_desired = mKinetics.m_velocity_max;
  mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
  mKinetics.SetMaxAcc(max_acce);
  if (max_dece > 0.0) {
    mKinetics.SetMaxDec(-1.0 * max_dece);
  } else {
    mKinetics.SetMaxDec(max_dece);
  }

  return true;
}
#if USE_SUDOKU_GRID
void SimPlanningVehicleElement::Get_Region_Clockwise_Close_By_Enum(
    const Base::Enums::NearestElementDirection eValue, std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT {
  mVehicleScanRegion2D.Get_Region_Clockwise_Close_By_Enum(eValue, retRegion);
}

void SimPlanningVehicleElement::Compute_Region_Clockwise_Close(const Coord::txENU& vehPos, const Base::txVec3& laneDir,
                                                               const Unit::txDegree& headingDegree) TX_NOEXCEPT {
  const auto vehicleOnLaneDegree = Utils::GetLaneAngleFromVectorOnENU(laneDir);
  const Base::txFloat v_cos = vehicleOnLaneDegree.Cos();
  const Base::txFloat v_sin = vehicleOnLaneDegree.Sin();

  mVehicleScanRegion2D.Rigid_Translate_Rotation(vehPos, v_cos, v_sin, headingDegree.Cos(), headingDegree.Sin());
}
#endif /*USE_SUDOKU_GRID*/
Base::txBool SimPlanningVehicleElement::FillingSpatialQuery() TX_NOEXCEPT {
  ParentClass::FillingSpatialQuery();
#if USE_SUDOKU_GRID
  Compute_Region_Clockwise_Close(GeomCenter().ToENU(), GetLaneDir(), GetHeadingWithAngle());
#endif /*USE_SUDOKU_GRID*/
  return true;
}

Base::txBool SimPlanningVehicleElement::InitializeEgo(sim_msg::Location& refEgoData, const control_path_node_vec&,
                                                      ISceneLoader::IViewerPtr _viewPtr,
                                                      ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  m_ego_init_location.CopyFrom(refEgoData);
#if __Control_V2__
  InitControlV2Pb();
#endif /*__Control_V2__*/
  if (NonNull_Pointer(_viewPtr) && NonNull_Pointer(_sceneLoader)) {
    if (CallFail(_sceneLoader->GetVehicleGeometory(0, Base::Enums::EgoSubType::eLeader, VehicleGeometory()))) {
      LogWarn << "GetVehicleGeometory failure.";
      return false;
    } else {
      LogInfo << VehicleGeometory().DebugString() << TX_VARS(VehicleGeometory().vehicle_geometory().length())
              << TX_VARS(VehicleGeometory().vehicle_geometory().rear_axle_to_rear());
    }

    auto scene_event_loader_ptr = std::dynamic_pointer_cast<PlanningSceneEvent_SceneLoader>(_sceneLoader);
    if (NonNull_Pointer(scene_event_loader_ptr)) {
      auto dummy_driver_view_vec = scene_event_loader_ptr->GetDummyDriverViewerVec();
      if (_NonEmpty_(dummy_driver_view_vec)) {
        auto dummy_view_ptr = dummy_driver_view_vec.front();
        for (auto iter : dummy_driver_view_vec) {
          if (iter->Group() == scene_event_loader_ptr->GetGroupName()) {
            LogWarn << "Current group name is " << scene_event_loader_ptr->GetGroupName();
            dummy_view_ptr = iter;
          }
        }
        if (CallSucc(InitializeSceneEvent(dummy_view_ptr, _sceneLoader))) {
          LogWarn << "InitializeSceneEvent is success.";
        } else {
          LogWarn << "InitializeSceneEvent is failure.";
          return false;
        }
      } else {
        LogWarn << "[on_cloud]dummy_driver_view_vec is empty. activate = sim_msg::Activate_Automode_AUTOMODE_AUTOPILOT";
        FLAGS_ego_use_activate = false;
        UpdateActivateMode(_plus_(Base::Enums::Planning_Activate::eAutopilot));
        /*return false;*/
      }
    } else {
      LogWarn << "scene_event_loader_ptr is nullptr.";
      return false;
    }

    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    SceneLoader::Planning_SceneLoader::EgoRouteViewerPtr _elemRouteViewPtr =
        std::dynamic_pointer_cast<SceneLoader::Planning_SceneLoader::EgoRouteViewer>(_viewPtr);
    if (NonNull_Pointer(_elemRouteViewPtr)) {
      LogInfo << "Ego Input Param :" << TX_VARS_NAME(initLocation, Utils::ProtobufDebugString(&m_ego_init_location))
              << TX_VARS_NAME(ego_route, _elemRouteViewPtr->Str());

      mIdentity.Id() = 0;
      mIdentity.SysId() = CreateSysId(mIdentity.Id());
      SM::txAITrafficState::Initialize(mIdentity.Id());
      mPRandom.Initialize(mIdentity.Id(), FLAGS_ego_rnd_seed);
      mLifeCycle.StartTime() = 0.0;

      mGeometryData.Length() = VehicleGeometory().vehicle_geometory().length();
      mGeometryData.Width() = VehicleGeometory().vehicle_geometory().width();
      mGeometryData.Height() = VehicleGeometory().vehicle_geometory().height();
      mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

      m_aggress = FLAGS_ego_aggress;
      TX_MARK("For lanekeep");
      mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());
      LOG(INFO) << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.RndFactor());
      mLaneKeepMgr.ResetCounter(LaneKeepTime(), m_aggress);
#if USE_SUDOKU_GRID
      mVehicleScanRegion2D.Initialize(mGeometryData.Length(), mGeometryData.Width());
#endif /*USE_SUDOKU_GRID*/

      /*Base::txVec3 init_velocity_3d(m_ego_init_location.velocity().x(), m_ego_init_location.velocity().y(),
       * m_ego_init_location.velocity().z());*/
      txFloat input_max_acc = 0.0;
      txFloat input_max_dece = 0.0;
      if (CallFail(FLAGS_planning_const_velocity_mode)) {
        input_max_acc = _sceneLoader->PlannerAccMax();
        input_max_dece = _sceneLoader->PlannerDeceMax();
      }
      LOG(INFO) << "[planning_const_velocity_mode] " << TX_COND(FLAGS_planning_const_velocity_mode)
                << TX_VARS(input_max_acc) << TX_VARS(input_max_dece);
      if (CallFail(Initialize_Kinetics(_sceneLoader->PlannerStartV(), _sceneLoader->PlannerVelocityMax(), input_max_acc,
                                       input_max_dece))) {
        LogWarn << "initialize kinetics error.";
        return false;
      }

      TX_MARK("ego must be on the road, not at the lanelink/junction.");
      txFloat distancePedal_lane = 0.0;
      RawVehicleCoord().FromWGS84(m_ego_init_location.position().x(), m_ego_init_location.position().y());
      if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
              RawVehicleCoord(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset())) {
        LOG(INFO) << "[ego_init_location] " << TX_VARS_NAME(egoId, mIdentity.Id())
                  << (mLocation.LaneLocInfo().IsOnLaneLink() ? " [LaneLink]" : " [Lane]");
        if (mLocation.LaneLocInfo().IsOnLane()) {
          TX_MARK("on lane");
          mLocation.NextLaneIndex() =
              TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(mLocation.LaneLocInfo().onLaneUid.laneId);
          hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
          if (Null_Pointer(initLane)) {
            LogWarn << ", GetLane Failure."
                    << TX_VARS_NAME(_laneUid, Utils::ToString(mLocation.LaneLocInfo().onLaneUid));
            return false;
          }

          if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(initLane->getTxLaneId(), mLocation.DistanceAlongCurve(),
                                                              mLocation.PosOnLaneCenterLinePos())) {
            mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
            mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, mLocation.DistanceAlongCurve());
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
            mRouteAI.SetExactEndPoint(TX_MARK("single end point"));
            if (CallSucc(mRouteAI.Initialize(_elemRouteViewPtr->id(), 0 /*SubRouteId*/, mIdentity.Id(),
                                             mLocation.PosOnLaneCenterLinePos().WGS84(), _elemRouteViewPtr->midGPS(),
                                             _elemRouteViewPtr->endGPS()))) {
              mRouteAI.ComputeRoute(mPRandom);
            } else {
              LogWarn << TX_VARS_NAME(VehicleId, mIdentity.Id()) << TX_VARS_NAME(RouteId, _elemRouteViewPtr->id())
                      << " mRouteAI.Initialize failure.";
            }
            LogInfo << TX_VARS(Id()) << TX_VARS(GetCurrentLaneInfo());
            return true;
          } else {
            LogWarn << ", Get_LAL_Lane_By_S Failure."
                    << TX_VARS_NAME(_laneUid, Utils::ToString(mLocation.LaneLocInfo().onLaneUid));
            return false;
          }
        } else {
          TX_MARK("on link");
          mLocation.NextLaneIndex() =
              TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(mLocation.LaneLocInfo().onLinkToLaneUid.laneId);
          LogWarn << "un support on lanelink." << TX_VARS_NAME(LaneLocInfo, mLocation.LaneLocInfo());
          return false;
        }
      } else {
        LogWarn << "ego initialize failure, position "
                << TX_VARS_NAME(x, Utils::FloatToString(m_ego_init_location.position().x()))
                << TX_VARS_NAME(y, Utils::FloatToString(m_ego_init_location.position().y()))
                << TX_VARS_NAME(z, Utils::FloatToString(m_ego_init_location.position().z()));
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemRouteViewPtr, NonNull_Pointer(_elemRouteViewPtr));
      return false;
    }
  } else {
    LogWarn << "Param Cast Error." << TX_COND_NAME(_elemRouteViewPtr, NonNull_Pointer(_viewPtr));
    return false;
  }
}

Base::txBool SimPlanningVehicleElement::Post_Update(Base::TimeParamManager const& _t) TX_NOEXCEPT {
  ParentClass::Post_Update(_t);
  SetCurTargetSignLightPtr(nullptr);
  SetCurDistanceToTargetSignLight(FLT_MAX);
  clear_env_signal();
  txEgoEnvVehicleInfo_Factory::Clear();
  UpdateLaneKeep(_t);
  return true;
}

Base::txBool SimPlanningVehicleElement::CheckDeadLine() TX_NOEXCEPT {
  /*if (mRouteAI.ArriveAtDeadlineArea(mLocation.LaneLocInfo().onLaneUid, mLocation.vPos().ToENU()))
  {
      StopVehicle();
  }*/
  return IsAlive();
}

void SimPlanningVehicleElement::ResetLaneKeep() TX_NOEXCEPT { mLaneKeepMgr.ResetCounter(LaneKeepTime(), m_aggress); }

#if USE_HashedRoadNetwork

void SimPlanningVehicleElement::RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT {
  m_curHashedLaneInfo = _location.GetHashedLaneInfo(Id());
}

void SimPlanningVehicleElement::UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT {
  m_curHashedLaneInfo = _location.GetHashedLaneInfo(Id());
}

#endif /*USE_HashedRoadNetwork*/

Base::txBool SimPlanningVehicleElement::LaneChangePostOp(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
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

    static const txFloat maxV_V = Get_MoveTo_Sideway_Angle(2.25);
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
      mLaneKeepMgr.ResetCounter(LaneKeepTime(), GetAggress());
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

Base::txBool SimPlanningVehicleElement::UpdateSwitchLaneRunning(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(State()._to_string());
  if (IsInTurn_Start()) {
    if (m_LaneChangeTimeMgr.Procedure() <= (m_LaneChangeTimeMgr.Duration() * (ChangeTopologyThreshold))) {
      ChaneFromStartToIng();
    }
  } else if (IsInTurn_Ing()) {
    if (m_LaneChangeTimeMgr.Procedure() <= 0.0) {
      hadmap::txLanePtr initLane = nullptr;
      hadmap::txLaneLinkPtr initLaneLink = nullptr;
      // LOG(WARNING) << "before reset " << TX_VARS(Id()) << TX_VARS(mLocation.DistanceAlongCurve()) <<
      // TX_VARS(mLocation.vPos());
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
      m_LaneChangeTimeMgr.SetProcedure(0.0);
      m_LaneChangeTimeMgr.SetDuration(0.0);
      /*mLocation.OnLanePos() = mLocation.PosWithoutOffset();*/
      /*mLocation.CurLaneIndex() = mLocation.NextLaneIndex();*/
      mKinetics.m_LateralDisplacement = 0.0;
      ChaneFromIngToFinish();
      SetLaneKeep();

      return true;
    }
  } else if (IsInTurn_Abort()) {
    /*changeLaneState = CHANGE_LANE_STATE_CANCEL_ING;*/
  }

  return false;
}

Base::txFloat SimPlanningVehicleElement::Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr)
    TX_NOEXCEPT {
  Base::txFloat cur_step_lateral_displacement = 0.0;
#if 1
  cur_step_lateral_displacement = (m_LaneChangeTimeMgr.Duration() > 0.0)
                                      ? (mKinetics.m_LateralVelocity * timeMgr.RelativeTime())
                                      : (-1.0 * mKinetics.m_LateralVelocity * timeMgr.RelativeTime());
#endif
  mKinetics.m_LateralDisplacement += cur_step_lateral_displacement;
  return mKinetics.m_LateralDisplacement;
}
Base::txFloat SimPlanningVehicleElement::Get_MoveTo_Sideway_Angle(const txFloat exper_time) TX_NOEXCEPT {
  // V_Y=CalculateLateralSpeed();//根据Duration(一共要换多久)和Procedure（换到什么程度了），还有车道宽度（缺省值？）来计算换道的贝塞尔轨迹曲线，进而算出横向的速度
  const txFloat t = (exper_time > 0.0) ? (exper_time) : (0.0);
  const txFloat lateral_velocity = t * (1.037) - (0.2304444) * (t * t);

  txFloat curLaneWidth = FLAGS_default_lane_width;
  if (mLocation.IsOnLane()) {
    curLaneWidth = mLocation.tracker()->getCurrentLane()->getLaneWidth();
  }
  return lateral_velocity * (curLaneWidth) / FLAGS_default_lane_width;
}

Base::txFloat SimPlanningVehicleElement::MoveBack_Sideway_Angle(const txFloat time) TX_NOEXCEPT {
  txFloat t = (time > 0) ? time : 0.0;

  txFloat V_Y = 0.0;
  txFloat a, b;
  // if (ZeroValueCheck(LaneChanging_Duration- Average_LaneChanging_Duration))
  /*b = 1.74 + 0.5*(3 - LaneAborting_Duration);*/
  b = 3.11 + 1.5 * (2.2 - m_LaneAbortingTimeMgr.Duration());
  // else if (ZeroValueCheck(LaneChanging_Duration - Average_LaneChanging_Duration_Short))
  //  b = 3.57 + 1.75*(2.2 - LaneAborting_Duration);
  a = b / m_LaneAbortingTimeMgr.Duration();
  V_Y = t * b - a * t * t;
  return V_Y;
}

Base::txBool SimPlanningVehicleElement::MoveStraightOnS(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  const txFloat moving_distance = Compute_Displacement(timeMgr);
  const auto curLaneInfo = mLocation.LaneLocInfo();

  const txFloat curveLength = mLocation.tracker()->getLaneGeomInfo()->GetLength();
  if ((mLocation.DistanceAlongCurve() + moving_distance) < curveLength) {
    mLocation.DistanceAlongCurve() += moving_distance;
  } else {
    const txFloat offset = (mLocation.DistanceAlongCurve() + moving_distance) - curveLength;
    if (mLocation.IsOnLaneLink()) {
      hadmap::txLanePtr nextLane =
          HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.tracker()->getCurrentLaneLink()->toTxLaneId());
      if (RelocateTracker(nextLane, timeMgr.TimeStamp())) {
        mLocation.DistanceAlongCurve() = offset;
      } else {
        StopVehicle(TODO("state machine"));
        LogWarn << "mLocation.tracker()->relocate error. StopVehicle";
        return false;
      }
    } else {
      if (mLocation.tracker()->isReachedRoadLastSection()) {
        hadmap::txLaneLinks nextLaneLinks =
            HdMap::HadmapCacheConCurrent::GetLaneNextLinks(mLocation.LaneLocInfo().onLaneUid);
        if (_NonEmpty_(nextLaneLinks)) {
          hadmap::txLaneLinks reachableLinks;
          reachableLinks.reserve(nextLaneLinks.size());
          for (auto curLaneLinkPtr : nextLaneLinks) {
            if (NonNull_Pointer(curLaneLinkPtr) && CallSucc(mRouteAI.CheckReachable(curLaneLinkPtr->toTxLaneId()))) {
              reachableLinks.emplace_back(curLaneLinkPtr);
            } else {
              RouteLogInfo << TX_VARS(Id()) << ", current next lanelinks could not reachable goal."
                           << TX_VARS_NAME(linkId, (curLaneLinkPtr->getId()))
                           << TX_VARS_NAME(link_from, Utils::ToString(curLaneLinkPtr->fromTxLaneId()))
                           << TX_VARS_NAME(link_to, Utils::ToString(curLaneLinkPtr->toTxLaneId()));
            }
          }
          hadmap::txLaneLinkPtr nextLaneLink = nullptr;
          if (_NonEmpty_(reachableLinks)) {
            nextLaneLink = RandomNextLink(reachableLinks);
          } else {
            nextLaneLink = RandomNextLink(nextLaneLinks);
            RouteLogInfo << TX_VARS(Id()) << ", current next lanelinks could not reachable goal."
                         << TX_VARS(nextLaneLinks.size());
          }

          if (NonNull_Pointer(nextLaneLink)) {
            if (NonNull_Pointer(nextLaneLink->getGeometry()) && (nextLaneLink->getGeometry()->getLength()) > offset) {
              /*TX_MARK("新的lane的长度大于offset，可以承载本次move的溢出");*/
              mLocation.DistanceAlongCurve() = offset;
              RelocateTracker(nextLaneLink, timeMgr.TimeStamp());
              mLocation.NextLaneIndex() = nextLaneLink->toLaneId();
              // GetNextCrossRoadFromLaneLink(m_nextCrossRoadInfo, nextLaneLink->fromRoadId(),
              // nextLaneLink->toRoadId());
            } else {
              hadmap::txLanePtr nextLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(nextLaneLink->toTxLaneId());
              if (NonNull_Pointer(nextLane)) {
                RelocateTracker(nextLane, timeMgr.TimeStamp());
                const Base::txFloat newOffset =
                    offset -
                    (NonNull_Pointer(nextLaneLink->getGeometry()) ? (nextLaneLink->getGeometry()->getLength()) : (0.0));
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
      } else { /*if (mLocation.tracker()->isReachedRoadLastSection())*/
        TX_MARK("本次到达的不是Road的最后一个section，在路内");
        hadmap::txLanes nextLanes =
            HdMap::HadmapCacheConCurrent::GetNextLanesByLaneUid(mLocation.tracker()->getCurrentLane());

        if (_NonEmpty_(nextLanes)) {
          hadmap::txLanes reachableLanes;
          reachableLanes.reserve(nextLanes.size());
          for (auto curLanePtr : nextLanes) {
            if (NonNull_Pointer(curLanePtr) && CallSucc(mRouteAI.CheckReachable(curLanePtr->getTxLaneId())) &&
                CallSucc(Utils::IsDrivingLane(curLanePtr->getLaneType()))) {
              reachableLanes.emplace_back(curLanePtr);
            }
          }
          hadmap::txLanePtr nextLanePtr = nullptr;
          if (_NonEmpty_(reachableLanes)) {
            nextLanePtr = RandomNextLane(reachableLanes);
          } else {
            nextLanePtr = RandomNextLane(nextLanes);
            RouteLogInfo << TX_VARS(Id()) << ", current next lane could not reachable goal.";
          }

          if (NonNull_Pointer(nextLanePtr)) {
            auto geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(nextLanePtr->getTxLaneId());
            if (NonNull_Pointer(geom_ptr)) {
              const Base::txFloat geom_len = geom_ptr->GetLength();
              if (geom_len > offset) {
                RelocateTracker(nextLanePtr, timeMgr.TimeStamp());
                mLocation.DistanceAlongCurve() = offset;
              } else {
                RelocateTracker(nextLanePtr, timeMgr.TimeStamp());
                mLocation.DistanceAlongCurve() = 0.001;
                TrajInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS_NAME(laneUid, nextLanePtr->getTxLaneId())
                         << TX_VARS(offset) << TX_VARS(nextLanePtr->getLength());
              }
              // UpdateNextCrossRoadPathPlaneDirection(m_laneInfo.onLaneUid.roadId, m_laneInfo.onLaneUid.sectionId);//
            } else {
              LogWarn << " GetLaneInfoByUid is null."
                      << TX_VARS_NAME(laneUid, Utils::ToString(nextLanePtr->getTxLaneId()));
              StopVehicle(); /*Kill();*/
              return false;
            }
          } else {
            LogWarn << " not empty vector 'nextLanes' first element ptr is null.";
            StopVehicle(); /*Kill();*/
            return false;
          }
        } else {
          LogWarn << " LaneLink Size is Zero. "
                  << TX_VARS_NAME(cur_lane_uid, Utils::ToString(mLocation.tracker()->getCurrentLane()->getTxLaneId()));
          StopVehicle(); /*Kill();*/
          return false;
        }
      }
    }
  }
  mLocation.UpdateNewMovePointWithoutOffset();
  return true;
}

#if __TX_Mark__("IEnvPerception")

void SimPlanningVehicleElement::SearchSurroundingSignal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const Base::txFloat ego_signal_env_dist_square = FLAGS_ego_signal_env_dist * FLAGS_ego_signal_env_dist;
  const Base::txVec3& vehicleLaneDir = StableLaneDir();
  const Base::txVec2 vehicleLaneDir2D = Utils::Vec3_Vec2(vehicleLaneDir);
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txInt pVehicleId = Id();
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t& selfLaneInfo = StableLaneInfo();
  const Base::txVec3 vehiclePos3dOnLane = StablePositionOnLane().ToENU().ENU();
  const Base::txVec2 vehiclePos2dOnLane = Utils::Vec3_Vec2(vehiclePos3dOnLane);
  const Base::txFloat half_veh_length = selfVehicleLength * 0.5;

  for (const sim_msg::TrafficLight& refSignal : m_env_traffic.trafficlights()) {
    Coord::txWGS84 signalPos;
    signalPos.FromWGS84(refSignal.x(), refSignal.y());
    const Base::txVec2 signalPos2d = signalPos.ToENU().ENU2D();
    const Base::txFloat distSquare = (vehiclePos2dOnLane - signalPos2d).squaredNorm();
    if (distSquare < ego_signal_env_dist_square) {
      Base::txLaneUId egoOnLaneUid;
      if (selfLaneInfo.IsOnLane()) {
        egoOnLaneUid = selfLaneInfo.onLaneUid;
      } else {
        egoOnLaneUid = selfLaneInfo.onLinkToLaneUid;
      }

      for (const sim_msg::LaneUid& refLaneUid : refSignal.control_lanes()) {
        if (refLaneUid.tx_road_id() == egoOnLaneUid.roadId && refLaneUid.tx_section_id() == egoOnLaneUid.sectionId &&
            refLaneUid.tx_lane_id() == egoOnLaneUid.laneId) {
          Base::txVec2 vDir = signalPos2d - vehiclePos2dOnLane;
          const Base::txFloat curDistance = vDir.norm();
          vDir.normalize();
          const Base::txFloat fAngle = Math::VectorRadian2D(vehicleLaneDir2D, vDir);
          const Base::txFloat distance_sub_half_length = curDistance * sin(Math::PI * 0.5 - fAngle) - half_veh_length;
          TX_MARK("One signal controls multiple lanes.");
          if (distance_sub_half_length < env_signal_dist()) {
            m_signal_ptr = std::make_shared<txEgoSignalInfo>();
            m_signal_ptr->Initialize(refSignal.id(),
                                     txEgoSignalInfo::SIGN_LIGHT_COLOR_TYPE::_from_integral(refSignal.color()));
            mDistanceToTargetSignLight = distance_sub_half_length;
          }
        }
      }
    }
  }
  EnvPerceptionInfo << TX_VARS(env_signal_dist())
                    << (NonNull_Pointer(env_signal_ptr()) ? (env_signal_ptr()->Str()) : ("NULL"));
}

void SimPlanningVehicleElement::CheckRouteEnd(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  clear_route_end();
  if (CallSucc(mRouteAI.IsValid())) {
    const hadmap::txPoint& EndPt = mRouteAI.EndPt();

    const Base::txFloat ego_route_end_dist_square = FLAGS_ego_signal_env_dist * FLAGS_ego_signal_env_dist;
    const Base::txVec3& vehicleLaneDir = StableLaneDir();
    const Base::txVec2 vehicleLaneDir2D = Utils::Vec3_Vec2(vehicleLaneDir);
    const Base::txFloat selfVehicleLength = GetLength();
    const Base::Info_Lane_t& selfLaneInfo = StableLaneInfo();
    const Base::txVec3 vehiclePos3dOnLane = StablePositionOnLane().ToENU().ENU();
    const Base::txVec2 vehiclePos2dOnLane = Utils::Vec3_Vec2(vehiclePos3dOnLane);
    const Base::txFloat half_veh_length = selfVehicleLength * 0.5;

    Coord::txWGS84 routeEndPos;
    routeEndPos.FromWGS84(EndPt.x, EndPt.y);
    const Base::txVec2 routeEndPos2d = routeEndPos.ToENU().ENU2D();
    const Base::txFloat distSquare = (vehiclePos2dOnLane - routeEndPos2d).squaredNorm();

    if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(mRouteAI.GetDeadLineMgr().GetDeadLineDir()),
                                       Utils::Vec3_Vec2(vehicleLaneDir))) &&
        (distSquare < ego_route_end_dist_square)) {
      // if (distSquare < ego_route_end_dist_square)
      Base::txVec2 vDir = routeEndPos2d - vehiclePos2dOnLane;
      const Base::txFloat curDistance = vDir.norm();
      vDir.normalize();
      const Base::txFloat fAngle = Math::VectorRadian2D(vehicleLaneDir2D, vDir);
      const Base::txFloat distance_sub_half_length = curDistance * sin(Math::PI * 0.5 - fAngle) - half_veh_length;
      TX_MARK("One signal controls multiple lanes.");
      if (distance_sub_half_length < route_end_dist()) {
        mDistanceToRouteEnd = distance_sub_half_length;
      }
    }
  }
}

void SimPlanningVehicleElement::SearchSurroundingObstacle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  /*const Base::txFloat ego_obs_env_dist_square = FLAGS_ego_obs_env_dist * FLAGS_ego_obs_env_dist;*/
}

void SimPlanningVehicleElement::Search_Follow_Front(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  Clear_Follow_Info();
  Set_Follow_Info(env_elemPtr(Ego_NearestObject::nFront), env_dist(Ego_NearestObject::nFront));
#  if __TX_Mark__("current lane front hash node.")
  const HashedLaneInfo& selfStableHashedLaneInfo = StableHashedLaneInfo();
  UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
  HashedLaneInfoOrthogonalListPtr center_node_ptr = Weak2SharedPtr(std::get<1>(m_HashedRoadInfoCache));
  HashedLaneInfoOrthogonalList::SearchOrthogonalListFront(center_node_ptr, FLAGS_ego_car_env_dist,
                                                          m_follow_front_hashnode);
#  endif /*__TX_Mark__("current lane front hash node.")*/

#  if __TX_Mark__("left/right lane front hash node.")
  if (IsInTurnLeft()) {
    const HashedLaneInfo leftHashedLaneInfo = mLocation.GetLeftHashedLaneInfo(Id());
    const HashedLaneInfoOrthogonalListPtr& left_node_ptr =
        HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(leftHashedLaneInfo);
    HashedLaneInfoOrthogonalList::SearchOrthogonalListFront(left_node_ptr, FLAGS_ego_car_env_dist,
                                                            m_follow_front_hashnode);
  } else if (IsInTurnRight()) {
    const HashedLaneInfo rightHashedLaneInfo = mLocation.GetRightHashedLaneInfo(Id());
    const HashedLaneInfoOrthogonalListPtr& right_node_ptr =
        HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(rightHashedLaneInfo);
    HashedLaneInfoOrthogonalList::SearchOrthogonalListFront(right_node_ptr, FLAGS_ego_car_env_dist,
                                                            m_follow_front_hashnode);
  }
  if (FLAGS_LogLevel_Ego_Front) {
    for (const auto& ref_front_hashed : m_follow_front_hashnode) {
      LOG(INFO) << "[front_follow][front_hash_node]" << ref_front_hashed.first;
    }
  }
#  endif /*__TX_Mark__("left/right lane front hash node.")*/
  const Base::txVec3 egoPos3d = StableGeomCenter().ENU();
  const Base::txVec2 egoPos2d = Utils::Vec3_Vec2(egoPos3d);
  const Base::txVec3& egoLaneDir = StableLaneDir();
  const Base::txFloat selfEgoLength = GetLength();
  const Base::txFloat selfEgoWidth = GetWidth();
  const Base::Info_Lane_t selfLaneInfo = StableLaneInfo();

  Base::txMat2 transMat, tranMatInv;
  const Unit::txDegree ego_lane_degree = Utils::GetLaneAngleFromVectorOnENU(egoLaneDir);
  Utils::CreateLocalCoordTranslationMat(egoPos2d, ego_lane_degree, transMat);
  tranMatInv = transMat.inverse();

  HdMap::HadmapCacheConCurrent::lanelinkExpandVisionType::mapped_type res_expand_vision;
  HdMap::HadmapCacheConCurrent::CheckExpandVision(selfLaneInfo, res_expand_vision);

  const std::vector<txEgoEnvVehicleInfoPtr>& cur_surround_element = m_surround_element_circle;
  for (const auto& elemPtr : cur_surround_element) {
    if (NonNull_Pointer(elemPtr)) {
      Coord::txWGS84 otherCarPos = elemPtr->StableGeomCenter().ToWGS84();
      const Base::txVec2 otherCarPos2d = otherCarPos.ToENU().ENU2D();
      const Base::txFloat distSquare = (egoPos2d - otherCarPos2d).squaredNorm();

      const txFloat dist = sqrtf(distSquare) - (selfEgoLength + elemPtr->StableLength()) * 0.5;
      const Base::txVec3 otherVehicleLaneDir = elemPtr->StableLaneDir();
      TX_MARK("support invert driving.");

      const Base::txVec2& local_obs_center = (otherCarPos2d - egoPos2d).transpose() * tranMatInv;
      const Base::txBool other_in_front = local_obs_center.y() > 0.0;
      FrontFollowInfo << "[front_follow][surround_element]" << TX_VARS(elemPtr->ConsistencyId()) << TX_VARS(dist)
                      << TX_COND_NAME(front_of_ego, other_in_front) << TX_VARS(elemPtr->StableHashedLaneInfo())
                      << TX_VARS(FollowFrontDistance());
      if (IsOnLane()) {
        if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoLaneDir), Utils::Vec3_Vec2(otherVehicleLaneDir)))) {
          if (_Contain_(m_follow_front_hashnode, elemPtr->StableHashedLaneInfo()) && CallSucc(other_in_front) &&
              dist < FollowFrontDistance()) {
            Set_Follow_Info(elemPtr, dist);
          }
        }
      } else if (IsOnLaneLink()) {
        if (_NonEmpty_(res_expand_vision) && CallSucc(elemPtr->StableLaneInfo().IsOnLaneLink()) &&
            _Contain_(res_expand_vision, (elemPtr->StableLaneInfo()))) {
          const auto& expandDist_lanelinkLength = res_expand_vision.at((elemPtr->StableLaneInfo()));
          if ((elemPtr->StableS() + std::get<0>(expandDist_lanelinkLength)) >
              (std::get<1>(expandDist_lanelinkLength))) {
            Set_Follow_Info(elemPtr, 0.0);
            return;
          } else {
            /*LOG(WARNING) << TX_VARS(Id()) << TX_VARS(vehiclePtr->Id())
                << TX_VARS(vehiclePtr->DistanceAlongCurve())
                << TX_VARS(std::get<0>(expandDist_lanelinkLength))
                << TX_VARS(std::get<1>(expandDist_lanelinkLength));*/
          }
        }

        if (_Contain_(m_follow_front_hashnode, elemPtr->StableHashedLaneInfo()) && CallSucc(other_in_front) &&
            dist < FollowFrontDistance()) {
          Set_Follow_Info(elemPtr, dist);
          FrontFollowInfo << "[front_follow][nearest][OOOO] Set_Follow_Info : " << TX_VARS(elemPtr->ConsistencyId())
                          << TX_VARS(dist);
        } else {
          FrontFollowInfo << "[front_follow][nearest][XXXX] " << TX_VARS(elemPtr->ConsistencyId()) << TX_VARS(dist)
                          << TX_VARS(FollowFrontDistance()) << TX_COND(other_in_front);
        }

        const Base::Info_Lane_t& otherLaneInfo = elemPtr->StableLaneInfo();
        if (CallSucc(otherLaneInfo.IsOnLaneLink()) && CallFail(otherLaneInfo == selfLaneInfo)) {
          const txBool bSameDst = (otherLaneInfo.onLinkToLaneUid == selfLaneInfo.onLinkToLaneUid);
          const txBool bSameSrc = (otherLaneInfo.onLinkFromLaneUid == selfLaneInfo.onLinkFromLaneUid);
          if (CallSucc(bSameDst) || CallSucc(bSameSrc TX_MARK("from same link start, but different link end"))) {
            const Base::txVec2& nearestElementCenter2d = otherCarPos2d;
            const Base::txFloat curGap = (selfEgoLength + elemPtr->StableLength()) * 0.5;
            TX_MARK("SIM-6412");
            const Base::txFloat curDistance = dist;
            if (curDistance < FollowFrontDistance() &&
                ((curDistance < FLAGS_default_lane_width) || (elemPtr->StableInvertS() < InvertDistanceAlongCurve()))) {
              TX_MARK("Put it last because of the large amount of calculation");
              Base::txFloat yaw = 0.0, distancePedal = 0.0, distanceCurve = 0.0;
              const auto& other_gps = otherCarPos; /*pOtherVehicle->StablePosition().ToWGS84();*/
              mLocation.tracker()->getCurrentCurve()->xy2sl(other_gps.Lon(), other_gps.Lat(), distanceCurve,
                                                            distancePedal, yaw);
              const txFloat nudgeWidth =
                  0.5 * selfEgoWidth + 0.5 * (elemPtr->StableWidth()) + FLAGS_lanelink_nudge_delta;
              if (fabs(distancePedal) <= nudgeWidth && curDistance < FollowFrontDistance()) {
                Set_Follow_Info(elemPtr, curDistance);
              }
            }
          }
        }
      }
    }
  }
}

void SimPlanningVehicleElement::SearchSurroundingPedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const Base::txFloat ego_pede_env_dist_square = FLAGS_ego_pede_env_dist * FLAGS_ego_pede_env_dist;
  m_NearestDynamic.clear();
  const Base::txVec3& vehicleLaneDir = StableLaneDir();
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txInt pVehicleId = Id();
  const Base::txVec3& vPos = StableGeomCenter().ENU();
  const Base::txVec2 ego_pos_2d = StableGeomCenter().ENU2D();
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t& selfLaneInfo = StableLaneInfo();
  // return;

  static std::map<Base::Enums::PedestrianTargetDirectionType, Base::Enums::NearestElementDirection> map_pede2vehicle{
      {(+Base::Enums::PedestrianTargetDirectionType::eFrontCenter), (+Base::Enums::NearestElementDirection::eFront)},
      {(+Base::Enums::PedestrianTargetDirectionType::eFrontLeft), (+Base::Enums::NearestElementDirection::eLeftFront)},
      {(+Base::Enums::PedestrianTargetDirectionType::eFrontRight),
       (+Base::Enums::NearestElementDirection::eRightFront)},
      {(+Base::Enums::PedestrianTargetDirectionType::eLeft), (+Base::Enums::NearestElementDirection::eLeft)},
      {(+Base::Enums::PedestrianTargetDirectionType::eRight), (+Base::Enums::NearestElementDirection::eRight)}};
  /*eLeft = 5, eRight = 6*/

  for (const auto& refDynamicObs : m_env_traffic.dynamicobstacles()) {
    txEgoDynamicObsInfo other_dynamic_obs(refDynamicObs);
    Coord::txWGS84 other_dynamic_obs_pos = other_dynamic_obs.Position();
    const Base::txVec2 other_dynamic_obs_pos_2d = other_dynamic_obs_pos.ToENU().ENU2D();
    const Base::txFloat distSquare = (ego_pos_2d - other_dynamic_obs_pos_2d).squaredNorm();
    EnvPerceptionInfo << TX_VARS(refDynamicObs.id()) << TX_VARS(refDynamicObs.type()) << TX_COND(Ignore(refDynamicObs))
                      << TX_VARS(distSquare) << TX_VARS(ego_pede_env_dist_square);
    if (CallFail(Ignore(refDynamicObs)) && distSquare < ego_pede_env_dist_square) {
      txBool loop = true;
      for (Base::txSize j_dir = 2; j_dir < (5 + 2) && loop; ++j_dir) {
        const auto curPedeDirType = Base::Enums::PedestrianTargetDirectionType::_from_integral(j_dir);
        const auto curVehicleDirType = map_pede2vehicle.at(curPedeDirType);

        std::vector<Base::txVec2> localPts_clockwise_close;
        Get_Region_Clockwise_Close_By_Enum(curVehicleDirType, localPts_clockwise_close);
        if (CallSucc(Utils::PointInPolygon2D(other_dynamic_obs_pos_2d, localPts_clockwise_close))) {
          loop = false;
          EnvPerceptionInfo << TX_VARS(other_dynamic_obs.DynamicObsId()) << " PointInPolygon2D success."
                            << TX_VARS_NAME(dir, curVehicleDirType._to_string());

          const Base::txVec2 nearestElementCenter2d = other_dynamic_obs_pos_2d;
          const Base::txFloat curGap = (other_dynamic_obs.Length() + selfVehicleLength) * 0.5;
          const Base::txFloat curDistance = ((ego_pos_2d) - (nearestElementCenter2d)).norm() - curGap;

          if (curDistance < m_NearestDynamic.dist(j_dir)) {
            m_NearestDynamic.dist(j_dir) = curDistance;
            m_NearestDynamic.elemPtr(j_dir) = other_dynamic_obs;
          }
        }
      }
    }
  }
}

void SimPlanningVehicleElement::SearchNextCrossVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {}

void SimPlanningVehicleElement::SearchSurroundingVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  m_EgoNearestObject.clear();
  TX_MARK("must clear last frame result.");
  const Base::txVec3& vehicleLaneDir = StableLaneDir();
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txInt pVehicleId = Id();
  const Base::txVec3& vPos = StableGeomCenter().ENU();
  const Base::txVec2 ego_pos_2d = StableGeomCenter().ENU2D();
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t& selfLaneInfo = StableLaneInfo();
  const std::array<Base::txVec3, Ego_NearestObject::N_Dir>& vTargetPos = ComputeTargetPos(vPos, vehicleLaneDir);

  const std::vector<txEgoEnvVehicleInfoPtr>& cur_surround_element = m_surround_element_circle;
  for (const auto& elemPtr : cur_surround_element) {
    if (NonNull_Pointer(elemPtr)) {
      Coord::txWGS84 otherCarPos = elemPtr->StableGeomCenter().ToWGS84();
      const Base::txVec2 otherCarPos2d = otherCarPos.ToENU().ENU2D();
      const Base::txFloat distSquare = (ego_pos_2d - otherCarPos2d).squaredNorm();
      const Base::txSysId otherElemId = elemPtr->ConsistencyId();

      const Base::txVec3 otherVehicleLaneDir = elemPtr->StableLaneDir();
      if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(vehicleLaneDir), Utils::Vec3_Vec2(otherVehicleLaneDir)))) {
        EnvPerceptionInfo << TX_VARS(otherElemId) << " IsSynchronicity success.";
        for (Base::txSize j_dir = 0; j_dir < Ego_NearestObject::N_Dir; ++j_dir) {
          std::vector<Base::txVec2> localPts_clockwise_close;
          localPts_clockwise_close.reserve(10);
          Get_Region_Clockwise_Close_By_Enum(Base::Enums::NearestElementDirection::_from_index(j_dir),
                                             localPts_clockwise_close);
          if (CallSucc(Utils::PointInPolygon2D(otherCarPos2d, localPts_clockwise_close))) {
            EnvPerceptionInfo << TX_VARS(otherElemId) << " PointInPolygon2D success."
                              << TX_VARS_NAME(dir,
                                              Base::Enums::NearestElementDirection::_from_integral(j_dir)._to_string());
            const Base::txVec3& vTraget = vTargetPos[j_dir];
            const Base::txVec2& nearestElementCenter2d = otherCarPos2d;
            const Base::txFloat curGap = (elemPtr->StableLength() + selfVehicleLength) * 0.5;
            const Base::txFloat curDistance =
                ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
            if (curDistance < m_EgoNearestObject.dist(j_dir)) {
              m_EgoNearestObject.dist(j_dir) = curDistance;
              m_EgoNearestObject.setElemPtr(j_dir, elemPtr);
            }
          }
        }
      } else {
        EnvPerceptionInfo << TX_VARS(otherElemId) << " IsSynchronicity failure.";
      }
    }
  }
  EnvPerceptionInfo << TX_VARS(timeMgr.PassTime()) << m_EgoNearestObject.Str();
}
#  if USE_SUDOKU_GRID
Base::Info_NearestObject SimPlanningVehicleElement::SearchSurroundingVehicle_RTree(
    const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  return Base::Info_NearestObject();
}

#  endif /*USE_SUDOKU_GRID*/

Base::Info_NearestObject SimPlanningVehicleElement::SearchSurroundingVehicle_HLane(
    const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  return Base::Info_NearestObject();
}

Base::txBool SimPlanningVehicleElement::Ignore(const sim_msg::StaticObstacle& refObs) const TX_NOEXCEPT {
  static constexpr auto lower_bound =
      __enum2int__(STATIC_ELEMENT_TYPE, _plus_(Base::Enums::STATIC_ELEMENT_TYPE::J_001));
  static constexpr auto upper_bound =
      __enum2int__(STATIC_ELEMENT_TYPE, _plus_(Base::Enums::STATIC_ELEMENT_TYPE::J_015));
  if (refObs.type() >= lower_bound && refObs.type() <= upper_bound) {
    return true;
  } else {
    return false;
  }
}

Base::txBool SimPlanningVehicleElement::Ignore(const sim_msg::DynamicObstacle& refPed) const TX_NOEXCEPT {
  static constexpr auto lower_bound =
      __enum2int__(PEDESTRIAN_TYPE, _plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_001));
  static constexpr auto upper_bound =
      __enum2int__(PEDESTRIAN_TYPE, _plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_004));
  if (refPed.type() >= lower_bound && refPed.type() <= upper_bound) {
    return true;
  } else {
    return false;
  }
}

void SimPlanningVehicleElement::ExecuteEnvPerception(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const Base::txFloat ego_car_env_dist_square = FLAGS_ego_car_env_dist * FLAGS_ego_car_env_dist;
  const Base::txFloat ego_obs_env_dist_square = FLAGS_ego_obs_env_dist * FLAGS_ego_obs_env_dist;
  const Base::txVec2 ego_pos_2d = StableGeomCenter().ENU2D();

  m_surround_element_circle.clear();
  for (const sim_msg::Car& refCar : m_env_traffic.cars()) {
    Coord::txWGS84 otherCarPos;
    otherCarPos.FromWGS84(refCar.x(), refCar.y());
    const Base::txVec2 otherCarPos2d = otherCarPos.ToENU().ENU2D();
    const Base::txFloat distSquare = (ego_pos_2d - otherCarPos2d).squaredNorm();
    if (CallFail(Ignore(refCar)) && distSquare < ego_car_env_dist_square) {
      auto elemPtr = txEgoEnvVehicleInfo_Factory::GetEnvVehicleInfoPtr(refCar);
      if (NonNull_Pointer(elemPtr)) {
        m_surround_element_circle.emplace_back(elemPtr);
        EnvPerceptionInfo << "[success]" << TX_VARS_NAME(Idx, m_surround_element_circle.size()) << TX_VARS(refCar.id())
                          << TX_VARS(distSquare) << TX_VARS(ego_car_env_dist_square);
      }
    }
  }

  for (const sim_msg::StaticObstacle& refObs : m_env_traffic.staticobstacles()) {
    Coord::txWGS84 otherObsPos;
    otherObsPos.FromWGS84(refObs.x(), refObs.y());
    const Base::txVec2 otherObsPos2d = otherObsPos.ToENU().ENU2D();
    const Base::txFloat distSquare = (ego_pos_2d - otherObsPos2d).squaredNorm();
    // LOG(INFO) << TX_VARS(refObs.id()) << TX_VARS(refObs.type())<< TX_COND(Ignore(refObs)) << TX_VARS(distSquare) <<
    // TX_VARS(ego_obs_env_dist_square);
    if (CallSucc(refObs.id() >= 0) && CallFail(Ignore(refObs)) && distSquare < ego_obs_env_dist_square) {
      auto elemPtr = txEgoEnvVehicleInfo_Factory::GetEnvObstacleInfoPtr(refObs);
      if (NonNull_Pointer(elemPtr)) {
        m_surround_element_circle.emplace_back(elemPtr);
        EnvPerceptionInfo << "[success]" << TX_VARS_NAME(Idx, m_surround_element_circle.size()) << TX_VARS(refObs.id())
                          << TX_VARS(distSquare) << TX_VARS(ego_obs_env_dist_square);
      }
    }
  }
  EnvPerceptionInfo << timeMgr << TX_VARS(m_surround_element_circle.size());
  SearchSurroundingVehicle(timeMgr);
  Search_Follow_Front(timeMgr);
  SearchSurroundingPedestrian(timeMgr);
  SearchSurroundingObstacle(timeMgr);
  SearchSurroundingSignal(timeMgr);
  CheckRouteEnd(timeMgr);
}

void SimPlanningVehicleElement::ConfigureFocusTypes() TX_NOEXCEPT {}
#endif /*__TX_Mark__("IEnvPerception")*/

SimPlanningVehicleElement::VehicleMoveLaneState SimPlanningVehicleElement::GetSwitchLaneState() const TX_NOEXCEPT {
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

Base::txBool SimPlanningVehicleElement::Release() TX_NOEXCEPT {
  mLifeCycle.SetEnd();
  return true;
}

void SimPlanningVehicleElement::CheckWayPoint() TX_NOEXCEPT {
  TrafficFlow::Component::RouteAI::RouteAI_EdgeData hint;
  if (CallSucc(mRouteAI.IsReachable(mLocation.LaneLocInfo(), hint))) {
    RouteLogInfo << "The vehicle can reach the destination." << TX_VARS_NAME(VehicleId, Id())
                 << TX_VARS_NAME(reachable_uid, mLocation.LaneLocInfo()) << TX_COND(hint.NeedStraight())
                 << TX_COND(hint.NeedTurnLeft()) << TX_COND(hint.NeedTurnRight());
    if (CallSucc(hint.NeedStraight())) {
      mRouteAI.SetTrendOff();
    } else {
      mRouteAI.SetTrendOn();
    }

  } else {
    RouteLogInfo << "The vehicle cannot reach the destination." << TX_VARS_NAME(VehicleId, Id());
    if (+VehicleMoveLaneState::eStraight == GetSwitchLaneState()) {
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

Base::txBool SimPlanningVehicleElement::IsUnderLaneKeep() const TX_NOEXCEPT {
  if (+VehicleMoveLaneState::eStraight == GetSwitchLaneState() && CallFail(mLaneKeepMgr.Expired())) {
    return true;
  } else {
    return false;
  }
}

Base::txBool SimPlanningVehicleElement::Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallSucc(IsAlive()) && CallFail(IsStop())) {
    if (FLAGS_ego_use_activate) {
      UpdateActivateMode(pb2enum(m_pb_vehicle_interaction.activate()));
    }

    LogInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS(Id()) << TX_VARS_NAME(AutoMode, AutoMode()._to_string());
    if (FLAGS_LogLevel_Ego_EnvPerception) {
      LOG(INFO) << "[Surrounding]" << timeMgr << TX_VARS(Id()) << m_EgoNearestObject.Str() << TX_VARS(StableLaneInfo())
                << TX_VARS(mLocation.LaneOffset());
      /*LOG(INFO) << "[Surrounding_follow]" << timeMgr << TX_VARS(Id())
          << TX_VARS_NAME(front_element_id,
         ((NonNull_Pointer(FollowFrontPtr()))?(FollowFrontPtr()->ConsistencyId()):(-1)))
          << TX_VARS_NAME(front_element_dist, FollowFrontDistance())
          << TX_VARS_NAME(GeomCenter, StableGeomCenter().ToWGS84().StrWGS84())
          << TX_VARS(StableLaneInfo());*/
    }

    LogInfo << "[Surrounding_follow]" << timeMgr << TX_VARS(Id())
            << TX_VARS_NAME(front_element_id,
                            ((NonNull_Pointer(FollowFrontPtr())) ? (FollowFrontPtr()->ConsistencyId()) : (-1)))
            << TX_VARS_NAME(front_element_dist, FollowFrontDistance())
            << TX_VARS_NAME(GeomCenter, StableGeomCenter().ToWGS84().StrWGS84()) << TX_VARS(StableLaneInfo())
            << TX_VARS(DistanceAlongCurve());

    if (CallFail(Need_Autopilot_Activate())) {
      SetPlanningVelocity(0.0);
      mKinetics.m_acceleration = 0.0;
      mKinetics.m_LateralVelocity = 0.0;
      return true;
    }
    CheckWayPoint();
    UpdateNextCrossDirection();
    if (FLAGS_planning_const_velocity_mode) {
      mKinetics.m_acceleration = 0.0;
      mKinetics.m_LateralVelocity = 0.0;
      LOG(INFO) << "[CONST_VELOCITY]" << TX_VARS(mKinetics.m_acceleration) << TX_VARS(mKinetics.m_LateralVelocity);
    } else {
      if (CallSucc(Need_Longitudinal_Activate())) {
        mKinetics.m_acceleration = DrivingFollow(timeMgr);
        /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_acceleration);*/
        // if (CallFail(CheckPedestrianWalking(timeMgr.RelativeTime(), mKinetics.m_LateralVelocity)))
        CheckPedestrianWalking(timeMgr.RelativeTime(), mKinetics.m_LateralVelocity);
        /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_acceleration);*/
        if (NonNull_Pointer(env_signal_ptr())) {
          const txFloat signal_dist = env_signal_dist();
          const auto signal_color = env_signal_ptr()->GetCurrentSignLightColorType();
          const txSysId signalId = env_signal_ptr()->Id();
          LogInfo << TX_VARS(Id()) << "[signal] " << TX_VARS(signalId) << TX_VARS(signal_dist)
                  << TX_VARS_NAME(Color, signal_color._to_string());
#if 1
          if (signal_dist < FollowFrontDistance()) {
            LogInfo << "[signal][succes] " << TX_VARS(timeMgr.PassTime()) << TX_VARS(signal_dist)
                    << TX_VARS(FollowFrontDistance());
            mKinetics.m_acceleration = CheckSignLight(signalId, signal_dist, signal_color);
          } else {
            LogInfo << "[signal][ignore] " << TX_VARS(timeMgr.PassTime()) << TX_VARS(signal_dist)
                    << TX_VARS(FollowFrontDistance());
          }
#else
          mKinetics.m_acceleration = CheckSignLight(signalId, signal_dist, signal_color);
#endif
        }
        /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_acceleration) << TX_VARS(StableVelocity());*/

        // if (route_end_dist() < FollowFrontDistance())
        if (CallSucc(FLAGS_route_end_behavior_force_stop) ||
            (CallFail(FLAGS_route_end_behavior_force_stop) && CallSucc(FLAGS_route_end_behavior_desired_stop))) {
          mKinetics.m_acceleration = CheckRouteEnd(route_end_dist());
        }
        LogInfo << TX_VARS(route_end_dist()) << TX_VARS(SignalReactionGap()) << TX_VARS(mKinetics.m_acceleration);
        mKinetics.LimitAcceleration();
        SetPlanningVelocity(StableVelocity() + mKinetics.m_acceleration * timeMgr.RelativeTime());
      } else {
        mKinetics.m_acceleration = 0.0;
        SetPlanningVelocity(0.0);
      }

      if (CallSucc(Need_Lateral_Activate())) {
        ChangeLaneLogInfo << TX_VARS(mEventChangeLane);
        mKinetics.m_LateralVelocity = DrivingChangeLane(timeMgr);
        mEventChangeLane = 0;
        ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(mKinetics.m_LateralVelocity);
      } else {
        mKinetics.m_LateralVelocity = 0.0;
      }
    }

#if 0
    if (GetVelocity() <= 0.0) {
      mKinetics.m_velocity = 0.0;
      Compute_Displacement(timeMgr);
      mLocation.vPos() =
          ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(), mLocation.LaneOffset());
    }  // else
#endif
    {
      /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_velocity);*/
      mKinetics.LimitVelocity();
      LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_velocity) << " from_LimitVelocity";
      if (MoveStraightOnS(timeMgr)) {
        mKinetics.LimitVelocity();
        ComputeRealAcc(timeMgr);
        TX_MARK("move to Component::Location, pImpl->NormalTargetRotateAngle();");
        LaneChangePostOp(timeMgr);
        mGeometryData.ComputePolygon(StableGeomCenter(), mLocation.vLaneDir());
        return true;
      } else {
        StopVehicle();
        LogWarn << TX_VARS_NAME(vehicle_id, Id()) << " MoveStraightOnS failure. stop!";
        return false;
      }
    }
    return true;
  } else {
    LogWarn << "vehicle is not update. " << TX_VARS(Id()) << TX_COND_NAME(IsAlive, IsAlive())
            << TX_COND_NAME(IsStop, IsStop());
    return (IsStop());
  }
}

Base::txFloat SimPlanningVehicleElement::CheckRouteEnd(const txFloat distance) TX_NOEXCEPT {
  txFloat acceleration = mKinetics.m_acceleration;
  txFloat SlowingDownDec = MAX_SPEED;

  if ((distance >= 0.0) && (distance < SignalReactionGap())) {
    SlowingDownDec = Deceleration(distance, StableVelocity(), 0.5, Max_Deceleration);
  }
  // LogWarn << TX_VARS(distance) << TX_VARS(SignalReactionGap()) << TX_VARS(StableVelocity())<< TX_VARS(SlowingDownDec)
  // << TX_VARS(acceleration);
  acceleration = fmin(acceleration, SlowingDownDec);
  return acceleration;
}

Base::txFloat SimPlanningVehicleElement::CheckSignLight(const txSysId signalId, const txFloat distance,
                                                        const SIGN_LIGHT_COLOR_TYPE lightType) TX_NOEXCEPT {
  /*TX_MARK("distance这个数值，如果已经跨过了停止线，则是车头到线的负数距离，即，如果是负数，则证明已经过线了"); */
  txFloat acceleration = mKinetics.m_acceleration;
  txFloat SlowingDownDec = MAX_SPEED;
  const txBool b20201212_trick = true;

  if ((distance >= 0.0) && (distance < SignalReactionGap()) &&  // 1230 HD
      (+SIGN_LIGHT_COLOR_TYPE::eRed == lightType) && (b20201212_trick || CallFail(IsTurnRightAtNextIntersection()))) {
    SlowingDownDec = Deceleration(distance, StableVelocity(), 0.5,
                                  Max_Deceleration);  // 逐渐减速到0,以固定的减速度减速,在信号灯停止线前0.5米处停下来
    LogInfo << TX_VARS(Id()) << TX_VARS(distance) << TX_VARS(lightType) << TX_VARS(SlowingDownDec)
            << TX_VARS(acceleration) << TX_VARS(signalId);
  }
  // if ((distance < SignalReactionGap()) && (lightType == SIGN_LIGHT_COLOR_YELLOW) &&
  // (m_nextCrossRoadPathPlaneDirection != TRAFFIC_VEHICLE_CROSS_ROAD_RIGHT_SIDE))//黄灯时候的反应
  if ((distance < SignalReactionGap()) && (+SIGN_LIGHT_COLOR_TYPE::eYellow == lightType) &&
      (b20201212_trick || CallFail(IsTurnRightAtNextIntersection()))) {  // 黄灯时候的反应
    if ((StableVelocity() * 2) <
        distance) {  // 如果车辆以当前速度在两秒钟内行驶的距离比这个当前与信号灯之间的距离要短，则减速
      SlowingDownDec =
          Deceleration(distance, StableVelocity(), 0.5, Max_Deceleration);  // 逐渐减速到0,以固定的减速度减速
    } else if (((-0.5 * StableVelocity() * StableVelocity()) / Max_Comfort_Deceleration) > distance) {
      TX_MARK("//v1t+(1/2)at2//t=(v2-v1)/MaxA//以最大舒适加速度仍然无法停住，则选择不减速 ");
      SlowingDownDec = mKinetics.m_acceleration + 0.5;  // 稍微加速闯过去,只是借用SlowingDownDec这个变量名
    } else if (m_aggress <= Yielding_Aggressive_Threshold) {
      /*TX_MARK("不属于上述两种情形，则取决于车辆的aggressiveness, aggressive越大，越不愿意减速停车");*/
      SlowingDownDec =
          Deceleration(distance, StableVelocity(), 0.5, Max_Deceleration);  // 逐渐减速到0,以固定的减速度减速
    }
  }

  acceleration = fmin(acceleration, SlowingDownDec);  // 取跟车计算的加速度和上面加速度的最小值来进行减速

  // ////////////////////1230 HD/////////////////////////////////////////////////////

  Base::IVehicleElementPtr pOpposingVeh = std::get<0>(OpposeSideVeh_NextCross());
  Base::IVehicleElementPtr pLeftApproachingVeh = std::get<0>(LeftSideVeh_NextCross());
  // ////////////////////////////////////////////Haining Du
  // char szText1[512];
  // sprintf(szText1, "OwnDistance: %lf", distance);
  // MessageBoxA(nullptr, szText1, "", 0);
  // ////////////////////////////////////////////
  // if (fabs(distance - 999999)<1.0)//此时车已经彻底离开直路部分，进入路口的link部分
  // 999999是在TrafficVehcicleData.h里面定义的MAX_DISTANCE
  // {
  //  distance = 0.0;//则置0，仅用WaitingBuffer_LeftTurn和WaitingBuffer_RightTurn来考虑
  // }

  acceleration = TurnLeft_vs_GoStraight(pOpposingVeh, distance, acceleration); /*TX_MARK("左转让直行");*/

  acceleration = TurnRight_vs_TurnLeft_GoStraight(pOpposingVeh, pLeftApproachingVeh, distance,
                                                  acceleration); /*TX_MARK("右转让左转和从左侧道路驶来的直行车")*/

  acceleration = fmin(acceleration, SlowingDownDec);

  return acceleration;
}

Base::txFloat SimPlanningVehicleElement::TurnLeft_vs_GoStraight(const Base::IVehicleElementPtr pOpposingVeh,
                                                                const txFloat distance2signal,
                                                                const txFloat acceleration) const TX_NOEXCEPT {
  return acceleration;
}

Base::txFloat SimPlanningVehicleElement::TurnRight_vs_TurnLeft_GoStraight(
    const Base::IVehicleElementPtr pOpposingVeh, const Base::IVehicleElementPtr pLeftApproachingVeh,
    const txFloat distance2signal, const txFloat acceleration) const TX_NOEXCEPT {
  return acceleration;
}

Base::txBool SimPlanningVehicleElement::CheckPedestrianWalking(const txFloat relative_time,
                                                               txFloat& refLateralVelocity) TX_NOEXCEPT {
  const auto front_center_idx = (+Base::Enums::PedestrianTargetDirectionType::eFrontCenter)._to_integral();
  const auto right_front_idx = (+Base::Enums::PedestrianTargetDirectionType::eFrontRight)._to_integral();
  const auto left_front_idx = (+Base::Enums::PedestrianTargetDirectionType::eFrontLeft)._to_integral();
  const auto left_idx = (+Base::Enums::PedestrianTargetDirectionType::eLeft)._to_integral();
  const auto right_idx = (+Base::Enums::PedestrianTargetDirectionType::eRight)._to_integral();
  Base::txFloat minDist = FLT_MAX;
  txEgoDynamicObsInfo dynamic_obj;
  if (minDist > m_NearestDynamic.dist(front_center_idx)) {
    minDist = m_NearestDynamic.dist(front_center_idx);
    dynamic_obj = m_NearestDynamic.elemPtr(front_center_idx);
  }

  if (minDist > m_NearestDynamic.dist(right_front_idx)) {
    minDist = m_NearestDynamic.dist(right_front_idx);
    dynamic_obj = m_NearestDynamic.elemPtr(right_front_idx);
  }

  if (minDist > m_NearestDynamic.dist(left_front_idx)) {
    minDist = m_NearestDynamic.dist(left_front_idx);
    dynamic_obj = m_NearestDynamic.elemPtr(left_front_idx);
  }

  if (minDist > m_NearestDynamic.dist(left_idx)) {
    minDist = m_NearestDynamic.dist(left_idx);
    dynamic_obj = m_NearestDynamic.elemPtr(left_idx);
  }

  if (minDist > m_NearestDynamic.dist(right_idx)) {
    minDist = m_NearestDynamic.dist(right_idx);
    dynamic_obj = m_NearestDynamic.elemPtr(right_idx);
  }

  if (dynamic_obj.IsValid()) {
    const Base::txFloat tmpSlowingDownDec = Deceleration(minDist, GetVelocity(), FLAGS_Deceleration_Safe_Distance,
                                                         Max_Deceleration);  // 逐渐减速到0,以固定的减速度减速
    mKinetics.m_acceleration = fmin(tmpSlowingDownDec, mKinetics.m_acceleration);
    EnvPerceptionInfo << TX_VARS(Id()) << TX_VARS(dynamic_obj.DynamicObsId()) << TX_VARS(minDist)
                      << TX_VARS(GetVelocity()) << TX_VARS(FLAGS_Deceleration_Safe_Distance)
                      << TX_VARS(Max_Deceleration)
                      << TX_VARS_NAME(Dec, (GetVelocity() * GetVelocity()) /
                                               (-2.0 * (minDist - FLAGS_Deceleration_Safe_Distance)))
                      << TX_VARS(tmpSlowingDownDec) << TX_VARS(mKinetics.m_acceleration);
    return true;
  } else {
    EnvPerceptionInfo << TX_VARS(Id()) << " front clear.";
    return false;
  }
}

void SimPlanningVehicleElement::UpdateLaneKeep(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (CallFail(mLaneKeepMgr.Expired())) {
    ChangeLaneLogInfo << "[LaneKeep]" << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.Duration())
                      << TX_VARS(timeMgr.AbsTime());
    mLaneKeepMgr.Decrement(timeMgr.RelativeTime());
  }
}

Base::txFloat SimPlanningVehicleElement::Driving_SwitchLaneOrg(
    const Base::TimeParamManager& timeMgr, Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base,
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
        mLaneKeepMgr.ResetCounter(LaneKeepTime(), m_aggress);
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
      DecelerateLaneChange(DecelActionSpeedFactor());
      ChangeLaneLogInfo << TX_VARS(Id()) << " under lane change.";
      Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
      Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
      Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
      Base::txFloat FrontGap_TargetLane = 999.0;
      Base::txFloat RearGap_TargetLane = 999.0;
      Base::txFloat SideGap_TargetLane = 999.0;

      std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) = FrontVeh_TargetLane();
      std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) = RearVeh_TargetLane();
      // 0205 HD 变量名称改变MAX_SPEED
      const Base::txFloat RearVeh_TargetLaneVelocity =
          (NonNull_Pointer(RearVeh_TargetLane_Base)) ? (RearVeh_TargetLane_Base->StableVelocity()) : MAX_SPEED;
      // float FrontGap_TargetLane = (FrontVeh_TargetLane) ? (VectorSub(FrontVeh_TargetLane->m_vPos, m_vPos).Magnitude()
      // - m_Data.length) : 100.0; const Base::txFloat FrontGap_TargetLane =
      // m_NearestObject.aroudDistance[Base::OBJECT_TARGET_FRONT_DIRECTION];

      // if ((RearGap_TargetLane < SafetyGap_Rear) || (FrontGap_TargetLane < SafetyGap_Front)/* ||
      // (RearVeh_TargetLan_Velocity > m_Data.velocity_comfort)*/) if (((RearGap_TargetLane < SafetyGap_Rear) ||
      // (FrontGap_TargetLane < SafetyGap_Front)) && (LaneChanging_Procedure > LaneChanging_Min_Cancel_Duration)) 0203
      // HD/////////////////////////////////////////////////////
      std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) = SideVeh_TargetLane();
      ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(FrontGap_TargetLane) << TX_VARS(RearGap_TargetLane)
                        << TX_VARS(m_LaneChangeTimeMgr.Procedure())
                        << TX_VARS(m_LaneChangeTimeMgr.Min_Cancel_Duration()) << TX_VARS(SideGap_TargetLane);
      // 0203 HD/////////////////////////////////////////////////////
      const PedestrianSearchType searchDirectionDir =
          (IsInTurnLeft()) ? (PedestrianSearchType::eLeft) : (PedestrianSearchType::eRight);
      const Base::txFloat SafetyGap_V_AdjWeight = 0.5;
      const Base::txFloat SafetyGap_Rear_Special =
          SafetyGap_Rear *
          pow(RearVeh_TargetLaneVelocity / (StableVelocity() + SpeedDenominatorOffset), SafetyGap_V_AdjWeight);
      if (((RearGap_TargetLane < SafetyGap_Rear_Special) || (FrontGap_TargetLane < SafetyGap_Front) ||
           (SideGap_TargetLane < SafetyGap_Side) ||
           (FrontPedWalkingInSight(searchDirectionDir, Max_Ped_Reaction_Distance) > 0)) &&
          (IsInTurn_Start())) {
        ChangeLaneLogInfo << TX_VARS(Id()) << "abort_lane_change  " << TX_VARS(RearGap_TargetLane)
                          << TX_VARS(SafetyGap_Rear_Special) << TX_VARS(FrontGap_TargetLane) << TX_VARS(SafetyGap_Front)
                          << TX_VARS(SideGap_TargetLane) << TX_VARS(SafetyGap_Side) << TX_VARS(State()._to_string());
        m_LaneAbortingTimeMgr.SetDuration(m_LaneChangeTimeMgr.Experience());
        m_LaneAbortingTimeMgr.SetProcedure(m_LaneAbortingTimeMgr.Duration());
        m_LaneChangeTimeMgr.SetProcedure(0.0);
        m_LaneChangeTimeMgr.SetDuration(0.0);
        SetTurnAbort();
        mLaneKeepMgr.ResetCounter(LaneKeepTime(), m_aggress);
        ChangeLaneLogInfo << TX_VARS(Id()) << " GetLaneKeepDuration " << TX_VARS(mLaneKeepMgr.Duration());
      } else {
        ChangeLaneLogInfo << TX_VARS(Id()) << "abort_lane_change_error  " << TX_VARS(RearGap_TargetLane)
                          << TX_VARS(SafetyGap_Rear_Special) << TX_VARS(FrontGap_TargetLane) << TX_VARS(SafetyGap_Front)
                          << TX_VARS(SideGap_TargetLane) << TX_VARS(SafetyGap_Side) << TX_VARS(State()._to_string());
      }

      if (IsInLaneChange_StartIng()) {
        resLateralVelocity = MoveTo_Sideway(timeMgr);
        ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS_NAME(MoveTo_Sideway_return, resLateralVelocity);
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
#if 1
      if (NonNull_Pointer(FollowFrontPtr()) && (FollowFrontDistance() < FrontGap_CurrentLane)) {
        ChangeLaneLogInfo << TX_VARS(Id()) << " Employ IDM Front Info for LaneChange : "
                          << TX_VARS_NAME(SUDOKU_GRID_FrontDist, FrontGap_CurrentLane)
                          << TX_VARS_NAME(IDM_FrontDist, FollowFrontDistance());
        FrontVeh_CurrentLane_Base = FollowFrontPtr();
        FrontGap_CurrentLane = FollowFrontDistance();
      }
#endif
      std::tie(RearVeh_CurrentLane_Base, RearGap_CurrentLane) = RearVeh_CurrentLane();

      const Base::txBool bCanSwitchToRightLane = ValidSampling(timeMgr) && CheckCanSwitchRightLaneState(timeMgr);
      Base::txBool bCanSwitchToLeftLane = ValidSampling(timeMgr) && CheckCanSwitchLeftLaneState(timeMgr);

#if 1
      const Base::txFloat RearVeh_CurrentLanePushVelocity =
          (NonNull_Pointer(RearVeh_CurrentLane_Base)) ? (RearVeh_CurrentLane_Base->StableVelocity()) : (0.0);
      if ((RearGap_CurrentLane < ComfortGap) && (RearVeh_CurrentLanePushVelocity > mKinetics.velocity_comfort)) {
        /*if (FLAGS_EnableRearComfortGap)*/
        {
          ChangeLaneLogInfo << "mLaneKeepMgr.Clear() : " << TX_VARS(Id()) << TX_VARS(RearGap_CurrentLane)
                            << TX_VARS(ComfortGap) << TX_VARS(RearVeh_CurrentLanePushVelocity)
                            << TX_VARS(mKinetics.velocity_comfort);
          mLaneKeepMgr.Clear();
        }
      }
      // 如果当前车道不可达，需要考虑换道处理, 需要取消mLaneKeepMgr
      Base::txInt dir = 0;
      if (StableVelocity() > 8 && mRouteAI.IsValid() && mLocation.IsOnLane() &&
          CallFail(mRouteAI.CheckReachableDirection(mLocation.LaneLocInfo(), dir))) {
        if (dir == -1 && bCanSwitchToRightLane) {
          bCanSwitchToLeftLane = false;
          // 右边是一个可达车道，开始换道
          const Base::txLaneUId currentLane = mLocation.LaneLocInfo().onLaneUid;
          Base::txLaneUId rightLane = currentLane;
          auto rightLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(rightLane);
          if (rightLanePtr) {
            Base::Info_Lane_t right_info_lane;
            right_info_lane.FromLane(rightLane);
            if (mRouteAI.CheckReachable(right_info_lane)) {
              mLaneKeepMgr.Clear();
              ChangeLaneLogInfo << "right can arrive , mLaneKeepMgr Clear  : " << TX_VARS(dir)
                                << TX_VARS(bCanSwitchToRightLane);
            }
          }
        }
      }
#else
      if (CallFail(mLaneKeepMgr.Expired())) {
        ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.Duration());
        mLaneKeepMgr.Decrement(timeMgr.RelativeTime());
        const Base::txFloat RearVeh_CurrentLanePushVelocity =
            (NonNull_Pointer(RearVeh_CurrentLane_Base)) ? (RearVeh_CurrentLane_Base->StableVelocity()) : (0.0);
        if ((RearGap_CurrentLane < ComfortGap) && (RearVeh_CurrentLanePushVelocity > mKinetics.velocity_comfort)) {
          /*if (FLAGS_EnableRearComfortGap)*/
          {
            ChangeLaneLogInfo << "mLaneKeepMgr.Clear() : " << TX_VARS(Id()) << TX_VARS(RearGap_CurrentLane)
                              << TX_VARS(ComfortGap) << TX_VARS(RearVeh_CurrentLanePushVelocity)
                              << TX_VARS(mKinetics.velocity_comfort);
            mLaneKeepMgr.Clear();
          }
        }
      }

#endif

      if (CallSucc(mLaneKeepMgr.Expired()) || mEventChangeLane != 0) {
        if (dir != 0) {
          if (mRouteAI.CheckReachableDirection(mLocation.LaneLocInfo(), dir)) {
            if (dir == -1 && bCanSwitchToRightLane) {
              // 优先考虑右边换道
              bCanSwitchToLeftLane = false;
            }
          }
        }
        if (bCanSwitchToLeftLane) {  // 如果左边有车道，先考虑往左边换道
          // LOG_IF(WARNING, 13 == Id()) /*ChangeLaneLogInfo*/ << TX_VARS(Id()) << " CheckCanSwitchLeftLaneState true.";

          Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
          Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
          Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
          Base::txFloat FrontGap_TargetLane = 999.0;
          Base::txFloat RearGap_TargetLane = 999.0;
          Base::txFloat SideGap_TargetLane = 999.0;

          std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
              FrontVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eLeft);
          std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
              RearVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eLeft);
          std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
              SideVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eLeft);
          if (Null_Pointer(LeftVeh_TargetLane_Base) &&
              LaneChangeIntention::eYes ==
                  LaneChanging(FrontVeh_CurrentLane_Base_4_lanechange /*FrontVeh_CurrentLane_Base*/,
                               RearVeh_CurrentLane_Base, FrontVeh_TargetLane_Base, RearVeh_TargetLane_Base,
                               FrontGap_CurrentLane_4_lanechange /*FrontGap_CurrentLane*/, RearGap_CurrentLane,
                               FrontGap_TargetLane, RearGap_TargetLane, SideGap_TargetLane,
                               PedestrianSearchType::eLeft)) {
            if (mEventChangeLane != -1) {
              if (SwitchLane(PedestrianSearchType::eLeft)) {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_LEFT SwitchLane true.";
                resLateralVelocity = MoveTo_Sideway(timeMgr);
              } else {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_LEFT SwitchLane failure.";
              }
            }
          } else if (bCanSwitchToRightLane) {
            ChangeLaneLogInfo << TX_VARS(Id()) << " bCanSwitchToRightLane true.";
            std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
                FrontVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eRight);
            std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
                RearVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eRight);
            std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
                SideVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eRight);

            if (LaneChangeIntention::eYes ==
                LaneChanging(FrontVeh_CurrentLane_Base_4_lanechange /*FrontVeh_CurrentLane_Base*/,
                             RearVeh_CurrentLane_Base, FrontVeh_TargetLane_Base, RearVeh_TargetLane_Base,
                             FrontGap_CurrentLane_4_lanechange /*FrontGap_CurrentLane*/, RearGap_CurrentLane,
                             FrontGap_TargetLane, RearGap_TargetLane, SideGap_TargetLane,
                             PedestrianSearchType::eRight)) {
              if (SwitchLane(PedestrianSearchType::eRight)) {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane true.";
                resLateralVelocity = MoveTo_Sideway(timeMgr);
              } else {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane false.";
              }
            }
            /*else
            {
                DecelerateLaneChange(DecelIntenseSpeedFactor(), DecelIntenseTimeFactor());
                ChangeLaneLogInfo << "right LaneChanging failure." << TX_VARS(LaneKeepTime()) <<
            TX_VARS(mKinetics.m_velocity_max);
            }*/
          }
          /*else
          {
              DecelerateLaneChange(DecelIntenseSpeedFactor(), DecelIntenseTimeFactor());
              ChangeLaneLogInfo << "left LaneChanging failure." << TX_VARS(LaneKeepTime()) <<
          TX_VARS(mKinetics.m_velocity_max);
          }*/
        } else if (bCanSwitchToRightLane) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " bCanSwitchToRightLane true.";
          Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base = nullptr;
          Base::SimulationConsistencyPtr RearVeh_TargetLane_Base = nullptr;
          Base::SimulationConsistencyPtr SideVeh_TargetLane_Base = nullptr;
          Base::txFloat FrontGap_TargetLane = 999.0;
          Base::txFloat RearGap_TargetLane = 999.0;
          Base::txFloat SideGap_TargetLane = 999.0;

          std::tie(FrontVeh_TargetLane_Base, FrontGap_TargetLane) =
              FrontVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eRight);
          std::tie(RearVeh_TargetLane_Base, RearGap_TargetLane) =
              RearVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eRight);
          std::tie(SideVeh_TargetLane_Base, SideGap_TargetLane) =
              SideVeh_TargetLane(+Base::Enums::VehicleMoveLaneState::eRight);

          if (LaneChangeIntention::eYes ==
              LaneChanging(FrontVeh_CurrentLane_Base_4_lanechange /*FrontVeh_CurrentLane_Base*/,
                           RearVeh_CurrentLane_Base, FrontVeh_TargetLane_Base, RearVeh_TargetLane_Base,
                           FrontGap_CurrentLane_4_lanechange /*FrontGap_CurrentLane*/, RearGap_CurrentLane,
                           FrontGap_TargetLane, RearGap_TargetLane, SideGap_TargetLane, PedestrianSearchType::eRight)) {
            if (mEventChangeLane != 1) {
              if (SwitchLane(PedestrianSearchType::eRight)) {
                resLateralVelocity = MoveTo_Sideway(timeMgr);
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane true.";
              } else {
                ChangeLaneLogInfo << TX_VARS(Id()) << "VEHICLE_MOVE_LANE_RIGHT SwitchLane false.";
              }
            }
          } else {
            // DecelerateLaneChange(DecelIntenseSpeedFactor(), DecelIntenseTimeFactor());
            ChangeLaneLogInfo << "LaneChanging failure." << TX_VARS(LaneKeepTime())
                              << TX_VARS(mKinetics.m_velocity_max);
          }
        }
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

Base::txBool SimPlanningVehicleElement::SwitchLane(const PedestrianSearchType state,
                                                   Base::txFloat laneChangeTime) TX_NOEXCEPT {
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

#if 1
  if (mLocation.IsOnLaneLink()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " in lane link, cannot lane change.";
    return false;
  }

#  if 0
  if (mLocation.tracker()->isReachedRoadLastSection()) {
    const Base::txFloat distanceToRoadEnd =
        (mLocation.tracker()->getCurrentLane()->getGeometry()->getLength() - mLocation.DistanceAlongCurve());
    if (distanceToRoadEnd < FLAGS_Ego_RefuseLaneChangeDistance) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " near to junction , cannot lane change." << TX_VARS(distanceToRoadEnd)
                        << TX_VARS(FLAGS_Ego_RefuseLaneChangeDistance);
      return false;
    } else {
      ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(distanceToRoadEnd);
    }
  } else {
    TX_MARK("maybe section is too short.");
    Coord::txWGS84 roadEnd_GPS = HdMap::HadmapCacheConCurrent::GetRoadEnd(mLocation.LaneLocInfo().onLaneUid.roadId);
    const Base::txFloat distanceToRoadEnd =
        Coord::txENU::EuclideanDistanceLocal_2D(roadEnd_GPS.ToENU(), StableGeomCenter());
    if (distanceToRoadEnd < FLAGS_Ego_RefuseLaneChangeDistance) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " near to junction , cannot lane change." << TX_VARS(distanceToRoadEnd)
                        << TX_VARS(FLAGS_Ego_RefuseLaneChangeDistance);
      return false;
    }
  }
#  endif

#endif

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

  laneChangeTime = ComputeMinLaneChangeTime(laneChangeTime);
#if 1
  if (laneChangeTime <= 0.0) {
    ChangeLaneLogInfo << TX_VARS(laneChangeTime) << " invalid.";
    return false;
  }
#endif
  m_LaneChangeTimeMgr.SetDuration(laneChangeTime);
  m_LaneChangeTimeMgr.SetProcedure(m_LaneChangeTimeMgr.Duration());
  m_LaneChangeTimeMgr.SetMinCancelDuration(NoCheckLCAbortThreshold * m_LaneChangeTimeMgr.Duration());  // 1229 HD

  if (PedestrianSearchType::eLeft == state) {
    SetTurnLeft_Start();
  } else if (PedestrianSearchType::eRight == state) {
    SetTurnRight_Start();
  }

  DecelerateLaneChange(DecelActionSpeedFactor(), DecelActionTimeFactor());
  return true;
}

Base::txBool SimPlanningVehicleElement::ValidSampling(const Base::TimeParamManager& timeMgr) const TX_NOEXCEPT {
  return true;
  TX_MARK("tad_ego evn perception per frame.");
  /*return m_NearestObject.IsValid();*/
}

SimPlanningVehicleElement::LaneChangeIntention SimPlanningVehicleElement::LaneChanging(
    Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base, Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base,
    Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base, Base::SimulationConsistencyPtr RearVeh_TargetLane_Base,
    const txFloat FrontGap_Current, const txFloat RearGap_Current, txFloat FrontGap_TargetLane,
    const txFloat RearGap_TargetLane, const txFloat SideGap_TargetLane,
    const PedestrianSearchType targetType) TX_NOEXCEPT {
  LaneChangeIntention LaneChange_Intention = LaneChangeIntention::eNo;  // 标记是否有换车道的意愿

  const txFloat FrontVeh_CurrentLaneVelocity =
      (NonNull_Pointer(FrontVeh_CurrentLane_Base)) ? (FrontVeh_CurrentLane_Base->StableVelocity()) : MAX_SPEED;
#if __TX_Mark__("crash stop lane change")
  const txFloat FrontVeh_TargetLaneVelocity = (NonNull_Pointer(FrontVeh_TargetLane_Base))
                                                  ? ((+Base::ISimulationConsistency::DrivingStatus::stop_crash ==
                                                      FrontVeh_TargetLane_Base->StableDriving_Status())
                                                         ? (5.0)
                                                         : (FrontVeh_TargetLane_Base->StableVelocity()))
                                                  : MAX_SPEED;
#else
  const txFloat FrontVeh_TargetLaneVelocity =
      (NonNull_Pointer(FrontVeh_TargetLane_Base)) ? (FrontVeh_TargetLane_Base->StableVelocity()) : MAX_SPEED;
#endif /*__TX_Mark__("crash stop lane change")*/
  const txFloat RearVeh_TargetLaneVelocity =
      (NonNull_Pointer(RearVeh_TargetLane_Base)) ? (RearVeh_TargetLane_Base->StableVelocity()) : MAX_SPEED;
  const txFloat RearVeh_CurrentLanePushVelocity =
      (NonNull_Pointer(RearVeh_CurrentLane_Base)) ? RearVeh_CurrentLane_Base->StableVelocity() : 0.0;

#if __TX_Mark__("crash stop lane change")
  if (NonNull_Pointer(FrontVeh_TargetLane_Base) &&
      (+Base::ISimulationConsistency::DrivingStatus::stop_crash == FrontVeh_TargetLane_Base->StableDriving_Status())) {
    FrontGap_TargetLane = SafetyGap_Front + 1.0;
    // LOG(WARNING) << TX_VARS(Id()) << TX_VARS(FrontVeh_TargetLaneVelocity) << TX_VARS(FrontGap_TargetLane);
  }
#endif /*__TX_Mark__("crash stop lane change")*/

  ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(FrontGap_Current) << TX_VARS(RearGap_Current)
                    << TX_VARS(FrontGap_TargetLane) << TX_VARS(RearGap_TargetLane) << TX_VARS(SideGap_TargetLane)
                    << TX_VARS(FrontVeh_CurrentLaneVelocity) << TX_VARS(FrontVeh_TargetLaneVelocity)
                    << TX_VARS(RearVeh_TargetLaneVelocity) << TX_VARS(SideGap_TargetLane)
                    << TX_VARS(RearVeh_CurrentLanePushVelocity);

  // 自由换道：
  //  1.当旁边车道的前车的gap比本车道前车的gap大，而且开得更快,而且本车车速慢于本车的期望速度
  //  2.本车道后面的车跟得太紧了，超出本车司机的comfort zone（距离和速度）了
  //  if ((RearGap_Current < ComfortGap) && RearVeh_CurrentLanePushVelocity > velocity_comfort)
  //  //本车道后面的车跟得太紧了，超出本车司机的comfort zone（距离和速度）了,则必然(不进行LaneChangeProb的判断)换道
  if (FLAGS_ego_use_lanechange &&
      (((RearGap_Current < ComfortGap) && RearVeh_CurrentLanePushVelocity > mKinetics.velocity_comfort) ||
       ((FrontGap_TargetLane > EmptyTargetLaneGapThreshold) &&
        (m_aggress > LaneChanging_Aggressive_Threshold)))) {  // 2020/0203 HD
    // ：旁边车道前面的gap如果大于EmptyTargetLaneGap=50，而且是比较激进的车，则有换道意愿
    /*if (FLAGS_EnableRearComfortGap)*/
    if (FLAGS_ego_use_lanechange) {
      LaneChange_Intention = LaneChangeIntention::eYes;
      ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = yes [1]";
    }
    // 当旁边车道的前车的gap比本车道前车的gap大，而且开得更快,而且本车车速慢于本车的期望速度
    //  else if ((FrontGap_Current< MAX_REACTION_GAP) && (FrontVeh_CurrentLaneVelocity < FrontVeh_TargetLaneVelocity) &&
    //  (m_objectData.velocity < m_Data.velocity_desired) && ((FrontGap_Current< FrontGap_TargetLane *
    //  pow(FrontVeh_TargetLaneVelocity / FrontVeh_CurrentLaneVelocity, 2.0)) ||
    //  (FrontGap_Current<FrontGap_TargetLane)))
    /*else if (FrontGap_Current < MAX_REACTION_GAP &&
        ( (FrontVeh_CurrentLaneVelocity < FrontVeh_TargetLaneVelocity) &&
          (m_velocity < velocity_desired) &&
            ( (FrontGap_Current < FrontGap_TargetLane * pow(FrontVeh_TargetLaneVelocity /
       FrontVeh_CurrentLaneVelocity, 2.0)) || (FrontGap_Current < FrontGap_TargetLane)
            )
        ))*/
  } else if (FLAGS_ego_use_lanechange && FrontGap_Current < MAX_REACTION_GAP &&
             ((FrontVeh_CurrentLaneVelocity < FrontVeh_TargetLaneVelocity) &&
              (StableVelocity() < mKinetics.velocity_desired) &&
              ((FrontGap_Current <
                FrontGap_TargetLane *
                    pow(FrontVeh_TargetLaneVelocity / (FrontVeh_CurrentLaneVelocity + SpeedDenominatorOffset), 2.0)) ||
               (FrontGap_Current < FrontGap_TargetLane))) &&
             (m_aggress > LaneChanging_Aggressive_Threshold ||
              (StableVelocity() < (mKinetics.velocity_desired * Tolerance_Threshold)))) {  // 1229 HD
    ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [2] " << TX_VARS(m_aggress)
                      << TX_VARS(LaneChanging_Aggressive_Threshold) << TX_VARS(StableVelocity())
                      << TX_VARS(mKinetics.velocity_desired) << TX_VARS(Tolerance_Threshold)
                      << TX_COND(m_aggress > LaneChanging_Aggressive_Threshold)
                      << TX_COND((StableVelocity() < (mKinetics.velocity_desired * Tolerance_Threshold)));
    if (FLAGS_ego_use_lanechange) LaneChange_Intention = LaneChangeIntention::eYes;
  } else if (FLAGS_ego_use_lanechange && (mRouteAI.IsValid() && mLocation.IsOnLane() &&
                                          CallFail(mRouteAI.CheckReachable(mLocation.LaneLocInfo())))) {
    /*else if (FrontPedWalkingInSight(NEAR_DYNAMIC_SEARCH_FRONT, MAX_REACTION_GAP) > 0) LaneChange_Intention = 1;*/
    LogWarn << mLocation.LaneLocInfo();
    LaneChange_Intention = LaneChangeIntention::eYes;
    ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [3] ";
  } else if (mEventChangeLane != 0) {
    LaneChange_Intention = LaneChangeIntention::eYes;
    ChangeLaneLogInfo << TX_VARS(Id()) << " set LaneChange_Intention = 1  [4] ";
  } else {
    ChangeLaneLogInfo
        << TX_VARS(Id()) << " lanechanging refuse lane change. lanechange return 0  [1]" << TX_VARS(RearGap_Current)
        << TX_VARS(ComfortGap) << TX_VARS(RearVeh_CurrentLanePushVelocity) << TX_VARS(mKinetics.velocity_comfort)
        << TX_VARS(FrontGap_TargetLane) << TX_VARS(EmptyTargetLaneGapThreshold) << TX_VARS(m_aggress)
        << TX_VARS(LaneChanging_Aggressive_Threshold) << TX_VARS(FrontGap_Current) << TX_VARS(MAX_REACTION_GAP)
        << TX_VARS(FrontVeh_CurrentLaneVelocity) << TX_VARS(FrontVeh_TargetLaneVelocity) << TX_VARS(StableVelocity())
        << TX_VARS(mKinetics.velocity_desired) << TX_VARS(FrontGap_Current)
        << TX_VARS((FrontGap_TargetLane *
                    pow(FrontVeh_TargetLaneVelocity / (FrontVeh_CurrentLaneVelocity + SpeedDenominatorOffset), 2.0)))
        << TX_VARS(FrontGap_Current) << TX_VARS(FrontGap_TargetLane);
    return LaneChangeIntention::eNo;
  }

  if (LaneChangeIntention::eNo == LaneChange_Intention) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " lanechanging refuse lane change. lanechange return 0  [2]";
    return LaneChangeIntention::eNo;  // 如果没有换道意愿，则返回（不换道）
  }

#if _refuse_lane_change_before_signal_
  if (approach_signal()) {
#  if _20210103_debug_
    if (mShowLog) {
      LOG(INFO) << TXST_TRACE_VARIABLES(m_id) << " lanechanging refuse lane change.";
    }
#  endif /*_20210103_debug_*/
    return 0;
  }
#endif
  // _refuse_lane_change_before_signal_
  // 此换道安全的条件以后可以替换,后面安全条件加了一个系数，即在旁边车道的后车速度较大的情况下需要留出更大的后车距
  // if ((FrontGap_TargetLane > SafetyGap_Front) &&
  // ((RearGap_TargetLane > SafetyGap_Rear * pow(RearVeh_TargetLaneVelocity / m_velocity, 1.5))) &&
  // (FrontPedWalkingInSight(targetType, MAX_REACTION_GAP) == 0))*/
  const Base::txFloat SafetyGap_V_AdjWeight = 0.5;                                         // 1229 HD
  if ((FrontGap_TargetLane > SafetyGap_Front) && (SideGap_TargetLane > SafetyGap_Side) &&  // 0103 HD
      /*if ((FrontGap_TargetLane > SafetyGap_Front) &&*/
      ((RearGap_TargetLane >
        SafetyGap_Rear *
            pow(RearVeh_TargetLaneVelocity / (StableVelocity() + SpeedDenominatorOffset), SafetyGap_V_AdjWeight))) &&
      (Math::isZero(FrontPedWalkingInSight(targetType, MAX_REACTION_GAP)))) {  // 1229 HD
    ChangeLaneLogInfo << TX_VARS(Id()) << " lanechanging enable lane change. ";
    return LaneChangeIntention::eYes;  // 0103 YC
  } else {
    ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS(FrontGap_TargetLane) << TX_VARS(SafetyGap_Front)
                      << TX_VARS(SideGap_TargetLane) << TX_VARS(SafetyGap_Side) << TX_VARS(RearGap_TargetLane)
                      << TX_VARS((SafetyGap_Rear *
                                  pow(RearVeh_TargetLaneVelocity / (StableVelocity() + SpeedDenominatorOffset),
                                      SafetyGap_V_AdjWeight)))
                      << " lanechanging refuse lane change. return 0.0";
    return LaneChangeIntention::eNo;  // 换道不安全，返回
  }
}

Base::txFloat SimPlanningVehicleElement::SearchRoadEnd(const Base::txFloat search_distance) TX_NOEXCEPT { return 0.0; }

Base::txFloat SimPlanningVehicleElement::ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT {
  Base::txFloat retTime = 0.0;
  if (Math::isNotZero(StableVelocity())) {
#if 1
    const Coord::txWGS84 curRoadEndPt =
        HdMap::HadmapCacheConCurrent::GetRoad2Junction(mLocation.LaneLocInfo().onLaneUid.roadId);
    const Base::txFloat distToRoadEnd =
        Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(mLocation.PosOnLaneCenterLinePos(), curRoadEndPt) -
        FLAGS_Ego_RefuseLaneChangeDistance;
#else
    const Coord::txWGS84 curRoadEndPt =
        HdMap::HadmapCacheConCurrent::GetRoadEnd(mLocation.LaneLocInfo().onLaneUid.roadId);
    const Base::txFloat distToRoadEnd =
        Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(mLocation.PosOnLaneCenterLinePos(), curRoadEndPt) /*- 10.0*/;
#endif

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

Base::txBool SimPlanningVehicleElement::CheckCanSwitchRightLaneState(const Base::TimeParamManager&) TX_NOEXCEPT {
  if (route_end_dist() < FLAGS_RefuseLaneChangeDistance) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false arrive route end."
                      << TX_VARS(GetVelocity()) << TX_VARS(RefuseLaneChangeVelocity());
    return false;
  }
  if (GetVelocity() < RefuseLaneChangeVelocity()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false velocity too slow."
                      << TX_VARS(GetVelocity()) << TX_VARS(RefuseLaneChangeVelocity());
    return false;
  }
  if (mLocation.IsOnLaneLink()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false isOnLaneLink.";
    return false;
  } else if (mLocation.IsOnRightestLane()) {
    TX_MARK("on the right boundary, could not right switch.");
    ChangeLaneLogInfo << TX_VARS(Id()) << " on the right boundary, could not right switch.";
    return false;
  } else if (mLocation.tracker()->IsRightBoundarySolid()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " bInRightSolidLane.";
    return false;
  } else if (CallFail(Utils::IsLaneValidDriving(mLocation.tracker()->getRightLane()))) {
    return false;
  } else {
    const Base::txLaneUId& onLaneUid = mLocation.LaneLocInfo().onLaneUid;
#if 1
    if (HdMap::HadmapCacheConCurrent::IsSpecialRefuseChangeLane_Right(onLaneUid, DistanceAlongCurve())) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][special][right]" << Utils::ToString(onLaneUid)
                        << TX_VARS(DistanceAlongCurve());
      return false;
    }
#endif
    if (HdMap::HadmapCacheConCurrent::IsRefuseChangeLane_Right(onLaneUid)) {
      ChangeLaneLogInfo << TX_VARS(Id()) << " [RefuseChangeLane][right]" << Utils::ToString(onLaneUid);
      return false;
    }
#if __TX_Mark__(ID101432729)
    /*const auto rightLaneUid = Utils::RightLaneUid(onLaneUid);*/
    if (CallSucc(mRouteAI.IsValid())) {
      TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
      if (CallFail(mRouteAI.IsReachable(onLaneUid, routeHint))) {
        TX_MARK("current lane donot reach dest.");
        TX_MARK("avoid ");
        Base::txBool reachableFlag = false;
        Base::txLaneUId srcLaneUid = onLaneUid;
        const Base::txSize nLaneSizeOnSection =
            HdMap::HadmapCacheConCurrent::GetLaneCountOnSection(Utils::LaneUid2SectionUid(srcLaneUid));
        Base::txInt nCnt = 0;
        while (CallSucc(Utils::HasRightLane(srcLaneUid, nLaneSizeOnSection)) && nCnt < 5) {
          ++nCnt;
          srcLaneUid = Utils::RightLaneUid(srcLaneUid);
          if (mRouteAI.CheckReachable(srcLaneUid)) {
            reachableFlag = true;
            ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS_NAME(right_lane, Utils::ToString(srcLaneUid))
                              << " is reachable.";
            break;
          }
        }
        if (reachableFlag) {
          /*const auto rightLaneUid = Utils::RightLaneUid(onLaneUid);*/
        } else {
          ChangeLaneLogInfo << TX_VARS(Id()) << " do not turn right. " << TX_VARS(nCnt) << TX_VARS(nLaneSizeOnSection)
                            << TX_VARS(onLaneUid);
          return false;
        }
#  if 0
        const auto leftLaneUid = Utils::LeftLaneUid(onLaneUid);
        const txBool leftReachable = mRouteAI.CheckReachable(leftLaneUid);
        const txBool rightReachable = mRouteAI.CheckReachable(rightLaneUid);
        if (CallSucc(leftReachable) && CallFail(rightReachable)) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " do not turn right. " << TX_COND(rightReachable)
                            << TX_COND(leftReachable) << TX_VARS(onLaneUid);
          return false;
        }
#  endif
      }
    } else {
      ChangeLaneLogInfo << TX_VARS(Id()) << " do not have route, under free lane change. [right]";
    }
#endif
  }
  return true;
}

Base::txBool SimPlanningVehicleElement::CheckCanSwitchLeftLaneState(const Base::TimeParamManager&) TX_NOEXCEPT {
  if (route_end_dist() < FLAGS_RefuseLaneChangeDistance) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchRightLaneState false arrive route end."
                      << TX_VARS(GetVelocity()) << TX_VARS(RefuseLaneChangeVelocity());
    return false;
  }

  if (GetVelocity() < RefuseLaneChangeVelocity()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchLeftLaneState false velocity too slow."
                      << TX_VARS(GetVelocity()) << TX_VARS(RefuseLaneChangeVelocity());
    return false;
  }
  if (mLocation.IsOnLaneLink()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << "CheckCanSwitchLeftLaneState false isOnLaneLink.";
    return false;
  } else if (mLocation.IsOnLeftestLane()) {
    TX_MARK("on the left boundary, could not left switch.");
    ChangeLaneLogInfo << TX_VARS(Id()) << " on the left boundary, could not left switch.";
    return false;
  } else if (mLocation.tracker()->IsLeftBoundarySolid()) {
    ChangeLaneLogInfo << TX_VARS(Id()) << " bInLeftSolidLane.";
    return false;
  } else if (CallFail(Utils::IsLaneValidDriving(mLocation.tracker()->getLeftLane()))) {
    return false;
  } else {
    const Base::txLaneUId& onLaneUid = mLocation.LaneLocInfo().onLaneUid;
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
#if __TX_Mark__(ID101432729)
    if (CallSucc(mRouteAI.IsValid())) {
      TrafficFlow::Component::RouteAI::RouteAI_EdgeData routeHint;
      if (CallFail(mRouteAI.IsReachable(onLaneUid, routeHint))) {
        TX_MARK("current lane donot reach dest.");
        TX_MARK("avoid ");
#  if 1
        Base::txBool reachableFlag = false;
        Base::txLaneUId srcLaneUid = onLaneUid;
        Base::txInt nCnt = 0;
        while (CallSucc(Utils::HasLeftLane(srcLaneUid)) && nCnt < 5) {
          ++nCnt;
          srcLaneUid = Utils::LeftLaneUid(srcLaneUid);
          if (mRouteAI.CheckReachable(srcLaneUid)) {
            reachableFlag = true;
            ChangeLaneLogInfo << TX_VARS(Id()) << TX_VARS_NAME(left_lane, Utils::ToString(srcLaneUid))
                              << " is reachable.";
            break;
          }
        }
        if (!reachableFlag) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " do not turn left. " << TX_VARS(nCnt) << TX_VARS(onLaneUid);
          return false;
        }
#  else
        const auto rightLaneUid = Utils::RightLaneUid(onLaneUid);

        const txBool leftReachable = mRouteAI.CheckReachable(leftLaneUid);
        const txBool rightReachable = mRouteAI.CheckReachable(rightLaneUid);
        if (CallSucc(rightReachable) && CallFail(leftReachable)) {
          ChangeLaneLogInfo << TX_VARS(Id()) << " do not turn left. " << TX_COND(leftReachable)
                            << TX_COND(rightReachable) << TX_VARS(onLaneUid);
          return false;
        }
#  endif
      }
    } else {
      ChangeLaneLogInfo << TX_VARS(Id()) << " do not have route, under free lane change. [left]";
    }
#endif
  }
  return true;
}

Base::txFloat SimPlanningVehicleElement::MoveTo_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const txFloat exper_time = m_LaneChangeTimeMgr.Experience();
  const txFloat lateral_velocity = Get_MoveTo_Sideway(exper_time);
  m_LaneChangeTimeMgr.Step(timeMgr.RelativeTime());
  return lateral_velocity;
}

Base::txFloat SimPlanningVehicleElement::Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT {
  if (IsInLaneChange_StartIng() && m_LaneChangeTimeMgr.Procedure() > 0.0) {
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    if (IsInTurnLeft()) {
#if USE_MAPSDK_XY2SL
#else  /*USE_MAPSDK_XY2SL*/
      if (NonNull_Pointer(mLocation.tracker()->get_Left_LaneGeomInfo())) {
        mLocation.tracker()->get_Left_LaneGeomInfo()->xy2sl(mLocation.PosWithLateralWithoutOffset().ToENU().ENU2D(), s,
                                                            l);
      }
#endif /*USE_MAPSDK_XY2SL*/
    } else if (IsInTurnRight()) {
#if USE_MAPSDK_XY2SL
#else  /*USE_MAPSDK_XY2SL*/
      if (NonNull_Pointer(mLocation.tracker()->get_Right_LaneGeomInfo())) {
        mLocation.tracker()->get_Right_LaneGeomInfo()->xy2sl(mLocation.PosWithLateralWithoutOffset().ToENU().ENU2D(), s,
                                                             l);
      }
#endif /*USE_MAPSDK_XY2SL*/
    }

    return std::fabs(l) / m_LaneChangeTimeMgr.Procedure();
  } else {
    return 0.0;
  }
}

Base::txFloat SimPlanningVehicleElement::DrivingChangeLane(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base = std::get<0>(FrontVeh_CurrentLane());
  Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base = std::get<0>(RearVeh_CurrentLane());
  Base::SimulationConsistencyPtr LeftVeh_TargetLane_Base = std::get<0>(LeftVeh_TargetLane());
  Base::SimulationConsistencyPtr RightVeh_TargetLane_Base = std::get<0>(RightVeh_TargetLane());
  return Driving_SwitchLaneOrg(timeMgr, FrontVeh_CurrentLane_Base, RearVeh_CurrentLane_Base, LeftVeh_TargetLane_Base,
                               RightVeh_TargetLane_Base);
}

Base::txFloat SimPlanningVehicleElement::Get_MoveBack_Sideway(const txFloat exper_time) TX_NOEXCEPT {
#if USE_MAPSDK_XY2SL
#else  /*USE_MAPSDK_XY2SL*/
  if (m_LaneAbortingTimeMgr.Procedure() > 0.0 && NonNull_Pointer(mLocation.tracker()->getLaneGeomInfo())) {
    txFloat s = 0.0, l = 0.0;
    mLocation.tracker()->getLaneGeomInfo()->xy2sl(mLocation.PosWithLateralWithoutOffset().ToENU().ENU2D(), s, l);
    return std::fabs(l) / m_LaneAbortingTimeMgr.Procedure();
  } else {
    return 0.0;
  }
#endif /*USE_MAPSDK_XY2SL*/
}

Base::txFloat SimPlanningVehicleElement::MoveBack_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const txFloat exper_time = m_LaneAbortingTimeMgr.Experience();
  const txFloat lateral_velocity = Get_MoveBack_Sideway(exper_time);
  m_LaneAbortingTimeMgr.Step(timeMgr.RelativeTime());
  return lateral_velocity;
}

Base::txString follower_status_2_string(const SimPlanningVehicleElement::DRIVING_FOLLOW_STATE e) TX_NOEXCEPT {
  /*enum class DRIVING_FOLLOW_STATE : txInt { N = 0, A = 1, B = 2, F = 3, W = 4 };*/
  switch (e) {
    case SimPlanningVehicleElement::DRIVING_FOLLOW_STATE::A:
      return "A";
    case SimPlanningVehicleElement::DRIVING_FOLLOW_STATE::N:
      return "N";
    case SimPlanningVehicleElement::DRIVING_FOLLOW_STATE::B:
      return "B";
    case SimPlanningVehicleElement::DRIVING_FOLLOW_STATE::F:
      return "F";
    case SimPlanningVehicleElement::DRIVING_FOLLOW_STATE::W:
      return "W";
    default:
      return "unknown";
      break;
  }
}

Base::txFloat SimPlanningVehicleElement::DrivingFollowNormal(const Base::TimeParamManager& timeMgr,
                                                             Base::SimulationConsistencyPtr frontVehicleElementPtr,
                                                             const txFloat frontDist,
                                                             const Base::txBool bHurryToExitState) TX_NOEXCEPT {
  // LOG(INFO) << TX_VARS(timeMgr.PassTime()) << TX_VARS(frontDist) << TX_VARS_NAME(frontId,
  // ((NonNull_Pointer(frontVehicleElementPtr))?(frontVehicleElementPtr->ConsistencyId()):(-1)));
  txFloat acceleration = 0.0;

#if 1
  txFloat fLeaderVelocity = (NonNull_Pointer(frontVehicleElementPtr)) ? (frontVehicleElementPtr->StableVelocity())
                                                                      : (mKinetics.m_velocity_max + 10.0);
  txFloat fLeaderAcceleration = (NonNull_Pointer(frontVehicleElementPtr)) ? (frontVehicleElementPtr->StableAcc())
                                                                          : (mKinetics.m_velocity_max + 10.0);
#else
  txFloat fLeaderVelocity = (NonNull_Pointer(frontVehicleElementPtr))
                                ? ((StableLaneInfo().isOnLaneLink) ? (0.0) : (frontVehicleElementPtr->StableVelocity()))
                                : (mKinetics.m_velocity_max + 10.0);

  txFloat fLeaderAcceleration = (NonNull_Pointer(frontVehicleElementPtr))
                                    ? ((StableLaneInfo().isOnLaneLink) ? (0.0) : (frontVehicleElementPtr->StableAcc()))
                                    : (mKinetics.m_velocity_max + 10.0);
#endif
  /*NearestObjectWrap nearest_object(m_NearestObject, changeLaneState, moveState);*/
  Base::txFloat dx = frontDist;
  // Base::txFloat dx = m_NearestObject.aroudDistance[Base::OBJECT_TARGET_FRONT_DIRECTION];
  Base::txFloat dv = fLeaderVelocity - StableVelocity();  // 速度差, 前车比本车快, 则为正, 反正为负
  Base::txFloat sdxc = 0.0;
  // 0205 HD加了摩擦系数////////////////////////////////////////////////////
  Base::txFloat Min_Gap = Driving_Parameters(0);  // 0206 HD
#if 1
  Base::SimulationConsistencyPtr sideVehiclePtr = nullptr;
  Base::txFloat SideGap_TargetLane = 999.0;
  std::tie(sideVehiclePtr, SideGap_TargetLane) = Any_SideVeh_TargetLane();
#endif
  const Base::txFloat SideVehicle_Velocity =
      (NonNull_Pointer(sideVehiclePtr) ? (sideVehiclePtr->StableVelocity()) : (MAX_SPEED));
  Base::txFloat Max_Speed = mKinetics.velocity_desired;
  if (SideVehicle_Velocity < Utils::VehicleBehaviorCfg::SideGapMinVelocity()) {
    TX_MARK("旁边有车，但速度为0");
    Max_Speed = mKinetics.velocity_desired * Friction_Factor;
    Min_Gap = Min_Gap * (2 - Friction_Factor);  // 0206 HD
    __Trace_PushBack_("2");
  }
  // 0205 HD加了摩擦系数////////////////////////////////////////////////////
  if (bHurryToExitState) {
    static const txInt front_idx = (+Base::Enums::NearestElementDirection::eFront)._to_index();
    static const txInt back_idx = (+Base::Enums::NearestElementDirection::eBack)._to_index();
    /*yangchen 前方和后前方的最小距离*/
    dx = fmin(env_dist(front_idx), env_dist(back_idx));
    __Trace_PushBack_("3");
  }

  if (fLeaderVelocity <= 0) {
    /*yangchen  前车停了： sdxc = 1.50*/
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
#if _0206_log_switch_
    LOG(INFO) << "[20210414]" << TXST_TRACE_VARIABLES(m_id) << " (dx < 0) " << TXST_TRACE_VARIABLES(dx);
#endif /*_0206_log_switch_*/
    return Max_Deceleration;
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
    acceleration = fmax(0.5 * dv * dv / (-dx + sdxc - 0.1), Max_Deceleration);  // 不超过重力加速度
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
      } else {  // 0.004035057 - 0.224349145  4.613966138
        a_Cap = 0.004035 * StableVelocity() * StableVelocity() - 0.22435 * StableVelocity() + 4.614;
        __Trace_PushBack_("24");
      }

      acceleration = fmin(acceleration, Max_Speed - StableVelocity());  // 0205 HD加了摩擦系数
      acceleration = fmin(acceleration, a_Cap);
      // if (fabs(follower.v_desired - follower.v) < 0.1) {
      // follower_status.message_action = '在限速下行驶';
    }
  }
  __Trace_PushBack_("25");
  m_follower_status.status = m_follower_status.code;
  EnvPerceptionInfo << TX_VARS(Id())
                    << TX_VARS_NAME(m_follower_status.code, follower_status_2_string(m_follower_status.code));
  return acceleration;
}

Base::txFloat SimPlanningVehicleElement::DrivingFollow(
    const Base::TimeParamManager& timeMgr /*, Base::IVehicleElementPtr frontElementPtr, const txFloat frontDist*/)
    TX_NOEXCEPT {
  Base::txSurroundVehiclefo frontVehicleInfo = DrivingFollow_Front();
  Base::SimulationConsistencyPtr frontElementPtr = Weak2SharedPtr(std::get<_ElementIdx_>(frontVehicleInfo));
  Base::txFloat return_acc = DrivingFollowNormal(
      timeMgr,
      frontElementPtr, /*std::dynamic_pointer_cast<Base::IVehicleElement>(std::get<_ElementIdx_>(frontVehicleInfo)), */
      std::get<_DistIdx_>(frontVehicleInfo));
  LOG_IF(INFO, FLAGS_LogLevel_Ego_Front || FLAGS_LogLevel_Ego_EnvPerception)
      << TX_VARS(Id())
      << TX_VARS_NAME(FrontId, (NonNull_Pointer(frontElementPtr) ? (frontElementPtr->ConsistencyId()) : (-1)))
      << TX_VARS(std::get<_DistIdx_>(frontVehicleInfo)) << TX_VARS(return_acc) << TX_VARS(mKinetics.raw_velocity_max)
      << TX_VARS(StableLaneInfo()) << TX_VARS(GetOnLaneLinkTimeStamp());
  return return_acc;
}

#if __TX_Mark__("nearest object wrap")

/*
BETTER_ENUM(NearestElementDirection, txInt, eFront = 0, eBack = 1, eLeft = 2, eLeftFront = 3, eLeftBack = 4, eRight = 5,
eRightFront = 6, eRightBack = 7)
*/
std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::FrontVeh_CurrentLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eFront)._to_index();
  return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::RearVeh_CurrentLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eBack)._to_index();
  return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::FrontVeh_TargetLane() const
    TX_NOEXCEPT {
  Base::txInt dir = (+Base::Enums::NearestElementDirection::eFront)._to_index();
  if (IsInTurnLeft()) {
    dir = (+Base::Enums::NearestElementDirection::eLeftFront)._to_index();
  } else if (IsInTurnRight()) {
    dir = (+Base::Enums::NearestElementDirection::eRightFront)._to_index();
  }
  return std::make_tuple(env_elemPtr(dir), env_dist(dir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::FrontVeh_TargetLane(
    const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT {
  if ((+Base::Enums::VehicleMoveLaneState::eLeft) == (dest_move_state)) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eLeftFront)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  } else if ((+Base::Enums::VehicleMoveLaneState::eRight) == (dest_move_state)) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eRightFront)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  }
  LOG(FATAL) << "FrontVeh_TargetLane only support left/right";
  static const Base::txInt cFrontDir = (+Base::Enums::NearestElementDirection::eFront)._to_index();
  return std::make_tuple(env_elemPtr(cFrontDir), env_dist(cFrontDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::RearVeh_TargetLane() const
    TX_NOEXCEPT {
  Base::txInt dir = (+Base::Enums::NearestElementDirection::eBack)._to_index();
  if (IsInTurnLeft()) {
    dir = (+Base::Enums::NearestElementDirection::eLeftBack)._to_index();
  } else if (IsInTurnRight()) {
    dir = (+Base::Enums::NearestElementDirection::eRightBack)._to_index();
  }
  return std::make_tuple(env_elemPtr(dir), env_dist(dir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::RearVeh_TargetLane(
    const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT {
  if ((+Base::Enums::VehicleMoveLaneState::eLeft) == (dest_move_state)) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eLeftBack)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  } else if ((+Base::Enums::VehicleMoveLaneState::eRight) == (dest_move_state)) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eRightBack)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  }
  LOG(FATAL) << "RearVeh_TargetLane only support left/right";
  static const Base::txInt cBackDir = (+Base::Enums::NearestElementDirection::eBack)._to_index();
  return std::make_tuple(env_elemPtr(cBackDir), env_dist(cBackDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::Any_SideVeh_TargetLane() const
    TX_NOEXCEPT {
  if (NonNull_Pointer(env_elemPtr(Ego_NearestObject::nRight))) {
    return std::make_tuple(env_elemPtr(Ego_NearestObject::nRight), 5.0);
  }
  if (NonNull_Pointer(env_elemPtr(Ego_NearestObject::nLeft))) {
    return std::make_tuple(env_elemPtr(Ego_NearestObject::nLeft), 5.0);
  }
  return std::make_tuple(nullptr, 999.9);
#  if USE_SUDOKU_GRID
  std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
  std::vector<Base::txVec2> localPts_clockwise_close;
  {
    mVehicleScanRegion2D.Get_SideGap_Region_Clockwise_Close_By_Enum(Scene::VehicleScanRegion2D::SideGapType::RightSide,
                                                                    localPts_clockwise_close);
    if (Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInAreaByType(
            SysId(), localPts_clockwise_close, m_vehicle_type_filter_set, resultElementPtrVec) &&
        CallFail(resultElementPtrVec.empty())) {
      if (FLAGS_LogLevel_AI_Traffic) {
        std::ostringstream oss;
        for (const auto ptr : resultElementPtrVec) {
          oss << (ptr->Id()) << ", ";
        }
        LogInfo << "SideVeh " << TX_VARS(Id()) << " Right Side Vehicle : " << oss.str();
      }
      return std::make_tuple(std::dynamic_pointer_cast<Base::IVehicleElement>(resultElementPtrVec.front()), 5.0);
    }
  }
  {
    mVehicleScanRegion2D.Get_SideGap_Region_Clockwise_Close_By_Enum(Scene::VehicleScanRegion2D::SideGapType::LeftSide,
                                                                    localPts_clockwise_close);
    if (Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInAreaByType(
            SysId(), localPts_clockwise_close, m_vehicle_type_filter_set, resultElementPtrVec) &&
        CallFail(resultElementPtrVec.empty())) {
      if (FLAGS_LogLevel_AI_Traffic) {
        std::ostringstream oss;
        for (const auto ptr : resultElementPtrVec) {
          oss << (ptr->Id()) << ", ";
        }
        LogInfo << "SideVeh " << TX_VARS(Id()) << " Left Side Vehicle : " << oss.str();
      }
      return std::make_tuple(std::dynamic_pointer_cast<Base::IVehicleElement>(resultElementPtrVec.front()), 5.0);
    }
  }
  TX_MARK("left and right side do not have vehicle");
  return std::make_tuple(nullptr, 999.9);
#  endif /* USE_SUDOKU_GRID */

#  if UseHash
  // txMsg("UseHash");
  return std::make_tuple(nullptr, 999.9);
#  endif /*UseHash*/
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::SideVeh_TargetLane() const
    TX_NOEXCEPT {
  if (IsInTurnLeft()) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eLeft)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  } else if (IsInTurnRight()) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eRight)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  }
  return std::make_tuple(nullptr, 999.9);
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::SideVeh_TargetLane(
    const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT {
  if ((+Base::Enums::VehicleMoveLaneState::eLeft) == (dest_move_state)) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eLeft)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  } else if ((+Base::Enums::VehicleMoveLaneState::eRight) == (dest_move_state)) {
    static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eRight)._to_index();
    return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
  }
  LOG(FATAL) << "RearVeh_TargetLane only support left/right";
  return std::make_tuple(nullptr, 999.9);
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::LeftVeh_TargetLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eLeft)._to_index();
  return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
}

std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SimPlanningVehicleElement::RightVeh_TargetLane() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::NearestElementDirection::eRight)._to_index();
  return std::make_tuple(env_elemPtr(cDir), env_dist(cDir));
}

#endif /*__TX_Mark__("nearest object wrap")*/

#if __TX_Mark__("nearest cross vehicle wrap")
std::tuple<Base::IVehicleElementPtr, Base::txFloat> SimPlanningVehicleElement::OpposeSideVeh_NextCross() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::Vehicle2Vehicle_RelativeDirection::eOpposite)._to_index();
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

std::tuple<Base::IVehicleElementPtr, Base::txFloat> SimPlanningVehicleElement::SameSideVeh_NextCross() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::Vehicle2Vehicle_RelativeDirection::eSameSide)._to_index();
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

std::tuple<Base::IVehicleElementPtr, Base::txFloat> SimPlanningVehicleElement::LeftSideVeh_NextCross() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::Vehicle2Vehicle_RelativeDirection::eLeftSide)._to_index();
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

std::tuple<Base::IVehicleElementPtr, Base::txFloat> SimPlanningVehicleElement::RightSideVeh_NextCross() const
    TX_NOEXCEPT {
  static const Base::txInt cDir = (+Base::Enums::Vehicle2Vehicle_RelativeDirection::eRightSide)._to_index();
  return std::make_tuple(m_NearestCrossRoadVehicle.aroudElemPtr.at(cDir),
                         m_NearestCrossRoadVehicle.aroudDistance.at(cDir));
}

#endif /*__TX_Mark__("nearest cross vehicle wrap")*/

Base::txBool SimPlanningVehicleElement::CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallFail(IsStart())) {
    if (timeMgr.RelativeTime() > 0.0 && timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      /*LOG(WARNING) << TX_VARS(Id()) << timeMgr << TX_VARS(mLifeCycle.StartTime());*/
      mLifeCycle.SetStart();
      SM::txAITrafficState::SetStart();
      SM::txAITrafficState::SetLaneKeep();
      OnStart(timeMgr);
    } else {
      // LOG(WARNING) << "[failure]" << TX_VARS(Id()) << timeMgr << TX_VARS(mLifeCycle.StartTime());
    }
  }
  return IsStart();
}

Base::txBool SimPlanningVehicleElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                       sim_msg::Location& refLocation) TX_NOEXCEPT {
  refLocation.Clear();
  SaveStableState();
  if (IsAlive()) {
    refLocation.set_t(timeMgr.TimeStamp());
    auto pPosition = refLocation.mutable_position();
    const Coord::txWGS84 veh_coord_pt = RawVehicleCoord();
    pPosition->set_x(veh_coord_pt.Lon());
    pPosition->set_y(veh_coord_pt.Lat());
    pPosition->set_z(Altitude() /*veh_coord_pt.Alt()*/);

    auto pVelocity = refLocation.mutable_velocity();
    pVelocity->set_x(GetVelocity());
    pVelocity->set_y(mKinetics.m_LateralVelocity);
    pVelocity->set_z(0.0);

    auto pRpy = refLocation.mutable_rpy();
    const Base::txFloat _headingRadian = GetHeadingWithAngle().GetRadian();
    pRpy->set_z(_headingRadian);
    return true;
  } else {
    return false;
  }
}

Base::txBool SimPlanningVehicleElement::FillingTrajectory(Base::TimeParamManager const& timeMgr,
                                                          sim_msg::Trajectory& refTraj) TX_NOEXCEPT {
  refTraj.Clear();
  mPlanningTrajectoryPtr = nullptr;
  SaveStableState();
  if (IsAlive()) {
    /*for lane change finish*/
    // if (mLocation.IsOnLane()) {
    UpdateHashedLaneInfo(mLocation);
    //}
    const HashedLaneInfo& selfStableHashedLaneInfo = StableHashedLaneInfo();
    UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
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
      if (CallFail(SearchOrthogonalListFrontByLevel(bCheckReachable, start_hashed_node_ptr, real_search_dist, 0,
                                                    search_result_by_level))) {
        search_result_by_level.clear();
        SearchOrthogonalListFrontByLevel(bDoNotCheckReachable, start_hashed_node_ptr, real_search_dist, 0,
                                         search_result_by_level);
        TrajInfo << "[trajectory][using_random_select]";
      } else {
        TrajInfo << "[trajectory][using_check_reachable]";
      }

      TrajInfo << "[trajectory]" << TX_VARS(reverse_base) << TX_VARS(traj_length) << TX_VARS(real_search_dist)
               << TX_VARS(search_result_by_level.size());
      if (FLAGS_LogLevel_Ego_Traj) {
        Base::txInt idx = 0;
        for (const auto& ref : search_result_by_level) {
          LOG(INFO) << "[trajectory]" << TX_VARS(idx) << (ref->GetSelfHashedInfo());
          ++idx;
        }
      }

      std::vector<Base::TrajectorySamplingNode> vec_sampling_traj_pts;
      TrajInfo << "[trajectory]" << TX_VARS(DistanceAlongCurve())
               << TX_VARS_NAME(curHashInfo, start_hashed_node_ptr->GetSelfHashedInfo());

      vec_sampling_traj_pts.emplace_back(
          Base::TrajectorySamplingNode(start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo() /*current lane info*/,
                                       DistanceAlongCurve(), LaneOffset()));

      if (CallSucc(IsInLaneChange())) {
        for (const auto& refTrajOrthogonalListPtr : search_result_by_level) {
          const auto& cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();
          vec_sampling_traj_pts.emplace_back(
              Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                           cur_level_HashNodeInfo.EndScope(), LaneOffset()));
        }
      } else {
        const auto& start_lane_info = start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo();

        for (const auto& refTrajOrthogonalListPtr : search_result_by_level) {
          const auto& cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();

          if (CallSucc(start_lane_info == cur_level_HashNodeInfo.LaneInfo())) {
            if (DistanceAlongCurve() < cur_level_HashNodeInfo.EndScope() &&
                cur_level_HashNodeInfo.StartScope() < DistanceAlongCurve()) {
              const Base::txFloat real_length = cur_level_HashNodeInfo.EndScope() - DistanceAlongCurve();

              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               DistanceAlongCurve() + 0.3 * real_length, LaneOffset()));
              TrajInfo << TX_VARS(real_length) << vec_sampling_traj_pts.back().Str();
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               DistanceAlongCurve() + 0.6 * real_length, LaneOffset()));
              TrajInfo << TX_VARS(real_length) << vec_sampling_traj_pts.back().Str();
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
              TrajInfo << TX_VARS(real_length) << vec_sampling_traj_pts.back().Str();
            } else {
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
              TrajInfo << TX_VARS(cur_level_HashNodeInfo.EndScope()) << vec_sampling_traj_pts.back().Str();
            }
          } else {
            const Base::txFloat real_length = cur_level_HashNodeInfo.RealLength();
            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.StartScope() + 0.35 * real_length, LaneOffset()));
            TrajInfo << TX_VARS(real_length) << vec_sampling_traj_pts.back().Str();
            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.StartScope() + 0.7 * real_length, LaneOffset()));
            TrajInfo << TX_VARS(real_length) << vec_sampling_traj_pts.back().Str();
            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.EndScope(), LaneOffset()));
            TrajInfo << TX_VARS(real_length) << vec_sampling_traj_pts.back().Str();
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
      if (IsOnLane() && CallSucc(IsInLaneChange())) {
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
      if (FLAGS_LogLevel_Ego_Traj) {
        for (const auto& ref : vec_sampling_traj_pts) {
          LOG(INFO) << "[trajectory]" << TX_VARS_NAME(SamplingNode, ref.Str());
        }
      }

      const Base::txFloat ego_alt = Altitude();
      if (vec_sampling_traj_pts.size() > 1) {
        TrafficFlow::CentripetalCatMull::control_path_node_vec ref_control_path;
        {
          /*first node real position*/
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = RawVehicleCoord().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        for (Base::txSize idx = 1; idx < vec_sampling_traj_pts.size(); ++idx) {
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = vec_sampling_traj_pts[idx].SamplingPt().ToWGS84().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        mPlanningTrajectoryPtr = std::make_shared<TrafficFlow::CentripetalCatMull>(ref_control_path);
        {
          TrafficFlow::CentripetalCatMull::control_path_node_vec ref_control_path_centerline;
          auto center_sampling_pts = vec_sampling_traj_pts;
          for (auto& refpts : center_sampling_pts) {
            refpts.ClearLaneOffset();
            refpts.ComputeSamplingPoint();

            Base::ISceneLoader::IRouteViewer::control_path_node path_node;
            path_node.waypoint = refpts.SamplingPt().ToWGS84().WGS84();
            ref_control_path_centerline.emplace_back(path_node);
          }
          mPlanningCenterLineTrajectoryPtr =
              std::make_shared<TrafficFlow::CentripetalCatMull>(ref_control_path_centerline);
        }
        if (NonNull_Pointer(mPlanningTrajectoryPtr)) {
          const Base::txFloat traj_real_length = mPlanningTrajectoryPtr->GetLength();
          Coord::txWGS84 end_wgs84;
          end_wgs84.FromENU(mPlanningTrajectoryPtr->GetEndPt());
#if 1
          const Base::txFloat _zeroRadian =
              Utils::GetLaneAngleFromVectorOnENU(mPlanningTrajectoryPtr->GetLaneDir(0.0)).GetRadian();
          Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::Half_PI(),
                                     _zeroRadian + Unit::CircleRadian::Half_PI(), _zeroRadian);
          refTraj.set_a(GetAcc());
          TrajInfo << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());
          Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
          Base::txFloat cur_acc = 0.0;  // GetAcc();
          Base::txFloat cur_s = 0.0;
          Base::txFloat cur_radian = m_dynamic_location.mutable_rpy()->z();

          for (Base::txFloat start_s = 0.0; start_s <= FLAGS_ego_trajectory_duration_s;
               start_s += timeMgr.RelativeTime()) {
            if (cur_s < traj_real_length && cur_s < (cur_velocity * FLAGS_ego_trajectory_duration_s)) {
              sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
              new_traj_point_ptr->set_z(ego_alt);
              new_traj_point_ptr->set_t(start_s + timeMgr.AbsTime());
              new_traj_point_ptr->set_v(cur_velocity);
              new_traj_point_ptr->set_a(GetAcc());
              new_traj_point_ptr->set_s(cur_s);

              /*1. update velocity*/
              cur_velocity += cur_acc * timeMgr.RelativeTime();
              if (cur_velocity >= mKinetics.m_velocity_max) {
                cur_velocity = mKinetics.m_velocity_max;
                cur_acc = 0.0;
              }
              if (cur_velocity < 0.0) {
                cur_velocity = 0.0;
                cur_acc = 0.0;
              }

              Coord::txWGS84 cur_pt_wgs84;
              cur_pt_wgs84.FromENU(mPlanningTrajectoryPtr->GetLocalPos(new_traj_point_ptr->s()));
              new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
              new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(mPlanningTrajectoryPtr->GetLaneDir(new_traj_point_ptr->s()))
                      .GetRadian();
              const Base::txFloat wrap_headingRadian = circRad.Wrap(_headingRadian);
              // LogWarn << TX_VARS_NAME(from, _headingRadian) << TX_VARS_NAME(wrap, wrap_headingRadian);

              cur_radian =
                  AdjustThetaFromLastRpy(cur_radian, wrap_headingRadian,
                                         0.02 * cur_velocity * Unit::CircleRadian::PI() * timeMgr.RelativeTime());
              new_traj_point_ptr->set_theta(cur_radian);
              /*2. update s*/
              cur_s += cur_velocity * timeMgr.RelativeTime();
            } else {
              break;
            }
          }
          TrajInfo << TX_VARS(cur_s) << TX_VARS(cur_velocity) << TX_VARS(cur_acc) << TX_VARS(timeMgr.RelativeTime());

          if (refTraj.point_size() < 2) {
            refTraj.clear_point();
            sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
            new_traj_point_ptr->set_z(ego_alt);
            new_traj_point_ptr->set_t(timeMgr.AbsTime());
            new_traj_point_ptr->set_v(cur_velocity);
            new_traj_point_ptr->set_a(0.0);
            new_traj_point_ptr->set_s(0.0);

            Coord::txWGS84 cur_pt_wgs84 = RawVehicleCoord();
            new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
            new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
            const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
            new_traj_point_ptr->set_theta(_headingRadian);

            sim_msg::TrajectoryPoint* new_clone_traj_point_ptr = refTraj.add_point();
            new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
          }
#else
          for (Base::txSize idx = 0; idx < vec_ms_time_traj_point.size(); ++idx) {
            auto& ref_traj_sample = vec_ms_time_traj_point[idx];
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
          LOG(WARNING) << "laneGeomPtr is nullptr.";
          ParentClass::FillingTrajectory(timeMgr, refTraj);
          Base::txFloat _curent = m_dynamic_location.mutable_rpy()->z();
          for (int i = 0; i < refTraj.mutable_point()->size(); ++i) {
            sim_msg::TrajectoryPoint* point = refTraj.mutable_point()->Mutable(i);
            _curent = AdjustThetaFromLastRpy(_curent, point->theta(),
                                             0.08 * Unit::CircleRadian::PI() * timeMgr.RelativeTime());
            point->set_theta(_curent);
          }
          return false;
        }
      } else if (1 == vec_sampling_traj_pts.size()) {
        sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(timeMgr.AbsTime());
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(0.0);
        new_traj_point_ptr->set_s(0.0);

        Coord::txWGS84 cur_pt_wgs84 = vec_sampling_traj_pts.front().SamplingPt().ToWGS84();
        new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
        new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
        const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
        const Base::txFloat _adjust_headingRadian =
            AdjustThetaFromLastRpy(m_dynamic_location.mutable_rpy()->z(), _headingRadian,
                                   0.08 * Unit::CircleRadian::PI() * timeMgr.RelativeTime());
        new_traj_point_ptr->set_theta(_adjust_headingRadian);

        sim_msg::TrajectoryPoint* new_clone_traj_point_ptr = refTraj.add_point();
        new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
      } else {
#if 1
        sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(timeMgr.AbsTime());
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(0.0);
        new_traj_point_ptr->set_s(0.0);

        Coord::txWGS84 cur_pt_wgs84 = RawVehicleCoord();
        new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
        new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
        const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
        const Base::txFloat _adjust_headingRadian =
            AdjustThetaFromLastRpy(m_dynamic_location.mutable_rpy()->z(), _headingRadian,
                                   0.08 * Unit::CircleRadian::PI() * timeMgr.RelativeTime());
        new_traj_point_ptr->set_theta(_adjust_headingRadian);

        sim_msg::TrajectoryPoint* new_clone_traj_point_ptr = refTraj.add_point();
        new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
#endif
      }
    }
    // LOG(WARNING) << TX_VARS(timeMgr.PassTime()) << TX_VARS(refTraj.point_size());
    if (FLAGS_LogLevel_Ego_Traj) {
      std::ostringstream oss;
      oss << "[trajectory]" << timeMgr.AbsTime();
      for (const auto& refNode : refTraj.point()) {
                oss << _StreamPrecision_ << TX_VARS_NAME({ x, refNode.x()) << TX_VARS_NAME(y, refNode.y())
                    << TX_VARS_NAME(t, refNode.t()) << TX_VARS_NAME(theta, refNode.theta())
                    << TX_VARS_NAME(v, refNode.v()) << TX_VARS_NAME(a, refNode.a())
                    << TX_VARS_NAME(s}, refNode.s());
      }
      LogWarn << oss.str();
    }
    return true;
  } else {
    ParentClass::FillingTrajectory(timeMgr, refTraj);
    Base::txFloat _curent = m_dynamic_location.mutable_rpy()->z();
    for (int i = 0; i < refTraj.mutable_point()->size(); ++i) {
      sim_msg::TrajectoryPoint* point = refTraj.mutable_point()->Mutable(i);
      _curent =
          AdjustThetaFromLastRpy(_curent, point->theta(), 0.08 * Unit::CircleRadian::PI() * timeMgr.RelativeTime());
      point->set_theta(_curent);
    }
    return false;
  }
}

Base::txBool SimPlanningVehicleElement::InjectEnv(Base::TimeParamManager const& timeMgr,
                                                  const txString& strPB) TX_NOEXCEPT {
  m_env_traffic.ParseFromString(strPB);
  LogInfo << "[ego_rsv_env_perception]" << TX_VARS_NAME(TimeStamp, timeMgr.TimeStamp())
          << TX_VARS_NAME(car_size, m_env_traffic.cars_size())
          << TX_VARS_NAME(signal_size, m_env_traffic.trafficlights_size())
          << TX_VARS_NAME(pedestrian_size, m_env_traffic.dynamicobstacles_size())
          << TX_VARS_NAME(obstacle_size, m_env_traffic.staticobstacles_size());
  return true;
}

Base::txBool SimPlanningVehicleElement::InjectLoc(Base::TimeParamManager const& timeMgr,
                                                  const txString& strPB) TX_NOEXCEPT {
  m_dynamic_location.ParseFromString(strPB);
  LOG_IF(INFO, FLAGS_LogLevel_Ego_Traj_ClosedLoop)
      << "[ego_rsv_location]" << TX_VARS_NAME(TimeStamp, timeMgr.TimeStamp())
      << TX_VARS_NAME(msg_t, m_dynamic_location.t())
      << TX_VARS_NAME(ego_pos, Utils::ToString(m_dynamic_location.position()))
      << TX_VARS_NAME(ego_angular, Utils::ToString(m_dynamic_location.angular()))
      << TX_VARS_NAME(ego_velocity, Utils::ToString(m_dynamic_location.velocity()))
      << TX_VARS_NAME(ego_acc, Utils::ToString(m_dynamic_location.acceleration()))
      << TX_VARS_NAME(ego_rpy, Utils::ToString(m_dynamic_location.rpy()));
  return true;
}

Base::txBool SimPlanningVehicleElement::InjectUnionLoc(Base::TimeParamManager const& timeMgr, const txString groupName,
                                                       const txString& strPB) TX_NOEXCEPT {
  m_groupName = groupName;
  m_union_location.ParsePartialFromString(strPB);
  LogInfo << "[ego_rsv_union_loc]";
  for (int i = 0; i < m_union_location.messages_size(); ++i) {
    if (txString(m_union_location.messages().at(i).groupname()) != m_groupName) {
      LogWarn << TX_VARS_NAME(TimeStamp, timeMgr.TimeStamp()) << TX_VARS_NAME(msg_t, m_dynamic_location.t())
              << TX_VARS_NAME(ego_pos, Utils::ToString(m_dynamic_location.position()))
              << TX_VARS_NAME(ego_angular, Utils::ToString(m_dynamic_location.angular()))
              << TX_VARS_NAME(ego_velocity, Utils::ToString(m_dynamic_location.velocity()))
              << TX_VARS_NAME(ego_acc, Utils::ToString(m_dynamic_location.acceleration()))
              << TX_VARS_NAME(ego_rpy, Utils::ToString(m_dynamic_location.rpy()));
    }
  }
  return true;
}

hadmap::txLaneLinkPtr SimPlanningVehicleElement::RandomNextLink(const hadmap::txLaneLinks& nextLaneLinks) TX_NOEXCEPT {
  if (_NonEmpty_(nextLaneLinks)) {
    for (const auto& refLinkPtr : nextLaneLinks) {
      if (NonNull_Pointer(refLinkPtr) && CallSucc(mRouteAI.CheckReachableRoad(refLinkPtr->toRoadId()))) {
        return refLinkPtr;
      }
    }
    const auto nRnd = mPRandom.GetRandomInt();
    return nextLaneLinks.at(Math::RangeInt32(nRnd, nextLaneLinks.size()));
  } else {
    return nullptr;
  }
}

Base::txBool SimPlanningVehicleElement::UpdateLocation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  RawVehicleCoord().FromWGS84(m_dynamic_location.position().x(), m_dynamic_location.position().y(),
                              m_dynamic_location.position().z());
  SyncPosition(timeMgr.PassTime());
  const Base::txVec2 vPos_Enu2D = RawVehicleCoord().ToENU().ENU2D();
  const Base::txVec2 last_EgoMassCenter = StableRawVehicleCoord().ENU2D();
  if (timeMgr.RelativeTime() > 0.0) {
    const Base::txFloat o_dist = (vPos_Enu2D - last_EgoMassCenter).norm();
    Base::Info_Lane_t new_laneLocInfo;
    Base::txFloat new_dist_on_curve = 0.0;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
        RawVehicleCoord(), new_laneLocInfo, new_dist_on_curve);
    const Base::Info_Lane_t& old_laneLocInfo = mLocation.LaneLocInfo();
    const Base::txFloat old_dist_on_curve = mLocation.DistanceAlongCurve();

    Base::txFloat p_dist = 0.0;
    if (new_laneLocInfo == old_laneLocInfo) {
      p_dist = new_dist_on_curve - old_dist_on_curve;
      p_dist = (p_dist > 0.0) ? (p_dist) : (0.0);
    } else {
      txFloat old_delta = mLocation.tracker()->getLaneGeomInfo()->GetLength() - old_dist_on_curve;
      old_delta = (old_delta > 0.0) ? (old_delta) : (0.0);
      p_dist = old_delta + new_dist_on_curve;
    }
    const txFloat dist = (o_dist > p_dist) ? (p_dist) : (o_dist);
    SetPlanningVelocity(dist / timeMgr.RelativeTime());
    mKinetics.m_acceleration = (mKinetics.m_velocity - StableVelocity()) / timeMgr.RelativeTime();
    LogInfo << TX_VARS_NAME(ego_dist, dist) << TX_VARS_NAME(RawVehicleCoord, RawVehicleCoord().StrWGS84())
            << TX_VARS_NAME(StableRawVehicleCoord, StableRawVehicleCoord().ToWGS84().StrWGS84());
    LogInfo << "[Ego]" /* << TX_VARS(StablePosition()) << TX_VARS(GetPosition()) */ << TX_VARS(dist)
            << TX_VARS(mKinetics.m_velocity);
  } else {
    SetPlanningVelocity(0.0);
    mKinetics.m_acceleration = 0.0;
  }

  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
          RawVehicleCoord(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset())) {
    LogInfo << "Get_S_Coord_By_cEnu_Pt : " << mLocation.LaneLocInfo();
    if (mLocation.IsOnLane()) {
      hadmap::txLanePtr resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      if (NonNull_Pointer(resLane)) {
        LogInfo << "hadmap::getLane success.";
        RelocateTracker(resLane, timeMgr.TimeStamp());
      } else {
        LogWarn << "Ego not on road.";
        return false;
      }
      m_lastLaneLink = false;
    } else {
      hadmap::txLaneLinkPtr _curresLaneLink = NULL;
      if (m_lastLaneLink == false && m_LastLocation.IsOnLane()) {
        hadmap::txLaneLinks resLinks =
            HdMap::HadmapCacheConCurrent::GetLaneNextLinks(m_LastLocation.LaneLocInfo().onLaneUid);
        if (resLinks.size() > 0) {
          double fmAngle = 360;
          for (auto it : resLinks) {
            double startYaw = 0;
            if (it->getGeometry() && !it->getGeometry()->empty()) {
              const hadmap::txLineCurve* pLinecurve = dynamic_cast<const hadmap::txLineCurve*>(it->getGeometry());
              startYaw = pLinecurve->yaw(0);
              int nSize = pLinecurve->size();
              double endYaw = pLinecurve->yaw(pLinecurve->size() - 1);
              double yawDiff = (endYaw - startYaw);
              if (yawDiff > 180) yawDiff -= 360;
              if (yawDiff < -180) yawDiff += 360;
              // LogInfo <<"toroad id  " <<it->toRoadId();
              // LogInfo <<"yawDiff "  <<yawDiff;
              // LogInfo <<"startYaw " <<startYaw;
              // LogInfo <<"endYaw "   <<endYaw;
              if (std::abs(yawDiff) < fmAngle) {
                _curresLaneLink = it;
                fmAngle = std::abs(yawDiff);
              }
            }
          }
        }
        mLocation.LaneLocInfo() =
            Base::Info_Lane_t(_curresLaneLink->getId(), _curresLaneLink->fromTxLaneId(), _curresLaneLink->toTxLaneId());
      }
      if (m_lastLaneLink == true) {
        if (!(mLocation.LaneLocInfo().onLinkFromLaneUid == m_LastLocation.LaneLocInfo().onLinkFromLaneUid) ||
            !(mLocation.LaneLocInfo().onLinkToLaneUid == m_LastLocation.LaneLocInfo().onLinkToLaneUid)) {
          mLocation.LaneLocInfo() = m_LastLocation.LaneLocInfo();
        }
        _curresLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                         mLocation.LaneLocInfo().onLinkToLaneUid);
      }

      // if (m_lastLaneLink == true) {
      //   double _ss = 0;
      //     double _tt = 0;
      //     if (NonNull_Pointer(_curresLaneLink)){
      //         const hadmap::txLineCurve* pLinecurve = dynamic_cast<const
      //         hadmap::txLineCurve*>(_curresLaneLink->getGeometry()); double yaw = 0;
      //         pLinecurve->xy2sl(m_dynamic_location.position().x(), m_dynamic_location.position().y(),_ss, _tt ,
      //         yaw);
      //     }
      //     if (mLocation.DistanceAlongCurve() > 5.0 || _ss > 5.0) {

      //   }
      // }
      // hadmap::txLaneLinkPtr resLaneLink =
      // HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
      // mLocation.LaneLocInfo().onLinkToLaneUid);
      if (NonNull_Pointer(_curresLaneLink)) {
        const hadmap::txLineCurve* pLinecurve =
            dynamic_cast<const hadmap::txLineCurve*>(_curresLaneLink->getGeometry());
        double yaw = 0;
        pLinecurve->xy2sl(m_dynamic_location.position().x(), m_dynamic_location.position().y(),
                          mLocation.DistanceAlongCurve(), mLocation.LaneOffset(), yaw);
        // mLocation.LaneOffset() = 0;
        // LogInfo<<"mLocation.DistanceAlongCurve()   " << mLocation.DistanceAlongCurve();
        LogInfo << "hadmap::getLaneLink success.";
        RelocateTracker(_curresLaneLink, timeMgr.TimeStamp());
      } else {
        LogWarn << "Ego not on link.";
        return false;
      }
      m_lastLaneLink = true;
    }
    // LogInfo << "Get_S_Coord_By_Enu_Pt222 : " << mLocation.LaneLocInfo();
    m_LastLocation = mLocation;
  } else {
    LogWarn << "Get_S_Coord_By_Enu_Pt failure.";
    return false;
  }

  Base::txVec2 yawDir(std::cos(m_dynamic_location.rpy().z()), std::sin(m_dynamic_location.rpy().z()));
  ComputeProjectionMat(vPos_Enu2D, (vPos_Enu2D + 2.0 * yawDir));
  mLocation.vLaneDir() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);

#if __TX_Mark__("accept controler acc&velocity")
  Base::txVec3 inject_velocity(m_dynamic_location.velocity().x(), m_dynamic_location.velocity().y(),
                               m_dynamic_location.velocity().z());
  SetPlanningVelocity(Utils::Vec3_Vec2(inject_velocity).norm());
  Base::txVec3 inject_acc(m_dynamic_location.acceleration().x(), m_dynamic_location.acceleration().y(),
                          m_dynamic_location.acceleration().z());
  mKinetics.m_acceleration = Utils::Vec3_Vec2(inject_acc).norm();

  Unit::txDegree velocity_deg, acc_deg;
  velocity_deg.FromRadian(Utils::RadianBetweenVector2d(
      yawDir, Base::txVec2(m_dynamic_location.velocity().x(), m_dynamic_location.velocity().y())));
  acc_deg.FromRadian(Utils::RadianBetweenVector2d(
      yawDir, Base::txVec2(m_dynamic_location.acceleration().x(), m_dynamic_location.acceleration().y())));
  if (acc_deg.GetDegree() > 90.0) {
    mKinetics.m_acceleration *= -1.0;
  }
  /*mKinetics.m_velocity = m_dynamic_location.velocity().y();
  mKinetics.m_acceleration = m_dynamic_location.acceleration().y();*/
  LOG(WARNING) << "[Inject_controler_acc_v]" << TX_VARS(Utils::ToString(yawDir)) << TX_VARS(timeMgr.PassTime())
               << TX_VARS(velocity_deg.GetDegree()) << TX_VARS(mKinetics.m_velocity) << TX_VARS(acc_deg.GetDegree())
               << TX_VARS(mKinetics.m_acceleration);
#endif /*__TX_Mark__("accept controler acc&velocity")*/

  mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());

  if (CallSucc(mLocation.IsOnLaneLink())) {
    mLocation.LocalCoord_AxisY() =
        HdMap::HadmapCacheConCurrent::GetLaneLinkDir(mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve());
    LogInfo << TX_VARS_NAME(Ego_OnLink_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else if (CallSucc(mLocation.IsOnLane())) {
    mLocation.LocalCoord_AxisY() =
        HdMap::HadmapCacheConCurrent::GetLaneDir(mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve());
    LogInfo << TX_VARS_NAME(Ego_OnLane_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else {
    mLocation.LocalCoord_AxisY() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);
    LogInfo << TX_VARS_NAME(Ego_NotOnMap_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  }
  mLocation.LocalCoord_AxisX() = Utils::VetRotVecByDegree(mLocation.LocalCoord_AxisY(),
                                                          Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
  LogInfo << TX_VARS_NAME(Ego_LocalCoord_AxisX, Utils::ToString(mLocation.LocalCoord_AxisX()));
  mLocation.NextLaneIndex() = Utils::ComputeLaneIndex(mLocation.LaneLocInfo().onLaneUid.laneId);
  return true;
}

Base::txBool SimPlanningVehicleElement::SyncPosition(const txFloat _passTime) TX_NOEXCEPT {
  const Base::txVec2 dir2d(GetHeadingWithAngle().Cos(), GetHeadingWithAngle().Sin());
  if (CallSucc(RawVehicleCoord_Is_RearAxleCenter())) {
    TX_MARK("given rear_axle_center, compute geom_center");
    mLocation.RearAxleCenter() = RawVehicleCoord();
    if (CallSucc(IsLeader())) {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step;
      mLocation.GeomCenter() = mLocation.RearAxleCenter();
      mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      // LOG(INFO) << "[ego_leader][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step);
    } else {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step;
      mLocation.GeomCenter() = mLocation.RearAxleCenter();
      mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      // LOG(INFO) << "[ego_follow][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step);
    }
  } else if (CallSucc(RawVehicleCoord_Is_GeomCenter())) {
    TX_MARK("given geom_center, compute rear_axle_center");
    mLocation.GeomCenter() = RawVehicleCoord();
    if (CallSucc(IsLeader())) {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step * -1.0;
      mLocation.RearAxleCenter() = mLocation.GeomCenter();
      mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      LOG(INFO) << "[ego_leader][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step * -1.0);
    } else {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step * -1.0;
      mLocation.RearAxleCenter() = mLocation.GeomCenter();
      mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      LOG(INFO) << "[ego_follow][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step * -1.0);
    }
  } else {
    mLocation.RearAxleCenter() = RawVehicleCoord();
    mLocation.GeomCenter() = RawVehicleCoord();
    LogWarn << "unsupport Vehicle Coord Type.";
    return false;
  }
  return true;
}

Coord::txENU SimPlanningVehicleElement::StableRawVehicleCoord() const TX_NOEXCEPT {
  if (CallSucc(RawVehicleCoord_Is_RearAxleCenter())) {
    return StableRearAxleCenter();
  } else if (CallSucc(RawVehicleCoord_Is_GeomCenter())) {
    return StableGeomCenter();
  } else {
    return StableGeomCenter();
  }
}

SimPlanningVehicleElement::SceneEventTriggerInfoPtr SimPlanningVehicleElement::QuerySceneEventTriggerInfoFromEnvData(
    const ElementType _type, const Base::txSysId _elemId) const TX_NOEXCEPT {
  switch (_type) {
    case ElementType::TAD_Vehicle: {
      return QuerySceneEventTriggerInfoFromEnvData_Car(_elemId);
    }
    case ElementType::TAD_Pedestrian: {
      return QuerySceneEventTriggerInfoFromEnvData_Ped(_elemId);
    }
    case ElementType::TAD_Immovability: {
      return QuerySceneEventTriggerInfoFromEnvData_Obs(_elemId);
    }
    case ElementType::TAD_SignalLight: {
    }
    case ElementType::TAD_Ego: {
    }
  }
  return nullptr;
}

SimPlanningVehicleElement::SceneEventTriggerInfoPtr
SimPlanningVehicleElement::QuerySceneEventTriggerInfoFromEnvData_Car(const Base::txSysId _elemId) const TX_NOEXCEPT {
  for (const auto& refCar : m_env_traffic.cars()) {
    if (_elemId == refCar.id()) {
      SceneEventTriggerInfoPtr retPtr = std::make_shared<SceneEventTriggerInfo>();
      retPtr->mLateralVelocity = refCar.vl();
      retPtr->mRawVelocity = refCar.v();
      retPtr->mId = refCar.id();
      retPtr->mStableGeomCenter.FromWGS84(refCar.x(), refCar.y(), refCar.z());
      retPtr->mLength = refCar.length();
      retPtr->mWidth = refCar.width();
      retPtr->mHeight = refCar.height();

      Unit::txRadian rad;
      rad.FromRadian(refCar.heading());
      const Base::txVec2 frontAxis2d_hasNormalized(rad.Cos(), rad.Sin());
      const Base::txVec3 frontAxis3d_hasNormalized = Utils::Vec2_Vec3(frontAxis2d_hasNormalized);
      const Base::txVec3 rightAxis3d = Utils::VetRotVecByDegree(
          frontAxis3d_hasNormalized, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
      const Base::txVec2 rightAxis2d = Utils::Vec3_Vec2(rightAxis3d);
      TAG("enu2d");
      Geometry::OBB2D obb(retPtr->mStableGeomCenter.ENU2D(), frontAxis2d_hasNormalized, rightAxis2d, refCar.length(),
                          refCar.width());
      retPtr->mPolygonArray = obb.GetPolygon();
      return retPtr;
    }
  }
  return nullptr;
}

SimPlanningVehicleElement::SceneEventTriggerInfoPtr
SimPlanningVehicleElement::QuerySceneEventTriggerInfoFromEnvData_Ped(const Base::txSysId _elemId) const TX_NOEXCEPT {
  for (const auto& refPed : m_env_traffic.dynamicobstacles()) {
    if (std::abs(_elemId) == std::abs(refPed.id())) {
      SceneEventTriggerInfoPtr retPtr = std::make_shared<SceneEventTriggerInfo>();
      retPtr->mLateralVelocity = refPed.vl();
      retPtr->mRawVelocity = refPed.v();
      retPtr->mId = refPed.id();
      retPtr->mStableGeomCenter.FromWGS84(refPed.x(), refPed.y(), refPed.z());
      retPtr->mLength = refPed.length();
      retPtr->mWidth = refPed.width();
      retPtr->mHeight = refPed.height();

      Unit::txRadian rad;
      rad.FromRadian(refPed.heading());
      const Base::txVec2 frontAxis2d_hasNormalized(rad.Cos(), rad.Sin());
      const Base::txVec3 frontAxis3d_hasNormalized = Utils::Vec2_Vec3(frontAxis2d_hasNormalized);
      const Base::txVec3 rightAxis3d = Utils::VetRotVecByDegree(
          frontAxis3d_hasNormalized, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
      const Base::txVec2 rightAxis2d = Utils::Vec3_Vec2(rightAxis3d);
      TAG("enu2d");
      Geometry::OBB2D obb(retPtr->mStableGeomCenter.ENU2D(), frontAxis2d_hasNormalized, rightAxis2d, refPed.length(),
                          refPed.width());
      retPtr->mPolygonArray = obb.GetPolygon();

      return retPtr;
    }
  }
  return nullptr;
}

SimPlanningVehicleElement::SceneEventTriggerInfoPtr
SimPlanningVehicleElement::QuerySceneEventTriggerInfoFromEnvData_Obs(const Base::txSysId _elemId) const TX_NOEXCEPT {
  for (const auto& refObs : m_env_traffic.staticobstacles()) {
    if (_elemId == refObs.id()) {
      SceneEventTriggerInfoPtr retPtr = std::make_shared<SceneEventTriggerInfo>();
      retPtr->mLateralVelocity = 0.0;
      retPtr->mRawVelocity = 0.0;
      retPtr->mId = refObs.id();
      retPtr->mStableGeomCenter.FromWGS84(refObs.x(), refObs.y(), refObs.z());
      retPtr->mLength = refObs.length();
      retPtr->mWidth = refObs.width();
      retPtr->mHeight = refObs.height();

      Unit::txRadian rad;
      rad.FromRadian(refObs.heading());
      const Base::txVec2 frontAxis2d_hasNormalized(rad.Cos(), rad.Sin());
      const Base::txVec3 frontAxis3d_hasNormalized = Utils::Vec2_Vec3(frontAxis2d_hasNormalized);
      const Base::txVec3 rightAxis3d = Utils::VetRotVecByDegree(
          frontAxis3d_hasNormalized, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
      const Base::txVec2 rightAxis2d = Utils::Vec3_Vec2(rightAxis3d);
      TAG("enu2d");
      Geometry::OBB2D obb(retPtr->mStableGeomCenter.ENU2D(), frontAxis2d_hasNormalized, rightAxis2d, refObs.length(),
                          refObs.width());
      retPtr->mPolygonArray = obb.GetPolygon();

      return retPtr;
    }
  }
  return nullptr;
}

Base::txFloat SimPlanningVehicleElement::Compute_EGO_Distance(SceneEventTriggerInfoPtr target_ptr,
                                                              const DistanceProjectionType projType) TX_NOEXCEPT {
  using namespace Geometry::CD2D;

  if (GJK::HitPolygons(target_ptr->GetPolygon(), GetPolygon())) {
    return 0.0;
  } else {
    if ((_plus_(DistanceProjectionType::Lane)) == projType) {
      TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
      return ComputeProjectDistance(GetPolygon(), target_ptr->GetPolygon());
    } else if ((_plus_(DistanceProjectionType::Frenet)) == projType) {
      const Base::ILocalCoord::FrenetProjInfo projInfo = ComputeRoadProjectDistance(target_ptr->StableGeomCenter());
      if (projInfo.mIsValid) {
        return projInfo.mProjDistance;
      } else {
        return FLT_MAX;
      }
    } else {
      Base::txVec2 nouse_intersectPt1, nouse_intersectPt2;
      return smallest_dist_between_ego_and_obsVehicle(target_ptr->GetPolygon(), GetPolygon(), nouse_intersectPt1,
                                                      nouse_intersectPt2);
    }
  }
}

Base::txFloat SimPlanningVehicleElement::Compute_TTC_Distance(SceneEventTriggerInfoPtr target_ptr,
                                                              const DistanceProjectionType projType) TX_NOEXCEPT {
  TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
  Base::txFloat ttc = FLT_MAX;
  Base::txFloat distance = 0.0, sign = 0.0;
  if (_plus_(DistanceProjectionType::Frenet) == projType) {
    const Base::ILocalCoord::FrenetProjInfo projInfo = ComputeRoadProjectDistance(target_ptr->StableGeomCenter());
    if (CallSucc(projInfo.mIsValid)) {
      distance = projInfo.mProjDistance;
      sign = __enum2int__(Base::Enums::Element_Spatial_Relationship, projInfo.mTarget2Source);
    } else {
      LogWarn << "ComputeRoadProjectDistance failure. " << TX_VARS(Id()) << TX_VARS(target_ptr->Id());
      return ttc;
    }
  } else {
    distance = Compute_EGO_Distance(target_ptr, projType);
    sign = Compute_EGO_Distance(target_ptr, DistanceProjectionType::Lane);
  }

  const Base::txFloat egoVelocity = target_ptr->GetRawVelocity();
  const Base::txFloat vehicleVelocity = GetRawVelocity();
  LOG(INFO) << TX_VARS_NAME(srcId, Id()) << TX_VARS_NAME(targetId, target_ptr->Id())
            << TX_VARS_NAME(relationship, ((sign > 0.0) ? ("src front of target")
                                                        : ((sign < 0.0) ? ("src behind of target") : ("overlap"))))
            << TX_VARS_NAME(dist, std::fabs(distance)) << TX_VARS_NAME(src_v, vehicleVelocity)
            << TX_VARS_NAME(target_v, egoVelocity);

  if (sign > 0.0) {
    LogInfo << "[TTC_Distance] on lane projection, vehicle is front of ego." << _StreamPrecision_ << TX_VARS(sign);
    TX_MARK("on laneprojection, vehicle is front of ego.");
    if (!Math::isZero(egoVelocity - vehicleVelocity, 0.1)) {
      if (egoVelocity > vehicleVelocity) {
        ttc = std::fabs(distance) / (egoVelocity - vehicleVelocity);
      } else {
        TX_MARK("impossible collision");
      }
    } else {
      TX_MARK("impossible collision");
    }
  } else if (sign < 0.0) {
    LogInfo << "[TTC_Distance] on lane projection, vehicle is behind of ego." << _StreamPrecision_ << TX_VARS(sign);
    TX_MARK("on laneprojection, vehicle is behand of ego.");
    if (!Math::isZero(egoVelocity - vehicleVelocity, 0.1)) {
      if (vehicleVelocity > egoVelocity) {
        ttc = std::fabs(distance) / (vehicleVelocity - egoVelocity);
      } else {
        TX_MARK("impossible collision");
      }
    } else {
      TX_MARK("impossible collision");
    }
  } else {
    TX_MARK("has been collision.");
    LogInfo << "[TTC_Distance] vehicle and ego have collided." << _StreamPrecision_ << TX_VARS(sign);
    ttc = 0.0;
  }

  LogInfo << "[TTC_Distance] " << TX_VARS(distance) << TX_VARS(egoVelocity - vehicleVelocity) << TX_VARS(egoVelocity)
          << TX_VARS(vehicleVelocity) << TX_VARS(ttc);
  return std::fabs(ttc);
}

SimPlanningVehicleElement::FrenetProjInfo SimPlanningVehicleElement::ComputeRoadProjectDistance(
    const Coord::txENU& target_element_geom_center) const TX_NOEXCEPT {
  FrenetProjInfo res;
  if (NonNull_Pointer(mLocation.tracker()) && NonNull_Pointer(mLocation.tracker()->getRoadGeomInfo())) {
    Base::txVec2 sourceST, targetST;
    mLocation.tracker()->getRoadGeomInfo()->xy2sl(StableGeomCenter().ENU2D(), sourceST.x(), sourceST.y());
    mLocation.tracker()->getRoadGeomInfo()->xy2sl(target_element_geom_center.ENU2D(), targetST.x(), targetST.y());

    Base::txFloat signDistance = targetST.x() - sourceST.x();
    res.mIsValid = true;
    res.mProjDistance = std::fabs(signDistance);
    if (signDistance > 0.0) {
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eFront);
    } else if (signDistance < 0.0) {
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eRear);
    } else {
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eOverlap);
    }
  }
  return res;
}

Base::txBool SimPlanningVehicleElement::RelocateTracker(hadmap::txLanePtr pLane,
                                                        const txFloat& _timestamp) TX_NOEXCEPT {
  if (CallSucc(ParentClass::RelocateTracker(pLane, _timestamp))) {
    if (FLAGS_planning_speedlimit_mode && pLane->getSpeedLimit() > SpeedLimitThreshold()) {
      LOG_IF(INFO, FLAGS_LogLevel_SpeedLimit)
          << "[SpeedLimit] " << TX_VARS_NAME(Id, Id()) << TX_VARS_NAME(laneUid, Utils::ToString(pLane->getTxLaneId()))
          << TX_VARS_NAME(SpeedLimitFromMap, pLane->getSpeedLimit())
          << TX_VARS_NAME(kmh_ms, Unit::kmh_ms(pLane->getSpeedLimit()));
      mKinetics.m_velocity_max = Unit::kmh_ms(pLane->getSpeedLimit());
    } else {
      mKinetics.m_velocity_max = mKinetics.raw_velocity_max;
    }
    Base::txFloat spLaneKeepTime = FLAGS_ego_lane_keep;
    if (HdMap::HadmapCacheConCurrent::QueryTrick_LaneKeepTime(pLane->getRoadId(), spLaneKeepTime)) {
      mLaneKeepTime = spLaneKeepTime;
    } else {
      mLaneKeepTime = RawLaneKeepTime();
    }
    return true;
  } else {
    return false;
  }
}

Base::txFloat SimPlanningVehicleElement::SignalReactionGap() const TX_NOEXCEPT {
  const Base::txFloat u = mKinetics.raw_velocity_max * 1.5; /*initial velocity*/
  const Base::txFloat v = 0.0;                              /*final velocity*/
  const Base::txFloat t = (0.0 - mKinetics.raw_velocity_max) / Max_Deceleration;
  const Base::txFloat s = 0.5 * (u + v) * t;

  return (Signal_Reaction_Gap < 0.0) ? (s) : (Signal_Reaction_Gap);
}

void SimPlanningVehicleElement::DecelerateLaneChange(const Base::txFloat velocity_factor,
                                                     const Base::txFloat time_factor) TX_NOEXCEPT {
  mBeforeChangeLane = velocity_factor * mKinetics.m_velocity;
  // mKinetics.m_velocity_max *= velocity_factor;
  mLaneKeepTime *= time_factor;
}

void SimPlanningVehicleElement::DecelerateLaneChange(const Base::txFloat velocity_factor) TX_NOEXCEPT {
  LogInfo << "mBeforeChangeLane = " << mBeforeChangeLane;
  if (mKinetics.m_velocity_max > mBeforeChangeLane) {
    mKinetics.m_velocity_max -= (velocity_factor * 0.02 * 20);
  }
}

#if __Control_V2__
#  define ControlInfo LOG_IF(INFO, FLAGS_LogLevel_DummyDriver_Control) << TX_VARS(timeMgr.PassTime())
void SimPlanningVehicleElement::UpdateControlPb(Base::TimeParamManager const& timeMgr, const Base::txFloat _acc,
                                                const Base::txFloat _front_wheel_angle) TX_NOEXCEPT {
  ClearControlPb();
  m_pb_control_v2.mutable_header()->set_time_stamp(timeMgr.TimeStamp());
  auto cmd_ptr = m_pb_control_v2.mutable_control_cmd();
  if (NonNull_Pointer(cmd_ptr)) {
    cmd_ptr->set_max_velocity(mKinetics.m_velocity_max);
    cmd_ptr->set_request_acc(_acc);
    cmd_ptr->set_request_steer_wheel_angle(0.0);

    if (CallSucc(FLAGS_Use_Pure_Pursuit) && NonNull_Pointer(mPlanningCenterLineTrajectoryPtr)) {
      ControlInfo << " start ...";
      /*1. alpha */
      const txFloat wheel_base = VehicleGeometory().vehicle_geometory().wheel_base();
      const Base::txVec2 rear_axle_pt = RawVehicleCoord().ToENU().ENU2D();
      AddDebugObject(timeMgr, -1, RawVehicleCoord().WGS84());
      txFloat g_fact = FLAGS_looking_distance_factor;

      const txFloat g_dist = (mKinetics.m_velocity * g_fact > 2.5 ? mKinetics.m_velocity * g_fact
                                                                  : 2.5);  //* FLAGS_looking_distance_factor;
      const Base::txVec2 g_pt = Utils::Vec3_Vec2(mPlanningCenterLineTrajectoryPtr->GetLocalPos(g_dist));
      Coord::txWGS84 tmpWGS84;
      tmpWGS84.FromENU(g_pt.x(), g_pt.y());
      Base::Info_Lane_t _tmplocInfo;
      Base::txFloat _ss;
      Base::txFloat _tt;
      bool flag = false;
      hadmap::txLanePtr resLane;
      if (mLocation.IsOnLane()) {
        resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
        if (resLane->getGeometry()->getLength() - mLocation.DistanceAlongCurve() < g_dist) {
          flag = true;
        }
      }
      AddDebugObject(timeMgr, -2, tmpWGS84.WGS84());
      Base::txFloat looking_distance = 0.0;
      if (flag == true)
        looking_distance = 1.0;
      else
        looking_distance = (rear_axle_pt - g_pt).norm();
      const Base::txVec2 g2r = (g_pt - rear_axle_pt);

      // const txFloat deg_angle = GetAngle().GetDegree();
      const txFloat rad_g2r = Utils::GetLaneAngleFromVectorOnENU(Utils::Vec2_Vec3(g2r)).GetRadian();

      /*const Base::txFloat _zeroRadian = GetHeadingWithAngle().GetRadian();
      {
          const Base::txVec2 heading_vec2 = Base::txVec2(std::cos(_zeroRadian) * looking_distance,
      std::sin(_zeroRadian)
      * looking_distance) + rear_axle_pt; tmpWGS84.FromENU(heading_vec2.x(), heading_vec2.y());
      AddDebugObject(timeMgr, -3, tmpWGS84.WGS84());
      }*/
      const Base::txFloat _zeroRadian = m_dynamic_location.rpy().z();
      const txFloat yaw_rad = Math::RadianNormalise(m_dynamic_location.rpy().z());
      {
        // Unit::txRadian yaw_rad; yaw_rad.FromRadian(m_dynamic_location.rpy().z());
        const Base::txVec2 rpy_z_vec2 =
            Base::txVec2(std::cos(yaw_rad) * looking_distance, std::sin(yaw_rad) * looking_distance) + rear_axle_pt;
        tmpWGS84.FromENU(rpy_z_vec2.x(), rpy_z_vec2.y());
        AddDebugObject(timeMgr, -4, tmpWGS84.WGS84());
      }
      Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::Half_PI(),
                                 _zeroRadian + Unit::CircleRadian::Half_PI(), _zeroRadian);

      Base::txFloat rad_alpha_t = circRad.Wrap(rad_g2r) - _zeroRadian;
      ControlInfo << " alpha" << TX_VARS(wheel_base) << TX_VARS(g_dist) << TX_VARS(mKinetics.m_velocity)
                  << TX_VARS(looking_distance)
                  << TX_VARS_NAME(yaw_rad, Math::RadianNormalise(m_dynamic_location.rpy().z()))
                  << TX_VARS_NAME(raw_yaw_rad, m_dynamic_location.rpy().z()) << TX_VARS(rad_alpha_t)
                  << TX_VARS(_zeroRadian) << TX_VARS(rad_g2r);

      if (looking_distance < 2.0) {
        cmd_ptr->set_request_steer_wheel_angle(0.0);
        ControlInfo << "looking_distance is zero, front_wheel_angle set 0.0.";
      } else {
        if (m_LaneChangeTimeMgr.Duration() <= 0.0) {
          looking_distance = 10;
        }
        const txFloat absLaneOffset = std::fabs(LaneOffset());
        const txFloat offset_factor = LaneOffset() / FLAGS_default_lane_width;
        rad_alpha_t += rad_alpha_t * offset_factor * FLAGS_control_centerline_factor;

        const txFloat param_1 = std::sin(rad_alpha_t) * wheel_base * 2.0;
        const txFloat param_2 = (param_1) / looking_distance;
        const txFloat front_wheel_angle_red = std::atan(param_2);
        const txFloat real_front_wheel_angle_rad = FLAGS_control_left_right_factor * front_wheel_angle_red;
        const txFloat real_front_wheel_degree = Math::Radians2Degrees(real_front_wheel_angle_rad);
        /*Unit::txDegree deg_front_wheel; deg_front_wheel.FromRadian(std::fabs(real_front_wheel_angle_rad));
        const txFloat real_front_wheel_degree = Math::Sign(real_front_wheel_angle_rad) *
        deg_front_wheel.GetDegree();*/
        const txFloat target_steer_angle_degree = real_front_wheel_degree * FLAGS_wheel2steer /* 18.0*/;
        /*Unit::txDegree target_steer_angle; target_steer_angle.FromRadian(front_wheel_angle * 18);*/
        ControlInfo << "m_LaneChangeTimeMgr.Duration()  = " << m_LaneChangeTimeMgr.Duration();
        if (m_LaneChangeTimeMgr.Duration() <= 0.0) {
          cmd_ptr->set_request_steer_wheel_angle(target_steer_angle_degree);
          mLastLaneChange = 9999;
        } else {
          if (mLastLaneChange > 1000) {
            mLastLaneChange = target_steer_angle_degree;
          }
          bool fLag_tmp = false;
          if (resLane) {
            double s = 0;
            double l = 0;
            double yaw = 0;
            resLane->getGeometry()->xy2sl(mLocation.GeomCenter().Lon(), mLocation.GeomCenter().Lat(), s, l, yaw);
            ControlInfo << yaw_rad * 180 / 3.14 - yaw << TX_VARS(yaw);
            if ((std::abs(std::abs(yaw_rad * 180 / 3.14) - yaw) > 10.0 &&
                 std::abs(std::abs(yaw_rad * 180 / 3.14) - yaw) < 90)) {
              mLastLaneChange = 0;
            }
            if ((std::abs(std::abs(std::abs(yaw_rad * 180 / 3.14) - yaw) - 360) > 10 &&
                 std::abs(std::abs(std::abs(yaw_rad * 180 / 3.14) - yaw) - 360) < 90)) {
              mLastLaneChange = 0;
            }
          }
          if (mLastLaneChange == 0) {
            cmd_ptr->set_request_steer_wheel_angle(0);
          } else if (mKinetics.m_velocity > 20) {
            cmd_ptr->set_request_steer_wheel_angle(mLastLaneChange / 3.6);
          } else if (mKinetics.m_velocity > 10) {
            cmd_ptr->set_request_steer_wheel_angle(mLastLaneChange / 3.4);
          } else if (mKinetics.m_velocity > 8) {
            cmd_ptr->set_request_steer_wheel_angle(mLastLaneChange / 3.0);
          } else {
            cmd_ptr->set_request_steer_wheel_angle(mLastLaneChange / 2.5);
          }
        }
        /*const txFloat trick_deg_alpha_t = Math::Radians2Degrees(rad_alpha_t);

        cmd_ptr->set_request_steer_wheel_angle(trick_deg_alpha_t * FLAGS_wheel2steer);*/
        const txFloat target_steer_angle_radian = Math::Degrees2Radians(cmd_ptr->request_steer_wheel_angle());

        ControlInfo << "[##########################]" << TX_VARS(real_front_wheel_degree)
                    << TX_VARS_NAME(request_steer_wheel_angle, cmd_ptr->request_steer_wheel_angle())
                    << TX_VARS(target_steer_angle_radian) << TX_VARS(param_1) << TX_VARS(param_2);
      }
    } else {
      ControlInfo << "do not use pursuit." << TX_VARS(_front_wheel_angle * FLAGS_control_left_right_factor);
    }
  }
}
#  undef ControlInfo
#endif /*__Control_V2__*/

void SimPlanningVehicleElement::SetPlanningVelocity(const txFloat v, const txBool _forced) TX_NOEXCEPT {
  if (_forced) {
    mKinetics.m_velocity = v;
    return;
  }

  if (CallFail(FLAGS_planning_const_velocity_mode)) {
    mKinetics.m_velocity = v;
  }
}

Base::txBool SimPlanningVehicleElement::EventChangeLane(Base::TimeParamManager const& timeMgr, txInt dir) TX_NOEXCEPT {
  mEventChangeLane = dir;
  return true;
}

double SimPlanningVehicleElement::AdjustThetaFromLastRpy(const Base::txFloat current_hdg,
                                                         const Base::txFloat target_hdg,
                                                         const Base::txFloat max_hdg_change) {
  Base::txFloat between_radian = std::abs(target_hdg - current_hdg);
  if (between_radian > Unit::CircleRadian::PI()) {
    between_radian = 2 * Unit::CircleRadian::PI() - between_radian;
  }
  if (between_radian > max_hdg_change) {
    if (target_hdg - current_hdg > Unit::CircleRadian::PI() ||
        (target_hdg - current_hdg < 0 && current_hdg - target_hdg < Unit::CircleRadian::PI())) {
      return current_hdg - max_hdg_change;
    } else {
      return current_hdg + max_hdg_change;
    }
  } else {
    return target_hdg;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
