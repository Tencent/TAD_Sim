// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_route.h"

namespace hadmap {
struct txRouteNode::RouteNodeData {
  RouteType type;
  roadpkid routeId;

  // only used for junction
  roadpkid preRouteId;
  roadpkid nextRouteId;

  // [ 0.0, 1.0 ]
  double startRange;
  double endRange;

  // reverse flag
  bool needReverse;

  // length
  double length;

  // attr
  uint64_t attr;

  RouteNodeData()
      : type(JUNCTION_ROUTE),
        routeId(ROAD_PKID_INVALID),
        preRouteId(ROAD_PKID_INVALID),
        nextRouteId(ROAD_PKID_INVALID),
        startRange(0.0),
        endRange(1.0),
        needReverse(false),
        length(0),
        attr(0) {}
};

txRouteNode::txRouteNode() : instancePtr(new RouteNodeData) {}

txRouteNode::txRouteNode(const txRouteNode& route) : instancePtr(new RouteNodeData) {
  instancePtr->type = route.instancePtr->type;
  instancePtr->routeId = route.instancePtr->routeId;
  instancePtr->preRouteId = route.instancePtr->preRouteId;
  instancePtr->nextRouteId = route.instancePtr->nextRouteId;
  instancePtr->startRange = route.instancePtr->startRange;
  instancePtr->endRange = route.instancePtr->endRange;
  instancePtr->needReverse = route.instancePtr->needReverse;
  instancePtr->length = route.instancePtr->length;
  instancePtr->attr = route.instancePtr->attr;
}

txRouteNode::~txRouteNode() {}

void txRouteNode::operator=(const txRouteNode& route) {
  instancePtr->type = route.instancePtr->type;
  instancePtr->routeId = route.instancePtr->routeId;
  instancePtr->preRouteId = route.instancePtr->preRouteId;
  instancePtr->nextRouteId = route.instancePtr->nextRouteId;
  instancePtr->startRange = route.instancePtr->startRange;
  instancePtr->endRange = route.instancePtr->endRange;
  instancePtr->needReverse = route.instancePtr->needReverse;
  instancePtr->length = route.instancePtr->length;
  instancePtr->attr = route.instancePtr->attr;
}

txRouteNode::RouteType txRouteNode::getRouteType() const { return instancePtr->type; }

roadpkid txRouteNode::getId() const { return instancePtr->routeId; }

// get pre route id
roadpkid txRouteNode::getPreId() const { return instancePtr->preRouteId; }

// get next route id
roadpkid txRouteNode::getNextId() const { return instancePtr->nextRouteId; }

// get start range from cur route
double txRouteNode::getStartRange() const { return instancePtr->startRange; }

// get end range from cur route
double txRouteNode::getEndRange() const { return instancePtr->endRange; }

// get attribute of cur route
uint64_t txRouteNode::getAttr() const { return instancePtr->attr; }

// get reverse flag
bool txRouteNode::needReverse() const { return instancePtr->needReverse; }

// get route length
double txRouteNode::getLength() const { return instancePtr->length; }

// set route type
txRouteNode& txRouteNode::setRouteType(RouteType type) {
  instancePtr->type = type;
  return *this;
}

// set route id
txRouteNode& txRouteNode::setId(roadpkid id) {
  instancePtr->routeId = id;
  return *this;
}

// set pre route id
txRouteNode& txRouteNode::setPreId(roadpkid id) {
  instancePtr->preRouteId = id;
  return *this;
}

// set next route id
txRouteNode& txRouteNode::setNextId(roadpkid id) {
  instancePtr->nextRouteId = id;
  return *this;
}

// set start range
txRouteNode& txRouteNode::setStartRange(double r) {
  instancePtr->startRange = r;
  return *this;
}

// set end range
txRouteNode& txRouteNode::setEndRange(double r) {
  instancePtr->endRange = r;
  return *this;
}

// set attribute
txRouteNode& txRouteNode::setAttr(uint64_t attr) {
  instancePtr->attr = attr;
  return *this;
}

// set reverse flag
txRouteNode& txRouteNode::setReverse(bool needReverse) {
  instancePtr->needReverse = needReverse;
  return *this;
}

// set route length
txRouteNode& txRouteNode::setLength(double l) {
  instancePtr->length = l;
  return *this;
}
}  // namespace hadmap
