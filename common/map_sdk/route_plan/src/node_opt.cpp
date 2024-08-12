// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routeplan/node_opt.h"

#include <cmath>
#include <exception>
#include "common/coord_trans.h"
#include "common/log.h"
#include "mapengine/hadmap_codes.h"
#include "routeplan/route_utils.h"
#include "types/map_defs.h"

namespace planner {
NodeOpt* NodeOpt::insPtr = NULL;

NodeOpt* NodeOpt::getInstance() {
  if (insPtr == NULL) insPtr = new NodeOpt;
  return insPtr;
}

void NodeOpt::setMapHandle(hadmap::txMapHandle* pH) { pHandle = pH; }

bool NodeOpt::initNode(NodePtr& nodePtr) {
  if (!nodePtr) throw std::runtime_error("initNode Error, nodePtr Empty");

  hadmap::txRoadPtr roadPtr;
  if (TX_HADMAP_DATA_OK == hadmap::getRoad(pHandle, nodePtr->nodeId.roadId, false, roadPtr)) {
    nodePtr->roadLen = roadPtr->getGeometry()->getLength();
    nodePtr->startPt = roadPtr->getGeometry()->getStart();
    nodePtr->endPt = roadPtr->getGeometry()->getEnd();
    return true;
  } else {
    char info[128];
    sprintf(info, "initNode Error, get road %s info error", std::to_string(nodePtr->nodeId.roadId).c_str());
    txlog::error(info);
    return false;
  }
}

bool NodeOpt::getNodeByLoc(const hadmap::txPoint& loc, NodePtr& nodePtr) {
  if (!nodePtr) nodePtr.reset(new Node);

  hadmap::txLanePtr lanePtr;
  if (TX_HADMAP_DATA_OK == hadmap::getLane(pHandle, loc, lanePtr)) {
    nodePtr->nodeId.roadId = lanePtr->getRoadId();
    nodePtr->nodeId.laneId = lanePtr->getId();
    return initNode(nodePtr);
  } else {
    txlog::error("getNodeByLoc Error");
    return false;
  }
}

bool NodeOpt::getOppositeNodeByLoc(const hadmap::txPoint& loc, NodePtr& nodePtr, hadmap::txPoint& tagP) {
  if (!nodePtr) nodePtr.reset(new Node);

  hadmap::txLanePtr lanePtr;
  if (TX_HADMAP_DATA_OK == hadmap::getLane(pHandle, loc, lanePtr)) {
    hadmap::txLanePtr oppositeLanePtr;
    hadmap::txLaneId oppositeLaneId;
    oppositeLaneId.roadId = lanePtr->getRoadId();
    oppositeLaneId.sectionId = lanePtr->getSectionId();
    oppositeLaneId.laneId = lanePtr->getId() > 0 ? -1 : 1;
    if (TX_HADMAP_DATA_OK == hadmap::getLane(pHandle, oppositeLaneId, oppositeLanePtr)) {
      nodePtr->nodeId.roadId = oppositeLanePtr->getRoadId();
      nodePtr->nodeId.laneId = oppositeLanePtr->getId();

      const hadmap::txLineCurve* lineCurve = dynamic_cast<const hadmap::txLineCurve*>(oppositeLanePtr->getGeometry());
      size_t nearestIndex = RouteUtil::nearestPoint(lineCurve, loc);
      if (nearestIndex == 0) {
        nearestIndex = 1;
      } else if (nearestIndex == lineCurve->size() - 1) {
        nearestIndex = lineCurve->size() - 2;
      }
      tagP = lineCurve->getPoint(nearestIndex);
      return initNode(nodePtr);
    } else {
      txlog::info("getOppositeNodeByLoc Error, No Opposite Node");
      return true;
    }
  } else {
    txlog::error("getOppositeNode - getNodeByLoc Error");
    return false;
  }
}

bool NodeOpt::getAdjNodes(NodePtr& nodePtr) {
  if (!nodePtr) {
    txlog::error("getAdjNodes Error, nodePtr is Null");
    return false;
  }

  hadmap::txLaneLinks lanelinks;
  int code = hadmap::getLaneLinks(pHandle, nodePtr->nodeId.roadId, ROAD_PKID_INVALID, lanelinks);
  if (TX_HADMAP_DATA_OK == code) {
    nodePtr->adjNodes.clear();
    for (auto itr = lanelinks.begin(); itr != lanelinks.end(); ++itr) {
      NodePtr adjNodePtr(new Node);
      adjNodePtr->nodeId.roadId = (*itr)->toRoadId();
      adjNodePtr->nodeId.laneId = (*itr)->toLaneId();
      if ((*itr)->fromLaneId() * nodePtr->nodeId.laneId < 0) continue;
      if (initNode(adjNodePtr)) {
        adjNodePtr->parrent = nodePtr;
        adjNodePtr->gCost = nodePtr->gCost + nodePtr->roadLen;
        nodePtr->adjNodes.push_back(adjNodePtr);
      }
    }
    return true;
  } else if (TX_HADMAP_DATA_EMPTY == code) {
    char info[128];
    sprintf(info, "getAdjNodes lanelinks empty from road %s", std::to_string(nodePtr->nodeId.roadId).c_str());
    txlog::info(info);
    return true;
  } else {
    txlog::error("getAdjNodes error");
    return false;
  }
}

double NodeOpt::euclideanDis(const NodePtr& srcPtr, const NodePtr& dstPtr) {
  double global0[] = {srcPtr->endPt.x, srcPtr->endPt.y, srcPtr->endPt.z};
  double global1[] = {dstPtr->startPt.x, dstPtr->startPt.y, dstPtr->startPt.z};

  coord_trans_api::lonlat2global(global0[0], global0[1], global0[2]);
  coord_trans_api::lonlat2global(global1[0], global1[1], global1[2]);

  double sqDis = std::pow(global1[0] - global0[0], 2) + std::pow(global1[1] - global0[1], 2) +
                 std::pow(global1[2] - global0[2], 2);
  if (sqDis < 1e-5) {
    return srcPtr->roadLen;
  } else {
    return srcPtr->roadLen + std::sqrt(sqDis);
  }
}

bool NodeOpt::getLaneLinks(const NodeId& fromId, const NodeId& toId, hadmap::txLaneLinks& lanelinks) {
  int code = hadmap::getLaneLinks(pHandle, fromId.roadId, toId.roadId, lanelinks);
  return TX_HADMAP_DATA_OK == code;
}

bool NodeOpt::getLaneLinks(hadmap::roadpkid fromId, hadmap::roadpkid toId, hadmap::txLaneLinks& lanelinks) {
  int code = hadmap::getLaneLinks(pHandle, fromId, toId, lanelinks);
  return TX_HADMAP_DATA_OK == code;
}

bool NodeOpt::getLaneLinks(const hadmap::txPoint& center, double radius, hadmap::txLaneLinks& lanelinks) {
  hadmap::PointVec envelope;
  radius /= 111000.0;
  envelope.push_back(hadmap::txPoint(center.x - radius, center.y - radius, 0.0));
  envelope.push_back(hadmap::txPoint(center.x + radius, center.y + radius, 0.0));
  int code = hadmap::getLaneLinks(pHandle, envelope, lanelinks);
  return TX_HADMAP_DATA_OK == code;
}

bool NodeOpt::getRoad(const NodeId& id, const bool& wholeData, hadmap::txRoadPtr& roadPtr) {
  int code = hadmap::getRoad(pHandle, id.roadId, wholeData, roadPtr);
  return TX_HADMAP_DATA_OK == code;
}

bool NodeOpt::getRoad(hadmap::roadpkid roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr) {
  int code = hadmap::getRoad(pHandle, roadId, wholeData, roadPtr);
  return TX_HADMAP_DATA_OK == code;
}

bool NodeOpt::getSection(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& secId,
                         const hadmap::lanepkid& laneId, hadmap::txSectionPtr& secPtr) {
  hadmap::txSections sections;
  int code = hadmap::getSections(pHandle, hadmap::txSectionId(roadId, secId), sections);
  if (TX_HADMAP_DATA_OK == code) {
    secPtr = sections[0];
    if (laneId != LANE_PKID_INVALID) {
      const hadmap::txLanes& lanes = secPtr->getLanes();
      std::vector<hadmap::lanepkid> uselessIds;
      for (size_t i = 0; i < lanes.size(); ++i) {
        if (laneId * lanes[i]->getId() < 0) uselessIds.push_back(lanes[i]->getId());
      }
      for (size_t i = 0; i < uselessIds.size(); ++i) secPtr->removeLane(uselessIds[i]);
      if (laneId > 0) secPtr->reverse();
    }
    return true;
  } else {
    return false;
  }
}

bool NodeOpt::getSections(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& secId,
                          const hadmap::lanepkid& laneId, double minLen, hadmap::txSections& sections) {
  hadmap::txSectionPtr secPtr;
  if (getSection(roadId, secId, laneId, secPtr)) {
    sections.clear();
    sections.push_back(secPtr);
    double totalLen = secPtr->getLength();
    if (totalLen >= minLen) {
      return true;
    } else {
      int8_t flag = -1;
      if (secId == 0) flag = 1;
      hadmap::sectionpkid curSecId = secId + flag;
      while (true) {
        hadmap::txSectionPtr curSecPtr;
        if (getSection(roadId, curSecId, laneId, curSecPtr)) {
          sections.push_back(curSecPtr);
          totalLen += curSecPtr->getLength();
          if (totalLen >= minLen || curSecId == 0) {
            return true;
          } else {
            curSecId += flag;
          }
        } else {
          return true;
        }
      }
    }
  } else {
    return false;
  }
}

bool NodeOpt::getLane(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId,
                      const hadmap::lanepkid& laneId, hadmap::txLanePtr& lanePtr) {
  int code = hadmap::getLane(pHandle, hadmap::txLaneId(roadId, sectionId, laneId), lanePtr);
  return TX_HADMAP_DATA_OK == code;
}

double NodeOpt::getLaneWidth(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId,
                             const hadmap::lanepkid& laneId) {
  hadmap::txSectionPtr secPtr;
  getSection(roadId, sectionId, laneId, secPtr);
  hadmap::txLanes lanes = secPtr->getLanes();
  for (size_t i = 0; i < lanes.size(); ++i)
    if (laneId == lanes[i]->getId()) return lanes[i]->getLaneWidth();
  return 0.0;
}

bool NodeOpt::getTrafficLights(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId,
                               const hadmap::lanepkid& laneId, hadmap::txObjects& objects) {
  int code = hadmap::getObjects(pHandle, std::vector<hadmap::txLaneId>(1, hadmap::txLaneId(roadId, sectionId, laneId)),
                                std::vector<hadmap::OBJECT_TYPE>(1, hadmap::OBJECT_TYPE_TrafficLights), objects);
  return TX_HADMAP_DATA_OK == code;
}

bool NodeOpt::getObjects(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId,
                         const hadmap::lanepkid& laneId, hadmap::OBJECT_TYPE type, hadmap::txObjects& objects) {
  int code = hadmap::getObjects(pHandle, std::vector<hadmap::txLaneId>(1, hadmap::txLaneId(roadId, sectionId, laneId)),
                                std::vector<hadmap::OBJECT_TYPE>(1, type), objects);
  return TX_HADMAP_DATA_OK == code;
}
}  // namespace planner
