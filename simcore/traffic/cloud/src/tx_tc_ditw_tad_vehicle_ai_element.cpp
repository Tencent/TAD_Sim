// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_ditw_tad_vehicle_ai_element.h"
#include "tx_frame_utils.h"
#include "tx_spatial_query.h"
#include "tx_units.h"
#include "tx_tc_gflags.h"

#define CloudDitwInfo LOG_IF(INFO, FLAGS_LogLevel_CloudDitw) << "[cloud_ditw_debug]"
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

Base::txSize DITW_TAD_Cloud_AI_VehicleElement::s3rdVehicleSize = 0;

Base::Enums::VEHICLE_TYPE to_VEHICLE_TYPE(const ObjectInfo& ref3rdCar) TX_NOEXCEPT {
  switch (ref3rdCar.type()) {
    case 1: {
      return _plus_(Base::Enums::VEHICLE_TYPE::Sedan);
      break;
    }
    case 4: {
      return _plus_(Base::Enums::VEHICLE_TYPE::Truck);
      break;
    }
    default:
      return _plus_(Base::Enums::VEHICLE_TYPE::Sedan);
  }
  return _plus_(Base::Enums::VEHICLE_TYPE::Sedan);
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::Initialize(TimeParamManager const& timeMgr, const ObjectInfo& ref3rdCar,
                                                          Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  CloudDitwInfo << ref3rdCar.ShortDebugString();
  m_ditw_status = +DITW_Status::eLog;
  m_vehicle_type = to_VEHICLE_TYPE(ref3rdCar);

  m_traj_mgr = std::make_shared<Cloud::txCloudTrajManager>();
  m_vehicle_come_from = Base::IVehicleElement::VehicleSource::_3rd;

  mIdentity.Id() = std::stoll(ref3rdCar.id());
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  SM::txAITrafficState::Initialize(mIdentity.Id());
  mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
  mLifeCycle.StartTime() = 0.0;

  mGeometryData.Length() =
      (ref3rdCar.extrainfo().length() > 1.0) ? (ref3rdCar.extrainfo().length()) : (FLAGS_EGO_Length);
  mGeometryData.Width() = ref3rdCar.extrainfo().width();
  mGeometryData.Height() = ref3rdCar.extrainfo().height();
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

  m_aggress = FLAGS_cloud_3rd_aggress;
  TX_MARK("For lanekeep");
  mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
  Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
  ConfigureFollowStrategy();
  //  InitializeRandomKinetics(veh_init_param.mStartV, veh_init_param.mMaxV, 2.0);
  mLocation.LaneOffset() = 0.0;
  Coord::txWGS84 _3rd_raw_gps;
  _3rd_raw_gps.FromWGS84(ref3rdCar.lng(), ref3rdCar.lat());
  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
          _3rd_raw_gps, mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve())) {
    mLocation.NextLaneIndex() =
        TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(mLocation.LaneLocInfo().onLaneUid.laneId);
    const Base::Info_Lane_t& StartLaneInfo = mLocation.LaneLocInfo();
    if (CallFail(StartLaneInfo.isOnLaneLink)) {
      hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetTxLanePtr(StartLaneInfo.onLaneUid);
      const Base::txLaneUId curLaneUid = StartLaneInfo.onLaneUid;
      if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(curLaneUid, (mLocation.DistanceAlongCurve()),
                                                          mLocation.PosOnLaneCenterLinePos())) {
        mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
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
#if __cloud_ditw__
        Injection3rdData(ref3rdCar);
        // AddTraj(timeMgr, ref3rdCar);
#endif /*__cloud_ditw__*/
        return true;
      } else {
        LogWarn << ", Get_LAL_Lane_By_S Failure.";
        return false;
      }
    } else {
      LogWarn << "cloud vehicle do not start lanelink.";
      return false;
    }
  } else {
    LogWarn << "[cloud_ditw_debug] Get_S_Coord_By_Enu_Pt failure. " << TX_VARS(_3rd_raw_gps);
    return false;
  }
}

void DITW_TAD_Cloud_AI_VehicleElement::AddTraj(Base::TimeParamManager const& timeMgr,
                                               const ObjectInfo& ref3rdCar) TX_NOEXCEPT {
  Cloud::txCloudTrajManager::history_pos_info infoNode;
  infoNode.bus_pos.FromWGS84(ref3rdCar.lng(), ref3rdCar.lat(), 0.0);
  infoNode.rsv_abs_time = timeMgr.AbsTime();
  infoNode.raw_pb_data.CopyFrom(ref3rdCar);
  m_traj_mgr->mId = mIdentity.Id();
  m_traj_mgr->add_new_pos(infoNode, GetLaneDir());
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsLogSim()) {
    return UpdateLogSim(timeMgr);
  } else if (IsWorldSim()) {
    return ParentClass::Update(timeMgr);
  } else {
    return false;
  }
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::UpdateLogSim(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsAlive()) {
    if (m_optional_object_info.has_value()) {
      const auto ref3rdCar = *m_optional_object_info;
      //      m_traj_mgr->Clear();
      AddTraj(timeMgr, ref3rdCar);
      /*StablePosition() = mLocation.vPos().ToENU();*/
      StableGeomCenter() = GeomCenter().ToENU();
      StableRearAxleCenter() = RearAxleCenter().ToENU();
      RawVehicleCoord().FromWGS84(ref3rdCar.lng(), ref3rdCar.lat());
      SyncPosition(timeMgr.AbsTime());

      /*LOG(WARNING) << TX_VARS(Id()) << TX_VARS(GetVelocity());*/
      Unit::txRadian heading;
      heading.FromRadian(ref3rdCar.heading());
      mLocation.vLaneDir() = Base::txVec3(heading.Cos(), heading.Sin(), 0.0);

      UpdateLogSimVehicle(timeMgr /*update velocity*/);
      mKinetics.m_velocity = Unit::kmh_ms(ref3rdCar.speed());
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), (mLocation.vLaneDir()));
      FillingSpatialQuery();
      SaveStableState();
      m_optional_object_info = boost::none;
    } else {
      // LOG(WARNING) << "m_optional_car_info.has_value() = false.";
    }
  } else {
    LOG(WARNING) << "is not alive." << TX_VARS(Id());
  }
  return true;
}

Base::txFloat DITW_TAD_Cloud_AI_VehicleElement::ComputeInjectionRelativeTime() const TX_NOEXCEPT {
  const Base::txSize nTrajSampleSize = m_traj_mgr->Size();
  if (nTrajSampleSize > 1) {
    const Base::txFloat rsv_abs_time_1 = m_traj_mgr->mVecPosInfo[nTrajSampleSize - 1].rsv_abs_time;
    const Base::txFloat rsv_abs_time_2 = m_traj_mgr->mVecPosInfo[nTrajSampleSize - 2].rsv_abs_time;
    return (rsv_abs_time_1 - rsv_abs_time_2);
  } else {
    return 0.0;
  }
}

void DITW_TAD_Cloud_AI_VehicleElement::ComputeScalarVelocity(Base::TimeParamManager const& /*timeMgr*/,
                                                             const Base::txVec2& egoMassCenter) TX_NOEXCEPT {
  TX_MARK("init (0, 0) is reasonable, ego start position is as same as local-enu (0,0)");
  const Base::txVec2 last_EgoMassCenter = StableGeomCenter().ENU2D();
  const Base::txFloat injectionDuration = ComputeInjectionRelativeTime();
  if (injectionDuration > 0.0) {
    TX_MARK("SIM-3793");
    const Base::txFloat dist = (egoMassCenter - last_EgoMassCenter).norm();

    mKinetics.m_velocity = dist / injectionDuration;
    mKinetics.m_acceleration = (mKinetics.m_velocity - StableVelocity()) / injectionDuration;
    // LogInfo << "[Ego]" << TX_VARS(StablePosition()) << TX_VARS(GetPosition()) << TX_VARS(dist) <<
    // TX_VARS(mKinetics.m_velocity);
  } else {
    mKinetics.m_velocity = 0.0;
    mKinetics.m_acceleration = 0.0;
  }
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::UpdateLogSimVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  hadmap::txLanePtr resLane = nullptr;
  hadmap::txLaneLinkPtr resLaneLink = nullptr;
  const Base::txVec2 vPos_Enu2D = GeomCenter().ToENU().ENU2D();
  ComputeScalarVelocity(TX_MARK("SIM-3793") timeMgr, vPos_Enu2D);

#if 1
  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
          GeomCenter(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset())) {
    if (mLocation.LaneOffset() > 5) {
      CloudDitwInfo << TX_VARS(Id()) << mLocation.LaneLocInfo() << TX_VARS(mLocation.DistanceAlongCurve())
                    << TX_VARS(mLocation.LaneOffset());
    }
    mLocation.LaneOffset() = 0.0;
    CloudDitwInfo << "Get_S_Coord_By_Enu_Pt : " << mLocation.LaneLocInfo();
    if (mLocation.IsOnLane()) {
      resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      if (NonNull_Pointer(resLane)) {
        CloudDitwInfo << "hadmap::getLane success.";
        RelocateTracker(resLane, 0.0);
        mLocation.PosOnLaneCenterLinePos() =
            HdMap::HadmapCacheConCurrent::GetLanePos(resLane, mLocation.DistanceAlongCurve());
        mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(resLane, mLocation.DistanceAlongCurve());
      } else {
        LogWarn << "GetTxLaneLinkPtr failure. " << mLocation.LaneLocInfo();
        return false;
      }
    } else {
      resLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                   mLocation.LaneLocInfo().onLinkToLaneUid);
      if (NonNull_Pointer(resLaneLink)) {
        CloudDitwInfo << "hadmap::getLaneLink success.";
        RelocateTracker(resLaneLink, FLT_MAX);
        mLocation.PosOnLaneCenterLinePos() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkPos(resLaneLink, mLocation.DistanceAlongCurve());
        mLocation.vLaneDir() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkDir(resLaneLink, mLocation.DistanceAlongCurve());
      } else {
        LogWarn << "GetTxLaneLinkPtr failure. " << mLocation.LaneLocInfo();
        return false;
      }
    }
  } else {
    LogWarn << "Get_S_Coord_By_Enu_Pt failure.";
    return false;
  }
#endif
  CloudDitwInfo << "hadmap::getLane end." << ((mLocation.IsOnLaneLink()) ? "[onLink]" : "[onLane]")
                << TX_VARS(mLocation.DistanceAlongCurve());

  if (NonNull_Pointer(resLaneLink)) {
    mLocation.LocalCoord_AxisY() =
        HdMap::HadmapCacheConCurrent::GetLaneLinkDir(resLaneLink, mLocation.DistanceAlongCurve());
    CloudDitwInfo << TX_VARS_NAME(DITW_OnLink_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else if (NonNull_Pointer(resLane)) {
    mLocation.LocalCoord_AxisY() = HdMap::HadmapCacheConCurrent::GetLaneDir(resLane, mLocation.DistanceAlongCurve());
    CloudDitwInfo << TX_VARS_NAME(DITW_OnLane_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else {
    mLocation.LocalCoord_AxisY() = mLocation.vLaneDir();
    CloudDitwInfo << TX_VARS_NAME(DITW_NotOnMap_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  }
  mLocation.LocalCoord_AxisX() = Utils::VetRotVecByDegree(mLocation.LocalCoord_AxisY(),
                                                          Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
  CloudDitwInfo << TX_VARS_NAME(DITW_LocalCoord_AxisX, Utils::ToString(mLocation.LocalCoord_AxisX()));

  return true;
}

void DITW_TAD_Cloud_AI_VehicleElement::SwitchLog2World() TX_NOEXCEPT {
  LOG(INFO) << TX_VARS(Id()) << " SET World Sim.";
  m_ditw_status = _plus_(DITW_Status::eWorld);
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::CheckAlive(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsLogSim()) {
    if (CallFail((m_traj_mgr)->Empty())) {
#if USE_Avoid_Bus_Retrograde
      const Base::txFloat rsv_abs_time = (pImpl->mTrajMgr).rsv_abs_time;
#else  /*USE_Avoid_Bus_Retrograde*/
      const Base::txFloat rsv_abs_time = (m_traj_mgr)->mVecPosInfo.back().rsv_abs_time;
#endif /*USE_Avoid_Bus_Retrograde*/
      if ((timeMgr.AbsTime() - rsv_abs_time) > GetMaxIdleTimeInSecond()) {
        CloudDitwInfo << TX_VARS(Id()) << TX_VARS((m_traj_mgr)->Size()) << TX_VARS(GetMaxIdleTimeInSecond())
                      << TX_VARS_NAME(aliveTime, (timeMgr.AbsTime() - (m_traj_mgr)->mVecPosInfo.back().rsv_abs_time));
        return false;
      }
      /*else {
              LogWarn << TX_VARS(Id()) << "(timeMgr.AbsTime() - rsv_abs_time) > GetMaxIdleTimeInSecond()"
                      << TX_VARS(timeMgr.AbsTime()) << TX_VARS(rsv_abs_time) << TX_VARS(GetMaxIdleTimeInSecond());
      }*/
    } else {
      LogWarn << TX_VARS(Id()) << "(m_traj_mgr)->Empty()";
    }
  }
  return true;
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                              sim_msg::Traffic& refTraffic) TX_NOEXCEPT {
  if (IsLogSim()) {
    if (m_traj_mgr->IsValid() && m_traj_mgr->EnableFilling()) {
      return ParentClass::FillingElement(timeMgr, refTraffic);
    } else {
      return false;
    }
  } else if (IsWorldSim()) {
    return ParentClass::FillingElement(timeMgr, refTraffic);
  }
  return false;
}

Base::txBool DITW_TAD_Cloud_AI_VehicleElement::PreFillingElement(AtomicSize& a_size) TX_NOEXCEPT {
  if (IsLogSim()) {
    if (m_traj_mgr->IsValid() && m_traj_mgr->EnableFilling()) {
      return ParentClass::PreFillingElement(a_size);
    } else {
      return false;
    }
  } else if (IsWorldSim()) {
    return ParentClass::PreFillingElement(a_size);
  }
  return true;
}

Base::txBool TAD_Cloud_Obstacle_VehicleElement::Initialize_Obstacle(const Base::txSysId _vehId,
                                                                    const Base::Info_Lane_t StartLaneInfo,
                                                                    const Base::txFloat& _s, const Base::txFloat fAngle,
                                                                    const int64_t endTime,
                                                                    Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  mDrivingStatus = _plus_(Base::Enums::DrivingStatus::stop_crash);
  mLocation.fAngle().FromDegree(fAngle);
  m_vehicle_come_from = Base::IVehicleElement::VehicleSource::veh_input;
  m_vehicle_type = _plus_(VEHICLE_TYPE::Sedan);
  mIdentity.Id() = _vehId;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  SM::txAITrafficState::Initialize(mIdentity.Id());
  mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
  mLifeCycle.StartTime() = 0.0;
  mLifeCycle.EndTime() = endTime;
  ConfigurePerceptionPeriod(mIdentity.Id() % FLAGS_AiVehEnvPercptionPeriod);

  mGeometryData.Length() = FLAGS_EGO_Length;
  if (-1 == StartLaneInfo.onLaneUid.laneId) {
    mGeometryData.Length() = FLAGS_EGO_Length * 5.5;
  } else if (-2 == StartLaneInfo.onLaneUid.laneId) {
    mGeometryData.Length() = FLAGS_EGO_Length * 3.5;
  } else {
    LOG(FATAL) << TX_VARS(StartLaneInfo.onLaneUid.laneId);
  }

  mGeometryData.Width() = FLAGS_EGO_Width;
  mGeometryData.Height() = FLAGS_EGO_Height;
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;
  m_aggress = 0.0;
  TX_MARK("For lanekeep");
  mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());

  mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
  Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
  ConfigureFollowStrategy();
  InitializeRandomKinetics(0.0, 0.0, 2.0);

  mLocation.LaneOffset() = 0.0;

  if (CallFail(StartLaneInfo.isOnLaneLink)) {
    hadmap::txLanePtr initLane_roadId_sectionId = nullptr;
    initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetTxLanePtr(StartLaneInfo.onLaneUid);

    const Base::txLaneUId curLaneUid = StartLaneInfo.onLaneUid;
    if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(curLaneUid, _s, mLocation.PosOnLaneCenterLinePos())) {
      mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
      mLocation.DistanceAlongCurve() = _s;
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

sim_msg::Car* TAD_Cloud_Obstacle_VehicleElement::FillingElement(txFloat const timeStamp,
                                                                sim_msg::Car* pSimVehicle) TX_NOEXCEPT {
  ParentClass::FillingElement(timeStamp, pSimVehicle);
  pSimVehicle->set_length(FLAGS_EGO_Length);
  // pSimVehicle->set_heading(Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian());
  return pSimVehicle;
}

Base::txBool TAD_Cloud_Obstacle_VehicleElement::CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (!IsEnd()) {
    if (timeMgr.PassTime() >= mLifeCycle.EndTime()) {
      mLifeCycle.SetEnd();
    }
  }
  return IsEnd();
}

Base::txBool TAD_Virtual_Obstacle_VehicleElement::Initialize_Closure_Obstacle(const Base::txSysId _vehId,
                                                                              const Base::Info_Lane_t StartLaneInfo,
                                                                              const Base::txFloat& _s,
                                                                              const int64_t endTime_s,
                                                                              Base::ISceneLoaderPtr) TX_NOEXCEPT {
  mDrivingStatus = _plus_(Base::Enums::DrivingStatus::normal);
  m_vehicle_come_from = Base::IVehicleElement::VehicleSource::veh_input;
  m_vehicle_type = _plus_(VEHICLE_TYPE::Sedan);
  mIdentity.Id() = _vehId;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  SM::txAITrafficState::Initialize(mIdentity.Id());
  mPRandom.Initialize(mIdentity.Id(), 0);
  mLifeCycle.StartTime() = 0.0;
  mLifeCycle.EndTime() = endTime_s;
  ConfigurePerceptionPeriod(mIdentity.Id() % FLAGS_AiVehEnvPercptionPeriod);

  mGeometryData.Length() = 1.0;
  mGeometryData.Width() = 1.0;
  mGeometryData.Height() = 1.0;
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;
  m_aggress = 0.0;
  TX_MARK("For lanekeep");
  mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());

  mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
  Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
  ConfigureFollowStrategy();
  InitializeRandomKinetics(0.0, 0.0, 2.0);

  mLocation.LaneOffset() = 0.0;

  if (CallFail(StartLaneInfo.isOnLaneLink)) {
    hadmap::txLanePtr initLane_roadId_sectionId = nullptr;
    initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetTxLanePtr(StartLaneInfo.onLaneUid);

    const Base::txLaneUId curLaneUid = StartLaneInfo.onLaneUid;
    if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(curLaneUid, _s, mLocation.PosOnLaneCenterLinePos())) {
      mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
      mLocation.DistanceAlongCurve() = _s;
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

TX_NAMESPACE_CLOSE(TrafficFlow)
