// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_env_vehicle_info.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_planning_flags.h"
#include "tx_spatial_query.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Element)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool txEgoEnvObstacleInfo::Initialize(const sim_msg::StaticObstacle& refObs) TX_NOEXCEPT {
  mConsistencyId = refObs.id();
  mStableEgoGeomInfo.x() = refObs.length();
  mStableEgoGeomInfo.y() = refObs.width();
  mStableEgoGeomInfo.z() = refObs.height();
  mStablePosition.FromWGS84(refObs.x(), refObs.y());

  mStableHeading.FromRadian(refObs.heading());
  mStableVelocity = 0.0;
  mStableAcc = 0.0;

  using RTree2D_NearestReferenceLine = Geometry::SpatialQuery::RTree2D_NearestReferenceLine;
  if (CallSucc(RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(mStablePosition.ToWGS84(),
                                                                                  mStableLaneInfo, mST.x(), mST.y()))) {
    if (mStableLaneInfo.IsOnLane()) {
      HdMap::txLaneInfoInterfacePtr laneGeomPtr =
          HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(mStableLaneInfo.onLaneUid);
      if (NonNull_Pointer(laneGeomPtr)) {
        mStablePositionOnLane.FromENU(laneGeomPtr->GetLocalPos(StableS()));
        mStableLaneDir = laneGeomPtr->GetLaneDir(StableS());
        mStableHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(laneGeomPtr, StableS(), mStableLaneInfo);
        mGeomLength = laneGeomPtr->GetLength();
        return true;
      } else {
        LogWarn << "GetLaneInfoByUid error. " << TX_VARS(refObs.id())
                << TX_VARS_NAME(LaneUid, Utils::ToString(mStableLaneInfo.onLaneUid));
        return false;
      }
    } else {
      HdMap::txLaneInfoInterfacePtr lanelinkGeomPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(mStableLaneInfo);
      if (NonNull_Pointer(lanelinkGeomPtr)) {
        mStablePositionOnLane.FromENU(lanelinkGeomPtr->GetLocalPos(StableS()));
        mStableLaneDir = lanelinkGeomPtr->GetLaneDir(StableS());
        mStableHashedLaneInfo =
            Geometry::SpatialQuery::GenerateHashedLaneInfo(lanelinkGeomPtr, StableS(), mStableLaneInfo);
        mGeomLength = lanelinkGeomPtr->GetLength();
        return true;
      } else {
        LogWarn << "GetLaneLinkInfoById error. " << TX_VARS(refObs.id()) << TX_VARS_NAME(LinkId, mStableLaneInfo);
        return false;
      }
    }
  } else {
    LogWarn << "obstacle Get_ST_Coord_By_Enu_Pt error. " << TX_VARS(refObs.id()) << TX_VARS(mStablePosition);
    return false;
  }
}

Base::txBool txEgoEnvVehicleInfo::Initialize(const sim_msg::Car& refCar) TX_NOEXCEPT {
  mConsistencyId = refCar.id();
  mStableEgoGeomInfo.x() = refCar.length();
  mStableEgoGeomInfo.y() = refCar.width();
  mStableEgoGeomInfo.z() = refCar.height();
  mStablePosition.FromWGS84(refCar.x(), refCar.y());
  if (LINK_PKID_INVALID == refCar.tx_lanelink_id()) {
    mStableLaneInfo.FromLane(Base::txLaneUId(refCar.tx_road_id(), refCar.tx_section_id(), refCar.tx_lane_id()));
  } else {
    Base::txLaneUId fromLaneUid(refCar.tx_road_id(), refCar.tx_section_id(), refCar.tx_lane_id());
    Base::txLaneUId toLaneUid(refCar.to_tx_road_id(), refCar.to_tx_section_id(), refCar.to_tx_lane_id());
    hadmap::txLaneLinkPtr linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(fromLaneUid, toLaneUid);
    if (NonNull_Pointer(linkPtr)) {
      mStableLaneInfo.FromLaneLink(linkPtr->getId(), linkPtr->fromTxLaneId(), linkPtr->toTxLaneId());
      /*auto geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneLinkInfoById(mStableLaneInfo.onLinkId);
      geom_ptr->xy2sl(mStablePosition.ENU2D(), mST.x(), mST.y());
      mStableHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, StableS(), mStableLaneInfo);*/
    } else {
      Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
          mStablePosition.ToWGS84(), mStableLaneInfo, mST.x());

      /*LogWarn << "can not find txLaneLinkPtr by id. " << TX_VARS(refCar.id()) << TX_VARS(refCar.tx_lanelink_id()) <<
      TX_VARS(fromLaneUid) << TX_VARS(toLaneUid); return false;*/
    }
  }
  mStableHeading.FromRadian(refCar.heading());
  mStableVelocity = refCar.v();
  mStableAcc = refCar.acc();

  if (mStableLaneInfo.IsOnLane()) {
    HdMap::txLaneInfoInterfacePtr laneGeomPtr =
        HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(mStableLaneInfo.onLaneUid);
    if (NonNull_Pointer(laneGeomPtr)) {
      laneGeomPtr->xy2sl(mStablePosition.ENU2D(), mST.x(), mST.y());
      mStablePositionOnLane.FromENU(laneGeomPtr->GetLocalPos(StableS()));
      mStableLaneDir = laneGeomPtr->GetLaneDir(StableS());
      mStableHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(laneGeomPtr, StableS(), mStableLaneInfo);
      mGeomLength = laneGeomPtr->GetLength();
    } else {
      LogWarn << "GetLaneInfoByUid error. " << TX_VARS(refCar.id())
              << TX_VARS_NAME(LaneUid, Utils::ToString(mStableLaneInfo.onLaneUid));
      return false;
    }
  } else {
    HdMap::txLaneInfoInterfacePtr lanelinkGeomPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(mStableLaneInfo);
    if (NonNull_Pointer(lanelinkGeomPtr)) {
      lanelinkGeomPtr->xy2sl(mStablePosition.ENU2D(), mST.x(), mST.y());
      mStablePositionOnLane.FromENU(lanelinkGeomPtr->GetLocalPos(StableS()));
      mStableLaneDir = lanelinkGeomPtr->GetLaneDir(StableS());
      mStableHashedLaneInfo =
          Geometry::SpatialQuery::GenerateHashedLaneInfo(lanelinkGeomPtr, StableS(), mStableLaneInfo);
      mGeomLength = lanelinkGeomPtr->GetLength();
    } else {
      LogWarn << "GetLaneLinkInfoById error. " << TX_VARS(refCar.id()) << TX_VARS_NAME(LinkId, mStableLaneInfo);
      return false;
    }
  }
  return true;
}

Base::txString txEgoSignalInfo::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS_NAME(signalId, mSignalId) << TX_VARS_NAME(color, m_color._to_string());
  return oss.str();
}

#if __TX_Mark__("txEgoEnvVehicleInfo_Factory")
std::map<Base::txSysId, txEgoEnvVehicleInfoPtr> txEgoEnvVehicleInfo_Factory::s_CarId2SurroundVehicleInfoPtr;
std::map<Base::txSysId, txEgoEnvObstacleInfoPtr> txEgoEnvVehicleInfo_Factory::s_CarId2SurroundObstacleInfoPtr;
tbb::mutex txEgoEnvVehicleInfo_Factory::tbbMutex;

txEgoEnvVehicleInfoPtr txEgoEnvVehicleInfo_Factory::GetEnvVehicleInfoPtr(const sim_msg::Car& refCar) TX_NOEXCEPT {
  tbb::mutex::scoped_lock lock(tbbMutex);
  if (_Contain_(s_CarId2SurroundVehicleInfoPtr, refCar.id())) {
    return s_CarId2SurroundVehicleInfoPtr.at(refCar.id());
  } else {
    txEgoEnvVehicleInfoPtr newCarInfoPtr(new txEgoEnvVehicleInfo());
    if (NonNull_Pointer(newCarInfoPtr) && CallSucc(newCarInfoPtr->Initialize(refCar))) {
      s_CarId2SurroundVehicleInfoPtr[refCar.id()] = newCarInfoPtr;
      return s_CarId2SurroundVehicleInfoPtr.at(refCar.id());
    } else {
      return nullptr;
    }
  }
}

txEgoEnvObstacleInfoPtr txEgoEnvVehicleInfo_Factory::GetEnvObstacleInfoPtr(const sim_msg::StaticObstacle& refObs)
    TX_NOEXCEPT {
  tbb::mutex::scoped_lock lock(tbbMutex);
  if (_Contain_(s_CarId2SurroundObstacleInfoPtr, refObs.id())) {
    return s_CarId2SurroundObstacleInfoPtr.at(refObs.id());
  } else {
    txEgoEnvObstacleInfoPtr newObsInfoPtr(new txEgoEnvObstacleInfo());
    if (NonNull_Pointer(newObsInfoPtr) && CallSucc(newObsInfoPtr->Initialize(refObs))) {
      s_CarId2SurroundObstacleInfoPtr[refObs.id()] = newObsInfoPtr;
      return s_CarId2SurroundObstacleInfoPtr.at(refObs.id());
    } else {
      return nullptr;
    }
  }
}

#endif /*__TX_Mark__("txEgoEnvVehicleInfo_Factory")*/
TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
