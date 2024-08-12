// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_junction.h"
#include "common/coord_trans.h"
#include "common/map_util.h"
#include "structs/hadmap_curve.h"

#include <exception>
#include <iostream>

namespace hadmap {
txJunction::txJunction(const junctionpkid& id) : jId(id) {}

txJunction::txJunction(const txJunction& ths) {
  jId = ths.jId;
  entranceRoads = ths.entranceRoads;
  exitRoads = ths.exitRoads;
  juncRoadsDir = ths.juncRoadsDir;
  juncRoads = ths.juncRoads;
  roadPriority = ths.roadPriority;
  laneLinkIds = ths.laneLinkIds;
  controllers.resize(ths.controllers.size());
  for (size_t i = 0; i < controllers.size(); i++) {
    controllers[i] = txControllerPtr(new txController(*ths.controllers[i]));
  }
}

txJunction::~txJunction() {}

junctionpkid txJunction::getId() const { return jId; }

void txJunction::getEntranceRoads(std::vector<roadpkid>& rIds) {
  rIds.clear();
  for (auto itr = entranceRoads.begin(); itr != entranceRoads.end(); ++itr) rIds.push_back(itr->first);
}

void txJunction::getExitRoads(std::vector<roadpkid>& rIds) {
  rIds.clear();
  for (auto itr = exitRoads.begin(); itr != exitRoads.end(); ++itr) rIds.push_back(itr->first);
}

void txJunction::getJuncRoads(std::vector<roadpkid>& rIds) {
  rIds.clear();
  for (auto itr = juncRoads.begin(); itr != juncRoads.end(); ++itr) {
    rIds.push_back(itr->first);
    // std::cout << itr->first << " " << itr->second.first << " -> " << itr->second.second;
    DIR_TYPE type = juncRoadsDir[itr->first];
    switch (type) {
      case DIR_None:
        // std::cout << " None" << std::endl;
        break;
      case DIR_Left:
        // std::cout << " Left" << std::endl;
        break;
      case DIR_Right:
        // std::cout << " Right" << std::endl;
        break;
      case DIR_Straight:
        // std::cout << " Straight" << std::endl;
        break;
      case DIR_Uturn:
        // std::cout << " Uturn" << std::endl;
      default:
        break;
    }
  }
}

const txControllers& txJunction::getControllers() const { return controllers; }

void txJunction::getLaneLink(std::vector<lanelinkpkid>& linkIds) const {
  linkIds.assign(laneLinkIds.begin(), laneLinkIds.end());
}

int txJunction::priorityCmp(const roadpkid& curRId, const roadpkid& cmpRId) {
  if (roadPriority.find(curRId) == roadPriority.end() || roadPriority.find(cmpRId) == roadPriority.end()) {
    throw std::runtime_error("priorityCmp Error, junc type or id error");
  } else {
    const std::unordered_set<roadpkid>& curPriorityAry = roadPriority[curRId];
    const std::unordered_set<roadpkid>& cmpPriorityAry = roadPriority[cmpRId];
    if (curPriorityAry.find(cmpRId) != curPriorityAry.end()) return 1;
    if (cmpPriorityAry.find(curRId) != cmpPriorityAry.end()) return -1;
    return 0;
  }
}

txJunction& txJunction::insertJuncRoad(txJunctionRoadPtr& juncRoadPtr) {
  if (juncRoadPtr->getJuncId() != jId) throw std::runtime_error("insertJuncRoad Error, jId not match");

  if (juncRoads.find(juncRoadPtr->getId()) != juncRoads.end()) {
    throw std::runtime_error("insertJuncRoad Error, road already inserted");
  }

  juncRoads.insert(
      std::make_pair(juncRoadPtr->getId(), std::make_pair(juncRoadPtr->fromRoadId(), juncRoadPtr->toRoadId())));
  juncRoadsDir.insert(std::make_pair(juncRoadPtr->getId(), juncRoadPtr->turnDirection()));
  roadPriority.insert(std::make_pair(juncRoadPtr->getId(), std::unordered_set<roadpkid>()));

  updateRoadYaw(juncRoadPtr);
  // turn direction may be reset after updateRoadYaw
  juncRoadsDir[juncRoadPtr->getId()] = juncRoadPtr->turnDirection();

  updateRoadPriority(juncRoadPtr->getId());

  return *this;
}

void txJunction::setControllers(const txControllers& ctls) { controllers = ctls; }

void txJunction::insertLaneLink(lanelinkpkid linkId) { laneLinkIds.insert(linkId); }

bool txJunction::isInteraction(const roadpkid& curRId, const roadpkid& cmpRId) {
  if (juncRoads.find(curRId) == juncRoads.end() || juncRoads.find(cmpRId) == juncRoads.end()) {
    throw std::runtime_error("isIntersection error, not in the same intersection");
  }

  roadpkid curFromRId = juncRoads[curRId].first;
  roadpkid curToRId = juncRoads[curRId].second;
  roadpkid cmpFromRId = juncRoads[cmpRId].first;
  roadpkid cmpToRId = juncRoads[cmpRId].second;

  DIR_TYPE curRoadDir = juncRoadsDir[curRId];
  DIR_TYPE cmpRoadDir = juncRoadsDir[cmpRId];

  if (curFromRId == cmpFromRId) {
    return true;
  } else if (curToRId == cmpToRId) {
    if ((curRoadDir == DIR_Left && cmpRoadDir == DIR_Straight) ||
        (curRoadDir == DIR_Straight && cmpRoadDir == DIR_Left)) {
      return false;
    } else {
      return true;
    }
  } else {
    double curFromYaw = entranceRoads[curFromRId];
    double cmpFromYaw = entranceRoads[cmpFromRId];

    Point2d curFromDir, cmpFromDir;
    map_util::yaw2dir(curFromYaw, true, curFromDir);
    map_util::yaw2dir(cmpFromYaw, true, cmpFromDir);

    double yawOffset = map_util::angle(curFromDir, cmpFromDir);
    if (yawOffset > 120.0) {
      return true;
    } else {
      return false;
    }
  }
}

void txJunction::updateRoadPriority(const roadpkid& curJuncRoadId) {
  for (auto itr = juncRoads.begin(); itr != juncRoads.end(); ++itr) {
    roadpkid cmpRId = itr->first;
    if (cmpRId == curJuncRoadId) continue;

    if (isInteraction(curJuncRoadId, cmpRId)) {
      roadpkid curFromRId = juncRoads[curJuncRoadId].first;
      roadpkid curToRId = juncRoads[curJuncRoadId].second;
      roadpkid cmpFromRId = juncRoads[cmpRId].first;
      roadpkid cmpToRId = juncRoads[cmpRId].second;
      DIR_TYPE curDir = juncRoadsDir[curJuncRoadId];
      DIR_TYPE cmpDir = juncRoadsDir[cmpRId];

      if (curFromRId == cmpFromRId) {
        continue;
      } else if (curToRId == cmpToRId) {
        if (curDir == DIR_Uturn) {
          roadPriority[cmpRId].insert(curJuncRoadId);
        } else if (cmpDir == DIR_Uturn) {
          roadPriority[curJuncRoadId].insert(cmpRId);
        } else if (curDir == DIR_Straight) {
          roadPriority[curJuncRoadId].insert(cmpRId);
        } else if (cmpDir == DIR_Straight) {
          roadPriority[cmpRId].insert(curJuncRoadId);
        } else if (curDir == DIR_Right) {
          roadPriority[cmpRId].insert(curJuncRoadId);
        } else if (cmpDir == DIR_Right) {
          roadPriority[curJuncRoadId].insert(cmpRId);
        }
      } else {
        if (curDir == DIR_Straight && cmpDir == DIR_Left) {
          roadPriority[curJuncRoadId].insert(cmpRId);
        } else if (curDir == DIR_Left && cmpDir == DIR_Straight) {
          roadPriority[cmpRId].insert(curJuncRoadId);
        }
      }
    }
  }
}

void txJunction::updateRoadYaw(txJunctionRoadPtr& juncRoadPtr) {
  txLanes& curLanes = juncRoadPtr->getSections()[0]->getLanes();
  txLanePtr curLanePtr = curLanes[curLanes.size() / 2];

  const txLineCurve* geomPtr = dynamic_cast<const txLineCurve*>(curLanePtr->getGeometry());
  txPoint center = geomPtr->getPoint(size_t(0));
  txPoint frontDir, backDir;
  txPoint p, q;
  p = geomPtr->getPoint(size_t(1));
  coord_trans_api::lonlat2enu(p.x, p.y, p.z, center.x, center.y, center.z);
  frontDir = p;

  p = geomPtr->getPoint(geomPtr->size() - 2);
  q = geomPtr->getPoint(geomPtr->size() - 1);
  coord_trans_api::lonlat2enu(p.x, p.y, p.z, center.x, center.y, center.z);
  coord_trans_api::lonlat2enu(q.x, q.y, q.z, center.x, center.y, center.z);
  backDir = txPoint(q.x - p.x, q.y - p.y, q.z - p.z);

  Point2d dir;
  double frontYaw, backYaw;

  dir.x = frontDir.x;
  dir.y = frontDir.y;
  frontYaw = map_util::dir2yaw(dir, true);

  dir.x = backDir.x;
  dir.y = backDir.y;
  backYaw = map_util::dir2yaw(dir, true);

  DIR_TYPE type = map_util::turnDir(frontYaw, backYaw);
  if (juncRoadPtr->turnDirection() == DIR_None) juncRoadPtr->setTurnDir(type);

  roadpkid fromRId = juncRoadPtr->fromRoadId();
  if (entranceRoads.find(juncRoadPtr->fromRoadId()) == entranceRoads.end()) {
    entranceRoads.insert(std::make_pair(juncRoadPtr->fromRoadId(), frontYaw));
  } else if (juncRoadPtr->turnDirection() == DIR_Straight) {
    entranceRoads[juncRoadPtr->fromRoadId()] = frontYaw;
  }

  if (exitRoads.find(juncRoadPtr->toRoadId()) == exitRoads.end()) {
    exitRoads.insert(std::make_pair(juncRoadPtr->toRoadId(), backYaw));
  } else if (juncRoadPtr->turnDirection() == DIR_Straight) {
    exitRoads[juncRoadPtr->toRoadId()] = backYaw;
  }
}
txController::txController(const controllerkid& cid) { id = cid; }

txController::txController(const txController& rhs) {
  id = rhs.id;
  ctlSignals = rhs.ctlSignals;
}

const std::vector<objectpkid>& txController::getSignals() const { return ctlSignals; }
void txController::setSignals(const std::vector<objectpkid>& signals) { ctlSignals = signals; }
}  // namespace hadmap
