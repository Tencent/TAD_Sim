// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "routingmap/routing_utils.h"
#include "structs/hadmap_predef.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace hadmap {
class IdGenerator {
 public:
  explicit IdGenerator(txMapHandle* pHandle) {
    roadpkid maxId = 0;
    if (pHandle != NULL && TX_HADMAP_DATA_OK == getRoadMaxId(pHandle, maxId)) {
      roadId = maxId + 1;
    } else {
      roadId = 0xffffffff;
    }

    lanelinkpkid linkMaxId = 0;
    if (pHandle != NULL && TX_HADMAP_DATA_OK == getLaneLinkMaxId(pHandle, linkMaxId)) {
      linkId = linkMaxId + 1;
    } else {
      linkId = 0xffffffff;
    }

    laneboundarypkid boundaryMaxId = 0;
    if (pHandle != NULL && TX_HADMAP_DATA_OK == getBoundaryMaxId(pHandle, boundaryMaxId)) {
      boundaryId = boundaryMaxId + 1;
    } else {
      boundaryId = 0xffffffff;
    }

    juncId = 1;
  }

  ~IdGenerator() {}

 public:
  // mockRoadIndex -> rid_rid
  roadpkid generateMockRoadId(const std::string& mockRoadIndex) {
    std::unique_lock<std::mutex> lck(mockRoadIdMutex);
    if (mockRoadIdMap.find(mockRoadIndex) == mockRoadIdMap.end()) {
      mockRoadIdMap.insert(std::make_pair(mockRoadIndex, roadId++));
    }
    return mockRoadIdMap[mockRoadIndex];
  }

  // reverseRoadIndex -> rid_1
  roadpkid generateReverseRoadId(const std::string& reverseRoadIndex) {
    std::unique_lock<std::mutex> lck(mockRoadIdMutex);
    if (mockRoadIdMap.find(reverseRoadIndex) == mockRoadIdMap.end()) {
      mockRoadIdMap.insert(std::make_pair(reverseRoadIndex, roadId++));
    }
    roadpkid originRId = RoutingUtils::reverseRoadIndex2RId(reverseRoadIndex);
    if (bidirectionRIdMap.find(mockRoadIdMap[reverseRoadIndex]) == bidirectionRIdMap.end()) {
      bidirectionRIdMap[mockRoadIdMap[reverseRoadIndex]] = originRId;
    }
    return mockRoadIdMap[reverseRoadIndex];
  }

  // get bidirectional road origin id
  roadpkid getOriginRoadId(const roadpkid& revRoadId) {
    std::unique_lock<std::mutex> lck(mockRoadIdMutex);
    if (bidirectionRIdMap.find(revRoadId) == bidirectionRIdMap.end()) {
      return revRoadId;
    }
    return bidirectionRIdMap[revRoadId];
  }

  // mockLinkIndex -> rid.sid.lid_rid.sid.lid
  lanelinkpkid generateLinkId(const std::string& mockLinkIndex) {
    std::unique_lock<std::mutex> lck(mockLinkIdMutex);
    if (mockLinkIdMap.find(mockLinkIndex) == mockLinkIdMap.end()) {
      mockLinkIdMap.insert(std::make_pair(mockLinkIndex, linkId++));
    }
    return mockLinkIdMap[mockLinkIndex];
  }

  laneboundarypkid generateBoundaryId() { return boundaryId++; }

  junctionpkid generateJuncId(const std::vector<lanelinkpkid>& juncLinksIndex) {
    lanelinkpkid index = *std::min_element(juncLinksIndex.begin(), juncLinksIndex.end());
    std::unique_lock<std::mutex> lck(juncIdMutex);
    if (juncIdMap.find(index) == juncIdMap.end()) {
      juncIdMap.insert(std::make_pair(index, juncId++));
    }
    return juncIdMap[index];
  }

 private:
  std::atomic<roadpkid> roadId;

  std::atomic<lanelinkpkid> linkId;

  std::atomic<laneboundarypkid> boundaryId;

  std::atomic<junctionpkid> juncId;

  std::mutex juncIdMutex;

  std::unordered_map<lanelinkpkid, junctionpkid> juncIdMap;

  std::unordered_map<std::string, lanelinkpkid> mockLinkIdMap;

  std::mutex mockLinkIdMutex;

  std::unordered_map<std::string, roadpkid> mockRoadIdMap;

  // mock road id -> origin road id
  std::unordered_map<roadpkid, roadpkid> bidirectionRIdMap;

  std::mutex mockRoadIdMutex;
};

using IdGeneratorPtr = std::shared_ptr<IdGenerator>;
}  // namespace hadmap