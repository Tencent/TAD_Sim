// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_vehicle_obstacle_element.h"

#include "tx_spatial_query.h"

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool PS_ObsVehicleElement::CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallFail(IsEnd())) {
    if (timeMgr.AbsTime() >= mLifeCycle.EndTime()) {
      mLifeCycle.SetEnd();
    }
  }
  return IsEnd();
}

Base::txBool PS_ObsVehicleElement::Initialize(const obs_vehicle_info& obsVehInfo,
                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  m_vehicle_type = +VEHICLE_TYPE::Sedan;

  mIdentity.Id() = obsVehInfo.vehId;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  SM::txAITrafficState::Initialize(mIdentity.Id());
  mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
  mLifeCycle.StartTime() = obsVehInfo.startTimeStamp;
  mLifeCycle.EndTime() = obsVehInfo.endTimeStamp;

  mGeometryData.Length() = obsVehInfo.length;
  mGeometryData.Width() = obsVehInfo.width;
  mGeometryData.Height() = obsVehInfo.height;
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

  m_aggress = 0.0;
  mLaneKeepMgr.ResetCounter(LaneKeep(), m_aggress);
#if USE_SUDOKU_GRID
  Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
  ConfigureFollowStrategy();
  mKinetics.m_velocity = 0.0;
  mKinetics.LastVelocity() = mKinetics.m_velocity;
  mKinetics.m_velocity_max = 0.0;
  mKinetics.velocity_desired = mKinetics.m_velocity_max;

  const Base::txLaneID startLaneId = obsVehInfo.laneUid.laneId;
  mLocation.NextLaneIndex() = TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(startLaneId);
  mLocation.LaneOffset() = 0.0;

  const txBool bLocateOnLaneLink = (0 == startLaneId);
  LOG(INFO) << "[vehicle_init_location] " << TX_VARS_NAME(VehicleId, mIdentity.Id())
            << (bLocateOnLaneLink ? " [LaneLink]" : " [Lane]");
  if (CallFail(bLocateOnLaneLink)) {
    TX_MARK("on lane");
    hadmap::txLanePtr initLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(obsVehInfo.laneUid);

    if (NonNull_Pointer(initLanePtr)) {
      LOG(INFO) << TX_VARS_NAME(initLaneUid, Utils::ToString(initLanePtr->getTxLaneId()));
      mLocation.PosOnLaneCenterLinePos() = HdMap::HadmapCacheConCurrent::GetLanePos(initLanePtr, obsVehInfo.s);
      mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
      mLocation.DistanceAlongCurve() = obsVehInfo.s;
      mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(initLanePtr, mLocation.DistanceAlongCurve());
      /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
      RawVehicleCoord() = mLocation.PosOnLaneCenterLinePos();
      SyncPosition(0.0);
      mLocation.InitTracker(mIdentity.Id());
      RelocateTracker(initLanePtr, 0.0);
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
      LOG(WARNING) << ", Get Init Lane Error.";
      return false;
    }
  } else {
    LOG(WARNING) << "start lane id error. " << TX_VARS(startLaneId);
    return false;
  }
  return true;
}

Base::txBool PS_ObsVehicleElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                  sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  return IsAlive();
  /*if (IsAlive()) {
      FillingSpatialQuery();
  }
  return true;*/
}

TX_NAMESPACE_CLOSE(TrafficFlow)
