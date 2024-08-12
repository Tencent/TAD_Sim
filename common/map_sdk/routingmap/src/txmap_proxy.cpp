// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/txmap_proxy.h"

#include <iomanip>
#include <iostream>

namespace hadmap {
txMapProxy::txMapProxy(CoordType mapGeomCoordType) {
  if (COORD_WGS84 == mapGeomCoordType) {
    mapPtr.reset(new txMap(COORD_WGS84));
  } else {
    mapPtr.reset(new txMap(COORD_ENU));
  }
}

txMapProxy::~txMapProxy() {}

CoordType txMapProxy::getGeomCoordType() { return mapPtr->getGeomCoordType(); }

txMapInterface::txProjectionConstPtr txMapProxy::getProjection() { return mapPtr->getProjection(); }

txRoadPtr txMapProxy::getRoadById(const roadpkid& roadId) { return mapPtr->getRoadById(roadId); }

txLanePtr txMapProxy::getLaneById(const txLaneId& laneId) { return mapPtr->getLaneById(laneId); }

txLanes txMapProxy::getLanes(const txPoint& pos, const double& radius) { return mapPtr->getLanes(pos, radius); }

txLanePtr txMapProxy::nearestLane(const txPoint& pos, double radius) { return mapPtr->nearestLane(pos, radius); }

txLanePtr txMapProxy::nearestLane(const txPoint& pos, double radius, const double& yaw) {
  return mapPtr->nearestLane(pos, radius, yaw);
}

txLanes txMapProxy::getNextLanes(const txLanePtr& lanePtr) { return mapPtr->getNextLanes(lanePtr); }

txLanes txMapProxy::getPreLanes(const txLanePtr& lanePtr) { return mapPtr->getPreLanes(lanePtr); }

txLanePtr txMapProxy::getLeftLane(const txLanePtr& lanePtr) { return mapPtr->getLeftLane(lanePtr); }

txLanePtr txMapProxy::getRightLane(const txLanePtr& lanePtr) { return mapPtr->getRightLane(lanePtr); }

txLaneBoundaries txMapProxy::getLaneBoundaries(const txPoint& pos, const double& yaw,
                                               const std::pair<double, double>& range) {
  try {
    return mapPtr->getLaneBoundaries(pos, yaw, range);
  } catch (...) {
    std::cout << std::setprecision(10) << "Get Lane Boundaries Error at(" << pos.x << "," << pos.y << "), yaw " << yaw
              << ", range(" << range.first << "," << range.second << ")" << std::endl;
  }
  return txLaneBoundaries();
}

txObjectPtr txMapProxy::getObject(const objectpkid& objId) { return mapPtr->getObject(objId); }

txObjectPtr txMapProxy::getObject(const txPoint& pos, const OBJECT_TYPE& obj_type) {
  return mapPtr->getObject(pos, obj_type);
}

txObjects txMapProxy::getObjects(const PointVec& envelope, const std::vector<OBJECT_TYPE>& objTypes) {
  return mapPtr->getObjects(envelope, objTypes);
}

txObjects txMapProxy::getObjects(const PointVec& envelope, const double& yaw,
                                 const std::vector<OBJECT_TYPE>& objTypes) {
  return mapPtr->getObjects(envelope, yaw, objTypes);
}

txObjects txMapProxy::getObjects(const txPoint& pos, const double& yaw, const std::pair<double, double>& range,
                                 const std::vector<OBJECT_TYPE>& objTypes) {
  try {
    return mapPtr->getObjects(pos, yaw, range, objTypes);
  } catch (...) {
    std::cout << std::setprecision(10) << "Get Objects Error at(" << pos.x << "," << pos.y << "), yaw " << yaw
              << ", range(" << range.first << "," << range.second << ")" << std::endl;
  }
  return txObjects();
}

txObjects txMapProxy::getObjects(const std::vector<OBJECT_TYPE>& objTypes, const std::vector<txLaneId>& relIds) {
  return mapPtr->getObjects(objTypes, relIds);
}

// update center
bool txMapProxy::updateCenter(const txPoint& lonlatele) { return mapPtr->updateCenter(lonlatele); }

// insert data
bool txMapProxy::insertRoad(const txRoadPtr& roadPtr) { return mapPtr->insertRoad(roadPtr); }

bool txMapProxy::insertRoads(const txRoads& roads) { return mapPtr->insertRoads(roads); }

bool txMapProxy::insertLaneLink(const txLaneLinkPtr& linkPtr) { return mapPtr->insertLaneLink(linkPtr); }

bool txMapProxy::insertLaneLinks(const txLaneLinks& links) { return mapPtr->insertLaneLinks(links); }

bool txMapProxy::insertObject(const txObjectPtr& objPtr) { return mapPtr->insertObject(objPtr); }

bool txMapProxy::insertObjects(const txObjects& objects) { return mapPtr->insertObjects(objects); }

bool txMapProxy::insertJunction(const txJunctionPtr& juncPtr) { return mapPtr->insertJunction(juncPtr); }

bool txMapProxy::insertJunctions(const txJunctions& juncs) { return mapPtr->insertJunctions(juncs); }

// remove data
bool txMapProxy::removeRoad(roadpkid roadId) { return mapPtr->removeRoad(roadId); }

bool txMapProxy::removeObject(objectpkid objId) { return mapPtr->removeObject(objId); }

bool txMapProxy::removeJunction(junctionpkid jId) { return mapPtr->removeJunction(jId); }

txJunctionPtr txMapProxy::getJunction(const junctionpkid& jId) { return mapPtr->getJunction(jId); }

txJunctions txMapProxy::getJunctions() {
  txJunctions juncs;
  return juncs;
}

PRIORITY_TYPE txMapProxy::priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr) {
  return mapPtr->priorityCmp(curPtr, cmpPtr);
}
}  // namespace hadmap
