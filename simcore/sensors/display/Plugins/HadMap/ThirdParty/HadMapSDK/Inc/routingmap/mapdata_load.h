// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "structs/hadmap_junction.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_object.h"
#include "structs/hadmap_road.h"
#include "structs/hadmap_route.h"

#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"

#include "routingmap/id_generator.hpp"

namespace hadmap {
class MapDataLoad {
 public:
  MapDataLoad(txMapHandle* pHandle, IdGeneratorPtr idPtr);

  ~MapDataLoad();

  MapDataLoad(const MapDataLoad& l) = delete;

 public:
  void setRouteInfo(const txRoute& route);

 public:
  // load map data by specified area
  // call this function before getting map data
  // radius -> m
  bool loadData(const hadmap::txPoint& center, double radius);

  // load map data by specified road id
  bool loadData(const std::vector<roadpkid>& roadIds);

  // get cur frame map data
  bool getMapData(hadmap::txRoads& roads, hadmap::txLaneLinks& links, hadmap::txObjects& objects);

  // get cur frame added map data
  bool getAddedMapData(hadmap::txRoads& roads, hadmap::txLaneLinks& links, hadmap::txObjects& objects);

  bool getAddedJuncData(hadmap::txJunctions& juncs);

  // get cur frame removed map data
  bool getRemovedMapData(hadmap::txRoads& roads, hadmap::txLaneLinks& links, hadmap::txObjects& objects);

  bool getRemovedJuncData(hadmap::txJunctions& juncs);

 private:
  // update cur frame road data
  bool updateRoads(const hadmap::txRoads& curLoadRoads);

  // update cur frame link data
  bool updateLinks(const hadmap::txLaneLinks& curLoadLinks);

  // update cur frame object data
  bool updateObjects(const hadmap::txObjects& curLoadObjects);

  // load link data by road
  bool loadLinksByRoad(const txRoads& roads, std::unordered_map<std::string, txLaneLinks>& directLinks,
                       std::unordered_map<std::string, txLaneLinks>& junctionLinks);

  // load link data by envelope
  bool loadLinksByEnvelope(const PointVec& envelope, std::unordered_map<std::string, txLaneLinks>& directLinks,
                           std::unordered_map<std::string, txLaneLinks>& junctionLinks);

  // is mock road existed
  bool isMockRoadExisted(const std::string& mockRoadIndex);

  // are links in junction
  // all links with same link index
  bool isJuncLink(const txLaneLinks& links);

  // generate mock road
  bool generateMockRoad(const txRoadPtr& fromRoadPtr, const txRoadPtr& toRoadPtr, const txLaneLinks& juncLinks,
                        txRoadPtr& roadPtr);

  // min dis lanelinks
  std::vector<size_t> minDisLaneLinks(const std::set<lanepkid>& fromLaneIds, std::set<lanepkid>& usedFromLaneIds,
                                      const std::set<lanepkid>& toLaneIds, std::set<lanepkid>& usedToLaneIds,
                                      const txLaneLinks& lanelinks, std::set<size_t>& usedLinkIndex);

  // add roads by links
  bool addRoadsByLinks(txRoads& roads, std::unordered_map<std::string, txLaneLinks>& links);

  // remove useless links
  bool removeUselessLinks(const std::unordered_map<roadpkid, txRoadPtr>& roads,
                          std::unordered_map<std::string, txLaneLinks>& links);

  // generate mock links
  bool generateMockLinks(const txRoadPtr& fromRoadPtr, const txRoadPtr& toRoadPtr, txLaneLinks& mockLinks);

  // generate reverse roads from bidirection roads
  bool generateReverseRoads(const txRoads& roads, txRoads& reverseRoads);

  // update reverse road id in links
  bool updateRoadIdInLinks(std::unordered_map<roadpkid, txRoadPtr>& roadMap, txLaneLinks& links);

  bool updateLinkMap(std::unordered_map<std::string, txLaneLinks>& linkMap);

  // generate junc info
  bool generateJunction(std::unordered_map<std::string, txLaneLinks>& juncLinks, txRoads& roads,
                        std::unordered_map<junctionpkid, std::unordered_set<std::string> >& juncInfo);

  // update cur frame junction data
  bool updateJunctions(const hadmap::txJunctions& curLoadJunctions);

  // load map data by roads
  bool loadDataByRoads(hadmap::txRoads& roads);

  // load obj data by envelope
  bool loadObjDataByEnvelope(const std::vector<OBJECT_TYPE>& types, const PointVec& envelope);

  // load obj by added roads
  bool loadObjDataByRoads(const std::vector<OBJECT_TYPE>& types);

  // generate envelope by roads
  bool genEnvelope(const txRoads& roads);

 private:
  std::mutex routeRoadMutex;

  std::unordered_set<roadpkid> routeRoadId;

  std::mutex mockRoadIdMapMutex;

  std::unordered_map<std::string, roadpkid> mockRoadIdMap;

  std::unordered_map<roadpkid, roadpkid> mockRoad2RealRoad;

  std::unordered_map<roadpkid, txRoadPtr> curRoads;

  txRoads curAddedRoads;

  txRoads curRemovedRoads;

  std::unordered_map<lanelinkpkid, txLaneLinkPtr> curLinks;

  txLaneLinks curAddedLinks;

  txLaneLinks curRemovedLinks;

  std::unordered_map<objectpkid, txObjectPtr> curObjects;

  txObjects curAddedObjects;

  txObjects curRemovedObjects;

  std::unordered_map<junctionpkid, txJunctionPtr> curJunctions;

  txJunctions curAddedJunctions;

  txJunctions curRemovedJunctions;

  PointVec cur_envelope_;

  txMapHandle* pHandle;

  IdGeneratorPtr idGenPtr;
};
}  // namespace hadmap
