// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routeplan/route_plan.h"
#include <float.h>
#include <string.h>
#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/log.h"
#include "routeplan/node_opt.h"
#include "routeplan/refine_traj.h"
#include "routeplan/route_generation.h"
#include "routeplan/route_struct.h"
#include "routeplan/route_utils.h"
#include "transmission/json_parser.h"
#include "transmission/post_param.h"
#include "transmission/trans_interface.h"
#include "transmission/url_gen.h"
#include "types/map_types.h"

#include "curl/curl.h"

using namespace hadmap;

namespace planner {
thread_local std::vector<double> tmpLaneWidth;

struct RoutePlan::Data {
 public:
  // path planer
  RouteGeneration* planerPtr;

  // cloud search
  bool cloudFlag;

  // transmission
  TransInterface* pTrans;

  std::string dbName;

 public:
  explicit Data(txMapHandle* pHandle) : planerPtr(NULL), cloudFlag(false), pTrans(NULL) {
    planerPtr = new RouteGeneration(pHandle);
  }

  explicit Data(const std::string& name) : planerPtr(NULL), cloudFlag(true), pTrans(new TransInterface), dbName(name) {}

  ~Data() {
    if (planerPtr != NULL) delete planerPtr;
    if (pTrans != NULL) delete pTrans;
  }
};

RoutePlan::RoutePlan(txMapHandle* pHandle) : dataInsPtr(new Data(pHandle)) {}

RoutePlan::RoutePlan(const std::string& dbName) : dataInsPtr(new Data(dbName)) {}

RoutePlan::~RoutePlan() {}

bool RoutePlan::searchPath(std::vector<txPoint>& dsts, RoadInfoArray& traj) {
  if (dataInsPtr->cloudFlag) return false;

  int32_t r = dataInsPtr->planerPtr->searchTrajectory(dsts, traj);
  if (r != 0) {
    txlog::info("search trajectory failed!");
    return false;
  } else {
    const int minPathSize = (traj.size() > 64 ? traj.size() + 1 : 64);
    char* info = new char[minPathSize * 32];
    sprintf(info, "Trajectory Size: %s", std::to_string(traj.size()).c_str());
    txlog::info(info);
    sprintf(info, "Trajectory Result: \n");
    char trajId[32];
    for (size_t i = 0; i < traj.size(); ++i) {
      if (traj[i].roadId > 0) {
        sprintf(trajId, "%s ", std::to_string(traj[i].roadId).c_str());
      } else {
        sprintf(trajId, "%s-%s", std::to_string(traj[i].roadId).c_str(), std::to_string(traj[i].attribute).c_str());
        switch (traj[i].attribute) {
          case U_TURN:
            strcat(trajId, "U ");
            break;
          case STRAIGHT:
            strcat(trajId, "S ");
            break;
          case TURN_LEFT:
            strcat(trajId, "L ");
            break;
          case TURN_RIGHT:
            strcat(trajId, "R ");
            break;
          case LEFT_WITH_TRAFFIC_LIGHT:
            strcat(trajId, "LwL ");
            break;
          case STRAIGHT_WITH_TRAFFIC_LIGHT:
            strcat(trajId, "SwL ");
            break;
          case RIGHT_WITH_TRAFFIC_LIGHT:
            strcat(trajId, "RwL ");
            break;
          case U_TURN_WITH_TRAFFIC_LIGHT:
            strcat(trajId, "UwL ");
            break;
        }
      }
      strcat(info, trajId);
    }
    txlog::info(info);
    delete[] info;
    info = NULL;
  }
  return true;
}

bool RoutePlan::searchPath(std::vector<txPoint>& dsts, hadmap::txRoute& traj) {
  if (dataInsPtr->cloudFlag) {
    std::string url = UrlGen(dataInsPtr->pTrans->location()).db(dataInsPtr->dbName).getRoute().url();

    std::vector<PostParamPtr> params;
    for (auto& p : dsts) params.push_back(PostParamPtr(new PosParam(p.x, p.y)));
    std::string postJson;
    generatePostJson(params, postJson);

    std::string jsonData;
    if (dataInsPtr->pTrans->post(url, postJson, jsonData) == CURLE_OK) {
      traj.clear();
      JsonParser::parseRoute(jsonData, traj);
      return true;
    } else {
      return false;
    }
  } else {
    RoadInfoArray roadAry;
    bool r = searchPath(dsts, roadAry);
    if (r) {
      for (auto road : roadAry) {
        hadmap::txRouteNode routeNode;
        routeNode.setRouteType(road.roadId < 0 ? hadmap::txRouteNode::JUNCTION_ROUTE : hadmap::txRouteNode::ROAD_ROUTE)
            .setId(road.roadId)
            .setPreId(road.fromRId)
            .setNextId(road.toRId)
            .setStartRange(road.begin)
            .setEndRange(road.end)
            .setLength(road.length)
            .setReverse(road.reverseFlag)
            .setAttr(road.attribute);
        traj.push_back(routeNode);
      }
    }
    return r;
  }
}

txRoadPtr RoutePlan::getRoad(const RoadInfo& node) {
  txRoadPtr tmpRoadPtr;
  txRoadPtr roadPtr;
  if (node.roadId < 0) {
    txlog::error("Road id error, not a real road.");
    return roadPtr;
  } else if (NodeOpt::getInstance()->getRoad(node.roadId, true, tmpRoadPtr)) {  // get road section lane info
    // reverse geom, section and lane id
    if (node.reverseFlag) {
      tmpRoadPtr->reverse();
      tmpRoadPtr->exchangeBoundaryIndex(false, true);
    } else if (tmpRoadPtr->isBidirectional()) {
      tmpRoadPtr->exchangeBoundaryIndex(false, true);
    }

    // need to be cut
    if (0.01 < node.begin || node.end < 0.99) {
      int beginSecIndex = RouteUtil::nearestSectionIndex(tmpRoadPtr, node.begin);
      int endSecIndex = RouteUtil::nearestSectionIndex(tmpRoadPtr, node.end);
      if (beginSecIndex < 0 || endSecIndex < 0) {
        return roadPtr;
      }

      // construct road
      roadPtr.reset(new txRoad);

      const txSections sections = tmpRoadPtr->getSections();
      for (size_t i = (size_t)beginSecIndex; i <= (size_t)endSecIndex; ++i) roadPtr->addSection(sections[i]);

      // cut road geom
      double startLen = 0.0;
      double endLen = 0.0;
      double totalLen = 0.0;
      for (size_t i = 0; i < sections.size(); ++i) {
        double sectionLen = sections[i]->getLength();
        if (i < (size_t)beginSecIndex) startLen += sectionLen;
        if (i <= (size_t)endSecIndex) endLen += sectionLen;
        totalLen += sectionLen;
      }
      const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(tmpRoadPtr->getGeometry());
      if (curvePtr != NULL) {
        // size_t startIndex = size_t( curvePtr->getPointIndex( curvePtr->getLength() * startLen / totalLen ) );
        // size_t endIndex = size_t( curvePtr->getPointIndex( curvePtr->getLength() * endLen / totalLen ) );
        double startDis = curvePtr->getLength() * node.begin - 10.0;
        if (startDis < 0.0) startDis = 0.0;
        size_t startIndex = size_t(curvePtr->getPointIndex(startDis));
        size_t endIndex = size_t(curvePtr->getPointIndex(curvePtr->getLength() * node.end));
        PointVec cutPoints;
        for (size_t i = startIndex; i <= endIndex; ++i) cutPoints.push_back(curvePtr->getPoint(i));
        tmpRoadPtr->setGeometry(cutPoints, COORD_WGS84);

        txPoint startP = curvePtr->getStart();
        txPoint endP = sections[beginSecIndex]->getLanes().front()->getGeometry()->getEnd();
        RouteUtil::cutSectionGeom(startP, endP, sections[beginSecIndex]);

        startP = sections[endSecIndex]->getLanes().front()->getGeometry()->getStart();
        endP = curvePtr->getEnd();
        RouteUtil::cutSectionGeom(startP, endP, sections[endSecIndex]);
      }
    } else {
      // construct road
      roadPtr.reset(new txRoad);

      const txSections sections = tmpRoadPtr->getSections();
      for (size_t i = 0; i < sections.size(); ++i) roadPtr->addSection(sections[i]);
      /*
      {
          roadPtr->addSection( sections[i] );
          txLanes lanes = sections[i]->getLanes();
          std::cout << lanes.size() << std::endl;
          for ( size_t j = 0; j < lanes.size(); ++ j )
          {
              std::cout << lanes[j]->getLeftBoundary().get() << std::endl;
          }
      }
      */
    }
    RouteUtil::copyRoadBaseInfo(tmpRoadPtr, roadPtr);
    roadPtr->buildInnerTopo();
    return roadPtr;

  } else {
    char info[128];
    sprintf(info, "Can not get road info by id %s", std::to_string(node.roadId).c_str());
    txlog::error(info);
    return roadPtr;
  }
}

txRoadPtr RoutePlan::getRoad(const hadmap::txRouteNode& route) {
  RoadInfo node;
  node.roadId = (int64_t)route.getId();
  node.reverseFlag = route.needReverse();
  node.begin = route.getStartRange();
  node.end = route.getEndRange();
  node.attribute = route.getAttr();
  return getRoad(node);
}

txRoadPtr RoutePlan::getMockRoad(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, int linkType) {
  std::vector<double> mockLaneWidth;
  txRoadPtr mockRoadPtr = getMockRoadByDis(fromRoadId, toRoadId);
  mockLaneWidth.assign(tmpLaneWidth.begin(), tmpLaneWidth.end());
  txRoadPtr negMockRoadPtr = getMockRoadByDis(toRoadId, fromRoadId);
  double speedLimitR = 1.0;
  if (linkType & STRAIGHT) {
    speedLimitR = 0.9;
  } else if ((linkType & TURN_LEFT) || (linkType & TURN_RIGHT)) {
    speedLimitR = 0.6;
  } else if (linkType & U_TURN) {
    speedLimitR = 0.3;
  }
  txlog::printf("Speed limit R: %f\n", speedLimitR);
  if (fromRoadId == toRoadId || negMockRoadPtr == NULL || negMockRoadPtr->getSections().size() == 0) {
    txlog::printf("NegMockRoadPtr is Null\n");
  } else {
    mockLaneWidth.insert(mockLaneWidth.begin(), tmpLaneWidth.begin(), tmpLaneWidth.end());
    txSectionPtr sectionPtr(new txSection);
    const txSections& sections = mockRoadPtr->getSections();
    const txSections& negSections = negMockRoadPtr->getSections();
    const txLanes& lanes = sections[0]->getLanes();
    const txLanes& negLanes = negSections[0]->getLanes();
    for (size_t i = negLanes.size() - 1;; --i) {
      negLanes[i]->reverse();
      sectionPtr->add(negLanes[i]);
      if (i == 0) break;
    }
    for (size_t i = 0; i < lanes.size(); ++i) sectionPtr->add(lanes[i]);
    sectionPtr->setId(0);
    mockRoadPtr->setSections(txSections(1, sectionPtr));
  }
  hadmap::txLanes& lanes = mockRoadPtr->getSections()[0]->getLanes();
  for (size_t i = 0; i < lanes.size(); ++i) lanes[i]->setSpeedLimit(uint32_t(lanes[i]->getSpeedLimit() * speedLimitR));
  hadmap::txLanePtr leftLanePtr = lanes.front();
  hadmap::txLanePtr rightLanePtr = lanes.back();

  double leftLaneStartWidth = mockLaneWidth[0];
  double leftLaneEndWidth = mockLaneWidth[1];
  double rightLaneStartWidth = mockLaneWidth[mockLaneWidth.size() - 2];
  double rightLaneEndWidth = mockLaneWidth[mockLaneWidth.size() - 1];

  // create left lane boundary
  hadmap::PointVec leftLaneGeom, leftBoundaryGeom, rightLaneGeom, rightBoundaryGeom;
  dynamic_cast<const hadmap::txLineCurve*>(leftLanePtr->getGeometry())->sample(2.0, leftLaneGeom);
  dynamic_cast<const hadmap::txLineCurve*>(rightLanePtr->getGeometry())->sample(2.0, rightLaneGeom);
  RouteUtil::createOffsetGeom(leftLaneGeom, leftLaneStartWidth * -0.5, leftLaneEndWidth * -0.5, leftBoundaryGeom);
  RouteUtil::createOffsetGeom(rightLaneGeom, rightLaneStartWidth * 0.5, rightLaneEndWidth * 0.5, rightBoundaryGeom);

  hadmap::txLaneBoundaryPtr leftBoundaryPtr(new hadmap::txLaneBoundary), rightBoundaryPtr(new hadmap::txLaneBoundary);
  leftBoundaryPtr->setGeometry(leftBoundaryGeom, COORD_WGS84);
  leftBoundaryPtr->setId(1);
  rightBoundaryPtr->setGeometry(rightBoundaryGeom, COORD_WGS84);
  rightBoundaryPtr->setId(2);

  leftLanePtr->setLeftBoundary(leftBoundaryPtr);
  rightLanePtr->setRightBoundary(rightBoundaryPtr);

  return mockRoadPtr;
  // return getMockRoadByDis( fromRoadId, toRoadId );
  // return getMockRoadByOffset( fromRoadId, toRoadId );
}

txRoadPtr RoutePlan::getMockRoadByDis(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId) {
  tmpLaneWidth.clear();

  txLaneLinks lanelinks;
  txRoadPtr mockRoadPtr;
  if (NodeOpt::getInstance()->getLaneLinks(fromRoadId, toRoadId, lanelinks)) {
    std::set<hadmap::lanepkid> fromLaneIds, toLaneIds, fromUsedIds, toUsedIds;
    std::set<size_t> usedLinkIndex;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      fromLaneIds.insert(lanelinks[i]->fromLaneId());
      toLaneIds.insert(lanelinks[i]->toLaneId());
    }
    std::vector<size_t> targetIndex;

    std::cout << "RoadId:" << fromRoadId << "ToRoadId" << toRoadId << std::endl;

    std::cout << "first" << std::endl;
    targetIndex = RouteUtil::minDisLaneLinks(fromLaneIds, fromUsedIds, toLaneIds, toUsedIds, lanelinks, usedLinkIndex);
    std::cout << std::endl;
    for (auto it : targetIndex) {
      std::cout << "targetIndex" << it;
    }
    // clear non-unique link
    std::unordered_map<hadmap::lanepkid, size_t> fromLaneCount, toLaneCount;
    std::vector<int> flag(targetIndex.size(), 0);
    int count = targetIndex.size();
    while (count > 0) {
      for (size_t i = 0; i < targetIndex.size(); ++i) {
        if (flag[i] != 0) continue;
        txLaneLinkPtr linkPtr = lanelinks[targetIndex[i]];
        if (fromLaneCount.find(linkPtr->fromLaneId()) == fromLaneCount.end()) {
          fromLaneCount.insert(std::make_pair(linkPtr->fromLaneId(), 1));
        } else {
          fromLaneCount[linkPtr->fromLaneId()] += 1;
        }
        if (toLaneCount.find(linkPtr->toLaneId()) == toLaneCount.end()) {
          toLaneCount.insert(std::make_pair(linkPtr->toLaneId(), 1));
        } else {
          toLaneCount[linkPtr->toLaneId()] += 1;
        }
      }
      for (size_t i = 0; i < targetIndex.size(); ++i) {
        if (flag[i] != 0) continue;
        txLaneLinkPtr linkPtr = lanelinks[targetIndex[i]];
        if (fromLaneCount[linkPtr->fromLaneId()] == 1 || toLaneCount[linkPtr->toLaneId()] == 1) {
          flag[i] = 1;
          count--;
        }
      }
      if (count == 0) break;

      double maxDis = 0;
      size_t maxIndex = 0;
      for (size_t i = 0; i < targetIndex.size(); ++i)
        if (flag[i] == 0) {
          double len = lanelinks[targetIndex[i]]->getGeometry()->getLength();
          if (len > maxDis) {
            maxDis = len;
            maxIndex = i;
          }
        }
      flag[maxIndex] = -1;
      count--;

      fromLaneCount.clear();
      toLaneCount.clear();
    }

    txSectionPtr sectionPtr(new txSection);
    sectionPtr->setId(0);
    for (int i = 0, index = 0; i < static_cast<int>(targetIndex.size()); ++i) {
      if (flag[i] != 1) continue;
      txLaneLinkPtr linkPtr = lanelinks[targetIndex[i]];

      txLanePtr fromLanePtr, toLanePtr;
      NodeOpt::getInstance()->getLane(linkPtr->fromRoadId(), linkPtr->fromSectionId(), linkPtr->fromLaneId(),
                                      fromLanePtr);
      NodeOpt::getInstance()->getLane(linkPtr->toRoadId(), linkPtr->toSectionId(), linkPtr->toLaneId(), toLanePtr);
      txLanePtr lanePtr(new txLane);
      lanePtr->setId(0 - index - 1);
      // char label[8];
      // sprintf( label, "%d", 0-i-1 );
      std::string laneName = "MockLane" + std::to_string(static_cast<int64_t>(0 - index - 1));
      lanePtr->setName(laneName);
      index++;

      // cut lane link
      txLineCurve* curvePtr = new txLineCurve(COORD_WGS84);
      txPoint startP, endP;
      if (fromLanePtr->getId() > 0) fromLanePtr->reverse();
      if (toLanePtr->getId() > 0) toLanePtr->reverse();

      if (dynamic_cast<const txLineCurve*>(fromLanePtr->getGeometry()) != NULL) {
        startP = dynamic_cast<const txLineCurve*>(fromLanePtr->getGeometry())->getEnd();
      }
      if (dynamic_cast<const txLineCurve*>(toLanePtr->getGeometry()) != NULL) {
        endP = dynamic_cast<const txLineCurve*>(toLanePtr->getGeometry())->getStart();
      }
      size_t frontIndex, backIndex;
      frontIndex = RouteUtil::nearestPoint(dynamic_cast<const txLineCurve*>(linkPtr->getGeometry()), startP);
      backIndex = RouteUtil::nearestPoint(dynamic_cast<const txLineCurve*>(linkPtr->getGeometry()), endP);
      if (frontIndex + 1 <= backIndex - 1) {
        const txLineCurve* cutCurvePtr = dynamic_cast<const txLineCurve*>(linkPtr->getGeometry());
        curvePtr->addPoint(startP, false);
        for (size_t i = frontIndex + 1; i < backIndex; ++i) curvePtr->addPoint(cutCurvePtr->getPoint(i), false);
        curvePtr->addPoint(endP, true);
      } else {
        curvePtr->addPoint(startP, false);
        curvePtr->addPoint(endP, true);
      }

      lanePtr->setGeometry(curvePtr);
      delete curvePtr;

      // set lane type & speed limit
      lanePtr->setRoadId(-1);
      lanePtr->setLaneType(LANE_TYPE_Driving);
      lanePtr->setLaneArrow(LANE_ARROW_None);
      lanePtr->setSpeedLimit(toLanePtr->getSpeedLimit());
      double fromLaneWidth = NodeOpt::getInstance()->getLaneWidth(fromLanePtr->getRoadId(), fromLanePtr->getSectionId(),
                                                                  fromLanePtr->getId());
      double toLaneWidth =
          NodeOpt::getInstance()->getLaneWidth(toLanePtr->getRoadId(), toLanePtr->getSectionId(), toLanePtr->getId());
      lanePtr->setLaneWidth((fromLaneWidth + toLaneWidth) / 2);

      tmpLaneWidth.push_back(fromLaneWidth);
      tmpLaneWidth.push_back(toLaneWidth);

      sectionPtr->add(lanePtr);
    }
    txRoadPtr fromRoadPtr;
    NodeOpt::getInstance()->getRoad(NodeId(fromRoadId), false, fromRoadPtr);

    // construct road
    mockRoadPtr.reset(new txRoad);
    mockRoadPtr->addSection(sectionPtr);
    // mockRoadPtr->setGeometry( mockRoadPtr->getSections()[0]->getLanes()[0]->getGeometry() );
    mockRoadPtr->setId(-1);
    mockRoadPtr->setRoadType(fromRoadPtr->getRoadType());
    return mockRoadPtr;
  } else {
    return mockRoadPtr;
  }
}

txRoadPtr RoutePlan::getMockRoadByOffset(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId) {
  txLaneLinks laneLinks;
  txRoadPtr mockRoadPtr;
  if (NodeOpt::getInstance()->getLaneLinks(fromRoadId, toRoadId, laneLinks)) {
    bool fromMore = true;
    std::set<hadmap::lanepkid> fromSet, toSet;
    std::map<std::pair<hadmap::lanepkid, hadmap::lanepkid>, const txCurve*> linkGeoms;
    hadmap::sectionpkid toSectionId = 0;
    for (size_t i = 0; i < laneLinks.size(); ++i) {
      hadmap::lanepkid fromLaneId = laneLinks[i]->fromLaneId();
      hadmap::lanepkid toLaneId = laneLinks[i]->toLaneId();
      toSectionId = laneLinks[i]->toSectionId();

      // abs
      laneLinks[i]->setFromLaneId(abs(laneLinks[i]->fromLaneId()));
      laneLinks[i]->setToLaneId(abs(laneLinks[i]->toLaneId()));

      fromSet.insert(laneLinks[i]->fromLaneId());
      toSet.insert(laneLinks[i]->toLaneId());

      txLanePtr fromLanePtr, toLanePtr;
      NodeOpt::getInstance()->getLane(laneLinks[i]->fromRoadId(), laneLinks[i]->fromSectionId(), fromLaneId,
                                      fromLanePtr);
      NodeOpt::getInstance()->getLane(laneLinks[i]->toRoadId(), laneLinks[i]->toSectionId(), toLaneId, toLanePtr);
      txLineCurve* curvePtr = new txLineCurve(COORD_WGS84);
      txPoint startP, endP;
      if (dynamic_cast<const txLineCurve*>(fromLanePtr->getGeometry()) != NULL) {
        startP = dynamic_cast<const txLineCurve*>(fromLanePtr->getGeometry())->getEnd();
      }
      if (dynamic_cast<const txLineCurve*>(toLanePtr->getGeometry()) != NULL) {
        endP = dynamic_cast<const txLineCurve*>(toLanePtr->getGeometry())->getStart();
      }
      if (laneLinks[i]->getGeometry() == NULL || laneLinks[i]->getGeometry()->empty()) {
        curvePtr->addPoint(startP, false);
        curvePtr->addPoint(endP, true);
      } else {
        size_t frontIndex, backIndex;
        frontIndex = RouteUtil::nearestPoint(dynamic_cast<const txLineCurve*>(laneLinks[i]->getGeometry()), startP);
        backIndex = RouteUtil::nearestPoint(dynamic_cast<const txLineCurve*>(laneLinks[i]->getGeometry()), endP);
        if (frontIndex + 1 <= backIndex - 1) {
          const txLineCurve* cutCurvePtr = dynamic_cast<const txLineCurve*>(laneLinks[i]->getGeometry());
          curvePtr->addPoint(startP, false);
          for (size_t i = frontIndex + 1; i < backIndex; ++i) curvePtr->addPoint(cutCurvePtr->getPoint(i), false);
          curvePtr->addPoint(endP, true);
        } else {
          curvePtr->addPoint(startP, false);
          curvePtr->addPoint(endP, true);
        }
      }
      laneLinks[i]->setGeometry(curvePtr);
      delete curvePtr;

      linkGeoms.insert(std::make_pair(std::make_pair(laneLinks[i]->fromLaneId(), laneLinks[i]->toLaneId()),
                                      laneLinks[i]->getGeometry()));
    }
    std::vector<hadmap::lanepkid> fromLaneId(fromSet.begin(), fromSet.end());
    std::vector<hadmap::lanepkid> toLaneId(toSet.begin(), toSet.end());
    if (toLaneId.size() > fromLaneId.size()) fromMore = false;

    std::vector<std::pair<hadmap::lanepkid, hadmap::lanepkid> > usefulLinks;
    if (fromMore) {
      size_t leftSize = (fromLaneId.size() - toLaneId.size()) / 2;
      std::vector<hadmap::lanepkid> leftId(leftSize, toLaneId.front());
      std::vector<hadmap::lanepkid> rightId(fromLaneId.size() - toLaneId.size() - leftSize, toLaneId.back());
      leftId.insert(leftId.end(), toLaneId.begin(), toLaneId.end());
      leftId.insert(leftId.end(), rightId.begin(), rightId.end());

      toLaneId.swap(leftId);
    } else {
      size_t leftSize = (toLaneId.size() - fromLaneId.size()) / 2;
      std::vector<hadmap::lanepkid> leftId(leftSize, fromLaneId.front());
      std::vector<hadmap::lanepkid> rightId(toLaneId.size() - fromLaneId.size() - leftSize, fromLaneId.back());
      leftId.insert(leftId.end(), fromLaneId.begin(), fromLaneId.end());
      leftId.insert(leftId.end(), rightId.begin(), rightId.end());

      fromLaneId.swap(leftId);
    }

    for (size_t i = 0; i < fromLaneId.size(); ++i) usefulLinks.push_back(std::make_pair(fromLaneId[i], toLaneId[i]));

    txSectionPtr sectionPtr(new txSection);
    sectionPtr->setId(0);
    for (int i = 0; i < static_cast<int>(usefulLinks.size()); ++i) {
      txLanePtr toLanePtr;
      NodeOpt::getInstance()->getLane(toRoadId, toSectionId, -usefulLinks[i].second, toLanePtr);
      txLanePtr lanePtr(new txLane);
      lanePtr->setId(0 - i - 1);
      // char label[8];
      // sprintf( label, "%d", 0-i-1 );
      std::string laneName = "MockLane" + std::to_string(static_cast<int64_t>(0 - i - 1));
      lanePtr->setName(laneName);
      lanePtr->setGeometry(linkGeoms[usefulLinks[i]]);

      // set lane type & speed limit
      lanePtr->setRoadId(-1);
      lanePtr->setLaneType(LANE_TYPE_Driving);
      lanePtr->setLaneArrow(LANE_ARROW_None);
      lanePtr->setSpeedLimit(toLanePtr->getSpeedLimit());

      sectionPtr->add(lanePtr);
    }

    txRoadPtr fromRoadPtr;
    NodeOpt::getInstance()->getRoad(NodeId(fromRoadId), false, fromRoadPtr);

    // construct road
    mockRoadPtr.reset(new txRoad);
    mockRoadPtr->addSection(sectionPtr);
    // mockRoadPtr->setGeometry( mockRoadPtr->getSections()[0]->getLanes()[0]->getGeometry() );
    mockRoadPtr->setId(-1);
    mockRoadPtr->setRoadType(fromRoadPtr->getRoadType());
    return mockRoadPtr;
  } else {
    return mockRoadPtr;
  }
}

txJunctionRoadPtr RoutePlan::getJunction(const RoadInfo& node) {
  if (node.fromRId < 0 || node.toRId < 0) {
    throw std::runtime_error("getJunction Error, not a junction");
  }
  hadmap::roadpkid fromRId = node.fromRId;
  hadmap::roadpkid toRId = node.toRId;
  txRoadPtr mockRoadPtr = getMockRoad(fromRId, toRId, static_cast<int>(node.attribute));
  txJunctionRoadPtr junctionPtr(new txJunctionRoad(*mockRoadPtr));
  junctionPtr->setTurnDir((DIR_TYPE)(node.attribute & 0xf));
  junctionPtr->bindRoadPtr();

  // get cross walk
  hadmap::txObjects crosswalks;
  if (getCrossWalks(node.fromRId, node.toRId, crosswalks)) {
    junctionPtr->setObjs(OBJECT_TYPE_CrossWalk, crosswalks);
  }

  // get stop line
  hadmap::txObjects stops;
  if (getStopLine(node.fromRId, node.toRId, stops)) {
    junctionPtr->setObjs(OBJECT_TYPE_Stop, stops);
  }

#if 0
  if (mockRoadPtr->getSections().size() > 0) {
    txLanePtr centerLanePtr = mockRoadPtr->getSections()[0]->get(-1);
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(centerLanePtr->getGeometry());
    txPoint centerPoint = curvePtr->getPoint(curvePtr->size() / 2);
    std::vector<std::pair<hadmap::roadpkid, hadmap::roadpkid> > linkIds;
    std::vector<std::pair<hadmap::sectionpkid, hadmap::sectionpkid> > secIds;
    std::vector<std::pair<hadmap::lanepkid, hadmap::lanepkid> > laneIds;
    RouteUtil::getJunctionLinks(centerPoint, fromRId, toRId, linkIds, secIds, laneIds);
    hadmap::sectionpkid junctionSecRoadId = 1000000;
    if (linkIds.size() > 0) {
      std::unordered_map<std::string, size_t> linkMap;
      std::unordered_map<std::string, size_t> secMap;
      for (size_t i = 0; i < linkIds.size(); ++i) {
        if (linkIds[i].first == fromRId && linkIds[i].second == toRId) {
          junctionPtr->linkMap.insert(RouteUtil::rsLinkIndex(node.fromRId, -1, true));
          junctionPtr->linkMap.insert(RouteUtil::rsLinkIndex(-1, node.toRId, false));
          continue;
        }
        if (linkIds[i].first == toRId && linkIds[i].second == fromRId) {
          junctionPtr->linkMap.insert(RouteUtil::rsLinkIndex(node.toRId, -1, true));
          junctionPtr->linkMap.insert(RouteUtil::rsLinkIndex(-1, node.fromRId, false));
          continue;
        }
        // std::cout << linkIds[i].first << " - " << linkIds[i].second << std::endl;
        txRoadPtr curRoadPtr = getMockRoadByDis(linkIds[i].first, linkIds[i].second);
        if (curRoadPtr->getSections().size() != 0) {
          curRoadPtr->getSections()[0]->setId(junctionSecRoadId++);
          junctionPtr->secList.push_back(curRoadPtr->getSections()[0]);
          linkMap.insert(std::make_pair(RouteUtil::linkIndex(linkIds[i].first, linkIds[i].second),
                                        junctionPtr->secList.size() - 1));
        }
        std::string fromSecIndex = RouteUtil::laneIndex(linkIds[i].first, secIds[i].first, laneIds[i].first);
        std::string toSecIndex = RouteUtil::laneIndex(linkIds[i].second, secIds[i].second, laneIds[i].second);
        if (secMap.find(fromSecIndex) == secMap.end() && linkIds[i].first != fromRId && linkIds[i].first != toRId) {
          // hadmap::txSectionPtr curSecPtr;
          hadmap::txSections sections;
          if (NodeOpt::getInstance()->getSections(linkIds[i].first, secIds[i].first, laneIds[i].first, 30.0,
                                                  sections)) {
            for (size_t j = 0; j < sections.size(); ++j) {
              hadmap::txSectionPtr curSecPtr = sections[j];
              curSecPtr->setId(junctionSecRoadId++);
              junctionPtr->secList.push_back(curSecPtr);
              if (j == 0) secMap.insert(std::make_pair(fromSecIndex, junctionPtr->secList.size() - 1));
              if (j + 1 == sections.size()) {
                continue;
              } else {
                RouteUtil::buildSecRelation(sections[j + 1], sections[j]);
              }
            }
            // curSecPtr->setId( junctionSecRoadId ++ );
            // junctionPtr->secList.push_back( curSecPtr );
            // secMap.insert( std::make_pair( fromSecIndex, junctionPtr->secList.size() - 1 ) );
          } else {
            txlog::error("Can not get section " + fromSecIndex);
          }
        }
#  ifdef KEEP_TO_SEC
        if (secMap.find(toSecIndex) == secMap.end() && linkIds[i].second != fromRId && linkIds[i].second != toRId) {
          // hadmap::txSectionPtr curSecPtr;
          hadmap::txSections sections;
          if (NodeOpt::getInstance()->getSections(linkIds[i].second, secIds[i].second, laneIds[i].second, 30,
                                                  sections)) {
            for (size_t j = 0; j < sections.size(); ++j) {
              hadmap::txSectionPtr curSecPtr = sections[j];
              curSecPtr->setId(junctionSecRoadId++);
              junctionPtr->secList.push_back(curSecPtr);
              if (j == 0) {
                secMap.insert(std::make_pair(toSecIndex, junctionPtr->secList.size() - 1));
              }
              if (j + 1 == sections.size()) {
                continue;
              } else {
                RouteUtil::buildSecRelation(sections[j], sections[j + 1]);
              }
            }
            // curSecPtr->setId( junctionSecRoadId ++ );
            // junctionPtr->secList.push_back( curSecPtr );
            // secMap.insert( std::make_pair( toSecIndex, junctionPtr->secList.size() - 1 ) );
          } else {
            txlog::error("Can not get section " + toSecIndex);
          }
        }
#  endif
        txSectionPtr fromSecPtr, toSecPtr, linkSecPtr;
        linkSecPtr = junctionPtr->secList[linkMap[RouteUtil::linkIndex(linkIds[i].first, linkIds[i].second)]];
        if (secMap.find(fromSecIndex) != secMap.end()) {
          fromSecPtr = junctionPtr->secList[secMap[fromSecIndex]];
        } else {
          /*
          if ( linkIds[i].first == fromRoadPtr->getId() ){
              fromSecPtr = fromRoadPtr->getSections().back();
          } else if ( linkIds[i].first == toRoadPtr->getId() ) {
              fromSecPtr = toRoadPtr->getSections().front();}
          */
          fromSecPtr = NULL;
          // std::cout << "   Late Build R - S " << RouteUtil::rsLinkIndex( linkIds[i].first, linkMap[
          // RouteUtil::linkIndex( linkIds[i].first, linkIds[i].second ) ], true ) << std::endl;
          junctionPtr->linkMap.insert(RouteUtil::rsLinkIndex(
              linkIds[i].first, linkMap[RouteUtil::linkIndex(linkIds[i].first, linkIds[i].second)], true));
        }
#  ifdef KEEP_TO_SEC
        if (secMap.find(toSecIndex) != secMap.end()) {
          toSecPtr = junctionPtr->secList[secMap[toSecIndex]];
        } else {
          /*
          if ( linkIds[i].second == fromRoadPtr->getId() ){
              toSecPtr = fromRoadPtr->getSections().back();
          } else if ( linkIds[i].second == toRoadPtr->getId() ) {
              toSecPtr = toRoadPtr->getSections().front();}
          */
          toSecPtr = NULL;
          // std::cout << "   Late Build S - R " << RouteUtil::rsLinkIndex( linkMap[ RouteUtil::linkIndex(
          // linkIds[i].first, linkIds[i].second ) ], linkIds[i].second, false ) << std::endl;
          junctionPtr->linkMap.insert(RouteUtil::rsLinkIndex(
              linkMap[RouteUtil::linkIndex(linkIds[i].first, linkIds[i].second)], linkIds[i].second, false));
        }
#  endif
        if (fromSecPtr != NULL) {
          RouteUtil::buildSecRelation(fromSecPtr, linkSecPtr);
        }
#  ifdef KEEP_TO_SEC
        if (toSecPtr != NULL) {
          RouteUtil::buildSecRelation(linkSecPtr, toSecPtr);
        }
#  endif
        // RouteUtil::buildSecRelation( fromSecPtr, toSecPtr, linkSecPtr );
      }
    } else {
      txlog::error("Can not find lanelinks by envelope!");
    }

    junctionPtr->setId(node.roadId);
    for (size_t i = 0; i < junctionPtr->secList.size(); ++i) {
      junctionPtr->secList[i]->setRoad(junctionPtr);
      junctionPtr->secList[i]->setRoadId(node.roadId);
    }
  }
#endif

  return junctionPtr;
}

txJunctionRoadPtr RoutePlan::getJunction(const hadmap::txRouteNode& route) {
  RoadInfo node;
  node.roadId = (int64_t)route.getId();
  node.fromRId = route.getPreId();
  node.toRId = route.getNextId();
  node.reverseFlag = route.needReverse();
  node.begin = route.getStartRange();
  node.end = route.getEndRange();
  node.attribute = route.getAttr();
  return getJunction(node);
}

bool RoutePlan::buildTopo(hadmap::txRoadPtr fromRoadPtr, hadmap::txRoadPtr toRoadPtr) {
#if 0
  if (fromRoadPtr == NULL || toRoadPtr == NULL) {
    return false;
  }
  txJunctionRoadPtr fromJuncPtr = std::dynamic_pointer_cast<txJunctionRoad>(fromRoadPtr);
  txJunctionRoadPtr toJuncPtr = std::dynamic_pointer_cast<txJunctionRoad>(toRoadPtr);
  if (fromJuncPtr == NULL && toJuncPtr == NULL) {
    fromRoadPtr->buildOuterTopo(toRoadPtr);
    return true;
  } else if (fromJuncPtr != NULL && toJuncPtr != NULL) {
    return false;
  } else if (fromJuncPtr != NULL) {
    std::unordered_set<std::string>& linkMap = fromJuncPtr->linkMap;
    // build topo between junc and road
    if (linkMap.find(RouteUtil::linkIndex(-1, toRoadPtr->getId())) != linkMap.end())
      RouteUtil::buildSecRelation(fromJuncPtr->getSections()[0], toRoadPtr->getSections().front());
    if (linkMap.find(RouteUtil::linkIndex(toRoadPtr->getId(), -1)) != linkMap.end())
      RouteUtil::buildSecRelation(toRoadPtr->getSections().front(), fromJuncPtr->getSections()[0]);

    // build topo between junc sec list and road
    for (size_t i = 0; i < fromJuncPtr->secList.size(); ++i) {
      txSectionPtr curSecPtr = fromJuncPtr->secList[i];
      if (linkMap.find(RouteUtil::linkIndex(i, toRoadPtr->getId())) != linkMap.end())
        RouteUtil::buildSecRelation(curSecPtr, toRoadPtr->getSections().front());
      if (linkMap.find(RouteUtil::linkIndex(toRoadPtr->getId(), i)) != linkMap.end())
        RouteUtil::buildSecRelation(toRoadPtr->getSections().front(), curSecPtr);
    }
    return true;
  } else if (toJuncPtr != NULL) {
    std::unordered_set<std::string>& linkMap = toJuncPtr->linkMap;
    // build topo between junc and road
    if (linkMap.find(RouteUtil::linkIndex(fromRoadPtr->getId(), -1)) != linkMap.end())
      RouteUtil::buildSecRelation(fromRoadPtr->getSections().back(), toJuncPtr->getSections()[0]);
    if (linkMap.find(RouteUtil::linkIndex(-1, fromRoadPtr->getId())) != linkMap.end())
      RouteUtil::buildSecRelation(toJuncPtr->getSections()[0], fromRoadPtr->getSections().back());

    // build topo between junc sec list and road
    for (size_t i = 0; i < toJuncPtr->secList.size(); ++i) {
      txSectionPtr curSecPtr = toJuncPtr->secList[i];
      if (linkMap.find(RouteUtil::linkIndex(fromRoadPtr->getId(), i)) != linkMap.end())
        RouteUtil::buildSecRelation(fromRoadPtr->getSections().back(), curSecPtr);
      if (linkMap.find(RouteUtil::linkIndex(i, fromRoadPtr->getId())) != linkMap.end())
        RouteUtil::buildSecRelation(curSecPtr, fromRoadPtr->getSections().back());
    }
    return true;
  } else {
    return false;
  }
#endif
  return false;
}

bool RoutePlan::getCrossWalks(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, hadmap::txObjects& objects) {
  hadmap::txLaneLinks lanelinks;
  if (NodeOpt::getInstance()->getLaneLinks(NodeId(fromRoadId), NodeId(toRoadId), lanelinks)) {
    if (NodeOpt::getInstance()->getObjects(lanelinks[0]->fromRoadId(), lanelinks[0]->fromSectionId(),
                                           lanelinks[0]->fromLaneId(), hadmap::OBJECT_TYPE_CrossWalk, objects)) {
      for (auto itr = objects.begin(); itr != objects.end();) {
        const txLineCurve* objCurve = dynamic_cast<const txLineCurve*>((*itr)->getGeom()->getGeometry());
        bool flag = false;
        for (size_t i = 0; i < lanelinks.size(); ++i) {
          const txLineCurve* linkCurve = dynamic_cast<const txLineCurve*>(lanelinks[i]->getGeometry());
          if (RouteUtil::hasIntersection(linkCurve, objCurve)) {
            flag = true;
            break;
          }
        }
        if (flag) {
          itr++;
        } else {
          itr = objects.erase(itr);
        }
      }
      if (objects.empty()) {
        return true;
      } else if (objects.size() == 1) {
        return true;
      } else {
        hadmap::txPoint linkFrontP(0.0, 0.0, 0.0);
        for (size_t i = 0; i < lanelinks.size(); ++i) {
          hadmap::txPoint p = lanelinks[i]->getGeometry()->getStart();
          linkFrontP.x += p.x;
          linkFrontP.y += p.y;
          linkFrontP.z += p.z;
        }
        linkFrontP.x /= lanelinks.size();
        linkFrontP.y /= lanelinks.size();
        linkFrontP.z /= lanelinks.size();

        hadmap::txPoint fObjP(0.0, 0.0, 0.0), sObjP(0.0, 0.0, 0.0);
        const txLineCurve* fObjGeom = dynamic_cast<const txLineCurve*>(objects.front()->getGeom()->getGeometry());
        const txLineCurve* sObjGeom = dynamic_cast<const txLineCurve*>(objects.back()->getGeom()->getGeometry());
        for (size_t i = 0, len = fObjGeom->size(); i < len; ++i) {
          hadmap::txPoint p = fObjGeom->getPoint(i);
          fObjP.x += p.x;
          fObjP.y += p.y;
          fObjP.z += p.z;
        }
        fObjP.x /= fObjGeom->size();
        fObjP.y /= fObjGeom->size();
        fObjP.z /= fObjGeom->size();

        for (size_t i = 0, len = sObjGeom->size(); i < len; ++i) {
          hadmap::txPoint p = sObjGeom->getPoint(i);
          sObjP.x += p.x;
          sObjP.y += p.y;
          sObjP.z += p.z;
        }
        sObjP.x /= sObjGeom->size();
        sObjP.y /= sObjGeom->size();
        sObjP.z /= sObjGeom->size();

        if (RouteUtil::pointsLonLatDis(linkFrontP, fObjP) > RouteUtil::pointsLonLatDis(linkFrontP, sObjP)) {
          std::swap(objects[0], objects[1]);
        }
        return true;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool RoutePlan::getStopLine(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, hadmap::txObjects& objects) {
  hadmap::txLaneLinks lanelinks;
  if (NodeOpt::getInstance()->getLaneLinks(NodeId(fromRoadId), NodeId(toRoadId), lanelinks)) {
    return NodeOpt::getInstance()->getObjects(lanelinks[0]->fromRoadId(), lanelinks[0]->fromSectionId(),
                                              lanelinks[0]->fromLaneId(), hadmap::OBJECT_TYPE_Stop, objects);
  } else {
    return false;
  }
}
}  // namespace planner
