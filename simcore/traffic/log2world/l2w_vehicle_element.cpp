// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "l2w_vehicle_element.h"
#include "tad_simrec_loader.h"
#include "tx_spatial_query.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_L2W)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

void L2W_VehicleElement::SwitchLog2World() TX_NOEXCEPT {
  m_l2w_status = L2W_Status::l2w_world;
  LOG(INFO) << TX_VARS(Id()) << " SET logsim to worldsim " << TX_VARS(Utils::to_underlying(m_l2w_status));
  if (SwitchAbsorbLane()) {
    mLocation.LaneOffset() = 0.0;
  }
}

Base::txBool L2W_VehicleElement::IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT {
  return ((_plus_(VEHICLE_BEHAVIOR::eTadAI) == behavior) || (_plus_(VEHICLE_BEHAVIOR::eTadAI_Arterial) == behavior) ||
          (_plus_(VEHICLE_BEHAVIOR::eL2W) == behavior));
}

Base::txBool L2W_VehicleElement::Initialize(ISceneLoader::IViewerPtr _viewPtr, ISceneLoaderPtr _loader) TX_NOEXCEPT {
  if (NonNull_Pointer(_viewPtr) && NonNull_Pointer(_loader)) {
    SceneLoader::Simrec_SceneLoader::L2W_VehiclesViewerPtr l2w_vehicle_view_ptr =
        std::dynamic_pointer_cast<SceneLoader::Simrec_SceneLoader::L2W_VehiclesViewer>(_viewPtr);
    SceneLoader::Simrec_SceneLoaderPtr l2w_sceneloader_ptr =
        std::dynamic_pointer_cast<SceneLoader::Simrec_SceneLoader>(_loader);

    if (NonNull_Pointer(l2w_vehicle_view_ptr) && NonNull_Pointer(l2w_sceneloader_ptr)) {
      m_scene_start_time_ms = l2w_sceneloader_ptr->SimrecStartTime();
      m_traj_mgr = l2w_vehicle_view_ptr->trajMgr()->clone_traj();

      if (CallFail(ParentClass::Initialize(_viewPtr, _loader))) {
        LogWarn << "vehicle ParentClass Initialize failed.";
        return false;
      }
      // for logsim first frame data
      if (mLifeCycle.StartTime() <= 0) {
        Base::TimeParamManager timeMgr(0, 0, 0, 0);
        sim_msg::Car resultCar;
        if (NonNull_Pointer(m_traj_mgr) && CallSucc(m_traj_mgr->IsValid()) &&
            CallSucc(m_traj_mgr->compute_pos(timeMgr, resultCar, mKinetics.m_velocity, mKinetics.m_acceleration))) {
          SetLogSimPbReadyFlag(true);
        }
      }
      return true;
    } else {
      LogWarn << "vehicle_view nullptr.";
      return false;
    }
  } else {
    LogWarn << "initialize failure. " << TX_COND(_viewPtr) << TX_COND(_loader);
    return false;
  }
}

Base::txBool L2W_VehicleElement::Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsLogSim()) {
    return UpdateLogSim(timeMgr);
  } else if (IsWorldSim()) {
    return ParentClass::Update(timeMgr);
  } else {
    return false;
  }
}

Base::txBool L2W_VehicleElement::UpdateLogSim(TimeParamManager const& timeMgr) TX_NOEXCEPT {
#define LogInfoL2W LOG_IF(INFO, FLAGS_LogLevel_L2W)
  if (IsAlive()) {
    LogInfo << TX_VARS(Id()) << TX_COND(IsAlive());
    sim_msg::Car resultCar;
    if (NonNull_Pointer(m_traj_mgr) && CallSucc(m_traj_mgr->IsValid()) &&
        CallSucc(m_traj_mgr->compute_pos(timeMgr, resultCar, mKinetics.m_velocity, mKinetics.m_acceleration))) {
      SetLogSimPbReadyFlag(true);
      mLocation.LastGeomCenter() = mLocation.GeomCenter().ToENU();
      mLocation.LastRearAxleCenter() = mLocation.RearAxleCenter().ToENU();

      RawVehicleCoord().FromWGS84(resultCar.x(),
                                  resultCar.y());  // mLocation.GeomCenter().FromWGS84(resultCar.x(), resultCar.y());
      Unit::txRadian heading;
      heading.FromRadian(resultCar.heading());
      mLocation.vLaneDir() = Base::txVec3(heading.Cos(), heading.Sin(), 0.0);
      SyncPosition(timeMgr.PassTime());
      UpdateLogSimVehicle(timeMgr);
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), (mLocation.vLaneDir()));
      FillingSpatialQuery();
      SaveStableState();
      LogInfo << TX_VARS(Id()) << ", UpdateLogSim." << TX_VARS(mKinetics.m_velocity)
              << TX_VARS(mKinetics.m_acceleration);
    } else {
      LOG(WARNING) << TX_VARS(Id()) << " m_traj_mgr.compute_pos() = false." << timeMgr;
      SetLogSimPbReadyFlag(false);
      // Kill();
      return false;
    }
  } else {
    LOG(WARNING) << "is not alive." << TX_VARS(Id());
  }
  return true;
}

Base::txBool L2W_VehicleElement::UpdateLogSimVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  hadmap::txLanePtr resLane = nullptr;
  hadmap::txLaneLinkPtr resLaneLink = nullptr;
  LogInfo << "hadmap::getLane start." << GeomCenter();

#if 1
  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
          GeomCenter(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset())) {
    if (mLocation.LaneOffset() > 5) {
      LogInfo << TX_VARS(Id()) << mLocation.LaneLocInfo() << TX_VARS(mLocation.DistanceAlongCurve())
              << TX_VARS(mLocation.LaneOffset());
    }

    LogInfo << "Get_S_Coord_By_Enu_Pt : " << mLocation.LaneLocInfo();
    if (mLocation.IsOnLane()) {
      resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      if (NonNull_Pointer(resLane)) {
        LogInfo << "hadmap::getLane success.";
        RelocateTracker(resLane, 0.0);
        mLocation.PosOnLaneCenterLinePos() =
            HdMap::HadmapCacheConCurrent::GetLanePos(resLane, mLocation.DistanceAlongCurve());
      } else {
        LogWarn << "Ego not on road.";
        return false;
      }
    } else {
      resLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                   mLocation.LaneLocInfo().onLinkToLaneUid);
      if (NonNull_Pointer(resLaneLink)) {
        LogInfo << "hadmap::getLaneLink success.";
        RelocateTracker(resLaneLink, FLT_MAX);
        mLocation.PosOnLaneCenterLinePos() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkPos(resLaneLink, mLocation.DistanceAlongCurve());
      } else {
        LogWarn << "Ego not on link.";
        return false;
      }
    }
  } else {
    LogWarn << "Get_S_Coord_By_Enu_Pt failure.";
    return false;
  }
#endif
  LogInfo << "hadmap::getLane end." << ((mLocation.IsOnLaneLink()) ? "[onLink]" : "[onLane]")
          << TX_VARS(mLocation.DistanceAlongCurve());

  if (NonNull_Pointer(resLaneLink)) {
    mLocation.LocalCoord_AxisY() =
        HdMap::HadmapCacheConCurrent::GetLaneLinkDir(resLaneLink, mLocation.DistanceAlongCurve());
    LogInfo << TX_VARS_NAME(L2W_OnLink_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else if (NonNull_Pointer(resLane)) {
    mLocation.LocalCoord_AxisY() = HdMap::HadmapCacheConCurrent::GetLaneDir(resLane, mLocation.DistanceAlongCurve());
    LogInfo << TX_VARS_NAME(L2W__OnLane_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else {
    mLocation.LocalCoord_AxisY() = mLocation.vLaneDir();
    LogInfo << TX_VARS_NAME(L2W__NotOnMap_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  }
  mLocation.LocalCoord_AxisX() = Utils::VetRotVecByDegree(mLocation.LocalCoord_AxisY(),
                                                          Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
  LogInfo << TX_VARS_NAME(L2W__LocalCoord_AxisX, Utils::ToString(mLocation.LocalCoord_AxisX()));

  return true;
}

Base::txBool L2W_VehicleElement::PreFillingElement(AtomicSize& a_size) TX_NOEXCEPT {
  return ParentClass::PreFillingElement(a_size);
}

Base::txBool L2W_VehicleElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                sim_msg::Traffic& refTraffic) TX_NOEXCEPT {
  if (IsLogSim()) {
    if (CallSucc(IsLogSimPbReady()) && NonNull_Pointer(m_traj_mgr) && CallSucc(m_traj_mgr->IsValid()) &&
        CallSucc(m_traj_mgr->EnableFilling(timeMgr))) {
      return ParentClass::FillingElement(timeMgr, refTraffic);
    } else {
      return false;
    }
  } else if (IsWorldSim()) {
    return ParentClass::FillingElement(timeMgr, refTraffic);
  }
  return false;
}

sim_msg::Car* L2W_VehicleElement::FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT {
  if (NonNull_Pointer(pSimVehicle)) {
    ParentClass::FillingElement(timeStamp, pSimVehicle);
    pSimVehicle->set_t(timeStamp + m_scene_start_time_ms);
    return pSimVehicle;
  }
  return pSimVehicle;
}

void L2W_VehicleElement::OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT { ParentClass::OnStart(timeMgr); }

Base::txBool L2W_VehicleElement::CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallSucc(IsStart())) {
    if (NonNull_Pointer(m_traj_mgr) && CallSucc(m_traj_mgr->NeedFinish(timeMgr))) {
      Kill();
      return true;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

#if __TX_Mark__("shadow")
Base::txBool L2W_VehicleElement_Shadow::FillingSpatialQuery() TX_NOEXCEPT { return true; }

#endif /*__TX_Mark__("shadow")*/
TX_NAMESPACE_CLOSE(TrafficFlow)
