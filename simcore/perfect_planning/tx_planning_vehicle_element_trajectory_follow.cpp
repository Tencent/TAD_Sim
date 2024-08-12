// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_vehicle_element_trajectory_follow.h"
#include "tx_planning_flags.h"
#include "tx_planning_scene_loader.h"
#include "tx_protobuf_utils.h"
#include "tx_spatial_query.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Element)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)
Base::txBool SimPlanningVehicleElement_TrajectoryFollow::InitializeEgo(sim_msg::Location& refEgoData,
                                                                       const control_path_node_vec& ref_control_path,
                                                                       ISceneLoader::IViewerPtr _viewPtr,
                                                                       ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
#if 1
  m_ego_init_location.CopyFrom(refEgoData);
#  if __Control_V2__
  InitControlV2Pb();
#  endif /*__Control_V2__*/
  if (NonNull_Pointer(_viewPtr) && NonNull_Pointer(_sceneLoader)) {
    if (CallFail(_sceneLoader->GetVehicleGeometory(0, Base::Enums::EgoSubType::eLeader, VehicleGeometory()))) {
      LogWarn << "GetVehicleGeometory failure.";
      return false;
    } else {
      LogInfo << VehicleGeometory().DebugString() << TX_VARS(VehicleGeometory().vehicle_geometory().length())
              << TX_VARS(VehicleGeometory().vehicle_geometory().rear_axle_to_rear());
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
#  if USE_SUDOKU_GRID
      mVehicleScanRegion2D.Initialize(mGeometryData.Length(), mGeometryData.Width());
#  endif /*USE_SUDOKU_GRID*/

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

          mLocation.LaneOffset() = 0.0;

          if (CallSucc(InitializeTrajMgr(ref_control_path, 0.0)) && NonNull_Pointer(m_trajMgr)) {
            LogWarn << "InitializeTrajMgr success";
            const Base::txVec3 enuPos = m_trajMgr->GetLocalPos(0.0);
            mLocation.PosOnLaneCenterLinePos().FromENU(enuPos);
            mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
            mLocation.vLaneDir() = m_trajMgr->GetLaneDir(0.0);
            RawVehicleCoord() =
                ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
            SyncPosition(0.0);
            mLocation.InitTracker(mIdentity.Id());
            RelocateTracker(initLane, 0.0);
#  if USE_SUDOKU_GRID
            /*heading_for_front_region_on_ENU = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());*/
            Compute_Region_Clockwise_Close(GeomCenter().ToENU(), mLocation.vLaneDir(),
                                           mLocation.heading_for_front_region_on_ENU());
#  endif /*USE_SUDOKU_GRID*/
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
            LogWarn << "InitializeTrajMgr failure.";
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
#endif
}

std::tuple<Base::txFloat /*acc*/, Base::txFloat /*duration*/>
SimPlanningVehicleElement_TrajectoryFollow::ComputeTrajKinetics(const txFloat init_v, const txFloat final_v,
                                                                const txFloat s) const TX_NOEXCEPT {
  const txFloat duration = (2.0 * s) / (init_v + final_v);
  const txFloat acc = (final_v - init_v) / (duration);
  return std::make_tuple(acc, duration);
}

void SimPlanningVehicleElement_TrajectoryFollow::OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  ParentClass::OnStart(timeMgr);
  SetPlanningVelocity(m_KinectParamList.front().mU);
  mKinetics.m_velocity_max = Math::Max(m_KinectParamList.front().mU, m_KinectParamList.front().mV);
  mKinetics.m_acceleration = m_KinectParamList.front().mA;
  mKinetics.acceleration_desired = mKinetics.m_acceleration;
  if (_plus_(ControlPathGear::reverse) == m_KinectParamList.front().mGear) {
    mKinetics.m_gear_angle.FromDegree(180.0);
  } else {
    mKinetics.m_gear_angle.FromDegree(0.0);
  }

  m_real_displacement = 0.0;
  m_KinectParamList.erase(m_KinectParamList.begin());
#if __Control_V2__
  UpdateControlV2_GearMode(m_KinectParamList.front().mGear);
#endif /*__Control_V2__*/
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::CheckDeadLine() TX_NOEXCEPT {
  if (std::fabs(m_real_displacement - (m_trajMgr->GetLength())) < DeadLineRadius()) {
    StopVehicle();
  }
  return IsAlive();
}

Base::txFloat SimPlanningVehicleElement_TrajectoryFollow::Compute_Displacement(Base::TimeParamManager const& timeMgr)
    TX_NOEXCEPT {
  mKinetics.m_displacement = GetVelocity() * timeMgr.RelativeTime();
  return GetDisplacement();
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::Update_Kinetics(Base::TimeParamManager const& timeMgr)
    TX_NOEXCEPT {
  if (IsAlive() && !IsStop()) {
    if (FLAGS_ego_location_closed_loop) {
      return true;
    }
    /*LOG(INFO) << "################################";
    for (const auto& node : m_KinectParamList)
    {
            LOG(INFO) << node;
    }
    LOG(INFO) << "################################";*/
#if 1
    if (_NonEmpty_(m_KinectParamList)) {
      for (auto itr = std::begin(m_KinectParamList); itr != std::end(m_KinectParamList); ++itr) {
        const Base::SegmentKinectParam curItem = *itr;
        if (curItem.mStartS <= m_real_displacement && m_real_displacement < curItem.mEndS) {
          m_cur_kiniect_param = curItem;
          mKinetics.m_velocity_max = Math::Max(curItem.mU, curItem.mV);
          mKinetics.m_acceleration = curItem.mA;
          mKinetics.acceleration_desired = mKinetics.m_acceleration;
          if (_plus_(ControlPathGear::reverse) == curItem.mGear) {
            mKinetics.m_gear_angle.FromDegree(180.0);
          } else {
            mKinetics.m_gear_angle.FromDegree(0.0);
          }
#  if __Control_V2__
          UpdateControlV2_GearMode(curItem.mGear);
#  endif /*__Control_V2__*/
          LogWarn << TX_VARS(timeMgr.AbsTime()) << curItem;
          m_KinectParamList.erase(std::begin(m_KinectParamList), std::next(itr));
          break;
        }
      }
    }
#else
    if (_NonEmpty_(m_KinectParamList) && (m_KinectParamList.front().mStartTime <= timeMgr.PassTime()) &&
        (timeMgr.PassTime() <= m_KinectParamList.front().EndTime())) {
      // mKinetics.m_velocity = m_KinectParamList.front().mU;
      mKinetics.m_velocity_max = Math::Max(m_KinectParamList.front().mU, m_KinectParamList.front().mV);
      mKinetics.m_acceleration = m_KinectParamList.front().mA;
      mKinetics.acceleration_desired = mKinetics.m_acceleration;
      if (_plus_(ControlPathGear::reverse) == m_KinectParamList.front().mGear) {
        mKinetics.m_gear_angle.FromDegree(180.0);
      } else {
        mKinetics.m_gear_angle.FromDegree(0.0);
      }
      m_KinectParamList.erase(m_KinectParamList.begin());
    }
#endif

    /*mKinetics.m_velocity = mKinetics.m_velocity + mKinetics.m_acceleration * timeMgr.RelativeTime();*/
  } else {
    SetPlanningVelocity(0.0);
    mKinetics.m_acceleration = 0.0;
  }
  return true;
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::InitializeTrajMgr(
    const control_path_node_vec& ref_control_path, const txFloat startTime) TX_NOEXCEPT {
  if (ref_control_path.size() >= 2) {
    auto tmp_centripetal_cat_mull_ptr = std::make_shared<CentripetalCatMull>(ref_control_path);
    const txFloat raw_segment_length = tmp_centripetal_cat_mull_ptr->GetLength();
    std::vector<controlPoint> controlPointVec;
    for (txFloat s = 0.0; s < raw_segment_length; s += FLAGS_MapLocationInterval) {
      const Base::txVec3 enu3d = tmp_centripetal_cat_mull_ptr->GetLocalPos(s);
      controlPointVec.emplace_back(controlPoint{__East__(enu3d), __North__(enu3d)});
    }
    const Base::txVec3 enu3d = tmp_centripetal_cat_mull_ptr->GetLocalPos(raw_segment_length);
    controlPointVec.emplace_back(controlPoint{__East__(enu3d), __North__(enu3d)});
    m_trajMgr = nullptr;
    const auto controlPointSize = controlPointVec.size();
    if (controlPointSize >= HdMap::txLaneInfo::CatmullRom_ControlPointSize) {
      m_trajMgr = std::make_shared<HdMap::txLaneInfo>(Base::txLaneUId(), controlPointVec);
    } else if (controlPointSize >= 2 TX_MARK("may be 3")) {
      m_trajMgr =
          std::make_shared<HdMap::txLaneShortInfo>(Base::txLaneUId(), controlPointVec.front(), controlPointVec.back());
    }

    if (NonNull_Pointer(m_trajMgr)) {
      txFloat last_s = 0.0;
      txFloat last_start_time = startTime;
      for (size_t idx = 1; idx < ref_control_path.size(); ++idx) {
        txFloat cur_s = 0.0, cur_t = 0.0;
        Coord::txWGS84 curWGS84(ref_control_path[idx].waypoint);
        m_trajMgr->xy2sl(curWGS84.ToENU().ENU2D(), cur_s, cur_t);
        /*const Base::txVec3 locPos = m_trajMgr->GetLocalPos(cur_s);
        Coord::txWGS84 projPt; projPt.FromENU(locPos);
        LOG(WARNING) << TX_VARS(idx) << TX_VARS(curWGS84) << TX_VARS(projPt) << TX_VARS(cur_t);*/
        /*txFloat cur_s = laneGeomPtr->ParameterAtPoint(idx);*/
        SegmentKinectParam param;
        param.mStartTime = last_start_time;
        param.mU = ref_control_path[idx - 1].speed_m_s;
        param.mV = ref_control_path[idx].speed_m_s;
        param.mGear = ref_control_path[idx - 1].gear;
        param.mS = cur_s - last_s;
        param.mStartS = last_s;
        param.mEndS = cur_s;

        LogInfo << TX_VARS(cur_s) << TX_VARS(param.mS)
                << TX_VARS_NAME(dist, Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(
                                          ref_control_path[idx - 1].waypoint, ref_control_path[idx].waypoint))
                << TX_VARS(param.mStartTime) << TX_VARS_NAME(Gear, __enum2lpsz__(ControlPathGear, param.mGear))
                << TX_VARS(param.mStartS) << TX_VARS(param.mEndS) << TX_VARS(param.mU) << TX_VARS(param.mV);
        std::tie(param.mA, param.mT) = ComputeTrajKinetics(param.mU, param.mV, param.mS);

        last_s = cur_s;
        last_start_time = param.EndTime();
        m_KinectParamList.emplace_back(param);
      }
    } else {
      return false;
    }
  } else if (1 == ref_control_path.size()) {
    Coord::txENU startEnu;
    startEnu.FromWGS84(ref_control_path[0].waypoint);
    Base::txVec3 _laneDir;
    Base::Info_Lane_t locInfo;
    Base::txFloat DistanceAlongCurve = 0.0;
    if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
            startEnu.ToWGS84(), locInfo, DistanceAlongCurve)) {
      LogInfo << "Get_S_Coord_By_Enu_Pt : " << locInfo;
      _laneDir = HdMap::HadmapCacheConCurrent::GetDir(locInfo, DistanceAlongCurve);
    } else {
      LogWarn << "Get_S_Coord_By_Enu_Pt failure." << startEnu;
      return false;
    }

    m_trajMgr = std::make_shared<HdMap::txLaneSinglePositionInfo>(Base::txLaneUId(0, 0, 0),
                                                                  controlPoint{startEnu.X(), startEnu.Y()}, _laneDir);

    SegmentKinectParam param;
    param.mStartTime = startTime;
    param.mU = 0.0;
    param.mV = 0.0;
    param.mGear = ref_control_path[0].gear;
    param.mS = 0.0;
    param.mA = 0.0;
    param.mT = 0.0;
    m_KinectParamList.emplace_back(param);
  } else {
    return false;
  }

  if (FLAGS_LogLevel_Ego_Element) {
    std::ostringstream oss;
    for (const auto& ref : m_KinectParamList) {
      oss << ref << ", ";
    }
    LogInfo << TX_VARS(Id()) << TX_VARS_NAME(control_size, ref_control_path.size())
            << TX_VARS_NAME(traj_length, m_trajMgr->GetLength()) << TX_VARS_NAME(info, oss.str());
  }
  return true;
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallSucc(IsAlive()) && CallFail(IsStop())) {
    if (FLAGS_ego_location_closed_loop) {
      const Base::txFloat last_displacement = m_real_displacement;
      Base::txFloat cur_t;
      m_trajMgr->xy2sl(RawVehicleCoord(/*fix bug perfect_control zero 0 move*/).ToENU().ENU2D(), m_real_displacement,
                       cur_t);
      LOG(WARNING) << TX_VARS(timeMgr.PassTime()) << TX_VARS(last_displacement) << TX_VARS(m_real_displacement);
    } else {
      Update_Kinetics(timeMgr);
    }

    /*Step 1 Update Kinetics*/
    /*if (GetVelocity() <= 0.0)
    {
            mKinetics.m_velocity = 0.0;
            Compute_Displacement(timeMgr);
            mLocation.vPos() = ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(),
    mLocation.LaneOffset()); mGeometryData.ComputePolygon(mLocation.vPos().ToENU(), mLocation.vLaneDir());
    }
    else*/

    {
#if 1
      if (CallFail(FLAGS_dummy_ignore_perception)) {
        mKinetics.m_acceleration = DrivingFollow(timeMgr);
        LOG(INFO) << "[DrivingFollow]" << TX_VARS(timeMgr.PassTime()) << TX_VARS(mKinetics.m_acceleration);
        if (NonNull_Pointer(env_signal_ptr())) {
          const txFloat signal_dist = env_signal_dist();
          const auto signal_color = env_signal_ptr()->GetCurrentSignLightColorType();
          const txSysId signalId = env_signal_ptr()->Id();
          LOG(INFO) << TX_VARS(Id()) << "[signal] " << TX_VARS(signalId) << TX_VARS(signal_dist)
                    << TX_VARS_NAME(Color, signal_color._to_string());
          if (signal_dist < FollowFrontDistance() || signal_dist < GetLength()) {
            mKinetics.m_acceleration = CheckSignLight(signalId, signal_dist, signal_color);
            LOG(INFO) << "[CheckSignLight]" << TX_VARS(timeMgr.PassTime()) << TX_VARS(mKinetics.m_acceleration)
                      << TX_VARS(signal_dist) << TX_VARS(FollowFrontDistance()) << TX_VARS(GetLength());
          }
        }
        LogInfo << TX_VARS(mKinetics.m_acceleration) << TX_VARS(mKinetics.acceleration_desired);
#  if 1
        if (CallFail(FLAGS_ego_location_closed_loop)) {
          if (mKinetics.m_acceleration > 0.0) {
            if (mKinetics.acceleration_desired < 0.0) {
              if (m_cur_kiniect_param.mV < GetVelocity()) {
                mKinetics.m_acceleration = mKinetics.acceleration_desired;
              }
            } else if (mKinetics.acceleration_desired > 0.0) {
              mKinetics.m_acceleration = Math::Min(mKinetics.m_acceleration, mKinetics.acceleration_desired);
            }
          } else {
            mKinetics.m_acceleration = Math::Min(mKinetics.m_acceleration, mKinetics.acceleration_desired);
          }
        }
#  endif
      }

      SetPlanningVelocity(mKinetics.m_velocity + mKinetics.m_acceleration * timeMgr.RelativeTime());
      mKinetics.LimitVelocity();
      LOG(INFO) << TX_VARS(timeMgr.PassTime()) << TX_VARS(mKinetics.m_acceleration);
#endif
      if (CallFail(FLAGS_ego_location_closed_loop)) {
        Compute_Displacement(timeMgr);
        m_real_displacement += GetDisplacement();
      }
      const Base::txVec3 enuPos = m_trajMgr->GetLocalPos(m_real_displacement);
      mLocation.PosOnLaneCenterLinePos().FromENU(enuPos);
      mLocation.vLaneDir() = m_trajMgr->GetLaneDir(m_real_displacement);

      if (CallFail(FLAGS_dummy_ignore_perception) && IsInLaneChange()) {
        mKinetics.m_LateralVelocity = MoveTo_Sideway(timeMgr);
      }

      LaneChangePostOp(timeMgr);
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
    }
    Base::txFloat s, l;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
        RawVehicleCoord(), mLocation.LaneLocInfo(), s, l);

    if (mLocation.LaneLocInfo().isOnLaneLink) {
      hadmap::txLaneLinkPtr linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
          mLocation.LaneLocInfo().onLinkFromLaneUid, mLocation.LaneLocInfo().onLinkToLaneUid);
      RelocateTracker(linkPtr, timeMgr.TimeStamp());
    } else {
      hadmap::txLanePtr lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      RelocateTracker(lanePtr, timeMgr.TimeStamp());
    }
    return true;
  } else {
    return (IsStop());
  }
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::Pre_Update(
    const TimeParamManager& timeMgr, std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT {
  return Update_Kinetics(timeMgr);
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::FillingTrajectory(Base::TimeParamManager const& timeMgr,
                                                                           sim_msg::Trajectory& refTraj) TX_NOEXCEPT {
#define VehicleTrajInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Element)
  refTraj.Clear();
  SaveStableState();
  if (IsAlive() && timeMgr.RelativeTime() > 0.0) {
    refTraj.set_a(GetAcc());
    const Base::txInt n_traj_duration_s = FLAGS_vehicle_trajectory_duration_s;
    VehicleTrajInfo << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());
    std::vector<std::tuple<Base::txInt /*million second*/, sim_msg::TrajectoryPoint*> > vec_ms_time_traj_point;
    Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
    const Base::txFloat cur_acc = GetAcc();  // GetAcc();
    Base::txFloat cur_s = 0.0;
    const Base::txFloat traj_total_length = m_trajMgr->GetLength();
    const Base::txFloat traj_base = m_real_displacement;
    const Base::txFloat dist_to_traj_end = DistanceToEnd();
    const Base::txFloat _zeroRadian =
        (Utils::GetLaneAngleFromVectorOnENU(m_trajMgr->GetLaneDir(traj_base)) + mKinetics.m_gear_angle).GetRadian();
    Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::Half_PI(), _zeroRadian + Unit::CircleRadian::Half_PI(),
                               _zeroRadian);

    const Base::txFloat ego_alt = Altitude();
    for (Base::txFloat start_s = 0.0; start_s <= FLAGS_vehicle_trajectory_duration_s;
         start_s += timeMgr.RelativeTime()) {
      if (cur_s < dist_to_traj_end) {
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
          // cur_acc = 0.0;
        }
        if (cur_velocity < 0.0) {
          cur_velocity = 0.0;
          // cur_acc = 0.0;
        }

        /*2. update s*/

        if (Math::isZero(cur_velocity) || cur_velocity < 0.3) {
          // cur_s += 0.01;
        } else {
          cur_s += cur_velocity * timeMgr.RelativeTime();
        }

        const Base::txFloat cur_s_local = new_traj_point_ptr->s();
        const Base::txFloat cur_s_global = cur_s_local + m_real_displacement;
        if (cur_s_local < dist_to_traj_end) {
          Coord::txWGS84 cur_pt_wgs84;
          cur_pt_wgs84.FromENU(m_trajMgr->GetLocalPos(cur_s_global));

          const Base::txVec3 tmpLaneDir = mLocation.vLaneDir();
          const Base::txVec3 vTargetDir =
              Utils::VetRotVecByDegree(tmpLaneDir, Unit::txDegree::MakeDegree(LeftOnENU), Utils::Axis_Enu_Up());
          cur_pt_wgs84.TranslateLocalPos(__East__(vTargetDir) * mLocation.LaneOffset(),
                                         __North__(vTargetDir) * mLocation.LaneOffset());

          if (CallSucc(IsInLaneChange())) {
            Base::txFloat l = m_LaneChangeTimeMgr.GetChangeInLaneOffset();
            if (m_LaneChangeTimeMgr.Duration() > 0.0) {
              l = l * ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()));
            }

            static const Unit::txDegree sRightDir = Unit::txDegree::MakeDegree(RightOnENU);
            static const Unit::txDegree sLeftDir = Unit::txDegree::MakeDegree(LeftOnENU);
            const Base::txVec3 vTargetDir_lanechange = Utils::VetRotVecByDegree(
                tmpLaneDir, ((IsInTurnRight() || IsInTurnRight_InLane()) ? (sRightDir) : (sLeftDir)),
                Utils::Axis_Enu_Up());
            cur_pt_wgs84.TranslateLocalPos(__East__(vTargetDir_lanechange) * l, __North__(vTargetDir_lanechange) * l);
            // LOG(INFO) << Utils::ToString(vTargetDir_lanechange);
          }

          new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
          new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());

          // const Base::txFloat _headingRadian =
          // Utils::GetLaneAngleFromVectorOnENU(m_trajMgr->GetLaneDir(cur_s_global)).GetRadian();

          const Base::txFloat _headingRadian =
              (Utils::GetLaneAngleFromVectorOnENU(m_trajMgr->GetLaneDir(cur_s_global)) + mKinetics.m_gear_angle)
                  .GetRadian();
          const Base::txFloat wrap_headingRadian = circRad.Wrap(_headingRadian);
          new_traj_point_ptr->set_theta(wrap_headingRadian);
        }
      } else {
        break;
      }
    }
    VehicleTrajInfo << TX_VARS(timeMgr.AbsTime()) << TX_VARS(refTraj.point_size()) << TX_VARS(cur_s)
                    << TX_VARS(cur_velocity);
    return true;
  } else {
    return false;
  }
#undef VehicleTrajInfo
}

Base::txBool SimPlanningVehicleElement_TrajectoryFollow::FillingElement(Base::TimeParamManager const& timeMgr,
                                                                        sim_msg::Location& refLoc) TX_NOEXCEPT {
  if (ParentClass::FillingElement(timeMgr, refLoc)) {
    LOG(INFO) << mKinetics.m_gear_angle;
    auto pRpy = refLoc.mutable_rpy();
    const Base::txFloat _headingRadian = (GetHeadingWithAngle() + mKinetics.m_gear_angle).GetRadian();
    pRpy->set_z(_headingRadian);
    return true;
  } else {
    return false;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
