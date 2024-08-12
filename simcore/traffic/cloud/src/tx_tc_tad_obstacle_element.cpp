// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_tad_obstacle_element.h"
#include "HdMap/tx_hashed_road.h"
#define LogWarn LOG(WARNING)
#define LogInfo LOG(INFO)
TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool TAD_Cloud_ObstacleElement::InitializeOnCloud(const Base::txSysId obsId, Coord::txWGS84 wgs84_pt,
                                                          Base::ISceneLoaderPtr) TX_NOEXCEPT {
  m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
  RawVehicleCoord() = wgs84_pt;
  hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetLaneForInit(
      RawVehicleCoord().Lon(), RawVehicleCoord().Lat(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
  if (NonNull_Pointer(initLane_roadId_sectionId)) {
    LogInfo << "initLaneUid = " << Utils::ToString(initLane_roadId_sectionId->getTxLaneId())
            << TX_VARS(mLocation.DistanceAlongCurve()) << TX_VARS(mLocation.LaneOffset());

    mIdentity.Id() = obsId;
    mIdentity.SysId() = CreateSysId(mIdentity.Id());

    mGeometryData.Length() = FLAGS_EGO_Length;
    mGeometryData.Width() = FLAGS_EGO_Width;
    mGeometryData.Height() = FLAGS_EGO_Height;
    mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
    mLocation.InitTracker(mIdentity.Id());
    RelocateTracker(initLane_roadId_sectionId, 0.0);
    mObstacleType = _plus_(Base::Enums::STATIC_ELEMENT_TYPE::Box);
    mObstacleTypeStr = "Box";
    mObstacleTypeId = 1;

    mLocation.vLaneDir() =
        HdMap::HadmapCacheConCurrent::GetLaneDir(initLane_roadId_sectionId, mLocation.DistanceAlongCurve());
    m_rot.FromDegree(mLocation.rot_for_display().GetDegree());

    const Base::txVec3 frontAxis3d(mLocation.vLaneDir().x(), mLocation.vLaneDir().y(), 0.0);
    mGeometryData.ComputePolygon(RawVehicleCoord().ToENU(), frontAxis3d);
    FillingSpatialQuery();

    mLifeCycle.SetStart();
    Base::IVehicleElement::OnStart(Base::TimeParamManager());

    LogInfo << "Random Obstacle Init Success. " << Str();
    LOG(WARNING) << TX_VARS(Id()) << TX_VARS(RawVehicleCoord());
    return true;
  } else {
    LogWarn << "random obstacle location failure.";
    return false;
  }
}

void TAD_Cloud_ObstacleElement::RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT {
  m_curHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  HdMap::HashedRoadCacheConCurrent::RegisterVehicle(m_curHashedLaneInfo, GetVehicleElementPtr());
}

void TAD_Cloud_ObstacleElement::UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT {
  /*const HashedLaneInfo newHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  if (CallFail(newHashedLaneInfo == m_curHashedLaneInfo)) {
          HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(m_curHashedLaneInfo, SysId());
          HdMap::HashedRoadCacheConCurrent::RegisterVehicle(newHashedLaneInfo, GetVehicleElementPtr());
  }
  m_curHashedLaneInfo = newHashedLaneInfo;*/
}

void TAD_Cloud_ObstacleElement::UnRegisterInfoOnKill() TX_NOEXCEPT {
  /*HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(m_curHashedLaneInfo, SysId());*/
}
TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogWarn
#undef LogInfo
