// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_signal_utils.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_hadmap_utils.h"
#include "tx_string_utils.h"

TX_NAMESPACE_OPEN(Utils)

std::set<Base::txLaneID> GetPreLinkFromLanes(const std::set<Base::txLaneUId>& laneUidSet,
                                             Base::txFloat& existingInflunceLength,
                                             std::set<Base::txLaneUId>& fromLaneUidSet) TX_NOEXCEPT {
  std::stringstream ss;
  for (const auto dd : laneUidSet) {
    ss << Utils::ToString(dd);
  }
  LOG_IF(INFO, FLAGS_LogLevel_Signal) << "GetPreLinkFromLanes laneUidSet :" << ss.str();

  fromLaneUidSet.clear();
  Base::txFloat curInflunceLength = 0.0;
  std::set<Base::txLaneID> retLinkSet;
  std::set<Base::txRoadID> roadIdSet;
  std::vector<hadmap::txPoint> srcLaneStartPtSet;
  for (const auto laneUid : laneUidSet) {
    roadIdSet.insert(laneUid.roadId);
    if (0 == laneUid.sectionId) {
      hadmap::txLanePtr _lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(laneUid);
      if (_lanePtr && (_lanePtr->getGeometry())) {
        srcLaneStartPtSet.push_back(_lanePtr->getGeometry()->getStart());
      }
    }
  }

  for (const auto roadId : roadIdSet) {
    hadmap::txLaneLinks nearLaneLinks;
    std::vector<Base::txRoadID> roadIdVec{roadId};
    hadmap::getLaneLinks(txMapHdr, roadIdVec, nearLaneLinks);

    for (const auto curLinkPtr : nearLaneLinks) {
      if (curLinkPtr && curLinkPtr->toRoadId() == roadId && curLinkPtr->getGeometry()) {
        const Base::txFloat curLinkLength = curLinkPtr->getGeometry()->getLength();
        if (curLinkLength > 0.1) {
          const hadmap::txPoint linkEndPt = curLinkPtr->getGeometry()->getEnd();
          for (const auto startPt : srcLaneStartPtSet) {
            if (Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(startPt, linkEndPt) < 0.2) {
              retLinkSet.insert(curLinkPtr->getId());
              if (curLinkLength > curInflunceLength) {
                curInflunceLength = curLinkLength;
              }
              fromLaneUidSet.insert(curLinkPtr->fromTxLaneId());
              break;
            }
          }
        } else {
          hadmap::txLanePtr _fromLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(curLinkPtr->fromTxLaneId());
          if (_fromLanePtr && _fromLanePtr->getGeometry()) {
            const hadmap::txPoint fromLaneEndPt = _fromLanePtr->getGeometry()->getEnd();
            for (const auto startPt : srcLaneStartPtSet) {
              if (Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(startPt, fromLaneEndPt) < 0.2) {
                fromLaneUidSet.insert(curLinkPtr->fromTxLaneId());
                break;
              }
            }
          }
        }
      }
    }
  }
  existingInflunceLength += curInflunceLength;
  std::stringstream ssfromLaneUidSet;
  for (const auto dd : fromLaneUidSet) {
    ssfromLaneUidSet << Utils::ToString(dd);
  }
  LOG_IF(INFO, FLAGS_LogLevel_Signal) << "GetPreLinkFromLanes fromLaneUidSet :" << ssfromLaneUidSet.str();
  return retLinkSet;
}

std::set<Base::txLaneUId> GetPreLanesFromLane(const hadmap::txLanePtr srcLanePtr) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retLaneUidSet;
  const auto laneUid = srcLanePtr->getTxLaneId();
  if (laneUid.sectionId > 0) {
    hadmap::txSectionPtr srcSectionPtr =
        HdMap::HadmapCacheConCurrent::GetTxSectionPtr(std::make_pair(laneUid.roadId, laneUid.sectionId - 1));
    if (srcSectionPtr) {
      hadmap::txLanes& preLanes = srcSectionPtr->getLanes();
      const hadmap::txPoint curLaneStartPt = srcLanePtr->getGeometry()->getStart();

      for (auto preLanePtr : preLanes) {
        if (Utils::IsLaneValid(preLanePtr)) {
          const hadmap::txPoint curPrevLaneEndPt = preLanePtr->getGeometry()->getEnd();

          if (Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(curLaneStartPt, curPrevLaneEndPt) <
              FLAGS_Connect_Lane_Dist) {
            retLaneUidSet.insert(preLanePtr->getTxLaneId());
          }
        }
      }
    }
  }
  return retLaneUidSet;
}

std::set<Base::txLaneUId> Get_1stRing_PreLanes_WithoutSelfLane_FromLaneUidSet(
    const std::set<Base::txLaneUId>& srcLaneUidSet) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retLaneUidSet;
  for (const auto laneUid : srcLaneUidSet) {
    hadmap::txLanePtr curLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(laneUid);
    if (curLanePtr) {
      std::set<Base::txLaneUId> _1stRingPreLanes = GetPreLanesFromLane(curLanePtr);
      std::copy(std::begin(_1stRingPreLanes), std::end(_1stRingPreLanes),
                std::inserter(retLaneUidSet, std::end(retLaneUidSet)));
    }
  }
  return retLaneUidSet;
}

std::set<Base::txLaneUId> GetAllPreLanesAndSelfLaneFromLane(const hadmap::txLanePtr srcLanePtr,
                                                            Base::txFloat& length) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retLaneUidSet;
  length = 0.0;
  if (srcLanePtr) {
    retLaneUidSet.insert(srcLanePtr->getTxLaneId());
    std::set<Base::txLaneUId> curStepInput = retLaneUidSet;
    Base::txInt itrCnt = 0;
    while (itrCnt < FLAGS_MaxSignalControlLaneIterateCount) {
      itrCnt++;
      std::set<Base::txLaneUId> curStepOutput_1stRing_preLanes =
          Get_1stRing_PreLanes_WithoutSelfLane_FromLaneUidSet(curStepInput);
      if (CallSucc(curStepOutput_1stRing_preLanes.empty())) {
        break;
      } else {
        std::copy(std::begin(curStepOutput_1stRing_preLanes), std::end(curStepOutput_1stRing_preLanes),
                  std::inserter(retLaneUidSet, std::end(retLaneUidSet)));
        curStepInput = curStepOutput_1stRing_preLanes;
      }
    }
  }

  const hadmap::txPoint stopLinePt = srcLanePtr->getGeometry()->getEnd();
  for (const auto _laneUid : retLaneUidSet) {
    hadmap::txLanePtr _lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(_laneUid);
    const hadmap::txPoint laneStartPt = _lanePtr->getGeometry()->getStart();
    const Base::txFloat curLength = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(laneStartPt, stopLinePt);
    if (curLength > length) {
      length = curLength;
    }
  }

  return retLaneUidSet;
}

void GetPreLaneAndLinkAndSelfLaneFromLane(const hadmap::txLanePtr curLanePtr, std::set<Base::txLaneUId>& refLaneUidSet,
                                          std::set<Base::txLaneID>& refLinkIdSet) TX_NOEXCEPT {
  refLaneUidSet.clear();
  refLinkIdSet.clear();
  if (curLanePtr) {
    const auto laneUid = curLanePtr->getTxLaneId();
    Base::txFloat curLaneInflunceLength = 0.0;
    std::set<Base::txLaneUId> fromLaneUidSet;
    refLaneUidSet = GetAllPreLanesAndSelfLaneFromLane(curLanePtr, curLaneInflunceLength);
    std::set<Base::txLaneUId> curRingLaneUidSet = refLaneUidSet;
    Base::txInt itrCnt = 0;
    while (itrCnt < FLAGS_MaxSignalControlLaneIterateCount) {
      LOG_IF(WARNING, FLAGS_LogLevel_Signal) << TX_VARS(curLaneInflunceLength) << TX_VARS(itrCnt);
      itrCnt++;
      if (curLaneInflunceLength > FLAGS_SignalMaxVisionDistance) {
        break;
      } else {
        std::set<Base::txLaneID> curLinkIdSet =
            GetPreLinkFromLanes(curRingLaneUidSet, curLaneInflunceLength, fromLaneUidSet);
        std::copy(std::begin(curLinkIdSet), std::end(curLinkIdSet),
                  std::inserter(refLinkIdSet, std::end(refLinkIdSet)));
        LOG_IF(WARNING, FLAGS_LogLevel_Signal) << TX_VARS(curLinkIdSet.size());
        if (curLaneInflunceLength > FLAGS_SignalMaxVisionDistance) {
          break;
        } else {
          curRingLaneUidSet.clear();
          Base::txFloat curRingMaxLength = 0.0;
          Base::txFloat curRingLength = 0.0;
          for (const auto fromLaneUid : fromLaneUidSet) {
            hadmap::txLanePtr fromLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(fromLaneUid);
            std::set<Base::txLaneUId> tmpLaneUidSet = GetAllPreLanesAndSelfLaneFromLane(fromLanePtr, curRingLength);
            LOG_IF(WARNING, FLAGS_LogLevel_Signal) << TX_VARS(tmpLaneUidSet.size());
            std::copy(std::begin(tmpLaneUidSet), std::end(tmpLaneUidSet),
                      std::inserter(curRingLaneUidSet, std::end(curRingLaneUidSet)));
            if (curRingLength > curRingMaxLength) {
              curRingMaxLength = curRingLength;
            }
          }
          LOG_IF(WARNING, FLAGS_LogLevel_Signal) << TX_VARS(curRingLaneUidSet.size());
          std::copy(std::begin(curRingLaneUidSet), std::end(curRingLaneUidSet),
                    std::inserter(refLaneUidSet, std::end(refLaneUidSet)));
          curLaneInflunceLength += curRingMaxLength;
        }
      }
    }

    LOG_IF(WARNING, FLAGS_LogLevel_Signal) << TX_VARS(curLaneInflunceLength) << TX_VARS(FLAGS_SignalMaxVisionDistance);
  }
}

TX_NAMESPACE_CLOSE(Utils)
