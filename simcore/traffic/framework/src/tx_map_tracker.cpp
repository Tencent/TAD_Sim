// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_map_tracker.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "hadmap.h"
#include "tx_hadmap_utils.h"
TX_NAMESPACE_OPEN(HdMap)

void MapTracker::updateLaneInfo(const hadmap::txLanePtr& _lanePtr,
                                const hadmap::txLaneLinkPtr& _lanelinkPtr) TX_NOEXCEPT {
  // 如果车道指针非空
  if (NonNull_Pointer(_lanePtr)) {
    m_LaneInfo.Clear();
    m_LaneInfo.FromLane(_lanePtr->getTxLaneId());
  } else if (NonNull_Pointer(_lanelinkPtr)) {  // 如果车道链接指针非空
    m_LaneInfo.Clear();
    // 从车道链接ID、起始车道ID和目标车道ID更新车道信息
    m_LaneInfo.FromLaneLink(_lanelinkPtr->getId(), _lanelinkPtr->fromTxLaneId(), _lanelinkPtr->toTxLaneId());
  }
}

inline Base::txBool UpdateLanBoundaryData(const hadmap::laneboundarypkid _laneBoundaryId) TX_NOEXCEPT {
#if 0
    hadmap::txLanePtr lanePtr = nullptr;
    hadmap::getLane(txMapHdr, hadmap::txLaneId(4, 0, -3), lanePtr);
    if (lanePtr) {
        const auto boundaryId = lanePtr->getRightBoundaryId();
        hadmap::txLaneBoundaryPtr laneboundaryPtr = nullptr;
        const auto opCode = hadmap::getBoundary(txMapHdr, boundaryId, laneboundaryPtr);
        if ((laneboundaryPtr)) {
            if (laneboundaryPtr->getLaneMark() != hadmap::LANE_MARK_Broken) {
                LOG(WARNING) << TX_VARS(boundaryId) << TX_VARS(laneboundaryPtr->getLaneMark()) << " solid";
            } else {
                LOG(WARNING) << TX_VARS(boundaryId) << TX_VARS(laneboundaryPtr->getLaneMark()) << "  not solid";
            }
        } else {
            LOG(WARNING) << TX_VARS(boundaryId) << " error.";
        }
    }
#endif
#if 0
    hadmap::txLaneBoundaryPtr laneboundaryPtr = nullptr;
    const auto opCode = hadmap::getBoundary(txMapHdr, _laneBoundaryId, laneboundaryPtr);
    if (Utils::CheckRetCode(opCode) && NonNull_Pointer(laneboundaryPtr)) {
        LOG(WARNING) << TX_VARS(_laneBoundaryId) << TX_VARS(laneboundaryPtr->getLaneMark());
        if (laneboundaryPtr->getLaneMark() != hadmap::LANE_MARK_Broken) {
            return true;
        } else {
            return false;
        }
    } else {
        LOG(WARNING) << TX_VARS(_laneBoundaryId) << " error.";
        return false;
    }
#endif
  // 获取车道边界指针
  hadmap::txLaneBoundaryPtr laneBoundaryPtr = HdMap::HadmapCacheConCurrent::GetTxBoundaryPtr(_laneBoundaryId);
  // 如果车道边界指针非空且车道标记不为"Broken"，则返回true，否则返回false
  return ((NonNull_Pointer(laneBoundaryPtr) && laneBoundaryPtr->getLaneMark() != hadmap::LANE_MARK_Broken) ? (true)
                                                                                                           : (false));
}

inline Base::txBool UpdateLanBoundaryData(const hadmap::txLaneBoundaryPtr& laneBoundaryPtr) TX_NOEXCEPT {
  txMsg("lane ptr getLeftBoundary() getRightBoundary() return nullptr.");
  return ((NonNull_Pointer(laneBoundaryPtr) && laneBoundaryPtr->getLaneMark() != hadmap::LANE_MARK_Broken) ? (true)
                                                                                                           : (false));
}

void MapTracker::update_Left_Right_Boundary(const hadmap::txLanePtr& curLanePtr) TX_NOEXCEPT {
  // 如果当前车道信息是车道链接
  if (m_LaneInfo.isOnLaneLink) {
    // 将左右边界设置为实线
    mIsLeftBoundarySolid = true;
    mIsRightBoundarySolid = true;
  } else if (NonNull_Pointer(curLanePtr)) {  // 如果当前车道指针非空
    // 获取左右边界的车道标记
    hadmap::LANE_MARK leftMark, rightMark;
    std::tie(leftMark, rightMark) = HadmapCacheConCurrent::GetLaneMark_LR(curLanePtr->getTxLaneId());

    // 判断左右边界是否为实线
    mIsLeftBoundarySolid = (0 != (leftMark & hadmap::LANE_MARK_Solid));
    mIsRightBoundarySolid = (0 != (rightMark & hadmap::LANE_MARK_Solid));
    // mIsLeftBoundarySolid = UpdateLanBoundaryData(curLanePtr->getLeftBoundaryId());
    // mIsRightBoundarySolid = UpdateLanBoundaryData(curLanePtr->getRightBoundaryId());
#if 0
        LOG(WARNING) << TX_VARS(mId) << TX_COND_NAME(mIsLeftBoundarySolid, mIsLeftBoundarySolid)
            << TX_COND_NAME(mIsRightBoundarySolid, mIsRightBoundarySolid)
            << TX_COND_NAME(has_LeftBoundary_pointer, NonNull_Pointer(curLanePtr->getLeftBoundary()))
            << TX_VARS_NAME(LeftBoundaryId, curLanePtr->getLeftBoundaryId())
            << TX_COND_NAME(has_RightBoundary_pointer, NonNull_Pointer(curLanePtr->getRightBoundary()))
            << TX_VARS_NAME(RightBoundaryId, curLanePtr->getRightBoundaryId())
            << TX_VARS(Utils::ToString(curLanePtr->getTxLaneId()));
#endif
  } else {
    mIsLeftBoundarySolid = true;
    mIsRightBoundarySolid = true;
    LOG(WARNING) << "[MapTracker::update_Left_Right_Boundary] state error.";
  }
}

void MapTracker::update_Left_Right_Lane(const Base::Info_Lane_t& _locInfo, const Base::txSize nLaneCnt) TX_NOEXCEPT {
  // 如果当前车道信息是车道链接
  if (_locInfo.isOnLaneLink) {
    // 清空左右车道指针和几何信息指针
    currentLeftLanePtr = nullptr;
    currentRightLanePtr = nullptr;
    m_Left_LaneGeomIntoPtr = nullptr;
    m_Right_LaneGeomIntoPtr = nullptr;
  } else {
    const Base::txLaneID laneIndex = Utils::ComputeLaneIndex(_locInfo.onLaneUid.laneId);

    // 如果车道索引大于0
    if (laneIndex > 0) {
      TX_MARK("has left lane");
      const Base::txLaneUId leftLaneUid = Utils::LeftLaneUid_UnCheck(_locInfo.onLaneUid);
      // 获取左侧车道指针和几何信息指针
      currentLeftLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(leftLaneUid);
      m_Left_LaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(leftLaneUid);
      // 如果左侧车道有效且几何信息指针非空，则执行空操作，否则清空左侧车道指针和几何信息指针
      if (Utils::IsLaneValidDriving(currentLeftLanePtr) && NonNull_Pointer(m_Left_LaneGeomIntoPtr)) {
      } else {
        currentLeftLanePtr = nullptr;
        m_Left_LaneGeomIntoPtr = nullptr;
      }
    } else {
      // LOG(INFO) << TXST_TRACE_VARIABLES_NAME(maptrack, mId) << " on the most left lane." <<
      // TXST_TRACE_VARIABLES(laneIndex);
      currentLeftLanePtr = nullptr;
      m_Left_LaneGeomIntoPtr = nullptr;
    }

    // 如果车道索引小于车道总数减1
    if (laneIndex < (nLaneCnt - 1)) {
      TX_MARK("has right lane");
      const Base::txLaneUId rightLaneUid = Utils::RightLaneUid_UnCheck(_locInfo.onLaneUid);
      // 获取右侧车道指针和几何信息指针
      currentRightLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(rightLaneUid);
      m_Right_LaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(rightLaneUid);
      // 如果右侧车道有效且几何信息指针非空，则执行空操作，否则清空右侧车道指针和几何信息指针
      if (Utils::IsLaneValidDriving(currentRightLanePtr) && NonNull_Pointer(m_Right_LaneGeomIntoPtr)) {
      } else {
        // 清空右侧车道指针和几何信息指针
        currentRightLanePtr = nullptr;
        m_Right_LaneGeomIntoPtr = nullptr;
      }
    } else {
      // LOG(INFO) << TXST_TRACE_VARIABLES_NAME(maptrack, mId) << " on the most right lane." <<
      // TXST_TRACE_VARIABLES(laneIndex);
      currentRightLanePtr = nullptr;
      m_Right_LaneGeomIntoPtr = nullptr;
    }
  }
}

Base::txBool MapTracker::isReachedRoadLastSection() const TX_NOEXCEPT {
  // 在车道连接上返回false
  if (m_LaneInfo.isOnLaneLink) {
    return false;
  } else {
    // 所在段的id等于最后段的索引返回true
    if ((mCurrentSectionSize - 1) == m_LaneInfo.onLaneUid.sectionId) {
      return true;
    } else {
      return false;
    }
  }
}

const std::set<Base::txLaneUId> MapTracker::getNextLanes() const TX_NOEXCEPT {
  using namespace HdMap;
  // 如果当前车道信息是车道链接
  if (m_LaneInfo.isOnLaneLink) {
    // 创建一个包含目标车道Uid的车道集合
    std::set<Base::txLaneUId> resLaneSet{m_LaneInfo.onLinkToLaneUid};
    return resLaneSet;
  } else {
    // 根据当前车道Uid获取下一个车道集合
    return HadmapCacheConCurrent::GetNextLaneSetByLaneUid(m_LaneInfo.onLaneUid);
  }
}

const Base::LocInfoSet MapTracker::getNextLaneLinks() const TX_NOEXCEPT {
  // 如果在道路末端
  if (isReachedRoadLastSection()) {
    // 返回连接处的车道连接
    return HdMap::HadmapCacheConCurrent::GetLaneNextLinkSet(m_LaneInfo.onLaneUid);
  }
  return Base::LocInfoSet();
}

Base::txBool MapTracker::relocate(const hadmap::txLanePtr& lanePtr) TX_NOEXCEPT {
  if (Null_Pointer(lanePtr)) {
    TX_MARK("input param error.");
    return false;
  }
  const Base::txLaneUId newLaneUid = lanePtr->getTxLaneId();
  if (Utils::IsEqual(currentLanePtr, lanePtr)) {
    TX_MARK("reloacte lane ptr is the same as current lane ptr.");
    return true;
  }

  // 如果当前车道指针非空
  if (NonNull_Pointer(currentLanePtr)) {
    TX_MARK("current lane valid, two road connect by zero link.");
    // 断言当前车道信息不是车道链接
    txAssert(CallFail(m_LaneInfo.isOnLaneLink));
    const Base::txLaneUId& curLaneUid = m_LaneInfo.onLaneUid;
    // 如果当前车道的道路ID与新车道的道路ID不同
    if (curLaneUid.roadId != newLaneUid.roadId) {
      TX_MARK("change road");
      currentRoadPtr = HadmapCacheConCurrent::GetTxRoadPtr(newLaneUid.roadId);
      // 更新当前区间指针
      currentSectionPtr =
          HadmapCacheConCurrent::GetTxSectionPtr(std::make_pair(newLaneUid.roadId, newLaneUid.sectionId));
    } else if (curLaneUid.sectionId != newLaneUid.sectionId) {  // 如果当前车道的区间ID与新车道的区间ID不同
      TX_MARK("same road, change section");
      txAssert(NonNull_Pointer(currentRoadPtr));
      // 更新当前区间指针
      currentSectionPtr =
          HadmapCacheConCurrent::GetTxSectionPtr(std::make_pair(newLaneUid.roadId, newLaneUid.sectionId));
    } else if (curLaneUid.laneId == newLaneUid.laneId) {  // 如果当前车道的车道ID与新车道的车道ID相同
      TX_MARK("impossible.");
      return false;
    }
  } else {
    TX_MARK("track first init, or last location is on the lanelink.");
    currentRoadPtr = HadmapCacheConCurrent::GetTxRoadPtr(newLaneUid.roadId);
    currentSectionPtr = HadmapCacheConCurrent::GetTxSectionPtr(std::make_pair(newLaneUid.roadId, newLaneUid.sectionId));
  }
  currentLanePtr = lanePtr;
  currentLaneLinkPtr = nullptr;

  // 更新车道信息
  updateLaneInfo(currentLanePtr, currentLaneLinkPtr);

  txAssert(NonNull_Pointer(currentRoadPtr) && NonNull_Pointer(currentSectionPtr) && NonNull_Pointer(currentLanePtr));
  // 更新当前区间数量和当前车道数量
  mCurrentSectionSize = currentRoadPtr->getSections().size();
  mCurrentLaneSize = currentSectionPtr->getLanes().size();

  // 更新左右车道信息
  update_Left_Right_Lane(m_LaneInfo, mCurrentLaneSize);
  // 更新左右边界信息
  update_Left_Right_Boundary(currentLanePtr);
  // 更新车道几何信息指针
  mLaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(newLaneUid);
  {
    const Base::Info_Road_t newRoadId(newLaneUid.roadId);
    // 如果当前道路几何信息指针非空
    if (NonNull_Pointer(mRoadGeomInfoPtr)) {
      const Base::Info_Road_t& oldRoadId = mRoadGeomInfoPtr->RoadLocInfo();
      // 如果新道路ID与旧道路ID不同
      if (CallSucc(newRoadId != oldRoadId)) {
        mRoadGeomInfoPtr = HdMap::HadmapCacheConCurrent::GetRoadGeomInfoById(newRoadId);
      } else {
        /*on the same road.*/
      }
    } else {
      // 更新道路几何信息指针
      mRoadGeomInfoPtr = HdMap::HadmapCacheConCurrent::GetRoadGeomInfoById(newRoadId);
    }
  }
  return true;
}

Base::txBool MapTracker::relocate(const hadmap::txLaneLinkPtr& laneLinkPtr) TX_NOEXCEPT {
  // 车道连接指针为空返回false
  if (Null_Pointer(laneLinkPtr)) {
    TX_MARK("input param error.");
    return false;
  }
  /*const Base::txLaneLinkID newLinkId = laneLinkPtr->getId();*/
  const Base::txLaneUId& refFromLaneUid = laneLinkPtr->fromTxLaneId();
  const Base::txLaneUId& refToLaneUid = laneLinkPtr->toTxLaneId();
  // 如果当前车道信息是车道链接，且车道链接的起始车道Uid和目标车道Uid与当前车道信息的车道链接起始车道Uid和目标车道Uid相同
  if (m_LaneInfo.isOnLaneLink && refFromLaneUid == m_LaneInfo.onLinkFromLaneUid &&
      refToLaneUid == m_LaneInfo.onLinkToLaneUid) {
    return true;
  }

  TX_MARK("two link connection. or relocate(jump) another link on junction.");
  currentLaneLinkPtr = laneLinkPtr;
  currentLanePtr = nullptr;
  currentSectionPtr = nullptr;
  currentRoadPtr = nullptr;
  currentLeftLanePtr = nullptr;
  currentRightLanePtr = nullptr;

  updateLaneInfo(currentLanePtr, currentLaneLinkPtr);

  mCurrentSectionSize = 1;
  mCurrentLaneSize = 1;

  update_Left_Right_Lane(m_LaneInfo, mCurrentLaneSize);
  update_Left_Right_Boundary(currentLanePtr);
  mLaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(m_LaneInfo);
  m_Left_LaneGeomIntoPtr = nullptr;
  m_Right_LaneGeomIntoPtr = nullptr;

  {
    // 获取新道路ID
    const Base::Info_Road_t newRoadId(m_LaneInfo.toRoadLocInfo());
    // 如果当前道路几何信息指针非空
    if (NonNull_Pointer(mRoadGeomInfoPtr)) {
      const Base::Info_Road_t& oldRoadId = mRoadGeomInfoPtr->RoadLocInfo();
      // 如果新道路ID与旧道路ID不同
      if (CallSucc(newRoadId != oldRoadId)) {
        mRoadGeomInfoPtr = HdMap::HadmapCacheConCurrent::GetRoadGeomInfoById(newRoadId);
      } else {
        /*on the same road.*/
      }
    } else {
      // 更新道路几何信息指针
      mRoadGeomInfoPtr = HdMap::HadmapCacheConCurrent::GetRoadGeomInfoById(newRoadId);
    }
  }

  return true;
}

const hadmap::txLineCurve* MapTracker::getCurrentCurve() const TX_NOEXCEPT {
  // 如果当前车道指针非空
  if (NonNull_Pointer(currentLanePtr)) {
    // 返回当前车道的几何信息
    return dynamic_cast<const hadmap::txLineCurve*>(currentLanePtr->getGeometry());
  } else if (NonNull_Pointer(currentLaneLinkPtr)) {  // 如果当前车道链接指针非空
    // 返回当前车道链接的几何信息
    return dynamic_cast<const hadmap::txLineCurve*>(currentLaneLinkPtr->getGeometry());
  } else {
    return nullptr;
  }
}

TX_NAMESPACE_CLOSE(HdMap)
