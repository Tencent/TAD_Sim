// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routeplan/route_utils.h"
#include <float.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <unordered_map>
#include <vector>

#include "common/coord_trans.h"
#include "routeplan/node_opt.h"

using namespace hadmap;

namespace planner {

#ifndef Deg2Rad
#  define Deg2Rad 0.01745329251994329576923690768  // (PI / 180.0)
#endif

#ifndef COSINE(V0, V1)
#  define DotProduct(V0, V1) (V0.x * V1.x + V0.y * V1.y + V0.z * V1.z)
#  define LengthProduct(V0) (V0.x * V0.x + V0.y * V0.y + V0.z * V0.z)
#  define COSINE(V0, V1) DotProduct(V0, V1) / (LengthProduct(V0) * LengthProduct(V1))
#endif
double RouteUtil::pointsDistance(const txPoint& p0, const txPoint& p1) {
  double square_distance = std::pow(p0.x - p1.x, 2) + std::pow(p0.y - p1.y, 2) + std::pow(p0.z - p1.z, 2);
  return std::sqrt(square_distance);
}

double RouteUtil::pointsLonLatDis(const txPoint& p0, const txPoint& p1) {
  txPoint globalP0(p0), globalP1(p1);
  coord_trans_api::lonlat2global(globalP0.x, globalP0.y, globalP0.z);
  coord_trans_api::lonlat2global(globalP1.x, globalP1.y, globalP1.z);
  return RouteUtil::pointsDistance(globalP0, globalP1);
}

int RouteUtil::nearestPoint(const txLineCurve* curvePtr, const txPoint& point) {
  if (curvePtr == NULL) return 0;

  txPoint globalP(point);
  globalP.z = 0.0;
  coord_trans_api::lonlat2global(globalP.x, globalP.y, globalP.z);

  int near = 0;
  double nearestDis = std::numeric_limits<double>::max();
  for (size_t i = 0; i < curvePtr->size(); ++i) {
    txPoint curveP(curvePtr->getPoint(i));
    curveP.z = 0.0;
    coord_trans_api::lonlat2global(curveP.x, curveP.y, curveP.z);

    double curDis = pointsDistance(globalP, curveP);
    if (nearestDis > curDis) {
      near = i;
      nearestDis = curDis;
    }
  }

  return near;
}

double RouteUtil::angleWithDir(const txLineCurve* curvePtr) {
  if (curvePtr == NULL || curvePtr->empty() || curvePtr->size() <= 2) {
    return 0.0;
  } else {
    size_t pointSize = curvePtr->size();
    txPoint curRoadFront(curvePtr->getStart()), curRoadBack(curvePtr->getPoint(size_t(1))),
        nextRoadFront(curvePtr->getPoint(size_t(pointSize - 2))), nextRoadBack(curvePtr->getEnd());

    double lonlatLoc[] = {(curRoadFront.x + nextRoadBack.x) / 2, (curRoadFront.y + nextRoadBack.y) / 2,
                          (curRoadFront.z + nextRoadBack.z) / 2};

    coord_trans_api::lonlat2local(curRoadFront.x, curRoadFront.y, curRoadFront.z, lonlatLoc[0], lonlatLoc[1],
                                  lonlatLoc[2]);

    coord_trans_api::lonlat2local(curRoadBack.x, curRoadBack.y, curRoadBack.z, lonlatLoc[0], lonlatLoc[1],
                                  lonlatLoc[2]);

    coord_trans_api::lonlat2local(nextRoadFront.x, nextRoadFront.y, nextRoadFront.z, lonlatLoc[0], lonlatLoc[1],
                                  lonlatLoc[2]);

    coord_trans_api::lonlat2local(nextRoadBack.x, nextRoadBack.y, nextRoadBack.z, lonlatLoc[0], lonlatLoc[1],
                                  lonlatLoc[2]);

    txPoint curDir(curRoadBack.x - curRoadFront.x, curRoadBack.y - curRoadFront.y, 0.0),
        nextDir(nextRoadBack.x - nextRoadFront.x, nextRoadBack.y - nextRoadFront.y, 0.0);
    // std::cout << "Dir " << curDir.x << " , " << curDir.y << std::endl;
    double cosine = COSINE(curDir, nextDir);
    double angle = acos(cosine) / Deg2Rad;

    double z = nextDir.x * curDir.y - nextDir.y * curDir.x;
    if (z > 0) {
      return angle;
    } else {
      return -angle;
    }
  }
}

int RouteUtil::nearestSectionIndex(const txRoadPtr& roadPtr, const double& prog) {
  const txSections& sections = roadPtr->getSections();
  if (sections.empty()) return -1;
  double len = 0.0;
  for (size_t i = 0; i < sections.size(); ++i) len += sections[i]->getLength();
  double secLen = 0.0;
  for (size_t i = 0; i < sections.size(); ++i) {
    secLen += sections[i]->getLength();
    if (prog < secLen / len) return i;
  }
  return sections.size() - 1;
}

void RouteUtil::copyRoadBaseInfo(const txRoadPtr& oriPtr, txRoadPtr& dstPtr) {
  if (!oriPtr) throw std::runtime_error("copyRoadBaseInfo Error, oriPtr Empty");
  if (!dstPtr) dstPtr.reset(new txRoad);

  // base info
  (*dstPtr).setId(oriPtr->getId()).setName(oriPtr->getName());

  // road info
  (*dstPtr)
      .setGeometry(oriPtr->getGeometry())
      .setGround(oriPtr->getGround())
      .setRoadType(oriPtr->getRoadType())
      .setDirection(oriPtr->getDirection())
      .setToll(oriPtr->getToll())
      .setFuncClass(oriPtr->getFuncClass())
      .setUrban(oriPtr->isUrban())
      .setPaved(oriPtr->isPaved())
      .setTransType(oriPtr->getTransType())
      .setLaneMarking(oriPtr->hasLaneMarking())
      .setMaterial(oriPtr->getMaterial())
      .setTaskId(oriPtr->getTaskId());
}
bool compare2set(std::set<hadmap::lanepkid> left, std::set<hadmap::lanepkid> right) {
  for (auto it : left) {
    if (right.find(it) == right.end()) {
      return true;
    }
  }
  for (auto it : right) {
    if (left.find(it) == left.end()) {
      return true;
    }
  }
  return false;
}
std::vector<size_t> RouteUtil::minDisLaneLinks(const std::set<hadmap::lanepkid>& fromLaneIds,
                                               std::set<hadmap::lanepkid>& usedFromLaneIds,
                                               const std::set<hadmap::lanepkid>& toLaneIds,
                                               std::set<hadmap::lanepkid>& usedToLaneIds,
                                               const hadmap::txLaneLinks& lanelinks, std::set<size_t>& usedLinkIndex) {
  std::cout << "fromLaneIds: ";
  std::set<hadmap::lanepkid> tmp1 = usedFromLaneIds;
  std::set<hadmap::lanepkid> tmp2 = usedToLaneIds;

  for (auto it : fromLaneIds) {
    std::cout << " " << it;
  }
  std::cout << "usedFromLaneIds: ";

  for (auto it : usedFromLaneIds) {
    std::cout << " " << it;
  }

  std::cout << "toLaneIds: ";

  for (auto it : toLaneIds) {
    std::cout << " " << it;
  }

  std::cout << "usedToLaneIds: ";

  for (auto it : usedToLaneIds) {
    std::cout << " " << it;
  }

  std::cout << std::endl;
  bool complete = true;
  for (auto itr = fromLaneIds.begin(); itr != fromLaneIds.end(); ++itr)
    if (usedFromLaneIds.find(*itr) == usedFromLaneIds.end()) complete = false;
  for (auto itr = toLaneIds.begin(); itr != toLaneIds.end(); ++itr)
    if (usedToLaneIds.find(*itr) == usedToLaneIds.end()) complete = false;
  if (complete) {
    return std::vector<size_t>();
  } else {
    double dis = DBL_MAX;
    size_t minIndex = 0;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      if (usedLinkIndex.find(i) != usedLinkIndex.end()) continue;
      hadmap::lanepkid fromId = lanelinks[i]->fromLaneId();
      hadmap::lanepkid toId = lanelinks[i]->toLaneId();
      if (fromLaneIds.find(fromId) == fromLaneIds.end() || toLaneIds.find(toId) == toLaneIds.end()) continue;
      if (dis > lanelinks[i]->getGeometry()->getLength()) {
        dis = lanelinks[i]->getGeometry()->getLength();
        minIndex = i;
      }
    }
    hadmap::lanepkid fromId = lanelinks[minIndex]->fromLaneId();
    hadmap::lanepkid toId = lanelinks[minIndex]->toLaneId();
    std::set<hadmap::lanepkid> leftFromLaneIds, leftToLaneIds, rightFromLaneIds, rightToLaneIds;
    leftFromLaneIds.insert(fromId);
    leftToLaneIds.insert(toId);
    rightFromLaneIds.insert(fromId);
    rightToLaneIds.insert(toId);
    for (auto itr = fromLaneIds.begin(); itr != fromLaneIds.end(); ++itr) {
      if (*itr > 0) {
        if (*itr < fromId) {
          leftFromLaneIds.insert(*itr);
        } else {
          rightFromLaneIds.insert(*itr);
        }
      }
      if (*itr < 0) {
        if (*itr > fromId) {
          leftFromLaneIds.insert(*itr);
        } else {
          rightFromLaneIds.insert(*itr);
        }
      }
    }
    for (auto itr = toLaneIds.begin(); itr != toLaneIds.end(); ++itr) {
      if (*itr > 0) {
        if (*itr < toId) {
          leftToLaneIds.insert(*itr);
        } else {
          rightToLaneIds.insert(*itr);
        }
      }
      if (*itr < 0) {
        if (*itr > toId) {
          leftToLaneIds.insert(*itr);
        } else {
          rightToLaneIds.insert(*itr);
        }
      }
    }
    usedFromLaneIds.insert(fromId);
    usedToLaneIds.insert(toId);
    usedLinkIndex.insert(minIndex);
    std::cout << "left: ";
    std::vector<size_t> leftIndex;

    if (!compare2set(leftFromLaneIds, fromLaneIds) && !compare2set(usedFromLaneIds, tmp1) &&
        !compare2set(leftToLaneIds, toLaneIds) && !compare2set(usedToLaneIds, tmp2)) {
      int a = 0;
    } else {
      leftIndex =
          minDisLaneLinks(leftFromLaneIds, usedFromLaneIds, leftToLaneIds, usedToLaneIds, lanelinks, usedLinkIndex);
    }

    std::cout << "right: ";
    std::vector<size_t> rightIndex;
    if (!compare2set(rightFromLaneIds, fromLaneIds) && !compare2set(usedFromLaneIds, tmp1) &&
        !compare2set(rightToLaneIds, toLaneIds) && !compare2set(usedToLaneIds, tmp2)) {
      int a = 0;
    } else {
      rightIndex =
          minDisLaneLinks(rightFromLaneIds, usedFromLaneIds, rightToLaneIds, usedToLaneIds, lanelinks, usedLinkIndex);
    }
    leftIndex.push_back(minIndex);
    leftIndex.insert(leftIndex.end(), rightIndex.begin(), rightIndex.end());
    return leftIndex;
  }
}

void RouteUtil::getJunctionLinks(const hadmap::txPoint& center, hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId,
                                 std::vector<std::pair<hadmap::roadpkid, hadmap::roadpkid> >& linkIds,
                                 std::vector<std::pair<hadmap::sectionpkid, hadmap::sectionpkid> >& secIds,
                                 std::vector<std::pair<hadmap::lanepkid, hadmap::lanepkid> >& laneIds) {
  txLaneLinks lanelinks;
  if (NodeOpt::getInstance()->getLaneLinks(center, 15.0, lanelinks)) {
    std::set<std::string> fromIndex;
    std::set<std::string> toIndex;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      hadmap::roadpkid fromRId = lanelinks[i]->fromRoadId();
      hadmap::sectionpkid fromSecId = lanelinks[i]->fromSectionId();
      hadmap::lanepkid fromLaneId = lanelinks[i]->fromLaneId() < 0 ? -1 : 1;
      hadmap::roadpkid toRId = lanelinks[i]->toRoadId();
      hadmap::sectionpkid toSecId = lanelinks[i]->toSectionId();
      hadmap::lanepkid toLaneId = lanelinks[i]->toLaneId() < 0 ? -1 : 1;
      if (fromRId == fromRoadId && toRId == toRoadId) {
        char index[64];
        sprintf(index, "%s,%s,%s", std::to_string(fromRId).c_str(), std::to_string(fromSecId).c_str(),
                std::to_string(fromLaneId).c_str());
        fromIndex.insert(index);

        sprintf(index, "%s,%s,%s", std::to_string(toRId).c_str(), std::to_string(toSecId).c_str(),
                std::to_string(toLaneId).c_str());
        toIndex.insert(index);
      }
    }
    size_t fromSize = fromIndex.size();
    size_t toSize = toIndex.size();
    do {
      fromSize = fromIndex.size();
      toSize = toIndex.size();
      for (size_t i = 0; i < lanelinks.size(); ++i) {
        hadmap::roadpkid fromRId = lanelinks[i]->fromRoadId();
        hadmap::sectionpkid fromSecId = lanelinks[i]->fromSectionId();
        hadmap::lanepkid fromLaneId = lanelinks[i]->fromLaneId() < 0 ? -1 : 1;
        hadmap::roadpkid toRId = lanelinks[i]->toRoadId();
        hadmap::sectionpkid toSecId = lanelinks[i]->toSectionId();
        hadmap::lanepkid toLaneId = lanelinks[i]->toLaneId() < 0 ? -1 : 1;
        char index[64];
        sprintf(index, "%s,%s,%s", std::to_string(fromRId).c_str(), std::to_string(fromSecId).c_str(),
                std::to_string(fromLaneId).c_str());
        std::string curFromIndex(index);
        sprintf(index, "%s,%s,%s", std::to_string(fromRId).c_str(), std::to_string(fromSecId).c_str(),
                std::to_string(fromLaneId * -1).c_str());
        std::string curNegFromIndex(index);

        sprintf(index, "%s,%s,%s", std::to_string(toRId).c_str(), std::to_string(toSecId).c_str(),
                std::to_string(toLaneId).c_str());
        std::string curToIndex(index);
        sprintf(index, "%s,%s,%s", std::to_string(toRId).c_str(), std::to_string(toSecId).c_str(),
                std::to_string(toLaneId * -1).c_str());
        std::string curNegToIndex(index);

        if (fromIndex.find(curFromIndex) != fromIndex.end()) {
          toIndex.insert(curToIndex);
          toIndex.insert(curNegFromIndex);
          fromIndex.insert(curNegToIndex);
        } else if (toIndex.find(curToIndex) != toIndex.end()) {
          fromIndex.insert(curFromIndex);
          fromIndex.insert(curNegToIndex);
          toIndex.insert(curNegFromIndex);
        }
      }
    } while (fromSize != fromIndex.size() || toSize != toIndex.size());

    std::set<std::string> usedLinks;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      hadmap::roadpkid fromRId = lanelinks[i]->fromRoadId();
      hadmap::sectionpkid fromSecId = lanelinks[i]->fromSectionId();
      hadmap::lanepkid fromLaneId = lanelinks[i]->fromLaneId() < 0 ? -1 : 1;
      hadmap::roadpkid toRId = lanelinks[i]->toRoadId();
      hadmap::sectionpkid toSecId = lanelinks[i]->toSectionId();
      hadmap::lanepkid toLaneId = lanelinks[i]->toLaneId() < 0 ? -1 : 1;
      char index[64];
      sprintf(index, "%s,%s,%s", std::to_string(fromRId).c_str(), std::to_string(fromSecId).c_str(),
              std::to_string(fromLaneId).c_str());
      std::string curFromIndex(index);

      sprintf(index, "%s,%s,%s", std::to_string(toRId).c_str(), std::to_string(toSecId).c_str(),
              std::to_string(toLaneId).c_str());
      std::string curToIndex(index);

      std::string curLinkIndex = RouteUtil::linkIndex(fromRId, toRId);
      if (usedLinks.find(curLinkIndex) != usedLinks.end()) {
        continue;
      } else {
        usedLinks.insert(curLinkIndex);
      }
      if (fromIndex.find(curFromIndex) != fromIndex.end()) {
        linkIds.push_back(std::make_pair(fromRId, toRId));
        secIds.push_back(std::make_pair(fromSecId, toSecId));
        laneIds.push_back(std::make_pair(fromLaneId, toLaneId));
      }
    }
  }
}

std::string RouteUtil::linkIndex(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId) {
  char index[32];
  sprintf(index, "%s_%s", std::to_string(fromRoadId).c_str(), std::to_string(toRoadId).c_str());
  return std::string(index);
}

std::string RouteUtil::rsLinkIndex(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, bool road2sec) {
  char index[64];
  if (road2sec) {
    sprintf(index, "R%s_S%s", std::to_string(fromRoadId).c_str(), std::to_string(toRoadId).c_str());
  } else {
    sprintf(index, "S%s_R%s", std::to_string(fromRoadId).c_str(), std::to_string(toRoadId).c_str());
  }
  return std::string(index);
}

std::string RouteUtil::secIndex(hadmap::roadpkid roadId, hadmap::sectionpkid secId) {
  char index[32];
  sprintf(index, "%s,%s", std::to_string(roadId).c_str(), std::to_string(secId).c_str());
  return std::string(index);
}

std::string RouteUtil::laneIndex(hadmap::roadpkid roadId, hadmap::sectionpkid secId, hadmap::lanepkid laneId) {
  char index[64];
  sprintf(index, "%s,%s,%s", std::to_string(roadId).c_str(), std::to_string(secId).c_str(),
          std::to_string(laneId).c_str());
  return std::string(index);
}

bool RouteUtil::buildSecRelation(hadmap::txSectionPtr& fromSecPtr, hadmap::txSectionPtr& toSecPtr) {
  const txLanes& fromSecLanes = fromSecPtr->getLanes();
  const txLanes& toSecLanes = toSecPtr->getLanes();
  for (size_t i = 0; i < fromSecLanes.size(); ++i) {
    if (fromSecLanes[i]->getGeometry() == NULL || fromSecLanes[i]->getGeometry()->empty()) continue;
    for (size_t j = 0; j < toSecLanes.size(); ++j) {
      if (toSecLanes[j]->getGeometry() == NULL || toSecLanes[j]->getGeometry()->empty()) continue;
      txPoint fromLaneP = fromSecLanes[i]->getGeometry()->getEnd();
      txPoint toLaneP = toSecLanes[j]->getGeometry()->getStart();
      if (fromSecLanes[i]->getId() > 0) fromLaneP = fromSecLanes[i]->getGeometry()->getStart();
      if (toSecLanes[j]->getId() > 0) toLaneP = toSecLanes[j]->getGeometry()->getEnd();
      if (RouteUtil::pointsLonLatDis(fromLaneP, toLaneP) < 0.15) {
        fromSecLanes[i]->addNext(toSecLanes[j]->getUniqueId());
        toSecLanes[j]->addPrev(fromSecLanes[i]->getUniqueId());
      }
    }
  }
  return true;
}
/*
bool RouteUtil::buildSecRelation( hadmap::txSectionPtr& fromSecPtr, \
        hadmap::txSectionPtr& toSecPtr, hadmap::txSectionPtr& linkSecPtr )
{
    const txLanes& fromSecLanes = fromSecPtr->getLanes();
    const txLanes& toSecLanes = toSecPtr->getLanes();
    const txLanes& linkSecLanes = linkSecPtr->getLanes();
    for ( size_t i = 0; i < fromSecLanes.size(); ++ i )
    {
        for ( size_t j = 0; j < linkSecLanes.size(); ++ j )
        {
            txPoint fromLaneP = fromSecLanes[i]->getGeometry()->getEnd();
            txPoint toLinkLaneP = linkSecLanes[j]->getGeometry()->getStart();
            if ( fromSecLanes[i]->getId() > 0 ){
                fromLaneP = fromSecLanes[i]->getGeometry()->getStart();}
            if ( linkSecLanes[j]->getId() > 0 ){
                toLinkLaneP = linkSecLanes[j]->getGeometry()->getEnd();}
            if ( RouteUtil::pointsLonLatDis( fromLaneP, toLinkLaneP ) < 0.15 )
            {
                fromSecLanes[i]->addNext( linkSecLanes[j] );
                linkSecLanes[j]->addPrev( fromSecLanes[i] );
            }
        }
    }
    for ( size_t i = 0; i < toSecLanes.size(); ++ i )
    {
        for ( size_t j = 0; j < linkSecLanes.size(); ++ j )
        {
            txPoint toLaneP = toSecLanes[i]->getGeometry()->getStart();
            txPoint fromLinkLaneP = linkSecLanes[j]->getGeometry()->getEnd();
            if ( toSecLanes[i]->getId() > 0 ){
                toLaneP = toSecLanes[i]->getGeometry()->getEnd();}
            if ( linkSecLanes[j]->getId() > 0 ){
                fromLinkLaneP = linkSecLanes[j]->getGeometry()->getStart();}
            if ( RouteUtil::pointsLonLatDis( toLaneP, fromLinkLaneP ) < 0.15 )
            {
                linkSecLanes[j]->addNext( toSecLanes[i] );
                toSecLanes[i]->addPrev( linkSecLanes[j] );
            }
        }
    }
    return true;
}
*/

bool RouteUtil::createOffsetGeom(const hadmap::PointVec& originGeom, double startOffset, double endOffset,
                                 hadmap::PointVec& geom) {
  geom.clear();
  if (originGeom.size() <= 1) {
    return false;
  } else {
    double offset = (endOffset - startOffset) / (originGeom.size() - 1);
    for (size_t i = 0; i < originGeom.size(); ++i) {
      hadmap::txPoint ver;
      if (i == 0) {
        hadmap::txPoint toP(originGeom[i + 1]);
        coord_trans_api::lonlat2local(toP.x, toP.y, toP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        ver.x = toP.y;
        ver.y = -toP.x;
        ver.z = 0.0;
        double len = RouteUtil::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
        ver.x /= len;
        ver.y /= len;
      } else if (i == originGeom.size() - 1) {
        hadmap::txPoint fromP(originGeom[i - 1]);
        coord_trans_api::lonlat2local(fromP.x, fromP.y, fromP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        ver.x = -fromP.y;
        ver.y = fromP.x;
        ver.z = 0.0;
        double len = RouteUtil::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
        ver.x /= len;
        ver.y /= len;
      } else {
        hadmap::txPoint fromP(originGeom[i - 1]);
        hadmap::txPoint toP(originGeom[i + 1]);
        coord_trans_api::lonlat2local(fromP.x, fromP.y, fromP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        coord_trans_api::lonlat2local(toP.x, toP.y, toP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        double fromLen = RouteUtil::pointsDistance(fromP, hadmap::txPoint(0.0, 0.0, 0.0));
        double toLen = RouteUtil::pointsDistance(toP, hadmap::txPoint(0.0, 0.0, 0.0));
        fromP.x /= -fromLen;
        fromP.y /= -fromLen;
        fromP.z /= -fromLen;
        toP.x /= toLen;
        toP.y /= toLen;
        toP.z /= toLen;
        ver.x = fromP.y + toP.y;
        ver.y = -(fromP.x + toP.x);
        ver.z = 0.0;
        double len = RouteUtil::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
        ver.x /= len;
        ver.y /= len;
        ver.z /= len;
      }
      hadmap::txPoint tagP;
      double curOffset = startOffset + i * offset;
      tagP.x = ver.x * curOffset;
      tagP.y = ver.y * curOffset;
      tagP.z = ver.z * curOffset;
      coord_trans_api::local2lonlat(tagP.x, tagP.y, tagP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
      geom.push_back(tagP);
    }
    if (geom.size() == 2) return true;
    for (size_t i = 0; i < geom.size(); ++i) coord_trans_api::lonlat2global(geom[i].x, geom[i].y, geom[i].z);
    for (auto frontItr = geom.begin(), curItr = geom.begin() + 1; curItr != geom.end();) {
      auto nextItr = curItr + 1;
      if (nextItr == geom.end()) break;
      hadmap::txPoint frontDir(curItr->x - frontItr->x, curItr->y - frontItr->y, curItr->z - frontItr->z);
      hadmap::txPoint nextDir(nextItr->x - curItr->x, nextItr->y - curItr->y, nextItr->z - curItr->z);
      double frontLen = RouteUtil::pointsDistance(frontDir, hadmap::txPoint(0.0, 0.0, 0.0));
      double nextLen = RouteUtil::pointsDistance(nextDir, hadmap::txPoint(0.0, 0.0, 0.0));
      double cosv = (frontDir.x * nextDir.x + frontDir.y * nextDir.y + frontDir.z * nextDir.z) / (frontLen * nextLen);
      if (fabs(cosv - 1.0) < 1e-5) cosv = 1.0;
      double arccos = acos(cosv);
      if (arccos < 3.1415926 / 2) {
        frontItr++;
        curItr++;
      } else {
        curItr = geom.erase(curItr);
      }
    }
    for (size_t i = 0; i < geom.size(); ++i) coord_trans_api::global2lonlat(geom[i].x, geom[i].y, geom[i].z);
    return true;
  }
}

bool RouteUtil::cutSectionGeom(const hadmap::txPoint& startPoint, const hadmap::txPoint& endPoint,
                               hadmap::txSectionPtr secPtr) {
  size_t startIndex, endIndex;
  startIndex = 999999;
  endIndex = 0;
  txLanes& lanes = secPtr->getLanes();
  std::unordered_map<hadmap::laneboundarypkid, hadmap::txLaneBoundaryPtr> boundaryMap;
  for (size_t i = 0; i < lanes.size(); ++i) {
    if (boundaryMap.find(lanes[i]->getLeftBoundaryId()) == boundaryMap.end()) {
      boundaryMap.insert(std::make_pair(lanes[i]->getLeftBoundaryId(), lanes[i]->getLeftBoundary()));
    }
    if (boundaryMap.find(lanes[i]->getRightBoundaryId()) == boundaryMap.end()) {
      boundaryMap.insert(std::make_pair(lanes[i]->getRightBoundaryId(), lanes[i]->getRightBoundary()));
    }
    if (lanes[i]->getGeometry() == NULL || lanes[i]->getGeometry()->empty()) continue;
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(lanes[i]->getGeometry());
    size_t curStartI = RouteUtil::nearestPoint(curvePtr, startPoint);
    size_t curEndI = RouteUtil::nearestPoint(curvePtr, endPoint);
    if (curStartI < startIndex) startIndex = curStartI;
    if (curEndI > endIndex) endIndex = curEndI;
  }
  if (startIndex >= endIndex) {
    return false;
  } else {
    for (size_t i = 0; i < lanes.size(); ++i) {
      if (lanes[i]->getGeometry() == NULL || lanes[i]->getGeometry()->empty()) continue;
      const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(lanes[i]->getGeometry());
      hadmap::PointVec points;
      curvePtr->getPoints(points);
      hadmap::PointVec cutPoints(points.begin() + startIndex, points.begin() + endIndex + 1);
      lanes[i]->setGeometry(cutPoints, COORD_WGS84);
    }

    for (auto itr = boundaryMap.begin(); itr != boundaryMap.end(); ++itr) {
      const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(itr->second->getGeometry());
      hadmap::PointVec points;
      curvePtr->getPoints(points);
      hadmap::PointVec cutPoints(points.begin() + startIndex, points.begin() + endIndex + 1);
      itr->second->setGeometry(cutPoints, COORD_WGS84);
    }
    return true;
  }
}

bool RouteUtil::isPointInPolygon(const hadmap::txPoint& point, const hadmap::PointVec& polygonGeoms) {
  if (polygonGeoms.size() < 3) return false;
  hadmap::PointVec localGeoms(polygonGeoms.begin(), polygonGeoms.end());
  for (size_t i = 0; i < localGeoms.size(); ++i) {
    coord_trans_api::lonlat2local(localGeoms[i].x, localGeoms[i].y, localGeoms[i].z, point.x, point.y, point.z);
  }
  double degree = 0.0;
  for (size_t i = 0; i < localGeoms.size(); ++i) localGeoms[i].z = 0.0;
  for (size_t i = 0; i < localGeoms.size(); ++i) {
    size_t j = i + 1 == localGeoms.size() ? 0 : i + 1;
    double cosine = COSINE(localGeoms[i], localGeoms[j]);
    double angle = acos(cosine) / Deg2Rad;
    degree += angle;
  }
  if (fabs(degree - 360.0) < 5.0) {
    return true;
  } else {
    return false;
  }
}

bool RouteUtil::hasIntersection(const hadmap::txLineCurve* linePtr, const hadmap::txLineCurve* polygonPtr) {
  if (linePtr == NULL || linePtr->empty() || polygonPtr == NULL || polygonPtr == NULL) return false;
  PointVec lineGeoms, polygonGeoms;
  linePtr->sample(0.75, lineGeoms);
  polygonPtr->getPoints(polygonGeoms);
  for (size_t i = 0; i < lineGeoms.size(); ++i) {
    if (RouteUtil::isPointInPolygon(lineGeoms[i], polygonGeoms)) return true;
  }
  return false;
}

}  // namespace planner
