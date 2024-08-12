// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_component.h"
#include "tx_frame_utils.h"
#include "tx_obb.h"
TX_NAMESPACE_OPEN(Base)
TX_NAMESPACE_OPEN(Component)

Info_Lane_t Location::GetCurrentLaneInfoByParam(const HdMap::MapTrackerPtr pTracker, Base::txBool& bCurInLaneLinkState,
                                                Unit::txDegree& crossRot) TX_NOEXCEPT {
  Info_Lane_t retInfo;
  if (NonNull_Pointer(pTracker)) {
    retInfo = pTracker->getLaneInfo();
    bCurInLaneLinkState = retInfo.isOnLaneLink;
    if (CallFail(retInfo.isOnLaneLink)) {
      TX_MARK("on lane");
      const Base::txVec3& vLaneDir = HdMap::HadmapCacheConCurrent::GetLaneDir(pTracker->getCurrentLane(), 0.3);
      crossRot = Utils::GetLaneAngleFromVectorOnENU(vLaneDir);
    }
  } else {
    LOG(WARNING) << "pTracker is nullptr.";
  }
  return retInfo;
}

void Location::UpdateLaneLocInfo() TX_NOEXCEPT {}

Location::txBool Location::InitTracker(const txInt _id) TX_NOEXCEPT {
  m_tracker = std::make_shared<HdMap::MapTracker>(_id);
  return NonNull_Pointer(m_tracker);
}

Location::txBool Location::RelocateTracker(hadmap::txLanePtr pLane, const txFloat& _timestamp) TX_NOEXCEPT {
  if (NonNull_Pointer(m_tracker) && NonNull_Pointer(pLane)) {
    m_tracker->relocate(pLane);
    static txBool bLink = false;
    m_laneInfo.Clear();
    m_laneInfo = GetCurrentLaneInfoByParam(m_tracker, bLink, CrossRot());
    m_curLaneIndex = Utils::ComputeLaneIndex(m_laneInfo.onLaneUid.laneId);
    m_TimeStampOnLanelink = 0.0;
    return true;
  } else {
    return false;
  }
}

Location::txBool Location::RelocateTracker(hadmap::txLaneLinkPtr pLaneLink, const txFloat& _timestamp) TX_NOEXCEPT {
  if (NonNull_Pointer(m_tracker) && NonNull_Pointer(pLaneLink)) {
    m_tracker->relocate(pLaneLink);
    static txBool bLink = false;
    m_laneInfo.Clear();
    m_laneInfo = GetCurrentLaneInfoByParam(m_tracker, bLink, CrossRot());
    m_curLaneIndex = Utils::ComputeLaneIndex(m_laneInfo.onLaneUid.laneId);
    m_TimeStampOnLanelink = _timestamp;
    return true;
  } else {
    return false;
  }
}

void Location::UpdateNewMovePointWithoutOffset() TX_NOEXCEPT {
  PosOnLaneCenterLinePos() = HdMap::HadmapCacheConCurrent::GetPos(LaneLocInfo(), DistanceAlongCurve());
  vLaneDir() = HdMap::HadmapCacheConCurrent::GetDir(LaneLocInfo(), DistanceAlongCurve());
}

txBool Location::IsOnLeftestLane() const TX_NOEXCEPT {
  if (IsOnLaneLink()) {
    return true;
  } else {
    return (-1 == LaneLocInfo().onLaneUid.laneId);
  }
}

txBool Location::IsOnRightestLane() const TX_NOEXCEPT {
  if (IsOnLaneLink()) {
    return true;
  } else {
    return ((-1 * LaneLocInfo().onLaneUid.laneId) >= tracker()->GetCurrentLaneCount());
  }
}

txFloat Location::Altitude() const TX_NOEXCEPT {
  if (NonNull_Pointer(tracker()) && NonNull_Pointer(tracker()->getCurrentCurve())) {
    return __Alt__(tracker()->getCurrentCurve()->getPoint(DistanceAlongCurve()));
  } else {
    return GeomCenter().Alt();
  }
}

void GeometryData::ComputePolygon(const Coord::txENU& Pos, const Base::txVec3& frontAxis3d_hasNormalized) TX_NOEXCEPT {
  const Base::txVec2 frontAxis2d_hasNormalized = Utils::Vec3_Vec2(frontAxis3d_hasNormalized);
  const Base::txVec3 rightAxis3d =
      Utils::VetRotVecByDegree(frontAxis3d_hasNormalized, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
  const Base::txVec2 rightAxis2d = Utils::Vec3_Vec2(rightAxis3d);
  TAG("enu2d");

  txVec2 objCenter = Pos.ENU2D();
  const txFloat objHalfLength = 0.5 * (Length());
  const txFloat objHalfWidth = 0.5 * (Width());
  if ((_plus_(Object_Type::eVehicle)) == m_type || (_plus_(Object_Type::ePedestrian)) == m_type ||
      (_plus_(Object_Type::eObstacle)) == m_type) {
    // objCenter = Pos.GetLocalPos_2D();
  } else if ((_plus_(Object_Type::EgoSUV)) == m_type) {
    objCenter = objCenter + FLAGS_EGO_MassStep * frontAxis2d_hasNormalized;
  } else if ((_plus_(Object_Type::EgoTruckLeader)) == m_type) {
    objCenter = objCenter + FLAGS_Truck_MassStep * frontAxis2d_hasNormalized;
  } else if ((_plus_(Object_Type::EgoTruckFollower)) == m_type) {
    objCenter = objCenter + FLAGS_Trailer_MassStep * frontAxis2d_hasNormalized;
  } else {
    LOG(WARNING) << "Object_Type::Unknown at " << Pos << TX_VARS(m_type);
  }
  // Base::txHdMapLocateInfo nouse(Base::txVec3(objCenter.x(), 0.0, objCenter.y()));
  // LOG(WARNING) << "\n\n#####################" << TX_VARS_NAME(geom_center, nouse) << TX_VARS_NAME(old_center, Pos);
  Geometry::OBB2D obb(objCenter, frontAxis2d_hasNormalized, rightAxis2d, objHalfLength, objHalfWidth);
  m_vecPolygon = obb.GetPolygon();
  // LOG_IF(INFO, FLAGS_LogLevel_EventTrigger) << "[GeometryData::ComputePolygon] " << TX_VARS(Pos) << TX_VARS_NAME(obb,
  // obb.Str());
}

Base::txString GeometryData::Str() const TX_NOEXCEPT {
  std::ostringstream ss_polygon;
  const PolygonArray& refPolygon = m_vecPolygon;
  for (const auto& refPt : refPolygon) {
    Coord::txENU enu_2d;
    enu_2d.FromENU(refPt.x(), refPt.y());
    ss_polygon << /*Utils::ToString(refPt)*/ enu_2d.ToWGS84().StrWGS84();
  }

  std::ostringstream ss;
  ss << "{" << TX_VARS_NAME(Length, Length()) << TX_VARS_NAME(Width, Width()) << TX_VARS_NAME(Height, Height())
     << TX_VARS_NAME(Polygon, ss_polygon.str()) << "}";

  return ss.str();
}

#if USE_HashedRoadNetwork
Location::HashedLaneInfo Location::GetHashedLaneInfo(const Base::txSysId _elemId) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_tracker)) {
    return Geometry::SpatialQuery::GenerateHashedLaneInfo(m_tracker->getLaneGeomInfo(), DistanceAlongCurve(),
                                                          LaneLocInfo());
  } else {
    LOG(FATAL) << "tracker is nullptr.";
    return HashedLaneInfo();
  }
}

Location::HashedLaneInfo Location::GetLeftHashedLaneInfo(const Base::txSysId _elemId) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_tracker)) {
    if (NonNull_Pointer(m_tracker->getLeftLane())) {
      const auto leftLaneUid = m_tracker->getLeftLane()->getTxLaneId();
      return Geometry::SpatialQuery::GenerateHashedLaneInfo(HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(leftLaneUid),
                                                            DistanceAlongCurve(), Info_Lane_t(leftLaneUid));
    } else {
      return HashedLaneInfo();
    }
  } else {
    LOG(FATAL) << "tracker is nullptr.";
    return HashedLaneInfo();
  }
}

Location::HashedLaneInfo Location::GetRightHashedLaneInfo(const Base::txSysId _elemId) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_tracker)) {
    if (NonNull_Pointer(m_tracker->getRightLane())) {
      const auto rightLaneUid = m_tracker->getRightLane()->getTxLaneId();
      return Geometry::SpatialQuery::GenerateHashedLaneInfo(
          HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(rightLaneUid), DistanceAlongCurve(),
          Info_Lane_t(rightLaneUid));
    } else {
      return HashedLaneInfo();
    }
  } else {
    LOG(FATAL) << "tracker is nullptr.";
    return HashedLaneInfo();
  }
}

#endif /*USE_HashedRoadNetwork*/

TX_NAMESPACE_CLOSE(Component)
TX_NAMESPACE_CLOSE(Base)
