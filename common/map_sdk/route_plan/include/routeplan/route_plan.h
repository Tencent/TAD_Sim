// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "mapengine/hadmap_engine.h"
#include "routeplan/traj_node.h"
#include "structs/hadmap_junctionroad.h"
#include "structs/hadmap_road.h"
#include "structs/hadmap_route.h"

#include <memory>

namespace planner {
class TXSIMMAP_API RoutePlan {
 private:
  struct Data;
  std::unique_ptr<Data> dataInsPtr;

 public:
  explicit RoutePlan(hadmap::txMapHandle* handle);

  // get route info from cloud
  explicit RoutePlan(const std::string& dbName);

  ~RoutePlan();

 public:
  // search path
  bool searchPath(std::vector<hadmap::txPoint>& dsts, RoadInfoArray& traj);

  bool searchPath(std::vector<hadmap::txPoint>& dsts, hadmap::txRoute& traj);

  // get road
  hadmap::txRoadPtr getRoad(const RoadInfo& node);

  hadmap::txRoadPtr getRoad(const hadmap::txRouteNode& route);

  // get mock road( lanelink )
  hadmap::txRoadPtr getMockRoad(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, int linkType = 0);

  // get junction ( lanelink + section )
  hadmap::txJunctionRoadPtr getJunction(const RoadInfo& juncNode);

  hadmap::txJunctionRoadPtr getJunction(const hadmap::txRouteNode& juncRoute);

  // build topo relation between roads
  bool buildTopo(hadmap::txRoadPtr fromRoadPtr, hadmap::txRoadPtr toRoadPtr);

 private:
  // get mock road by distance
  hadmap::txRoadPtr getMockRoadByDis(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId);

  // get mock road by offset
  hadmap::txRoadPtr getMockRoadByOffset(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId);

  // get cross walk in junction
  bool getCrossWalks(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, hadmap::txObjects& objects);

  // get stop line in junction
  bool getStopLine(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, hadmap::txObjects& objects);
};
}  // namespace planner
