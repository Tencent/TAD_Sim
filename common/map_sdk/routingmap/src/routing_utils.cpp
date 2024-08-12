// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/routing_utils.h"
#include <cassert>

#include "common/coord_trans.h"
#include "structs/hadmap_curve.h"

namespace hadmap {

#ifndef Deg2Rad
#  define Deg2Rad 0.01745329251994329576923690768  // (PI / 180.0)
#endif
#ifndef COSINE(V0, V1)
#  define DotProduct(V0, V1) (V0.x * V1.x + V0.y * V1.y + V0.z * V1.z)
#  define LengthProduct(V0) (V0.x * V0.x + V0.y * V0.y + V0.z * V0.z)
#  define COSINE(V0, V1) DotProduct(V0, V1) / (LengthProduct(V0) * LengthProduct(V1))
#endif
std::string RoutingUtils::reverseRoadIndex(const roadpkid& rid) {
  char index[128];
  sprintf(index, "%s_1", std::to_string(rid).c_str());
  return std::string(index);
}

roadpkid RoutingUtils::reverseRoadIndex2RId(const std::string& rIndex) {
  std::string ridStr = rIndex.substr(0, rIndex.length() - 2);
  return std::stoull(ridStr);
}

std::string RoutingUtils::linkIndex(const roadpkid& fromRId, const roadpkid& toRId) {
  char index[128];
  sprintf(index, "%s_%s", std::to_string(fromRId).c_str(), std::to_string(toRId).c_str());
  return std::string(index);
}

std::string RoutingUtils::linkIndex(const txLaneId& fromLaneId, const txLaneId& toLaneId) {
  char index[256];
  sprintf(index, "%s.%s.%s_%s.%s.%s", std::to_string(fromLaneId.roadId).c_str(),
          std::to_string(fromLaneId.sectionId).c_str(), std::to_string(fromLaneId.laneId).c_str(),
          std::to_string(toLaneId.roadId).c_str(), std::to_string(toLaneId.sectionId).c_str(),
          std::to_string(toLaneId.laneId).c_str());
  return std::string(index);
}

double RoutingUtils::pointsDistance(const txPoint& p0, const txPoint& p1) {
  double square_distance = std::pow(p0.x - p1.x, 2) + std::pow(p0.y - p1.y, 2) + std::pow(p0.z - p1.z, 2);
  return std::sqrt(square_distance);
}

double RoutingUtils::pointsDisWGS84(const txPoint& p0, const txPoint& p1) {
  txPoint globalP0(p0), globalP1(p1);

  // ignore altitude
  globalP0.z = 0.0;
  globalP1.z = 0.0;

  coord_trans_api::lonlat2global(globalP0.x, globalP0.y, globalP0.z);
  coord_trans_api::lonlat2global(globalP1.x, globalP1.y, globalP1.z);
  return RoutingUtils::pointsDistance(globalP0, globalP1);
}

bool RoutingUtils::createOffsetGeom(const PointVec& originGeom, double startOffset, double endOffset, PointVec& geom) {
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
        double len = RoutingUtils::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
        ver.x /= len;
        ver.y /= len;
      } else if (i == originGeom.size() - 1) {
        hadmap::txPoint fromP(originGeom[i - 1]);
        coord_trans_api::lonlat2local(fromP.x, fromP.y, fromP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        ver.x = -fromP.y;
        ver.y = fromP.x;
        ver.z = 0.0;
        double len = RoutingUtils::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
        ver.x /= len;
        ver.y /= len;
      } else {
        hadmap::txPoint fromP(originGeom[i - 1]);
        hadmap::txPoint toP(originGeom[i + 1]);
        coord_trans_api::lonlat2local(fromP.x, fromP.y, fromP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        coord_trans_api::lonlat2local(toP.x, toP.y, toP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
        double fromLen = RoutingUtils::pointsDistance(fromP, hadmap::txPoint(0.0, 0.0, 0.0));
        double toLen = RoutingUtils::pointsDistance(toP, hadmap::txPoint(0.0, 0.0, 0.0));
        fromP.x /= -fromLen;
        fromP.y /= -fromLen;
        fromP.z /= -fromLen;
        toP.x /= toLen;
        toP.y /= toLen;
        toP.z /= toLen;
        ver.x = fromP.y + toP.y;
        ver.y = -(fromP.x + toP.x);
        ver.z = 0.0;
        double len = RoutingUtils::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
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
      double frontLen = RoutingUtils::pointsDistance(frontDir, hadmap::txPoint(0.0, 0.0, 0.0));
      double nextLen = RoutingUtils::pointsDistance(nextDir, hadmap::txPoint(0.0, 0.0, 0.0));
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

bool RoutingUtils::geomConnectDirectly(const txCurve* fromCurvePtr, const txCurve* toCurvePtr) {
  if (fromCurvePtr == NULL || fromCurvePtr->empty() || toCurvePtr == NULL || toCurvePtr->empty()) return false;

  assert(fromCurvePtr->getCoordType() == toCurvePtr->getCoordType());

  const double DIS_T = 0.3;
  if (fromCurvePtr->getCoordType() == COORD_WGS84) {
    return pointsDisWGS84(fromCurvePtr->getEnd(), toCurvePtr->getStart()) < DIS_T;
  } else {
    return pointsDistance(fromCurvePtr->getEnd(), toCurvePtr->getStart()) < DIS_T;
  }
}

DIR_TYPE RoutingUtils::turnDir(const txCurve* curP) {
  const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(curP);
  double turnAngle;
  if (curvePtr == NULL || curvePtr->empty() || curvePtr->size() <= 2) {
    turnAngle = 0.0;
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
      turnAngle = angle;
    } else {
      turnAngle = -angle;
    }
  }

  const double angleOffset = 15.0;
  const double uturnAngle = 150.0;
  if (fabs(turnAngle) > uturnAngle) {
    return DIR_Uturn;
  } else if (turnAngle < -angleOffset) {
    return DIR_Left;
  } else if (-angleOffset <= turnAngle && turnAngle <= angleOffset) {
    return DIR_Straight;
  } else {
    return DIR_Right;
  }
}
}  // namespace hadmap
