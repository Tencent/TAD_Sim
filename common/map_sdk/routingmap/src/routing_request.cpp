// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/routing_request.h"

#include <iostream>

#include "routeplan/route_plan.h"
#include "routingmap/routing_utils.h"

namespace hadmap {
RoutingRequest::RoutingRequest(IdGeneratorPtr p) : idGenPtr(p), status(ROUTING_FAIL), frameId(0), pHandle(NULL) {}

RoutingRequest::~RoutingRequest() {}

void RoutingRequest::init(txMapHandle* pH, const std::string& dbName) {
  std::unique_lock<std::mutex> lck(handleMutex);
  pHandle = pH;
  this->dbName = dbName;
}

void RoutingRequest::resetRoadId(txRoute& route) {
  // reset road id by reverse flag
  for (auto& routeNode : route) {
    if (routeNode.getRouteType() == txRouteNode::ROAD_ROUTE && routeNode.needReverse()) {
      std::string index = RoutingUtils::reverseRoadIndex(routeNode.getId());
      roadpkid id = idGenPtr->generateReverseRoadId(index);
      routeNode.setId(id);
    }
  }

  // reset pre and next node id
  for (size_t i = 0; i < route.size(); ++i) {
    if (i < route.size() - 1) route[i].setNextId(route[i + 1].getId());
    if (i > 0) route[i].setPreId(route[i - 1].getId());
  }

  // reset junc id
  for (auto& routeNode : route) {
    if (routeNode.getRouteType() == txRouteNode::JUNCTION_ROUTE) {
      std::string index = RoutingUtils::linkIndex(routeNode.getPreId(), routeNode.getNextId());
      roadpkid id = idGenPtr->generateMockRoadId(index);
      routeNode.setId(id);
    }
  }
}

bool RoutingRequest::routingSync(const PointVec& poiLocs, const pkid_t& fId, txRoute& route) {
  if (poiLocs.size() < 2) return false;

  std::unique_lock<std::mutex> handleLck(handleMutex);
  if (pHandle == NULL) return false;

  {
    std::unique_lock<std::mutex> lck(routeMutex);
    if (fId <= frameId) return false;
    routeResult.clear();
    frameId = fId;
    status = ROUTING_WAIT;
  }

  planner::RoutePlan rp(pHandle);
  // planner::RoutePlan rp( dbName );
  PointVec locs(poiLocs);
  if (rp.searchPath(locs, route)) {
    resetRoadId(route);
    std::unique_lock<std::mutex> lck(routeMutex);
    if (fId >= frameId) {
      frameId = fId;
      routeResult.assign(route.begin(), route.end());
      status = ROUTING_SUCC;
      return true;
    } else {
      return false;
    }
  } else {
    std::unique_lock<std::mutex> lck(routeMutex);
    if (fId >= frameId) {
      frameId = fId;
      routeResult.clear();
      status = ROUTING_FAIL;
      route.clear();
    }
    return false;
  }
}

RoutingStatus RoutingRequest::getRoute(txRoute& route) {
  std::unique_lock<std::mutex> lck(routeMutex);
  if (status == ROUTING_SUCC) route.assign(routeResult.begin(), routeResult.end());
  return status;
}
}  // namespace hadmap
