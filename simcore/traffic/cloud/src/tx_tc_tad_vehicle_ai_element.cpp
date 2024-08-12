// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_tad_vehicle_ai_element.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_hadmap_utils.h"
#include "tx_pedestrian_element.h"
#include "tx_spatial_query.h"
#include "tx_units.h"
#include "tx_tc_cloud_loader.h"
#include "tx_tc_gflags.h"
#if 1
#endif
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_AI_Traffic)
#define LogWarn LOG(WARNING)
#define MultiRegionInfo LOG_IF(INFO, FLAGS_LogLevel_MultiRegion)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool TAD_Cloud_AI_VehicleElement::Initialize(const Base::txSysId vehId,
                                                     const VehicleInitParam_t& veh_init_param,
                                                     const Base::txLaneID startLaneId,
                                                     Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (Null_Pointer(veh_init_param.mVehTypePtr) || Null_Pointer(veh_init_param.mBehPtr)) {
    LogWarn << TX_VARS(vehId) << TX_COND(Null_Pointer(veh_init_param.mVehTypePtr))
            << TX_COND(Null_Pointer(veh_init_param.mBehPtr));
    return false;
  }

  InitializeBehavior(veh_init_param.mBehPtr);

  m_vehicle_come_from = Base::IVehicleElement::VehicleSource::veh_input;
  // m_vehicle_type = veh_init_param.mVehTypePtr->vehicleType();
  m_vehicle_type = __lpsz2enum__(VEHICLE_TYPE, (veh_init_param.mVehTypePtr->vehicleType()).c_str());
  m_vehicle_type_id = __enum2int__(VEHICLE_TYPE, m_vehicle_type);

  mIdentity.Id() = vehId;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  SM::txAITrafficState::Initialize(mIdentity.Id());
  mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
  mLifeCycle.StartTime() = 0.0;
  txFloat initCompliance = mPRandom.GetRandomValue();
  mRuleCompliance = (initCompliance < _sceneLoader->GetRuleComplianceProportion());
  LogInfo << TX_VARS(Id()) << TX_VARS(mRuleCompliance) << TX_VARS(initCompliance)
          << TX_VARS(_sceneLoader->GetRuleComplianceProportion());

  mGeometryData.Length() = veh_init_param.mVehTypePtr->length();
  mGeometryData.Width() = veh_init_param.mVehTypePtr->width();
  mGeometryData.Height() = veh_init_param.mVehTypePtr->height();
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

  m_aggress = veh_init_param.mAggress;
  TX_MARK("For lanekeep");
  mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());
  mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
  Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
  ConfigureFollowStrategy();
  InitializeRandomKinetics(veh_init_param);

  mLocation.NextLaneIndex() = TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(startLaneId);
  mLocation.LaneOffset() = 0.0;

  const Base::Info_Lane_t& StartLaneInfo = veh_init_param.mLocationPtr->GetLaneInfo();
  if (CallFail(StartLaneInfo.isOnLaneLink)) {
    hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetTxLanePtr(
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

Base::txBool TAD_Cloud_AI_VehicleElement::ReRoute(const Base::txSysId routeGroupId, const Base::txInt subRouteId,
                                                  const TrafficFlow::Component::RouteAI& newRouteAI) TX_NOEXCEPT {
  tbb::mutex::scoped_lock lock(m_tbbMutex);
  mRouteAI = newRouteAI;
  mRouteAI.ResetVehicleId(Id());
  return true;
  // return mRouteAI.ComputeRoute(mPRandom);
}

Base::txBool TAD_Cloud_AI_VehicleElement::Deserialization(const std::string& strArchive) TX_NOEXCEPT {
  using InputArchive = cereal::BinaryInputArchive;
  std::stringstream ss(strArchive);
  InputArchive arin(ss);
  arin(*this);
  MultiRegionInfo << "[cloud_debug] " << TX_VARS(Id()) << " _serialization_." << TX_VARS(GeomCenter().StrWGS84());
  return true;
}

Base::txBool TAD_Cloud_AI_VehicleElement::UpdateByTxVehicle(const its::txVehicle& _veh) TX_NOEXCEPT {
  if (_veh.serialization_archive().size() > 0) {
#if 1
    return Deserialization(_veh.serialization_archive());
#else
    using InputArchive = cereal::BinaryInputArchive;
    std::stringstream ss(_veh.serialization_archive());
    InputArchive arin(ss);
    arin(*this);
    MultiRegionInfo << "[cloud_debug] " << TX_VARS(Id()) << " _serialization_." << TX_VARS(mLocation.vPos().StrWGS84());
    return true;
#endif
  } else {
    return false;
  }
}

Base::txBool TAD_Cloud_AI_VehicleElement::UpdateByTxVehicle(const sim_msg::Car& _car) TX_NOEXCEPT {
  if (_car.serialization_archive().size() > 0) {
    return Deserialization(_car.serialization_archive());
  } else {
    return false;
  }
}

Base::txBool TAD_Cloud_AI_VehicleElement::UpdateCloud(Base::TimeParamManager const& timeMgr, const its::txVehicle& _veh,
                                                      const double maxErrDist) TX_NOEXCEPT {
  if (CallSucc(IsAlive())) {
    MultiRegionInfo << "[cloud_debug][update]" << _StreamPrecision_ << TX_VARS(_veh.vehicle_id()) << TX_VARS(_veh.lng())
                    << TX_VARS(_veh.lat()) << TX_VARS(_veh.speed()) << TX_VARS(_veh.yaw());

    RawVehicleCoord().FromWGS84(_veh.lng(), _veh.lat());
    SyncPosition(timeMgr.AbsTime());
    LogInfo << TX_VARS(RawVehicleCoord());

    if (CallFail(UpdateByTxVehicle(_veh))) {
      mGeometryData.SetInValid();
      LogWarn << "vehilce UpdateByTxVehicle failure." << TX_VARS(Id());
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

Base::Enums::VEHICLE_TYPE to_VEHICLE_TYPE(const its::txVehicle& _veh) TX_NOEXCEPT {
  if (its::txVehicleType::Vehicle_Type_Car == _veh.vehicle_type()) {
    return _plus_(Base::Enums::VEHICLE_TYPE::Sedan);
  } else if (its::txVehicleType::Vehicle_Type_Truck == _veh.vehicle_type()) {
    return _plus_(Base::Enums::VEHICLE_TYPE::Truck);
  } else if (its::txVehicleType::Vehicle_Type_Bus == _veh.vehicle_type()) {
    return _plus_(Base::Enums::VEHICLE_TYPE::Bus);
  } else {
    return _plus_(Base::Enums::VEHICLE_TYPE::Sedan);
  }
}

void TAD_Cloud_AI_VehicleElement::Cloud_MultiRegion_Params::Initialize(const its::txVehicle& _veh) TX_NOEXCEPT {
  vehicle_length = _veh.length();
  vehicle_width = _veh.vehicle_width();
  vehicle_height = _veh.vehicle_height();
  vehicle_id = _veh.vehicle_id();
  vehicle_type = to_VEHICLE_TYPE(_veh);

  if (_veh.on_link()) {
    lane_loc_info.FromLaneLink(0, Base::txLaneUId(_veh.road_id(), _veh.section_id(), _veh.lane_id()),
                               Base::txLaneUId(_veh.to_road_id(), _veh.to_section_id(), _veh.to_lane_id()));
  } else {
    lane_loc_info.FromLane(Base::txLaneUId(_veh.road_id(), _veh.section_id(), _veh.lane_id()));
  }

  vehicle_pos = hadmap::txPoint(_veh.lng(), _veh.lat(), 0.0);
  vehicle_speed = _veh.speed();
  vehicle_yaw = _veh.yaw();
}

void TAD_Cloud_AI_VehicleElement::Cloud_MultiRegion_Params::Initialize(const sim_msg::Car& _car) TX_NOEXCEPT {
  vehicle_length = _car.length();
  vehicle_width = _car.width();
  vehicle_height = _car.height();
  vehicle_id = _car.id();
  vehicle_type = __int2enum__(VEHICLE_TYPE, _car.type());

  if (Utils::IsOnLink(_car.tx_road_id(), _car.to_tx_road_id())) {
    /*on link*/
    lane_loc_info.FromLaneLink(0, Base::txLaneUId(_car.tx_road_id(), _car.tx_section_id(), _car.tx_lane_id()),
                               Base::txLaneUId(_car.to_tx_road_id(), _car.to_tx_section_id(), _car.to_tx_lane_id()));
  } else {
    /*on lane*/
    lane_loc_info.FromLane(Base::txLaneUId(_car.tx_road_id(), _car.tx_section_id(), _car.tx_lane_id()));
  }

  vehicle_pos = hadmap::txPoint(_car.x(), _car.y(), _car.z());
  vehicle_speed = _car.v();
  vehicle_yaw = _car.heading();
}

Base::txBool TAD_Cloud_AI_VehicleElement::Initialize_Cloud_MultiRegion(const sim_msg::Car& _car,
                                                                       Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Cloud_MultiRegion_Params param;
  param.Initialize(_car);
  return Initialize_Cloud_MultiRegion(param, _loader);
}

Base::txBool TAD_Cloud_AI_VehicleElement::Initialize_Cloud_MultiRegion(const its::txVehicle& _veh,
                                                                       Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Cloud_MultiRegion_Params param;
  param.Initialize(_veh);
  return Initialize_Cloud_MultiRegion(param, _loader);
}

Base::txBool TAD_Cloud_AI_VehicleElement::Initialize_Cloud_MultiRegion(const Cloud_MultiRegion_Params& _veh,
                                                                       Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  MultiRegionInfo << "[cloud_debug][Initialize_Cloud_MultiRegion]" << _StreamPrecision_ << TX_VARS(_veh.vehicle_id)
                  << TX_VARS(_veh.lng()) << TX_VARS(_veh.lat()) << TX_VARS(_veh.vehicle_speed)
                  << TX_VARS(_veh.vehicle_yaw) << TX_VARS(_veh.road_id()) << TX_VARS(_veh.section_id())
                  << TX_VARS(_veh.lane_id()) << TX_VARS(_veh.to_road_id()) << TX_VARS(_veh.to_section_id())
                  << TX_VARS(_veh.to_lane_id());

  using TAD_Cloud_VehType = SceneLoader::TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType;
  using TAD_Cloud_VehTypePtr = SceneLoader::TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehTypePtr;
  using VehType = SceneLoader::Traffic::VehType;
  TAD_Cloud_VehTypePtr cloud_veh_ptr = std::make_shared<TAD_Cloud_VehType>();
  VehType veh_info;
  veh_info.behavior = "TrafficVehicle";
  veh_info.length = std::to_string(_veh.vehicle_length);
  veh_info.width = std::to_string(_veh.vehicle_width);
  veh_info.height = std::to_string(_veh.vehicle_height);
  veh_info.id = std::to_string(_veh.vehicle_id);
  veh_info.Type = __enum2lpsz__(VEHICLE_TYPE, _veh.vehicle_type);
  cloud_veh_ptr->Init(veh_info);

  Base::Info_Lane_t laneLocInfo;
  if (_veh.on_link()) {
    laneLocInfo.FromLaneLink(0, Base::txLaneUId(_veh.road_id(), _veh.section_id(), _veh.lane_id()),
                             Base::txLaneUId(_veh.to_road_id(), _veh.to_section_id(), _veh.to_lane_id()));
  } else {
    laneLocInfo.FromLane(Base::txLaneUId(_veh.road_id(), _veh.section_id(), _veh.lane_id()));
  }

  Base::LocationAgent::LocationAgentPtr cloud_loc_ptr = std::make_shared<Base::LocationAgent>();
  const Base::txBool res = cloud_loc_ptr->Init(hadmap::txPoint(_veh.lng(), _veh.lat(), 0.0), laneLocInfo);
  LOG_IF(WARNING, CallFail(res)) << "its::txVehicle LocationAgent compute failure. " << TX_VARS(_veh.lng())
                                 << TX_VARS(_veh.lat()) << TX_VARS(laneLocInfo);
  Scene::TAD_Cloud_ElementGenerator::VehicleInputeAgent::VehicleInitParam_t veh_init_param;
  veh_init_param.mVehTypePtr = cloud_veh_ptr;
  veh_init_param.mLocationPtr = cloud_loc_ptr;

#if __cloud_ditw__
  veh_init_param.SetStartV_Cloud((_veh.vehicle_speed < FLAGS_vehicle_max_speed) ? (_veh.vehicle_speed)
                                                                                : (FLAGS_vehicle_max_speed));
  veh_init_param.SetMaxV_Cloud(FLAGS_vehicle_max_speed);
  TODO("extend protobuf");
#else
  veh_init_param.mStartV = _veh.vehicle_speed;
  veh_init_param.mMaxV = veh_init_param.mStartV * 2.0;
#endif

  veh_init_param.mAggress = 0.5;

  {
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::world_mgr;
    // m_vehicle_type = veh_init_param.mVehTypePtr->vehicleType();
    m_vehicle_type = __lpsz2enum__(VEHICLE_TYPE, (veh_init_param.mVehTypePtr->vehicleType()).c_str());

    mIdentity.Id() = _veh.vehicle_id;
    ConfigurePerceptionPeriod(mIdentity.Id() % FLAGS_AiVehEnvPercptionPeriod);
    mIdentity.SysId() = CreateSysId(mIdentity.Id());
    SM::txAITrafficState::Initialize(mIdentity.Id());
    mPRandom.Initialize(mIdentity.Id(), _loader->GetRandomSeed());
    mLifeCycle.StartTime() = 0.0;

    mGeometryData.Length() = veh_init_param.mVehTypePtr->length();
    mGeometryData.Width() = veh_init_param.mVehTypePtr->width();
    mGeometryData.Height() = veh_init_param.mVehTypePtr->height();
    mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

    m_aggress = veh_init_param.mAggress;
    TX_MARK("For lanekeep");
    mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
    Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
    ConfigureFollowStrategy();
    InitializeRandomKinetics(veh_init_param.StartV(), veh_init_param.MaxV(), 2.0);

    mLocation.NextLaneIndex() = 0;
#if __cloud_ditw__
    mLocation.LaneOffset() = 0.0;
#else
    mLocation.LaneOffset() = veh_init_param.mLocationPtr->GetLateralOffset();
#endif

    const Base::Info_Lane_t& StartLaneInfo = veh_init_param.mLocationPtr->GetLaneInfo();
    if (CallFail(StartLaneInfo.isOnLaneLink)) {
      hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetTxLanePtr(StartLaneInfo.onLaneUid);
      if (Null_Pointer(initLane_roadId_sectionId)) {
        LogWarn << " initLane_roadId_sectionId GetLane Failure."
                << TX_VARS_NAME(laneUid, Utils::ToString(StartLaneInfo.onLaneUid));
        return false;
      }
      if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(
              StartLaneInfo, (veh_init_param.mLocationPtr->GetDistanceOnCurve()), mLocation.PosOnLaneCenterLinePos())) {
        mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
        mLocation.DistanceAlongCurve() = veh_init_param.mLocationPtr->GetDistanceOnCurve();
        mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(StartLaneInfo, mLocation.DistanceAlongCurve());
#if __cloud_ditw__
        RawVehicleCoord() = mLocation.PosOnLaneCenterLinePos();
#else
        RawVehicleCoord() = veh_init_param.mLocationPtr->vPos();
#endif
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
      hadmap::txLaneLinkPtr init_link_ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
          StartLaneInfo.onLinkFromLaneUid, StartLaneInfo.onLinkToLaneUid);
      if (HdMap::HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(
              StartLaneInfo, (veh_init_param.mLocationPtr->GetDistanceOnCurve()), mLocation.PosOnLaneCenterLinePos())) {
        mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
        mLocation.DistanceAlongCurve() = veh_init_param.mLocationPtr->GetDistanceOnCurve();
        mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(StartLaneInfo, mLocation.DistanceAlongCurve());
        RawVehicleCoord() = veh_init_param.mLocationPtr->vPos();
        SyncPosition(0.0);
        mLocation.InitTracker(mIdentity.Id());
        RelocateTracker(init_link_ptr, 0.0);
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
      return false;
    }
  }
}

Base::txBool TAD_Cloud_AI_VehicleElement::FillingSimStatus(Base::TimeParamManager const& timeMgr,
                                                           its::txSimStatus& outSceneStatue) TX_NOEXCEPT {
  if ((IsAlive())) {
    mInRangeAfterSimulation = InRanges(GeomCenter().Lon(), GeomCenter().Lat());
    its::txVehicle* pTxVehicle = outSceneStatue.mutable_vehicle(IndexInPb());
    pTxVehicle->Clear(/*clear serialize archive*/);
    pTxVehicle = FillingSimStatus(timeMgr.TimeStamp(), pTxVehicle);
    MultiRegionInfo << "[cloud_debug][FillingSimStatus]" << TX_VARS(Id()) << TX_COND(IsAlive()) << _StreamPrecision_
                    << TX_VARS(pTxVehicle->lng()) << TX_VARS(pTxVehicle->lat());
    /*FillingSpatialQuery();*/
#if __TX_Mark__("simulation consistency")
    SaveStableState();
#endif /*__TX_Mark__("simulation consistency")*/
  } else {
    mGeometryData.SetInValid();
  }
  return true;
}

its::txVehicleType TAD_Cloud_AI_VehicleElement::CloudVehicleType() const TX_NOEXCEPT {
  switch (VehicleType()) {
    case VEHICLE_TYPE::Sedan:
    case VEHICLE_TYPE::Sedan_001:
    case VEHICLE_TYPE::Sedan_002:
    case VEHICLE_TYPE::Sedan_003:
    case VEHICLE_TYPE::Sedan_004:
    case VEHICLE_TYPE::Sedan_005: {
      return its::Vehicle_Type_Car;
    }
    case VEHICLE_TYPE::SUV:
    case VEHICLE_TYPE::SUV_001:
    case VEHICLE_TYPE::SUV_002:
    case VEHICLE_TYPE::SUV_003:
    case VEHICLE_TYPE::SUV_004:
    case VEHICLE_TYPE::SUV_005:
    case VEHICLE_TYPE::SUV_006: {
      return its::Vehicle_Type_Car;
    }
    case VEHICLE_TYPE::Bus:
    case VEHICLE_TYPE::Bus_001:
    case VEHICLE_TYPE::Bus_003:
    case VEHICLE_TYPE::Bus_004:
    case VEHICLE_TYPE::Bus_005: {
      return its::Vehicle_Type_Bus;
    }
    case VEHICLE_TYPE::Truck:
    case VEHICLE_TYPE::Truck_1:
    case VEHICLE_TYPE::Truck_002:
    case VEHICLE_TYPE::Truck_003:
    case VEHICLE_TYPE::Semi_Trailer_Truck_001: {
      return its::Vehicle_Type_Truck;
    }
    default: {
      return its::Vehicle_Type_Car;
    }
  }
  return its::Vehicle_Type_Car;
}

its::txVehicle* TAD_Cloud_AI_VehicleElement::FillingSimStatus(txFloat const timeStamp,
                                                              its::txVehicle* pTxVehicle) TX_NOEXCEPT {
  if (NonNull_Pointer(pTxVehicle)) {
    pTxVehicle->set_vehicle_id(mIdentity.Id());
    pTxVehicle->set_vehicle_type(CloudVehicleType());
    pTxVehicle->set_length(GetLength());
    pTxVehicle->set_vehicle_width(GetWidth());
    pTxVehicle->set_vehicle_height(GetHeigth());
    const hadmap::txPoint geom_center_gps = GeomCenter().WGS84();
    pTxVehicle->set_lng(__Lon__(geom_center_gps));
    pTxVehicle->set_lat(__Lat__(geom_center_gps));
    const Base::txFloat _headingRadian = GetHeadingWithAngle().GetRadian();
    pTxVehicle->set_yaw(_headingRadian);
    pTxVehicle->set_speed(GetVelocity());

    const Base::Info_Lane_t& curLaneInfo = mLocation.LaneLocInfo();
    pTxVehicle->set_on_link(curLaneInfo.isOnLaneLink);
    if (CallSucc(curLaneInfo.isOnLaneLink)) {
      /*on lanelink*/
      pTxVehicle->set_road_id(curLaneInfo.onLinkFromLaneUid.roadId);
      pTxVehicle->set_section_id(curLaneInfo.onLinkFromLaneUid.sectionId);
      pTxVehicle->set_lane_id(curLaneInfo.onLinkFromLaneUid.laneId);

      pTxVehicle->set_to_road_id(curLaneInfo.onLinkToLaneUid.roadId);
      pTxVehicle->set_to_section_id(curLaneInfo.onLinkToLaneUid.sectionId);
      pTxVehicle->set_to_lane_id(curLaneInfo.onLinkToLaneUid.laneId);
    } else {
      pTxVehicle->set_road_id(curLaneInfo.onLaneUid.roadId);
      pTxVehicle->set_section_id(curLaneInfo.onLaneUid.sectionId);
      pTxVehicle->set_lane_id(curLaneInfo.onLaneUid.laneId);

      pTxVehicle->set_to_road_id(curLaneInfo.onLaneUid.roadId);
      pTxVehicle->set_to_section_id(curLaneInfo.onLaneUid.sectionId);
      pTxVehicle->set_to_lane_id(curLaneInfo.onLaneUid.laneId);
    }

    pTxVehicle->set_string_id(std::to_string(mIdentity.Id()));
    if (CallFail(InRangeAfterSimulation())) {
      pTxVehicle->set_serialization_archive(binary_archive());
      MultiRegionInfo << "[cloud_debug][4b]" << TX_VARS(Id())
                      << " out of range after simulation, set_serialization_archive.";
    } else {
      MultiRegionInfo << "[cloud_debug][4a]" << TX_VARS(Id()) << " in range after simulation, set_simple_archive.";
    }
  }
  return pTxVehicle;
}

Base::txBool TAD_Cloud_AI_VehicleElement::CheckPedestrianWalking(const txFloat relative_time,
                                                                 txFloat& refLateralVelocity) TX_NOEXCEPT {
  Base::txFloat minDist = GetDistanceToPedestrian();
  Base::IPedestrianElementPtr pede_ptr = GetCurFrontPedestrianPtr();

  if (NonNull_Pointer(pede_ptr)) {
    const Base::txFloat tmpSlowingDownDec =
        Deceleration(minDist, GetVelocity(), FLAGS_Deceleration_Safe_Distance, Max_Deceleration());
    mKinetics.m_acceleration = fmin(tmpSlowingDownDec, mKinetics.m_acceleration);
    LogInfo << TX_VARS(Id()) << TX_VARS(pede_ptr->Id()) << TX_VARS(minDist) << TX_VARS(GetVelocity())
            << TX_VARS(FLAGS_Deceleration_Safe_Distance) << TX_VARS(Max_Deceleration())
            << TX_VARS_NAME(Dec,
                            (GetVelocity() * GetVelocity()) / (-2.0 * (minDist - FLAGS_Deceleration_Safe_Distance)))
            << TX_VARS(tmpSlowingDownDec) << TX_VARS(mKinetics.m_acceleration);
    return true;
  } else {
    LogInfo << TX_VARS(Id()) << " front clear.";
    return false;
  }
}

void TAD_Cloud_AI_VehicleElement::StopVehicle() TX_NOEXCEPT {
  MultiRegionInfo << "[cloud_debug][StopVehicle]" << TX_VARS(Id());
  TAD_AI_VehicleElement::StopVehicle();
}

Base::txString TAD_Cloud_AI_VehicleElement::binary_archive() const TX_NOEXCEPT {
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

Base::txString TAD_Cloud_AI_VehicleElement::xml_archive() const TX_NOEXCEPT { return ""; }

Base::txString TAD_Cloud_AI_VehicleElement::json_archive() const TX_NOEXCEPT {
#ifdef ON_CLOUD
  using OutputArchive = cereal::JSONOutputArchive;
  using InputArchive = cereal::JSONInputArchive;
  std::stringstream ss;        // any stream can be used
  OutputArchive oarchive(ss);  // Create an output archive
  oarchive(*this);             // Write the data to the archive
  // LOG(INFO) << TX_VARS(Id()) << TX_VARS(ss.str().size()) << ss.str();
  return std::move(ss.str());
#else
  return "";
#endif
}

Base::txBool TAD_Cloud_AI_VehicleElement::Post_Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(FollowFrontPtr()) &&
      (_plus_(Base::Enums::DrivingStatus::stop_crash) == FollowFrontPtr()->StableDriving_Status()) &&
      (_plus_(Base::Enums::DrivingStatus::stop_crash) != StableDriving_Status()) &&
      (FollowFrontDistance() < FLAGS_stop_crash_inverse_kill_distance)) {
    Kill();
  }
  return ParentClass::Post_Update(timeMgr);
}

sim_msg::Car* TAD_Cloud_AI_VehicleElement::FillingElement(txFloat const timeStamp,
                                                          sim_msg::Car* pSimVehicle) TX_NOEXCEPT {
  pSimVehicle = ParentClass::FillingElement(timeStamp, pSimVehicle);
  if (NonNull_Pointer(pSimVehicle)) {
    if (CallFail(InRangeAfterSimulation())) {
      pSimVehicle->set_serialization_archive(binary_archive());
      MultiRegionInfo << "[cloud_debug][4b]" << TX_VARS(Id())
                      << " out of range after simulation, set_serialization_archive.";
    } else {
      MultiRegionInfo << "[cloud_debug][4a]" << TX_VARS(Id()) << " in range after simulation, set_simple_archive.";
    }
  }
  return pSimVehicle;
}

Base::txBool TAD_Cloud_AI_VehicleElement::FillingElement(Base::TimeParamManager const& tm,
                                                         sim_msg::Traffic& t) TX_NOEXCEPT {
  mInRangeAfterSimulation = InRanges(GetLocation().Lon(), GetLocation().Lat());
  return ParentClass::FillingElement(tm, t);
}

TX_NAMESPACE_CLOSE(TrafficFlow)
#undef LogInfo
#undef LogWarn
