// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "common/macros.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_mapinterface.h"
#include "structs/hadmap_route.h"

namespace hadmap {
class DrivingTrack {
 public:
  enum TrkState { NO_ROUTE = 1, ON_ROUTE = 2, OFF_ROUTE = 3, REACH_2_END = 4 };

 public:
  explicit DrivingTrack(txMapInterfacePtr iMapPtr);

  ~DrivingTrack();

 public:
  // set route info
  bool setRoute(const txRoute& route, const txPoint& startPos);

  // set current pos
  // pos coord in wgs84
  bool setPos(const txPoint& pos);

  // mark target center point ( passed )
  // used for map center update and data load
  bool markPassedCenter(const std::string& targetStr);

 public:
  // get cur route
  txRouteNode curRouteNode();

  // get front route node from cur route node
  bool frontRouteNode(txRouteNode& front_route_node);

  // get back route node from cur route node
  bool backRouteNode(txRouteNode& back_route_node);

  // get ahead route info by specified distance
  void routeAhead(double distance, txRoute& route);

  // get behind route info by specified distance
  void routeBehind(double distance, txRoute& route);

  // get passed dis in cur route node
  double passedDisInCurNode();

  // get passed distance
  double passedDis(const std::string& targetStr);

  // get ahead pos on route
  txPoint posAhead(double distance);

  // dis to specified road
  double disToRoad(const roadpkid& rId);

  // next junction pkid & junc rid on route
  bool nextJunc(junctionpkid& jId, roadpkid& jrId);

 private:
  // calc passed distance
  void updatePassedDisInCurNode(txLanePtr curLanePtr, const txPoint& enuPos);

  // update track
  void updateTrack(const txPoint& pos);

 private:
  txMapInterfacePtr iMapPtr;

  txPoint lastPos;

  txRoute route;

  size_t curNodeIndex;

  double curNodePassedDis;

  std::unordered_map<std::string, double> markDis;

  txLanePtr cur_lane_ptr_;

  TrkState cur_state_;

  DISALLOW_COPY_AND_ASSIGN(DrivingTrack);
};

using DrivingTrackPtr = std::shared_ptr<DrivingTrack>;
}  // namespace hadmap
