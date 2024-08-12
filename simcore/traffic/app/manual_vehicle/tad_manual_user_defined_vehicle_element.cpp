// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_manual_user_defined_vehicle_element.h"
#include "tx_spatial_query.h"

TX_NAMESPACE_OPEN(TrafficFlow)
#if Use_ManualVehicle

Base::txBool TAD_Manual_UserDefined_VehicleElement::Pre_Update(
    const TimeParamManager& timeMgr, std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT {
  if (IsManualMode()) {
    return true;
  } else {
    return ParentClass::Pre_Update(timeMgr, map_elemId2Kinetics);
  }
}

Base::txBool TAD_Manual_UserDefined_VehicleElement::Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsManualMode()) {
    return true;
  } else {
    return ParentClass::Update(timeMgr);
  }
}

sim_msg::Car* TAD_Manual_UserDefined_VehicleElement::FillingElement(txFloat const timeStamp,
                                                                    sim_msg::Car* pSimVehicle) TX_NOEXCEPT {
  ClearManualMode();
  return ParentClass::FillingElement(timeStamp, pSimVehicle);
}

void TAD_Manual_UserDefined_VehicleElement::UpdateManualLocation(Base::TimeParamManager const& timeMgr,
                                                                 const sim_msg::Location& _location) TX_NOEXCEPT {
  m_ManualMode = sim_msg::ControlState::Manual;
  if (IsAlive()) {
    LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle)
        << TX_VARS(Id()) << TX_VARS(_location.t()) << TX_VARS(_location.position().x())
        << TX_VARS(_location.position().y()) << TX_VARS(_location.position().z()) << TX_VARS(_location.velocity().x())
        << TX_VARS(_location.velocity().y()) << TX_VARS(_location.velocity().z())
        << TX_VARS_NAME(veh_manual_heading, _location.rpy().z());

    mLocation.GeomCenter().FromWGS84(_location.position().x(), _location.position().y(), _location.position().z());
    m_velocity3D = Base::txVec3(_location.velocity().x(), _location.velocity().y(), _location.velocity().z());
    m_rpy_yaw = _location.rpy().z();

    if (CallFail(UpdateManualVehicle(timeMgr))) {
      mGeometryData.SetInValid();
      LOG(WARNING) << "UpdateManualVehicle failure." << TX_VARS(Id());
      return;
    }

    mGeometryData.ComputePolygon(GeomCenter().ToENU(), StableLaneDir());
    LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle)
        << TX_VARS_NAME(Pos, GeomCenter()) << TX_VARS_NAME(m_velocity, GetVelocity())
        << TX_VARS_NAME(dir, Utils::ToString(GetLaneDir())) << TX_VARS_NAME(Geometry, mGeometryData.Str());
  }
}

void TAD_Manual_UserDefined_VehicleElement::ComputeScalarVelocity(Base::TimeParamManager const& timeMgr,
                                                                  const Coord::txENU& egoMassCenter) TX_NOEXCEPT {
  const Coord::txENU& last_EgoMassCenter = StableGeomCenter();
  if (timeMgr.RelativeTime() > 0.0) {
    const Base::txVec2 dist_vec = egoMassCenter.ENU2D() - last_EgoMassCenter.ENU2D();
    const Base::txFloat dist = (dist_vec).norm();
    mKinetics.m_velocity = dist / timeMgr.RelativeTime();
    mKinetics.m_acceleration = (mKinetics.m_velocity - StableVelocity()) / timeMgr.RelativeTime();
  } else {
    mKinetics.m_velocity = 0.0;
    mKinetics.m_acceleration = 0.0;
  }
}

Base::txBool TAD_Manual_UserDefined_VehicleElement::UpdateManualVehicle(Base::TimeParamManager const& timeMgr)
    TX_NOEXCEPT {
  hadmap::txLanePtr resLane = nullptr;
  hadmap::txLaneLinkPtr resLaneLink = nullptr;
  ComputeScalarVelocity(timeMgr, GeomCenter().ToENU());
  LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle)
      << "ComputeScalarVelocity : " << timeMgr.str() << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());

  Base::Info_Lane_t locInfo;
  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
          GeomCenter(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve())) {
    LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << TX_VARS_NAME(Get_S_Coord_By_Enu_Pt, mLocation.LaneLocInfo());
    if (mLocation.IsOnLane()) {
      resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      if (NonNull_Pointer(resLane)) {
        LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << "hadmap::getLane success.";
        RelocateTracker(resLane, timeMgr.TimeStamp());
      } else {
        LOG_IF(WARNING, FLAGS_LogLevel_ManualVehicle) << "ManualVehicle not on road.";
        return false;
      }
    } else {
      resLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                   mLocation.LaneLocInfo().onLinkToLaneUid);
      if (NonNull_Pointer(resLaneLink)) {
        LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << "hadmap::getLaneLink success.";
        RelocateTracker(resLaneLink, timeMgr.TimeStamp());
      } else {
        LOG_IF(WARNING, FLAGS_LogLevel_ManualVehicle) << "ManualVehicle not on link.";
        return false;
      }
    }
  } else {
    LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << "Get_S_Coord_By_Enu_Pt failure.";
    return false;
  }

  LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle)
      << "hadmap::getLane end." << ((mLocation.IsOnLaneLink()) ? "[onLink]" : "[onLane]")
      << TX_VARS(mLocation.DistanceAlongCurve());

  Base::txVec2 yawDir(std::cos(m_rpy_yaw), std::sin(m_rpy_yaw));
  LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << TX_VARS_NAME(yawDir, Utils::ToString(yawDir));
  mLocation.vLaneDir() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);
  mLocation.NextLaneIndex() = Utils::ComputeLaneIndex(mLocation.LaneLocInfo().onLaneUid.laneId);

  return true;
}

Base::txString TAD_Manual_UserDefined_VehicleElement::ManualDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_VehicleElement::ManualDesc");
}

#endif /*Use_ManualVehicle*/
TX_NAMESPACE_CLOSE(TrafficFlow)
