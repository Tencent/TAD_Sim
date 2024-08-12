// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_dead_line_area_manager.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_spatial_query.h"
TX_NAMESPACE_OPEN(Base)
#if 1
Base::txBool DeadLineAreaManager::Initialize(Coord::txWGS84 endPt) TX_NOEXCEPT {
  using namespace Geometry::SpatialQuery;
  Base::txFloat distanceCurve = 0.0;
  hadmap::txLanePtr _EndPtLanePtr =
      RTree2D_NearestReferenceLine::getInstance().GetLaneNearBy(endPt.Lon(), endPt.Lat(), distanceCurve);
  if (NonNull_Pointer(_EndPtLanePtr)) {
    _valid = true;
    _deadline_LaneUid = _EndPtLanePtr->getTxLaneId();
    _EndPos = endPt.ToENU();

#  if 1
    m_vLaneDir_Deadline = HdMap::HadmapCacheConCurrent::GetLaneDir(_EndPtLanePtr, distanceCurve);
    _deadline_LaneUid = _EndPtLanePtr->getTxLaneId();
    hadmap::txLanes res;
    int32_t opCode = hadmap::getLanes(
        txMapHdr, hadmap::txLaneId(_deadline_LaneUid.roadId, _deadline_LaneUid.sectionId, LANE_PKID_INVALID), res);

    Base::txFloat roadWidth = 0.0;
    for (const auto& curLane : res) {
      if (NonNull_Pointer(curLane)) {
        if ((curLane->getLaneWidth()) > 0.0) {
          roadWidth += curLane->getLaneWidth();
        } else {
          roadWidth += FLAGS_default_lane_width;
        }
      }
    }
    if (roadWidth > FLAGS_default_lane_width) {
      _valid = true;
      const Base::txVec3& vLaneDir = m_vLaneDir_Deadline;
      /*const Base::txVec3& vLaneDir = mLoc.vLaneDir();*/
      const Base::txVec3 frontAxis3d = Utils::Vec2_Vec3(Utils::Vec3_Vec2(vLaneDir));
      const Base::txVec3 rightAxis3d =
          Utils::VetRotVecByDegree(frontAxis3d, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());

      const Base::txVec2 frontAxis2d = Utils::Vec3_Vec2(frontAxis3d);
      const Base::txVec2 rightAxis2d = Utils::Vec3_Vec2(rightAxis3d);

      Geometry::OBB2D obb(endPt.ToENU().ENU2D(), frontAxis2d, rightAxis2d, 0.5 * (DeadLineLength()), 0.5 * (roadWidth));

      auto refArray = obb.GetPolygon();
      m_vecPolygon_clockwise_close = {refArray.begin(), refArray.end()};
      std::reverse(m_vecPolygon_clockwise_close.begin(), m_vecPolygon_clockwise_close.end());
      m_vecPolygon_clockwise_close.push_back(m_vecPolygon_clockwise_close.front());

      if (FLAGS_LogLevel_VehExit) {
        std::ostringstream oss;
        oss << "DeadLineAreaManager_exit_region : " << std::endl;
        for (const auto pt : m_vecPolygon_clockwise_close) {
          Coord::txENU pos(Utils::Vec2_Vec3(pt));
          oss << _StreamPrecision_ << pos.ToWGS84().Lon() << "," << pos.ToWGS84().Lat() << std::endl;
        }
        _str_polygon = oss.str();
        LOG(INFO) << oss.str();
      }
      _EndPos = endPt.ToENU();
      // LOG_IF(INFO, FLAGS_LogLevel_Cloud)  << "VehExit Init Success. " << Str() << obb.Str();
    } else {
      _valid = false;
      LOG(WARNING) << "getLaneWidth Error, " << TX_VARS(roadWidth)
                   << TX_VARS_NAME(_deadline_LaneUid, Utils::ToString(_deadline_LaneUid));
    }
#  endif
  }
  return IsValid();
}

Base::txBool DeadLineAreaManager::ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid,
                                                       const Coord::txENU& elementPt) const TX_NOEXCEPT {
  if (IsValid()) {
    if (elemLaneUid.roadId == _deadline_LaneUid.roadId && elemLaneUid.sectionId == _deadline_LaneUid.sectionId) {
      const txFloat distToEnd = Coord::txENU::EuclideanDistanceLocal_2D(elementPt, _EndPos);
      if (distToEnd < DeadLineLength()) {
        return true;
      }
    }
  }
  return false;
}

Base::txBool DeadLineAreaManager::ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid, const Base::txVec3& elemDir,
                                                       const Coord::txENU& elementPt) const TX_NOEXCEPT {
  if (IsValid()) {
    if (elemLaneUid.roadId == _deadline_LaneUid.roadId && elemLaneUid.sectionId == _deadline_LaneUid.sectionId) {
      return true;
    }

    if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(m_vLaneDir_Deadline), Utils::Vec3_Vec2(elemDir))) &&
        Coord::txENU::EuclideanDistanceLocal_2D(elementPt, _EndPos) < DeadLineLength()) {
      return true;
    }
  }
  return false;
}

Base::txString DeadLineAreaManager::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "[DeadLineAreaManager]" << TX_COND_NAME(Valid, IsValid())
     << TX_VARS_NAME(_deadline_LaneUid, Utils::ToString(_deadline_LaneUid)) << TX_VARS_NAME(_EndPos, _EndPos)
     << TX_VARS(_str_polygon);
  return ss.str();
}
#endif
TX_NAMESPACE_CLOSE(Base)
