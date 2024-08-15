// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/hadmap_junctionroad.h"
#include "types/map_defs.h"
#include "types/map_types.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hadmap {
class TXSIMMAP_API txController {
 public:
  explicit txController(const controllerkid& cid);
  txController(const txController&);
  ~txController() = default;

  controllerkid getId() const { return id; }

  const std::vector<objectpkid>& getSignals() const;
  void setSignals(const std::vector<objectpkid>& signals);

 private:
  controllerkid id;
  std::vector<objectpkid> ctlSignals;
};
using txControllerPtr = std::shared_ptr<txController>;

using txControllers = std::vector<txControllerPtr>;

// create before road being inserted into map
class TXSIMMAP_API txJunction {
 public:
  explicit txJunction(const junctionpkid& jId);

  txJunction(const txJunction&);

  ~txJunction();

 public:
  // get junction pkid
  junctionpkid getId() const;

  // road priority compare
  // r > 0, cur road with higher priority
  // r == 0, same priority
  // r < 0, cmp road with higher priority
  // if cur road or cmp road is not junction road, or has different jId, then throw exception
  int priorityCmp(const roadpkid& curRId, const roadpkid& cmpRId);

  // judge both roads affect each other or not
  bool isInteraction(const roadpkid& curRId, const roadpkid& cmpRId);

  // get entrance roads
  void getEntranceRoads(std::vector<roadpkid>& rIds);

  // get exit roads
  void getExitRoads(std::vector<roadpkid>& rIds);

  // get junc roads
  void getJuncRoads(std::vector<roadpkid>& rIds);

  const txControllers& getControllers() const;

  void getLaneLink(std::vector<lanelinkpkid>& linkIds) const;

 public:
  // insert junc road
  // if junc road with different jId, then throw exception
  // road geom is wgs84 coord
  // will reset turn dir when dir type is DIR_None
  txJunction& insertJuncRoad(txJunctionRoadPtr& juncRoadPtr);

  void setControllers(const txControllers& ctls);
  void insertLaneLink(lanelinkpkid linkId);

 private:
  // calc road yaw by junc road
  void updateRoadYaw(txJunctionRoadPtr& juncRoadPtr);

  // calc road priority
  void updateRoadPriority(const roadpkid& curJuncRoadId);

 private:
  // junction id
  junctionpkid jId;

  // entrance roads in cur junction
  // id + yaw
  std::unordered_map<roadpkid, double> entranceRoads;

  // exit roads in cur junction
  // id + yaw
  std::unordered_map<roadpkid, double> exitRoads;

  // junc road turn direction
  std::unordered_map<roadpkid, DIR_TYPE> juncRoadsDir;

  // junc road info
  // id + < fromRId, toRId >
  std::unordered_map<roadpkid, std::pair<roadpkid, roadpkid> > juncRoads;

  // junction road priority
  // road with key id has higher priority than roads with value id
  std::unordered_map<roadpkid, std::unordered_set<roadpkid> > roadPriority;

  txControllers controllers;
  std::unordered_set<lanelinkpkid> laneLinkIds;
};

using txJunctionPtr = std::shared_ptr<txJunction>;
using txJunctions = std::vector<txJunctionPtr>;
}  // namespace hadmap
