// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "types/map_defs.h"
#include "types/map_types.h"

#include <memory>
#include <vector>

// used for routing map
// each route is a specified road

namespace hadmap {
class txRouteNode;
using txRoute = std::vector<txRouteNode>;

class TXSIMMAP_API txRouteNode {
 public:
  enum RouteType { ROAD_ROUTE = 1, JUNCTION_ROUTE = 2 };

 private:
  struct RouteNodeData;
  std::unique_ptr<RouteNodeData> instancePtr;

 public:
  txRouteNode();

  txRouteNode(const txRouteNode& r);

  ~txRouteNode();

  void operator=(const txRouteNode& r);

 public:
  // get route type
  RouteType getRouteType() const;

  // get route id, same as road id
  roadpkid getId() const;

  // get pre route id
  roadpkid getPreId() const;

  // get next route id
  roadpkid getNextId() const;

  // get start range from cur route
  double getStartRange() const;

  // get end range from cur route
  double getEndRange() const;

  // get attribute of cur route
  uint64_t getAttr() const;

  // get reverse flag
  bool needReverse() const;

  // get route length
  double getLength() const;

 public:
  // set route type
  txRouteNode& setRouteType(RouteType type);

  // set route id
  txRouteNode& setId(roadpkid id);

  // set pre route id
  txRouteNode& setPreId(roadpkid id);

  // set next route id
  txRouteNode& setNextId(roadpkid id);

  // set start range
  txRouteNode& setStartRange(double r);

  // set end range
  txRouteNode& setEndRange(double r);

  // set attribute
  txRouteNode& setAttr(uint64_t attr);

  // set reverse flag
  txRouteNode& setReverse(bool needReverse);

  // set route length
  txRouteNode& setLength(double l);
};
}  // namespace hadmap
