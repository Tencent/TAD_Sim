// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_map.h"
#include "common/coord_trans.h"
#include "common/log.h"
#include "common/map_util.h"
#include "spatialindex/hadmap_spatialindex.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_header.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_object.h"
#include "structs/map_structs.h"

#include <float.h>
#include <math.h>
#include <algorithm>
#include <ctime>
#include <deque>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// #define DATAMODEL_CLIENT

namespace hadmap {
// static const double METER_TO_DEG = 1.0 / 111000.0;

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

#define VALID_Z -9999.0

template <typename d2gDT, typename d2gGT, typename g2dGT, typename g2dDT>
class DataGeomIdEx {
 private:
  std::unordered_map<d2gDT, d2gGT> data2geom;
  std::unordered_map<g2dGT, g2dDT> geom2data;

 public:
  DataGeomIdEx() {}
  ~DataGeomIdEx() {}

 public:
  bool insertData2Geom(const d2gDT& dataIndex, const d2gGT& geomIndex) {
    if (data2geom.find(dataIndex) != data2geom.end()) return false;
    data2geom.insert(std::make_pair(dataIndex, geomIndex));
    return true;
  }

  bool insertGeom2Data(const g2dGT& geomIndex, const g2dDT& dataIndex) {
    if (geom2data.find(geomIndex) != geom2data.end()) return false;
    geom2data.insert(std::make_pair(geomIndex, dataIndex));
    return true;
  }

  bool getData2Geom(const d2gDT& dataIndex, d2gGT& geomIndex) {
    if (data2geom.find(dataIndex) == data2geom.end()) return false;
    geomIndex = data2geom[dataIndex];
    return true;
  }

  bool getGeom2Data(const g2dGT& geomIndex, g2dDT& dataIndex) {
    if (geom2data.find(geomIndex) == geom2data.end()) return false;
    dataIndex = geom2data[geomIndex];
    return true;
  }

  bool removeData2Geom(const d2gDT& dataIndex) {
    data2geom.erase(dataIndex);
    return true;
  }

  bool removeGeom2Data(const g2dGT& geomIndex) {
    geom2data.erase(geomIndex);
    return true;
  }

  bool clear() {
    data2geom.clear();
    geom2data.clear();
    return true;
  }
};

struct TXSIMMAP_API txMap::MapData {
 public:
  // srs
  uint32_t srs;

  // coord type
  CoordType coordType;

  // road datas
  std::unordered_map<roadpkid, txRoadPtr> roadDatas;

  // lane geom ids for spatial index
  DataGeomIdEx<roadpkid, std::vector<pkid_t>, pkid_t, txLaneId> roadgeomIdEx;

  // link datas
  std::unordered_map<lanelinkpkid, txLaneLinkPtr> linkDatas;

  std::unordered_map<roadpkid, std::unordered_set<lanelinkpkid> > fromLinkDatas;

  std::unordered_map<roadpkid, std::unordered_set<lanelinkpkid> > toLinkDatas;

  // link geom ids for spatial index
  DataGeomIdEx<lanelinkpkid, pkid_t, pkid_t, lanelinkpkid> linkgeomIdEx;

  // object datas
  std::unordered_map<objectpkid, txObjectPtr> objectDatas;

  std::unordered_map<roadpkid, std::unordered_set<objectpkid> > roadRelObjects;

  // object geom ids for spatial index
  DataGeomIdEx<objectpkid, pkid_t, pkid_t, objectpkid> objectgeomIdEx;

  // geom pkid
  // pkid == 0 -> invalid
  pkid_t lanegeomId;

  pkid_t linkgeomId;

  pkid_t objgeomId;

  // mutex
  std::mutex roaddataMutex;

  std::mutex linkdataMutex;

  std::mutex objdataMutex;

  typedef std::unique_lock<std::mutex> mutexlock;

  // spatialindex
  txSpatialIndex spIndex;

  txPoint centerLL;

  txPoint lastCenterLL;

  size_t centerId;

  std::mutex centerMutex;

  std::shared_ptr<txProjection> projPtr;

  // junction info
  std::mutex juncDataMutex;

  std::unordered_map<junctionpkid, txJunctionPtr> juncDatas;

  // header
  txOdHeaderPtr headerptr;

  //
  hadmap::txPoint ref_point;

 public:
  MapData(CoordType type, const txPoint& ll)
      : srs(4326),
        coordType(type),
        lanegeomId(1),
        linkgeomId(1),
        objgeomId(1),
        centerLL(ll),
        lastCenterLL(0.0, 0.0, 0.0),
        centerId(0),
        projPtr(new txProjection) {}

  ~MapData();

 private:
  void transferRoadGeomToENU(const txRoadPtr roadPtr, txPoint& enuCenter);

  bool getLaneUnsafe(const txLaneId& laneId, txLanePtr& lanePtr);

  double point2CurveDis(const txPoint& p, const txLineCurve* curvePtr);

  double point2CurveDis(const txPoint& p, const txLineCurve* curvePtr, const std::pair<size_t, size_t>& geomRange,
                        const bool useZ);

  double point2CurveDis(const txPoint& p, const double& yaw, const txLineCurve* curvePtr);

  double point2CurveDis(const txPoint& p, const double& yaw, const txLineCurve* curvePtr,
                        const std::pair<size_t, size_t>& geomRange, const bool useZ);
  // getMinHeight
  double getMinHeight(const txLineCurve* curvePtr);

  // depends on useLocalC, coordType
  double coordDis2D(const txPoint& p0, const txPoint& p1);

  // unsafe
  void setLaneGeomToENU(txLanePtr& lanePtr, const txPoint& center);

  void setRoadGeomToENU(txRoadPtr& roadPtr, const txPoint& center);

  void setBoundaryGeomToENU(txLaneBoundaryPtr& boundaryPtr, const txPoint& center);

  void setObjectGeomToENU(txObjectPtr& objectPtr, const txPoint& center);

  void setCurveToMercator(txLineCurve* curvePtr);

  // insert geom data into spindex
  bool buildSpatialIndex(SpatialDataType type, const txLineCurve* curvePtr, pkid_t curveIndex);

  bool spatialSearch(SpatialDataType type, txEnvelope& envelope, std::vector<pkid_t>& curveIndexes);

  bool spatialSearch(SpatialDataType type, txEnvelope& envelope, std::vector<pkid_t>& curveIndexes,
                     std::vector<std::pair<size_t, size_t> >& pointRanges);

  // judge laneId is matching
  bool laneIdMatching(const txLaneId& id0, const txLaneId& id1);

  // update projection
  bool updateProj();

 public:
  // get coord type
  CoordType getCoordType() const { return coordType; }

  // get center id
  size_t getCenterId();

  // get center
  const txPoint& getCenter();

  // get projection info
  txMapInterface::txProjectionConstPtr getProjection();

  // insert
  bool insertRoad(const txRoadPtr roadPtr);

  bool insertRoads(const txRoads& roads);

  bool insertLaneLink(const txLaneLinkPtr linkPtr);

  bool insertLaneLinks(const txLaneLinks& links);

  bool insertObject(const txObjectPtr objectPtr);

  bool insertObjects(const txObjects& objects);

  bool insertJunction(const txJunctionPtr& juncPtr);
  //  update
  bool updateRoad(const txRoadPtr roadPtr);

  bool updateLaneLink(const txLaneLinkPtr lanelinkPtr);

  // remove
  bool removeRoad(roadpkid roadId, bool relate = true);

  bool removeRoads(const std::vector<roadpkid>& roadIds);

  // remove lanelink
  bool removeLaneLink(lanelinkpkid lanelinkid);

  // remove lanelinks by id
  // specified fromRoadId and toRoadId, remove all links between
  // fromRoadId == ROAD_PKID_INVALID, remove all links to toRoadId
  // toRaodId == ROAD_PKID_INVALID, remove all links from fromRoadId
  // fromRoadId == ROAD_PKID_INVALID and toRoadId == ROAD_PKID_INVALID, remove all links in map
  bool removeLinks(roadpkid fromRoadId, roadpkid toRoadId);

  bool removeObject(objectpkid objectId);

  bool removeObjects(const std::vector<objectpkid>& objectIds);

  bool removeJunction(junctionpkid jId);

  // search
  bool getRoadById(const roadpkid& roadId, txRoadPtr& roadPtr);

  bool getRoads(txRoads& roads);

  bool getLanes(txLanes& lanes);

  bool getBoundaries(txLaneBoundaries& boundaries);

  bool getLaneLinks(txLaneLinks& links);

  bool getObjects(txObjects& objects);

  bool getObject(const objectpkid& objId, txObjectPtr& objPtr);

  bool getObjects(OBJECT_TYPE type, txObjects& objects);

  bool getObjects(const std::vector<txLaneId>& relIds, txObjects& objects);

  bool getJunctions(txJunctions& junctions);

  bool nearestLane(const txPoint& loc, double radius, txLanePtr& lanePtr);

  bool nearestLane(const txPoint& loc, double radius, double yaw, txLanePtr& lanePtr);

  bool nearestLaneLink(const txPoint& loc, double radius, txLaneLinkPtr& linkPtr);

  bool searchLaneLinks(const PointVec& envelope, txLaneLinks& links);

  bool searchLanes(const PointVec& envelope, txLanes& lanes);

  bool searchBoundaries(const PointVec& envelope, txLaneBoundaries& boundaries);

  bool searchRoads(const PointVec& envelope, txRoads& roads);

  bool specRoadLink(const roadpkid roadid, txLaneLinkPtr& link);

  bool specRoadLinks(const roadpkid roadid, txLaneLinks& links);

  bool searchObjects(const PointVec& envelope, const std::set<OBJECT_TYPE>& types, const txLaneId& relLaneId,
                     txObjects& objects);

  bool searchObjects(const PointVec& envelope, const std::set<OBJECT_SUB_TYPE>& types, const txLaneId& relLaneId,
                     txObjects& objects);

  bool laneWidth(const txPoint& loc, double& dis2Left, double& dis2Right);

  bool roadWidth(const txPoint& loc, double& dis2Left, double& dis2Right);

  bool nextLanes(const txLaneId& curLaneId, txLanes& lanes);

  bool preLanes(const txLaneId& curLaneId, txLanes& lanes);

  bool specLanes(const txLaneId& specLaneId, txLanes& lanes);

  bool specBoundaries(const std::vector<laneboundarypkid>& ids, txLaneBoundaries& boundaries);

  bool specLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, txLaneLinks& links);

  bool specLinks(const txLaneId& fromLaneId, const txLaneId& toLaneId, txLaneLinks& links);

  bool specLinks(const std::vector<lanelinkpkid>& link_ids, txLaneLinks& links);

  bool relObjects(const txLaneId& curLaneId, const std::unordered_set<OBJECT_TYPE>& types, txObjects& objects);

  bool specJunction(const junctionpkid& jId, txJunctionPtr& juncPtr);

  bool updateCenter(const txPoint& lonlatele);

  PRIORITY_TYPE priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr);
};

double txMap::MapData::coordDis2D(const txPoint& p0, const txPoint& p1) {
  double a[3], b[3];
  if (COORD_XYZ == coordType || COORD_ENU == coordType) {
    a[0] = p0.x;
    a[1] = p0.y;
    // a[2] = p0.z;
    a[2] = 0.0;
    b[0] = p1.x;
    b[1] = p1.y;
    // b[2] = p1.z;
    b[2] = 0.0;
  } else {
    a[0] = p0.x;
    a[1] = p0.y;
    // a[2] = p0.z;
    a[2] = 0.0;
    b[0] = p1.x;
    b[1] = p1.y;
    // b[2] = p1.z;
    b[2] = 0.0;
    coord_trans_api::lonlat2global(a[0], a[1], a[2]);
    coord_trans_api::lonlat2global(b[0], b[1], b[2]);
  }
  return map_util::directlyDis(a, b);
}

bool txMap::MapData::updateProj() {
  projPtr->originLL = centerLL;
  txPoint enu;
  projPtr->lonlat2enu(lastCenterLL, enu);
  if (centerId == 1) {
    projPtr->dx = 0.0;
    projPtr->dy = 0.0;
  } else {
    projPtr->dx = enu.x;
    projPtr->dy = enu.y;
  }
  projPtr->pId = centerId;
  return true;
}

// unsafe function
bool txMap::MapData::getLaneUnsafe(const txLaneId& laneId, txLanePtr& lanePtr) {
  if (roadDatas.find(laneId.roadId) == roadDatas.end()) {
    return false;
  } else {
    txRoadPtr roadPtr = roadDatas[laneId.roadId];
    txSections& sections = roadPtr->getSections();
    for (auto secPtr : sections) {
      if (secPtr->getId() == laneId.sectionId) {
        txLanes& lanes = secPtr->getLanes();
        for (auto curLanePtr : lanes) {
          if (curLanePtr->getId() == laneId.laneId) {
            lanePtr = curLanePtr;
            return true;
          }
        }
      }
    }
    return false;
  }
}

void txMap::MapData::setLaneGeomToENU(txLanePtr& lanePtr, const txPoint& center) {
  if (lanePtr && lanePtr->getGeometry() && !lanePtr->getGeometry()->empty()) {
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(lanePtr->getGeometry());
    PointVec points;
    curvePtr->getPoints(points);
    for (size_t i = 0; i < points.size(); ++i)
      coord_trans_api::lonlat2enu(points[i].x, points[i].y, points[i].z, center.x, center.y, center.z);
    lanePtr->setGeometry(points, COORD_ENU);
  }
}

void txMap::MapData::setRoadGeomToENU(txRoadPtr& roadPtr, const txPoint& center) {
  if (roadPtr && roadPtr->getGeometry() && !roadPtr->getGeometry()->empty()) {
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(roadPtr->getGeometry());
    PointVec points;
    curvePtr->getPoints(points);
    for (size_t i = 0; i < points.size(); ++i)
      coord_trans_api::lonlat2enu(points[i].x, points[i].y, points[i].z, center.x, center.y, center.z);
    roadPtr->setGeometry(points, COORD_ENU);
  }
}

void txMap::MapData::setBoundaryGeomToENU(txLaneBoundaryPtr& boundaryPtr, const txPoint& center) {
  if (boundaryPtr && boundaryPtr->getGeometry() && !boundaryPtr->getGeometry()->empty()) {
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(boundaryPtr->getGeometry());
    PointVec points;
    curvePtr->getPoints(points);
    for (size_t i = 0; i < points.size(); ++i)
      coord_trans_api::lonlat2enu(points[i].x, points[i].y, points[i].z, center.x, center.y, center.z);
    boundaryPtr->setGeometry(points, COORD_ENU);
  }
}

txMap::MapData::~MapData() { txlog::debug("txMap::MapData:~~:roadDatas.size()=" + std::to_string(roadDatas.size())); }

void txMap::MapData::transferRoadGeomToENU(const txRoadPtr roadPtr, txPoint& enuCenter) {
  if (roadPtr == NULL || roadPtr->getSections().size() == 0) {
    return;
  } else {
    if (fabs(enuCenter.x) < 1e-5 && fabs(enuCenter.y) < 1e-5 && fabs(enuCenter.z) < 1e-5) {
      enuCenter = roadPtr->getGeometry()->getStart();
    }
    roadPtr->transfer2ENU(enuCenter);
    txSections& sections = roadPtr->getSections();
    for (auto secPtr : sections) {
      txLanes& lanes = secPtr->getLanes();
      std::unordered_set<laneboundarypkid> boundarySets;
      for (auto lanePtr : lanes) {
        lanePtr->transfer2ENU(enuCenter);
        if (lanePtr->getLeftBoundaryId() != BOUN_PKID_INVALID &&
            boundarySets.find(lanePtr->getLeftBoundaryId()) == boundarySets.end()) {
          lanePtr->getLeftBoundary()->transfer2ENU(enuCenter);
          boundarySets.insert(lanePtr->getLeftBoundaryId());
        }
        if (lanePtr->getRightBoundaryId() != BOUN_PKID_INVALID &&
            boundarySets.find(lanePtr->getRightBoundaryId()) == boundarySets.end()) {
          lanePtr->getRightBoundary()->transfer2ENU(enuCenter);
          boundarySets.insert(lanePtr->getRightBoundaryId());
        }
      }
    }
  }
}

void txMap::MapData::setCurveToMercator(txLineCurve* curvePtr) {
  if (curvePtr) {
    PointVec points;
    curvePtr->getPoints(points);
    for (size_t i = 0; i < points.size(); ++i) coord_trans_api::lonlat2mercator(points[i].x, points[i].y);
    curvePtr->setCoordType(COORD_ENU);
    curvePtr->setCoords(points);
  }
}

bool txMap::MapData::buildSpatialIndex(SpatialDataType type, const txLineCurve* curvePtr, pkid_t curveIndex) {
  if (coordType != COORD_WGS84) {
    return spIndex.insert(type, curvePtr, curveIndex);
  } else {
    txLineCurve* mercatorGeom = new txLineCurve(COORD_WGS84);
    PointVec points;
    //  curvePtr->sample( 10.0, points );
    curvePtr->getPoints(points);
    mercatorGeom->setCoords(points);
    //  mercatorGeom->clone( curvePtr );
    setCurveToMercator(mercatorGeom);
    bool r = spIndex.insert(type, mercatorGeom, curveIndex);
    delete mercatorGeom;
    return r;
  }
}

bool txMap::MapData::spatialSearch(SpatialDataType type, txEnvelope& envelope, std::vector<pkid_t>& curveIndexes) {
  if (coordType == COORD_XYZ || coordType == COORD_ENU) {
    return spIndex.search(type, envelope, curveIndexes);
  } else {
    coord_trans_api::lonlat2mercator(envelope.bMin[0], envelope.bMin[1]);
    coord_trans_api::lonlat2mercator(envelope.bMax[0], envelope.bMax[1]);
    return spIndex.search(type, envelope, curveIndexes);
  }
}

bool txMap::MapData::spatialSearch(SpatialDataType type, txEnvelope& envelope, std::vector<pkid_t>& curveIndexes,
                                   std::vector<std::pair<size_t, size_t> >& pointRanges) {
  if (coordType == COORD_XYZ || coordType == COORD_ENU) {
    return spIndex.search(type, envelope, curveIndexes, pointRanges);
  } else {
    coord_trans_api::lonlat2mercator(envelope.bMin[0], envelope.bMin[1]);
    coord_trans_api::lonlat2mercator(envelope.bMax[0], envelope.bMax[1]);
    return spIndex.search(type, envelope, curveIndexes, pointRanges);
  }
}

bool txMap::MapData::laneIdMatching(const txLaneId& id0, const txLaneId& id1) {
  if (id0.roadId == ROAD_PKID_INVALID || id1.roadId == ROAD_PKID_INVALID || id0.roadId == id1.roadId) {
    if (id0.sectionId == SECT_PKID_INVALID || id1.sectionId == SECT_PKID_INVALID || id0.sectionId == id1.sectionId) {
      if (id0.laneId == LANE_PKID_INVALID || id1.laneId == LANE_PKID_INVALID || id0.laneId == id1.laneId) return true;
    }
  }
  return false;
}

// useless
double txMap::MapData::point2CurveDis(const txPoint& p, const txLineCurve* curvePtr) {
  if (curvePtr == NULL) return -1.0;
  PointVec curvePoints;
  curvePtr->getPoints(curvePoints);

  Points2d points(curvePoints.size());
  for (size_t i = 0; i < curvePoints.size(); ++i) {
    points[i].x = curvePoints[i].x;
    points[i].y = curvePoints[i].y;
  }

  size_t index;
  double dis;
  bool wgs84Flag = false;
  if (curvePtr->getCoordType() == COORD_WGS84) wgs84Flag = true;
  map_util::nearestPoint2d(points, Point2d(p.x, p.y), wgs84Flag, index, dis);
  return dis;
}

double txMap::MapData::point2CurveDis(const txPoint& p, const txLineCurve* curvePtr,
                                      const std::pair<size_t, size_t>& geomRange, const bool useZ) {
  if (curvePtr == NULL) return -1.0;
  PointVec curvePoints;
  curvePtr->getPoints(curvePoints);
  if (geomRange.first > geomRange.second || geomRange.second >= curvePoints.size()) return -1.0;

  size_t start = geomRange.first > 0 ? geomRange.first - 1 : 0;
  size_t end = geomRange.second == curvePoints.size() - 1 ? geomRange.second : geomRange.second + 1;

  Points2d points(end - start + 1);
  std::map<size_t, size_t> point_index;

  for (size_t i = 0, j = start; i < points.size() && j <= end; ++i, ++j) {
    points[i].x = curvePoints[j].x;
    points[i].y = curvePoints[j].y;
    point_index[i] = j;
  }

  size_t index;
  double dis;
  bool wgs84Flag = false;
  if (curvePtr->getCoordType() == COORD_WGS84) wgs84Flag = true;

  if (!wgs84Flag) {
    double s, l, y;
    if (curvePtr->xy2sl(p.x, p.y, s, l, y)) {
      if (useZ) {
        map_util::nearestPoint2d(points, Point2d(p.x, p.y), false, index, dis);
        if (index < 0 || index > points.size() - 1) return DBL_MAX;
        size_t curpt_index = point_index[index];
        double dis3d = std::sqrt(std::pow(dis, 2) + std::pow(curvePoints.at(curpt_index).z - p.z, 2));
        return dis3d;
      }
      return fabs(l);
    } else {
      return DBL_MAX;
    }
  } else {
    map_util::nearestPoint2d(points, Point2d(p.x, p.y), wgs84Flag, index, dis);
    if (useZ) {
      if (index < 0 || index > points.size() - 1) return DBL_MAX;

      size_t curpt_index = point_index[index];
      double dis3d = std::sqrt(std::pow(dis, 2) + std::pow(curvePoints.at(curpt_index).z - p.z, 2));

      return dis3d;
    }

    return dis;
  }
}

// useless
double txMap::MapData::point2CurveDis(const txPoint& p, const double& yaw, const txLineCurve* curvePtr) {
  if (curvePtr == NULL) return -1.0;
  PointVec curvePoints;
  curvePtr->getPoints(curvePoints);

  Points2d points(curvePoints.size());
  for (size_t i = 0; i < curvePoints.size(); ++i) {
    points[i].x = curvePoints[i].x;
    points[i].y = curvePoints[i].y;
  }

  size_t index;
  double dis;
  bool wgs84Flag = false;
  if (curvePtr->getCoordType() == COORD_WGS84) wgs84Flag = true;
  map_util::nearestPoint2d(points, Point2d(p.x, p.y), wgs84Flag, index, dis);

  double curYaw = curvePtr->yaw(index);
  double yawDis = fabs(curYaw - yaw);
  if (yawDis > 180.0) yawDis = 360.0 - yawDis;
  yawDis = sin(yawDis * M_PI / 180.0) * 3.0;

  return dis + yawDis;
}

double txMap::MapData::point2CurveDis(const txPoint& p, const double& yaw, const txLineCurve* curvePtr,
                                      const std::pair<size_t, size_t>& geomRange, const bool useZ) {
  if (curvePtr == NULL) return -1.0;
  PointVec curvePoints;
  curvePtr->getPoints(curvePoints);

  if (geomRange.first > geomRange.second || geomRange.second >= curvePoints.size()) return -1.0;

  size_t start = geomRange.first > 0 ? geomRange.first - 1 : 0;
  size_t end = geomRange.second == curvePoints.size() - 1 ? geomRange.second : geomRange.second + 1;

  Points2d points(end - start + 1);
  std::map<size_t, size_t> point_index;

  for (size_t i = 0, j = start; i < points.size() && j <= end; ++i, ++j) {
    points[i].x = curvePoints[j].x;
    points[i].y = curvePoints[j].y;
    point_index[i] = j;
  }

  size_t index;
  double dis;
  bool wgs84Flag = false;
  if (curvePtr->getCoordType() == COORD_WGS84) wgs84Flag = true;

  if (wgs84Flag) {
    map_util::nearestPoint2d(points, Point2d(p.x, p.y), wgs84Flag, index, dis);
    double curYaw = curvePtr->yaw(index);
    double yawDis = fabs(curYaw - yaw);
    if (yawDis > 180.0) yawDis = 360.0 - yawDis;
    yawDis = sin(yawDis * M_PI / 180.0) * 3.0;

    if (useZ) {
      if (index < 0 || index > points.size() - 1) return DBL_MAX;
      size_t curpt_index = point_index[index];
      double dis3d = std::sqrt(std::pow(dis, 2) + std::pow(curvePoints.at(curpt_index).z - p.z, 2));
      return dis3d + yawDis;
    }

    return dis + yawDis;  // not useZ
  } else {
    double s, l, y;
    if (curvePtr->xy2sl(p.x, p.y, s, l, y)) {
      double curYaw = y;
      double yawDis = fabs(curYaw - yaw);
      if (yawDis > 180.0) {
        yawDis = 360.0 - yawDis;
      }
      if (yawDis < 90.0) {
        yawDis = sin(yawDis * M_PI / 180.0) * 3.0;
      } else {
        yawDis = (2.0 - sin(yawDis * M_PI / 180.0)) * 3.0;
      }

      if (useZ) {
        map_util::nearestPoint2d(points, Point2d(p.x, p.y), false, index, dis);
        if (index < 0 || index > points.size() - 1) return DBL_MAX;
        size_t curpt_index = point_index[index];
        double dis3d = std::sqrt(std::pow(dis, 2) + std::pow(curvePoints.at(curpt_index).z - p.z, 2));
        return dis3d + yawDis;
      }

      return fabs(l) + yawDis;  // not useZ
    } else {
      return DBL_MAX;
    }
  }
}

double txMap::MapData::getMinHeight(const txLineCurve* curvePtr) {
  double minHeight = 10000.0;
  if (curvePtr == NULL) return -10000.0;
  PointVec curvePoints;
  curvePtr->getPoints(curvePoints);
  for (auto it : curvePoints) {
    if (it.z < minHeight) {
      minHeight = it.z;
    }
  }
  return minHeight;
}

size_t txMap::MapData::getCenterId() {
  mutexlock lck(centerMutex);
  return centerId;
}

const txPoint& txMap::MapData::getCenter() {
  mutexlock lck(centerMutex);
  return centerLL;
}

txMapInterface::txProjectionConstPtr txMap::MapData::getProjection() {
  if (coordType == COORD_ENU) {
    mutexlock lck(centerMutex);
    return projPtr;
  } else {
    return NULL;
  }
}

bool txMap::MapData::insertRoad(const txRoadPtr roadPtr) {
  if (!roadPtr || roadPtr->getSections().size() == 0) return false;

  // if coordType is WGS84 & useLocalC is true
  // then transfer road geom to enu
  // txPoint enuCenter;
  // if ( coordType == COORD_WGS84 && useLocalC )
  // transferRoadGeomToENU( roadPtr, enuCenter );

  mutexlock centerLck(centerMutex);
  txPoint curCenter;
  curCenter = centerLL;

  mutexlock lck(roaddataMutex);
  if (roadDatas.find(roadPtr->getId()) != roadDatas.end()) return false;

  roadDatas.insert(std::make_pair(roadPtr->getId(), roadPtr));
  // txlog::trace("roadDatas insert: " + std::to_string(roadPtr->getId()) + ":" + std::to_string(roadDatas.size()));

  std::unordered_set<laneboundarypkid> boundaryIds;

  std::vector<pkid_t> geomIds;
  txSections& sections = roadPtr->getSections();
  for (auto secPtr : sections) {
    txLanes& lanes = secPtr->getLanes();
    txLaneBoundaries bds = secPtr->getBoundaries();

    std::map<laneboundarypkid, txLaneBoundaryPtr> lanebds;
    for (const auto& bd : bds) {
      // std::cout << "bds" << bd->getId() << std::endl;
      if (bd) {
        lanebds[bd->getId()] = bd;
      }
    }
    for (auto lanePtr : lanes) {
      if (!lanePtr->getGeometry() || lanePtr->getGeometry()->empty()) continue;
      laneboundarypkid lbdid = lanePtr->getLeftBoundaryId();
      laneboundarypkid rbdid = lanePtr->getRightBoundaryId();

      txLineCurve curline(lanePtr->getGeometry()->getCoordType());
      if (lanebds.find(lbdid) != lanebds.end() && lanebds.find(rbdid) != lanebds.end()) {
        txLaneBoundaryPtr bdl = lanebds[lbdid];
        txLaneBoundaryPtr bdr = lanebds[rbdid];
        PointVec pointsl, pointsr;
        dynamic_cast<const txLineCurve*>(bdl->getGeometry())->getPoints(pointsl);
        dynamic_cast<const txLineCurve*>(bdr->getGeometry())->getPoints(pointsr);
        size_t N = std::max(pointsl.size(), pointsr.size());
        for (size_t i = 0; i < N; i++) {
          if (pointsl.empty() || pointsr.empty()) {
            std::cout << "lanePtr:" << lanePtr->getUniqueId();
          }
          const auto& pl = i >= pointsl.size() ? pointsl.back() : pointsl[i];
          const auto& pr = i >= pointsr.size() ? pointsr.back() : pointsr[i];

          double dis = map_util::distanceBetweenPoints(Point3d(pl.x, pl.y, pl.z), Point3d(pr.x, pr.y, pr.z),
                                                       bdl->getGeometry()->getCoordType() == COORD_WGS84);
          double edge = 1;
          if (dis > edge * 4.001) {
            double c = std::floor((dis - edge - edge) / edge / 2) * 2;
            double step = (dis - edge - edge) / c;
            std::vector<double> sstep;
            sstep.push_back(edge);
            for (int s = 1, ss = c; s <= ss; s++) {
              sstep.push_back(edge + s * step);
            }
            double dx = (pr.x - pl.x) / dis;
            double dy = (pr.y - pl.y) / dis;
            double dz = (pr.z - pl.z) / dis;
            for (auto s : sstep) {
              txPoint p = pl;
              p.x += dx * s;
              p.y += dy * s;
              p.z += dz * s;
              curline.addPoint(p, false);
            }
          } else {
            txPoint p = pl;
            p.x += pr.x;
            p.y += pr.y;
            p.z += pr.z;
            p.x *= 0.5;
            p.y *= 0.5;
            p.z *= 0.5;
            curline.addPoint(p, false);
          }
        }
      }
      const txLineCurve* curPtr = curline.empty() ? dynamic_cast<const txLineCurve*>(lanePtr->getGeometry()) : &curline;

      lanePtr->setDenseGeometry(*curPtr);
      pkid_t curLaneGeomId = lanegeomId++;
      if (buildSpatialIndex(LANE_SP, curPtr, curLaneGeomId)) {
        geomIds.push_back(curLaneGeomId);
        roadgeomIdEx.insertGeom2Data(curLaneGeomId,
                                     txLaneId(lanePtr->getRoadId(), lanePtr->getSectionId(), lanePtr->getId()));
      }
      /*
      if ( useLocalC && coordType == COORD_WGS84 )
      {
              // setLaneGeomToENU( lanePtr, curCenter );
              lanePtr->transfer( enuCenter, curCenter );
              if ( boundaryIds.find( lanePtr->getLeftBoundaryId() ) == boundaryIds.end() )
              {
                      boundaryIds.insert( lanePtr->getLeftBoundaryId() );
                      // setBoundaryGeomToENU( lanePtr->getLeftBoundary(), curCenter );
                      lanePtr->getLeftBoundary()->transfer( enuCenter, curCenter );
              }
              if ( boundaryIds.find( lanePtr->getRightBoundaryId() ) == boundaryIds.end() )
              {
                      boundaryIds.insert( lanePtr->getRightBoundaryId() );
                      // setBoundaryGeomToENU( lanePtr->getRightBoundary(), curCenter );
                      lanePtr->getRightBoundary()->transfer( enuCenter, curCenter );
              }
      }
      */
    }
  }
  roadgeomIdEx.insertData2Geom(roadPtr->getId(), geomIds);
  if (coordType == COORD_ENU) {
    // setRoadGeomToENU( roadDatas[ roadPtr->getId() ], curCenter );
    // roadPtr->transfer( enuCenter, curCenter );
    transferRoadGeomToENU(roadPtr, curCenter);
  }
  return true;
}

bool txMap::MapData::insertRoads(const txRoads& roads) {
  bool r = true;
  txlog::debug("MapData inserts roads : " + std::to_string(roads.size()));
  for (auto roadPtr : roads) {
    txlog::debug("insert roadid = : " + std::to_string(roadPtr->getId()));
    r &= insertRoad(roadPtr);
  }
  return r;
}

bool txMap::MapData::insertLaneLink(const txLaneLinkPtr linkPtr) {
  if (!linkPtr) return false;

  mutexlock lck(linkdataMutex);
  if (linkDatas.find(linkPtr->getId()) != linkDatas.end()) return false;

  linkDatas.insert(std::make_pair(linkPtr->getId(), linkPtr));

  if (fromLinkDatas.find(linkPtr->fromRoadId()) == fromLinkDatas.end()) {
    fromLinkDatas.insert(std::make_pair(linkPtr->fromRoadId(), std::unordered_set<lanelinkpkid>()));
  }
  if (toLinkDatas.find(linkPtr->toRoadId()) == toLinkDatas.end()) {
    toLinkDatas.insert(std::make_pair(linkPtr->toRoadId(), std::unordered_set<lanelinkpkid>()));
  }

  fromLinkDatas[linkPtr->fromRoadId()].insert(linkPtr->getId());
  toLinkDatas[linkPtr->toRoadId()].insert(linkPtr->getId());

  if (!linkPtr->getGeometry() || linkPtr->getGeometry()->empty()) {
    return true;
  } else {
    pkid_t curLinkGeomId = linkgeomId++;
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(linkPtr->getGeometry());
    if (buildSpatialIndex(LINK_SP, curvePtr, curLinkGeomId)) {
      linkgeomIdEx.insertData2Geom(linkPtr->getId(), curLinkGeomId);
      linkgeomIdEx.insertGeom2Data(curLinkGeomId, linkPtr->getId());
    }
    return true;
  }
}

bool txMap::MapData::insertLaneLinks(const txLaneLinks& links) {
  bool r = true;
  txlog::debug("MapData inserts lanelinks: " + std::to_string(links.size()));
  for (auto linkPtr : links) {
    txlog::debug("insert lanelink = : " + std::to_string(linkPtr->getId()));
    r &= insertLaneLink(linkPtr);
  }

  return r;
}

bool txMap::MapData::insertObject(const txObjectPtr objectPtr) {
  if (!objectPtr) return false;

  mutexlock centerLck(centerMutex);
  txPoint curCenter;
  curCenter = centerLL;

  mutexlock lck(objdataMutex);
  if (objectDatas.find(objectPtr->getId()) != objectDatas.end()) {
    // printf("find same id in objects: %lld\n", objectPtr->getId());
    return false;
  }

  objectDatas.insert(std::make_pair(objectPtr->getId(), objectPtr));

  if (!objectPtr->getGeom()->getGeometry() || objectPtr->getGeom()->getGeometry()->empty()) {
  } else {
    pkid_t curObjGeomId = objgeomId++;
    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(objectPtr->getGeom()->getGeometry());
    if (buildSpatialIndex(OBJ_SP, curvePtr, curObjGeomId)) {
      objectgeomIdEx.insertData2Geom(objectPtr->getId(), curObjGeomId);
      objectgeomIdEx.insertGeom2Data(curObjGeomId, objectPtr->getId());
    }
  }

  std::vector<txLaneId> relIds;
  objectPtr->getReliedLaneIds(relIds);
  for (auto laneId : relIds) {
    if (roadRelObjects.find(laneId.roadId) == roadRelObjects.end()) {
      roadRelObjects.insert(std::make_pair(laneId.roadId, std::unordered_set<objectpkid>()));
    }
    roadRelObjects[laneId.roadId].insert(objectPtr->getId());
  }

  if (coordType == COORD_ENU) objectPtr->transfer2ENU(curCenter);
  return true;
}

bool txMap::MapData::insertObjects(const txObjects& objects) {
  txlog::debug("MapData inserts objects: " + std::to_string(objects.size()));
  bool r = true;
  for (auto objectPtr : objects) {
    r &= insertObject(objectPtr);
    txlog::debug("insert object = : " + std::to_string(objectPtr->getId()));
  }
  return r;
}

bool txMap::MapData::insertJunction(const txJunctionPtr& juncPtr) {
  if (!juncPtr) return false;

  mutexlock lck(juncDataMutex);
  if (juncDatas.find(juncPtr->getId()) != juncDatas.end()) return false;

  juncDatas.insert(std::make_pair(juncPtr->getId(), juncPtr));
  return true;
}

bool txMap::MapData::updateRoad(const txRoadPtr roadPtr) {
  if (!roadPtr || roadPtr->getSections().size() == 0) return false;
  if (false == removeRoad(roadPtr->getId(), false)) return false;
  if (false == insertRoad(roadPtr)) return false;
  return true;
}

bool txMap::MapData::updateLaneLink(const txLaneLinkPtr lanelinkPtr) {
  if (!lanelinkPtr || !lanelinkPtr->getGeometry()) return false;
  if (false == removeLaneLink(lanelinkPtr->getId())) return false;
  if (false == insertLaneLink(lanelinkPtr)) return false;
  return true;
}

bool txMap::MapData::getRoadById(const roadpkid& roadId, txRoadPtr& roadPtr) {
  if (roadId == ROAD_PKID_INVALID) {
    txlog::debug("txMap::MapData::getRoadById: ROAD_PKID_INVALID");
    return false;
  }
  mutexlock lck(roaddataMutex);
  if (roadDatas.find(roadId) == roadDatas.end()) {
    txlog::debug("txMap::MapData::getRoadById: roadDatas cannot find:" + std::to_string(roadId));
    txlog::debug("txMap::MapData::getRoadById: roadDatas.size()=" + std::to_string(roadDatas.size()));

    roadPtr = NULL;
  } else {
    roadPtr = roadDatas[roadId];
  }
  return NULL != roadPtr;
}

bool txMap::MapData::getRoads(txRoads& roads) {
  roads.clear();
  mutexlock lck(roaddataMutex);
  for (auto itr = roadDatas.begin(); itr != roadDatas.end(); ++itr) roads.push_back(itr->second);
  return !roads.empty();
}

bool txMap::MapData::getLanes(txLanes& lanes) {
  lanes.clear();
  mutexlock lck(roaddataMutex);
  for (auto itr = roadDatas.begin(); itr != roadDatas.end(); ++itr) {
    auto& curSections = itr->second->getSections();
    for (auto& secPtr : curSections) {
      auto& curLanes = secPtr->getLanes();
      lanes.insert(lanes.end(), curLanes.begin(), curLanes.end());
    }
  }
  return !lanes.empty();
}

bool txMap::MapData::getBoundaries(txLaneBoundaries& boundaries) {
  boundaries.clear();
  std::unordered_set<laneboundarypkid> ids;
  mutexlock lck(roaddataMutex);
  for (auto itr = roadDatas.begin(); itr != roadDatas.end(); ++itr) {
    auto& curSections = itr->second->getSections();
    for (auto& secPtr : curSections) {
      auto& curLanes = secPtr->getLanes();
      for (auto& curLanePtr : curLanes) {
        if (ids.find(curLanePtr->getLeftBoundaryId()) == ids.end()) {
          ids.insert(curLanePtr->getLeftBoundaryId());
          boundaries.push_back(curLanePtr->getLeftBoundary());
        }
        if (ids.find(curLanePtr->getRightBoundaryId()) == ids.end()) {
          ids.insert(curLanePtr->getRightBoundaryId());
          boundaries.push_back(curLanePtr->getRightBoundary());
        }
      }
    }
  }
  return !boundaries.empty();
}

bool txMap::MapData::getLaneLinks(txLaneLinks& links) {
  links.clear();
  mutexlock lck(linkdataMutex);
  for (auto itr = linkDatas.begin(); itr != linkDatas.end(); ++itr) links.push_back(itr->second);
  return !links.empty();
}

bool txMap::MapData::getObjects(txObjects& objects) {
  objects.clear();
  mutexlock lck(objdataMutex);
  for (auto itr = objectDatas.begin(); itr != objectDatas.end(); ++itr) objects.push_back(itr->second);
  return !objects.empty();
}

// txObjectPtr txMap::MapData::getObject( objectpkid objId )
bool txMap::MapData::getObject(const objectpkid& objId, txObjectPtr& objPtr) {
  mutexlock lck(objdataMutex);
  if (objectDatas.find(objId) == objectDatas.end()) {
    return false;
  }

  objPtr = objectDatas.at(objId);
  return true;
}

bool txMap::MapData::getObjects(OBJECT_TYPE type, txObjects& objects) {
  objects.clear();
  mutexlock lck(objdataMutex);
  for (auto itr = objectDatas.begin(); itr != objectDatas.end(); ++itr)
    if (itr->second->getObjectType() == type) objects.push_back(itr->second);
  return !objects.empty();
}

bool txMap::MapData::getObjects(const std::vector<txLaneId>& relIds, txObjects& objects) {
  objects.clear();
  mutexlock lck(objdataMutex);
  std::unordered_set<objectpkid> objIds;
  for (auto& laneId : relIds) {
    if (roadRelObjects.find(laneId.roadId) != roadRelObjects.end()) {
      std::unordered_set<objectpkid>& curObjIds = roadRelObjects[laneId.roadId];
      for (auto& id : curObjIds) {
        std::vector<txLaneId> objRelIds;
        if (objectDatas.find(id) == objectDatas.end()) continue;
        objectDatas.at(id)->getReliedLaneIds(objRelIds);
        for (auto& objRelId : objRelIds) {
          if (laneIdMatching(laneId, objRelId)) {
            if (objIds.find(id) == objIds.end()) {
              objIds.insert(id);
              objects.push_back(objectDatas.at(id));
            }
            break;
          }
        }
      }
    }
  }
  return !objects.empty();
}

bool txMap::MapData::getJunctions(txJunctions& junctions) {
  junctions.clear();
  mutexlock lck(roaddataMutex);
  for (auto itr = juncDatas.begin(); itr != juncDatas.end(); ++itr) junctions.push_back(itr->second);
  return !junctions.empty();
}

bool txMap::MapData::nearestLane(const txPoint& loc, double radius, double yaw, txLanePtr& lanePtr) {
  mutexlock centerLck(centerMutex);
  txPoint searchLoc(loc);
  if (coordType == COORD_ENU) {
    coord_trans_api::enu2lonlat(searchLoc.x, searchLoc.y, searchLoc.z, centerLL.x, centerLL.y, centerLL.z);
  }
  double offset = radius;
  if (coordType == COORD_WGS84) {
    double x = radius, y = radius, z = 0;
    coord_trans_api::enu2lonlat(x, y, z, searchLoc.x, searchLoc.y, 0);
    offset = std::max(std::abs(x - searchLoc.x), std::abs(y - searchLoc.y));
  }
  txEnvelope envelope(searchLoc.x - offset, searchLoc.y - offset, searchLoc.x + offset, searchLoc.y + offset);

  std::vector<pkid_t> curveIndexes;
  std::vector<std::pair<size_t, size_t> > pointRanges;
  if (spatialSearch(LANE_SP, envelope, curveIndexes, pointRanges)) {
    mutexlock lck(roaddataMutex);
    lanePtr = NULL;
    double minDis = DBL_MAX;
    for (size_t i = 0; i < curveIndexes.size(); ++i) {
      pkid_t geomId = curveIndexes[i];
      txLaneId curLaneId;
      if (roadgeomIdEx.getGeom2Data(geomId, curLaneId)) {
        txLanePtr curLanePtr;
        if (getLaneUnsafe(curLaneId, curLanePtr)) {
          const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(curLanePtr->getDenseGeometry());
          double curDis = point2CurveDis(loc, yaw, curvePtr, pointRanges[i], loc.z > VALID_Z);
          if (minDis > curDis) {
            minDis = curDis;
            lanePtr = curLanePtr;
          }
        }
      }
    }
    return lanePtr != NULL;
  } else {
    return false;
  }
}

bool txMap::MapData::nearestLane(const txPoint& loc, double radius, txLanePtr& lanePtr) {
  mutexlock centerLck(centerMutex);
  txPoint searchLoc(loc);
  if (coordType == COORD_ENU) {
    coord_trans_api::enu2lonlat(searchLoc.x, searchLoc.y, searchLoc.z, centerLL.x, centerLL.y, centerLL.z);
  }
  double offset = radius;
  {
    double x = radius, y = radius, z = 0;
    coord_trans_api::enu2lonlat(x, y, z, searchLoc.x, searchLoc.y, 0);
    offset = std::max(std::abs(x - searchLoc.x), std::abs(y - searchLoc.y));
  }
  txEnvelope envelope(searchLoc.x - offset, searchLoc.y - offset, searchLoc.x + offset, searchLoc.y + offset);

  std::vector<pkid_t> curveIndexes;
  std::vector<std::pair<size_t, size_t> > pointRanges;
  if (spatialSearch(LANE_SP, envelope, curveIndexes, pointRanges)) {
    mutexlock lck(roaddataMutex);
    lanePtr = NULL;
    double minDis = DBL_MAX;
    for (size_t i = 0; i < curveIndexes.size(); ++i) {
      pkid_t geomId = curveIndexes[i];
      txLaneId curLaneId;
      std::vector<txLanePtr> laneVec;
      double minHeight = 9999;
      if (roadgeomIdEx.getGeom2Data(geomId, curLaneId)) {
        txLanePtr curLanePtr;
        if (getLaneUnsafe(curLaneId, curLanePtr)) {
          laneVec.push_back(curLanePtr);
        }
        const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(curLanePtr->getDenseGeometry());
        double laneMinHeight = getMinHeight(curvePtr);
        if (laneMinHeight > VALID_Z && laneMinHeight < minHeight) {
          minHeight = laneMinHeight;
        }
      }
      if (abs(loc.z + 9000) < 1e-6) {
        searchLoc.z = minHeight;
      }
      for (auto curLanePtr : laneVec) {
        const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(curLanePtr->getDenseGeometry());
        double curDis = point2CurveDis(searchLoc, curvePtr, pointRanges[i], loc.z > VALID_Z);
        double disToLaneStart =
            map_util::distanceBetweenPoints(hadmap::Point3d(curvePtr->getStart().x, curvePtr->getStart().y, 0),
                                            hadmap::Point3d(searchLoc.x, searchLoc.y, 0), true);
        if (curDis < 0.001 /*1 mm*/ && disToLaneStart < 0.001) {
          minDis = curDis;
          lanePtr = curLanePtr;
          break;
        }
        if (minDis > curDis && curDis >= 0.0) {
          minDis = curDis;
          lanePtr = curLanePtr;
        }
      }
    }
    return lanePtr != NULL;
  } else {
    return false;
  }
}

bool txMap::MapData::nearestLaneLink(const txPoint& loc, double radius, txLaneLinkPtr& linkPtr) {
  mutexlock centerLck(centerMutex);
  txPoint searchLoc(loc);
  if (coordType == COORD_ENU) {
    coord_trans_api::enu2lonlat(searchLoc.x, searchLoc.y, searchLoc.z, centerLL.x, centerLL.y, centerLL.z);
  }
  double offset = radius;
  if (coordType == COORD_WGS84) {
    double x = radius, y = radius, z = 0;
    coord_trans_api::enu2lonlat(x, y, z, searchLoc.x, searchLoc.y, 0);
    offset = std::max(std::abs(x - searchLoc.x), std::abs(y - searchLoc.y));
  }
  txEnvelope envelope(searchLoc.x - offset, searchLoc.y - offset, searchLoc.x + offset, searchLoc.y + offset);

  std::vector<pkid_t> curveIndexes;
  std::vector<std::pair<size_t, size_t> > pointRanges;
  if (spatialSearch(LINK_SP, envelope, curveIndexes, pointRanges)) {
    mutexlock lck(roaddataMutex);
    linkPtr = NULL;
    double minDis = DBL_MAX;
    for (size_t i = 0; i < curveIndexes.size(); ++i) {
      pkid_t geomId = curveIndexes[i];
      lanelinkpkid curLinkId;
      if (linkgeomIdEx.getGeom2Data(geomId, curLinkId)) {
        txLaneLinkPtr curLinkPtr;
        txLaneLinks links;

        if (specLinks(std::vector<lanelinkpkid>(1, curLinkId), links)) {
          const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(links[0]->getGeometry());
          double curDis = point2CurveDis(loc, curvePtr, pointRanges[i], loc.z > VALID_Z);

          double disToLaneStart =
              map_util::distanceBetweenPoints(hadmap::Point3d(curvePtr->getStart().x, curvePtr->getStart().y, 0),
                                              hadmap::Point3d(searchLoc.x, searchLoc.y, 0), true);

          if (curDis < 0.001 /*1 mm*/ && disToLaneStart < 0.001) {
            minDis = curDis;
            linkPtr = links[0];
            break;
          }
          if (minDis > curDis && curDis >= 0.0) {
            minDis = curDis;
            linkPtr = links[0];
          }
        }
      }
    }
    return linkPtr != NULL;
  } else {
    return false;
  }
}

bool txMap::MapData::nextLanes(const txLaneId& curLaneId, txLanes& lanes) {
  if (curLaneId.roadId == ROAD_PKID_INVALID || curLaneId.sectionId == SECT_PKID_INVALID ||
      curLaneId.laneId == LANE_PKID_INVALID) {
    return false;
  } else {
    lanes.clear();
    mutexlock lck(roaddataMutex);
    txLanePtr curLanePtr;
    if (getLaneUnsafe(curLaneId, curLanePtr)) {
      if (roadDatas.find(curLaneId.roadId) == roadDatas.end()) {
        txlog::debug("txMap::MapData::nextLanes: cannot find road.");
      }
      txRoadPtr curRoadPtr = roadDatas[curLaneId.roadId];
      txSections& curSections = curRoadPtr->getSections();
      txSectionPtr nextSecPtr;
      for (size_t i = 0; i < curSections.size(); ++i) {
        if (curSections[i]->getId() == curLaneId.sectionId) {
          if (i + 1 < curSections.size()) {
            nextSecPtr = curSections[i + 1];
            break;
          }
        }
      }
      if (nextSecPtr) {
        txLanes& nextLanes = nextSecPtr->getLanes();
        for (auto nextLanePtr : nextLanes) {
          if (!nextLanePtr->getGeometry() || nextLanePtr->getGeometry()->empty()) continue;
          if (coordDis2D(curLanePtr->getGeometry()->getEnd(), nextLanePtr->getGeometry()->getStart()) < 0.2) {
            lanes.push_back(nextLanePtr);
          }
        }
      } else {
        mutexlock linkLck(linkdataMutex);
        if (fromLinkDatas.find(curLaneId.roadId) != fromLinkDatas.end()) {
          std::unordered_set<lanelinkpkid>& links = fromLinkDatas[curLaneId.roadId];
          for (auto linkId : links) {
            txLaneId fromId = linkDatas[linkId]->fromTxLaneId();
            if (laneIdMatching(curLaneId, fromId)) {
              txLanePtr nextLanePtr;
              if (getLaneUnsafe(linkDatas[linkId]->toTxLaneId(), nextLanePtr)) lanes.push_back(nextLanePtr);
            }
          }
        }
      }
      return !lanes.empty();
    } else {
      return false;
    }
  }
}

bool txMap::MapData::preLanes(const txLaneId& curLaneId, txLanes& lanes) {
  if (curLaneId.roadId == ROAD_PKID_INVALID || curLaneId.sectionId == SECT_PKID_INVALID ||
      curLaneId.laneId == LANE_PKID_INVALID) {
    return false;
  } else {
    lanes.clear();
    mutexlock lck(roaddataMutex);
    txLanePtr curLanePtr;
    if (getLaneUnsafe(curLaneId, curLanePtr)) {
      if (roadDatas.find(curLaneId.roadId) == roadDatas.end()) {
        txlog::debug("txMap::MapData::preLanes: cannot find road.");
      }
      txRoadPtr curRoadPtr = roadDatas[curLaneId.roadId];
      txSections& curSections = curRoadPtr->getSections();
      txSectionPtr preSecPtr;
      for (size_t i = 0; i < curSections.size(); ++i) {
        if (curSections[i]->getId() == curLaneId.sectionId) {
          if (i > 0) {
            preSecPtr = curSections[i - 1];
            break;
          }
        }
      }
      if (preSecPtr) {
        txLanes& preLanes = preSecPtr->getLanes();
        for (auto preLanePtr : preLanes) {
          if (!preLanePtr->getGeometry() || preLanePtr->getGeometry()->empty()) continue;
          if (coordDis2D(curLanePtr->getGeometry()->getStart(), preLanePtr->getGeometry()->getEnd()) < 0.2) {
            lanes.push_back(preLanePtr);
          }
        }
      } else {
        mutexlock linkLck(linkdataMutex);
        if (toLinkDatas.find(curLaneId.roadId) != toLinkDatas.end()) {
          std::unordered_set<lanelinkpkid>& links = toLinkDatas[curLaneId.roadId];
          for (auto linkId : links) {
            txLaneId toId = linkDatas[linkId]->toTxLaneId();
            if (laneIdMatching(curLaneId, toId)) {
              txLanePtr preLanePtr;
              if (getLaneUnsafe(linkDatas[linkId]->fromTxLaneId(), preLanePtr)) lanes.push_back(preLanePtr);
            }
          }
        }
      }
      return !lanes.empty();
    } else {
      return false;
    }
  }
}
// search lane link by geom(if has)
bool txMap::MapData::searchLaneLinks(const PointVec& envelope, txLaneLinks& links) {
  if (envelope[0].x > envelope[1].x || envelope[0].y > envelope[1].y) return false;

  txEnvelope e(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  mutexlock lck(centerMutex);
  if (coordType == COORD_ENU) {
    double z = 0.0;
    coord_trans_api::enu2lonlat(e.bMin[0], e.bMin[1], z, centerLL.x, centerLL.y, centerLL.z);
    coord_trans_api::enu2lonlat(e.bMax[0], e.bMax[1], z, centerLL.x, centerLL.y, centerLL.z);
  }
  std::vector<pkid_t> curveIndexes;
  if (spatialSearch(LINK_SP, e, curveIndexes)) {
    links.clear();
    mutexlock lck(roaddataMutex);
    std::vector<lanelinkpkid> link_ids;

    for (auto geomId : curveIndexes) {
      lanelinkpkid curLinkId;
      if (linkgeomIdEx.getGeom2Data(geomId, curLinkId)) {
        link_ids.push_back(curLinkId);
      }
    }

    bool flag = specLinks(link_ids, links);
    return links.size() > 0;
  } else {
    return false;
  }
}

bool txMap::MapData::searchLanes(const PointVec& envelope, txLanes& lanes) {
  if (envelope[0].x > envelope[1].x || envelope[0].y > envelope[1].y) return false;

  txEnvelope e(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  mutexlock lck(centerMutex);
  if (coordType == COORD_ENU) {
    double z = 0.0;
    coord_trans_api::enu2lonlat(e.bMin[0], e.bMin[1], z, centerLL.x, centerLL.y, centerLL.z);
    coord_trans_api::enu2lonlat(e.bMax[0], e.bMax[1], z, centerLL.x, centerLL.y, centerLL.z);
  }
  std::vector<pkid_t> curveIndexes;
  if (spatialSearch(LANE_SP, e, curveIndexes)) {
    lanes.clear();
    mutexlock lck(roaddataMutex);
    for (auto geomId : curveIndexes) {
      txLaneId curLaneId;
      if (roadgeomIdEx.getGeom2Data(geomId, curLaneId)) {
        txLanePtr curLanePtr;
        if (getLaneUnsafe(curLaneId, curLanePtr)) lanes.push_back(curLanePtr);
      }
    }
    return lanes.size() > 0;
  } else {
    return false;
  }
}

bool txMap::MapData::searchBoundaries(const PointVec& envelope, txLaneBoundaries& boundaries) {
  txLanes lanes;
  std::unordered_set<laneboundarypkid> ids;
  if (searchLanes(envelope, lanes)) {
    for (auto& lanePtr : lanes) {
      if (ids.find(lanePtr->getLeftBoundaryId()) == ids.end()) {
        ids.insert(lanePtr->getLeftBoundaryId());
        boundaries.push_back(lanePtr->getLeftBoundary());
      }
      if (ids.find(lanePtr->getRightBoundaryId()) == ids.end()) {
        ids.insert(lanePtr->getRightBoundaryId());
        boundaries.push_back(lanePtr->getRightBoundary());
      }
    }
    return !boundaries.empty();
  }
  return false;
}

bool txMap::MapData::searchRoads(const PointVec& envelope, txRoads& roads) {
  txLanes lanes;
  std::unordered_set<roadpkid> ids;
  if (searchLanes(envelope, lanes)) {
    for (auto& curLanePtr : lanes) ids.insert(curLanePtr->getRoadId());
    mutexlock lck(roaddataMutex);
    for (auto itr = ids.begin(); itr != ids.end(); ++itr) {
      if (roadDatas.find(*itr) != roadDatas.end()) roads.push_back(roadDatas[*itr]);
    }

    return !roads.empty();
  }
  return false;
}

bool txMap::MapData::specBoundaries(const std::vector<laneboundarypkid>& specIds, txLaneBoundaries& boundaries) {
  boundaries.clear();
  std::unordered_map<laneboundarypkid, size_t> ids;
  for (auto& id : specIds) ids.insert(std::make_pair(id, 0));
  mutexlock lck(roaddataMutex);
  for (auto itr = roadDatas.begin(); itr != roadDatas.end(); ++itr) {
    auto& curSections = itr->second->getSections();
    for (auto& secPtr : curSections) {
      auto& curLanes = secPtr->getLanes();
      for (auto& curLanePtr : curLanes) {
        if (ids.find(curLanePtr->getLeftBoundaryId()) != ids.end()) {
          if (ids[curLanePtr->getLeftBoundaryId()] == 0) {
            ids[curLanePtr->getLeftBoundaryId()] = 1;
            boundaries.push_back(curLanePtr->getLeftBoundary());
          }
        }
        if (ids.find(curLanePtr->getRightBoundaryId()) != ids.end()) {
          if (ids[curLanePtr->getRightBoundaryId()] == 0) {
            ids[curLanePtr->getRightBoundaryId()] = 1;
            boundaries.push_back(curLanePtr->getRightBoundary());
          }
        }
      }
    }
  }
  for (auto link : linkDatas) {
    for (auto lk : link.second->getLeftBoundaries()) {
      if (lk && ids.find(lk->getId()) != ids.end()) {
        if (ids[lk->getId()] == 0) {
          ids[lk->getId()] = 1;
          boundaries.push_back(lk);
        }
      }
    }
    for (auto lk : link.second->getRightBoundaries()) {
      if (lk && ids.find(lk->getId()) != ids.end()) {
        if (ids[lk->getId()] == 0) {
          ids[lk->getId()] = 1;
          boundaries.push_back(lk);
        }
      }
    }
  }
  return !boundaries.empty();
}

bool txMap::MapData::specLanes(const txLaneId& specLaneId, txLanes& lanes) {
  lanes.clear();
  if (specLaneId.roadId == ROAD_PKID_INVALID) return false;
  mutexlock lck(roaddataMutex);
  if (roadDatas.find(specLaneId.roadId) == roadDatas.end()) return false;
  hadmap::txRoadPtr roadPtr = roadDatas[specLaneId.roadId];
  hadmap::txSections& sections = roadPtr->getSections();
  if (specLaneId.sectionId == SECT_PKID_INVALID) {
    for (auto secPtr : sections) {
      hadmap::txLanes& curLanes = secPtr->getLanes();
      lanes.insert(lanes.end(), curLanes.begin(), curLanes.end());
    }
  } else {
    for (auto secPtr : sections) {
      if (secPtr->getId() == specLaneId.sectionId) {
        hadmap::txLanes& curLanes = secPtr->getLanes();
        if (specLaneId.laneId == LANE_PKID_INVALID) {
          lanes.assign(curLanes.begin(), curLanes.end());
        } else {
          for (auto lanePtr : curLanes) {
            if (lanePtr->getId() == specLaneId.laneId) {
              lanes.push_back(lanePtr);
              break;
            }
          }
        }
      }
    }
  }
  return !lanes.empty();
}

bool txMap::MapData::specLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, txLaneLinks& links) {
  if (fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID) return false;

  links.clear();
  mutexlock lck(linkdataMutex);
  if (fromRoadId != ROAD_PKID_INVALID) {
    if (fromLinkDatas.find(fromRoadId) != fromLinkDatas.end()) {
      std::unordered_set<lanelinkpkid>& fromLinkIds = fromLinkDatas[fromRoadId];
      for (auto itr = fromLinkIds.begin(); itr != fromLinkIds.end(); ++itr) {
        txLaneLinkPtr curLinkPtr = linkDatas[*itr];
        if (toRoadId == ROAD_PKID_INVALID || curLinkPtr->toRoadId() == toRoadId) links.push_back(curLinkPtr);
      }
    }
  } else {
    if (toLinkDatas.find(toRoadId) != toLinkDatas.end()) {
      std::unordered_set<lanelinkpkid>& toLinkIds = toLinkDatas[toRoadId];
      for (auto itr = toLinkIds.begin(); itr != toLinkIds.end(); ++itr) links.push_back(linkDatas[*itr]);
    }
  }
  return !links.empty();
}

bool txMap::MapData::specLinks(const txLaneId& fromLaneId, const txLaneId& toLaneId, txLaneLinks& links) {
  if (fromLaneId.roadId == ROAD_PKID_INVALID && toLaneId.roadId == ROAD_PKID_INVALID) return false;

  links.clear();
  mutexlock lck(linkdataMutex);
  if (fromLaneId.roadId != ROAD_PKID_INVALID) {
    if (fromLinkDatas.find(fromLaneId.roadId) != fromLinkDatas.end()) {
      std::unordered_set<lanelinkpkid>& fromLinkIds = fromLinkDatas[fromLaneId.roadId];
      if (toLaneId.roadId == ROAD_PKID_INVALID) {
        for (auto itr = fromLinkIds.begin(); itr != fromLinkIds.end(); ++itr) {
          txLaneLinkPtr curLinkPtr = linkDatas[*itr];
          if (fromLaneId.sectionId == curLinkPtr->fromSectionId() && fromLaneId.laneId == curLinkPtr->fromLaneId()) {
            links.push_back(curLinkPtr);
          }
        }
      } else {
        for (auto itr = fromLinkIds.begin(); itr != fromLinkIds.end(); ++itr) {
          txLaneLinkPtr curLinkPtr = linkDatas[*itr];
          if (fromLaneId.sectionId == curLinkPtr->fromSectionId() && fromLaneId.laneId == curLinkPtr->fromLaneId() &&
              toLaneId.roadId == curLinkPtr->toRoadId() && toLaneId.sectionId == curLinkPtr->toSectionId() &&
              toLaneId.laneId == curLinkPtr->toLaneId())
            links.push_back(curLinkPtr);
        }
      }
    }
  } else {
    if (toLinkDatas.find(toLaneId.roadId) != toLinkDatas.end()) {
      std::unordered_set<lanelinkpkid>& toLinkIds = toLinkDatas[toLaneId.roadId];
      for (auto itr = toLinkIds.begin(); itr != toLinkIds.end(); ++itr) {
        txLaneLinkPtr curLinkPtr = linkDatas[*itr];
        if (toLaneId.sectionId == curLinkPtr->toSectionId() && toLaneId.laneId == curLinkPtr->toLaneId()) {
          links.push_back(curLinkPtr);
        }
      }
    }
  }
  return !links.empty();
}

bool txMap::MapData::specLinks(const std::vector<lanelinkpkid>& link_ids, txLaneLinks& links) {
  links.clear();
  if (link_ids.empty()) {
    // get all links
    for (auto itr = linkDatas.begin(); itr != linkDatas.end(); ++itr) links.push_back(itr->second);
  } else {
    for (auto& id : link_ids) {
      if (linkDatas.find(id) != linkDatas.end()) links.push_back(linkDatas[id]);
    }
  }
  return !links.empty();
}

bool txMap::MapData::specRoadLink(const roadpkid roadid, txLaneLinkPtr& link) {
  for (auto& it : linkDatas) {
    if (it.second->getOdrRoadId() == roadid) {
      link = it.second;
      return true;
    }
  }
  return false;
}
bool txMap::MapData::specRoadLinks(const roadpkid roadid, txLaneLinks& links) {
  for (auto& it : linkDatas) {
    if (it.second->getOdrRoadId() == roadid) {
      links.push_back(it.second);
    }
  }
  return (links.size() > 0);
}
bool txMap::MapData::searchObjects(const PointVec& envelope, const std::set<OBJECT_TYPE>& types,
                                   const txLaneId& relLaneId, txObjects& objects) {
  if (envelope[0].x > envelope[1].x || envelope[0].y > envelope[1].y) return false;

  txEnvelope e(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  mutexlock lck(centerMutex);
  if (coordType == COORD_ENU) {
    double z = 0.0;
    coord_trans_api::enu2lonlat(e.bMin[0], e.bMin[1], z, centerLL.x, centerLL.y, centerLL.z);
    coord_trans_api::enu2lonlat(e.bMax[0], e.bMax[1], z, centerLL.x, centerLL.y, centerLL.z);
  }

  std::vector<pkid_t> curveIndexes;
  if (spatialSearch(OBJ_SP, e, curveIndexes)) {
    objects.clear();
    mutexlock lck(objdataMutex);
    for (auto geomId : curveIndexes) {
      objectpkid objId;
      if (objectgeomIdEx.getGeom2Data(geomId, objId)) {
        txObjectPtr objPtr = objectDatas[objId];
        if (!types.empty() && types.find(objPtr->getObjectType()) == types.end()) continue;
        std::vector<txLaneId> objRelIds;
        objPtr->getReliedLaneIds(objRelIds);
        if (objRelIds.empty()) {
          if (laneIdMatching(relLaneId, txLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID))) {
            objects.push_back(objPtr);
          }
        } else {
          for (size_t i = 0; i < objRelIds.size(); ++i) {
            if (laneIdMatching(relLaneId, objRelIds[i])) {
              objects.push_back(objPtr);
              break;
            }
          }
        }
      }
    }
    return !objects.empty();
  } else {
    return false;
  }
}

bool txMap::MapData::searchObjects(const PointVec& envelope, const std::set<OBJECT_SUB_TYPE>& types,
                                   const txLaneId& relLaneId, txObjects& objects) {
  if (envelope[0].x > envelope[1].x || envelope[0].y > envelope[1].y) return false;

  txEnvelope e(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  mutexlock lck(centerMutex);
  if (coordType == COORD_ENU) {
    double z = 0.0;
    coord_trans_api::enu2lonlat(e.bMin[0], e.bMin[1], z, centerLL.x, centerLL.y, centerLL.z);
    coord_trans_api::enu2lonlat(e.bMax[0], e.bMax[1], z, centerLL.x, centerLL.y, centerLL.z);
  }

  std::vector<pkid_t> curveIndexes;
  if (spatialSearch(OBJ_SP, e, curveIndexes)) {
    objects.clear();
    mutexlock lck(objdataMutex);
    for (auto geomId : curveIndexes) {
      objectpkid objId;
      if (objectgeomIdEx.getGeom2Data(geomId, objId)) {
        txObjectPtr objPtr = objectDatas[objId];
        if (!types.empty() && types.find(objPtr->getObjectSubType()) == types.end()) continue;
        std::vector<txLaneId> objRelIds;
        objPtr->getReliedLaneIds(objRelIds);
        if (objRelIds.empty()) {
          if (laneIdMatching(relLaneId, txLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID))) {
            objects.push_back(objPtr);
          }
        } else {
          for (size_t i = 0; i < objRelIds.size(); ++i) {
            if (laneIdMatching(relLaneId, objRelIds[i])) {
              objects.push_back(objPtr);
              break;
            }
          }
        }
      }
    }
    return !objects.empty();
  } else {
    return false;
  }
}

bool txMap::MapData::specJunction(const junctionpkid& jId, txJunctionPtr& juncPtr) {
  mutexlock lck(juncDataMutex);
  if (juncDatas.find(jId) == juncDatas.end()) return false;
  juncPtr = juncDatas[jId];
  return true;
}

bool txMap::MapData::updateCenter(const txPoint& lonlatele) {
  mutexlock lck(centerMutex);
  txPoint oldEnuC = centerLL;
  lastCenterLL = centerLL;
  centerLL = lonlatele;
  centerId += 1;
  updateProj();

  if (coordType == COORD_ENU) {
    {
      mutexlock lck(roaddataMutex);
      std::unordered_set<laneboundarypkid> boundaryIds;
      for (auto itr = roadDatas.begin(); itr != roadDatas.end(); ++itr) {
        // setRoadGeomToENU( itr->second, lonlatele );
        itr->second->transfer(oldEnuC, lonlatele);
        txSections& sections = itr->second->getSections();
        for (auto secPtr : sections) {
          txLanes& lanes = secPtr->getLanes();
          for (auto lanePtr : lanes) {
            // setLaneGeomToENU( lanePtr, lonlatele );
            lanePtr->transfer(oldEnuC, lonlatele);
            if (lanePtr->getLeftBoundaryId() != BOUN_PKID_INVALID &&
                boundaryIds.find(lanePtr->getLeftBoundaryId()) == boundaryIds.end()) {
              boundaryIds.insert(lanePtr->getLeftBoundaryId());
              // setBoundaryGeomToENU( lanePtr->getLeftBoundary(), lonlatele );
              lanePtr->getLeftBoundary()->transfer(oldEnuC, lonlatele);
            }
            if (lanePtr->getRightBoundaryId() != BOUN_PKID_INVALID &&
                boundaryIds.find(lanePtr->getRightBoundaryId()) == boundaryIds.end()) {
              boundaryIds.insert(lanePtr->getRightBoundaryId());
              // setBoundaryGeomToENU( lanePtr->getRightBoundary(), lonlatele );
              lanePtr->getRightBoundary()->transfer(oldEnuC, lonlatele);
            }
          }
        }
      }
    }
    {
      mutexlock lck(objdataMutex);
      for (auto itr = objectDatas.begin(); itr != objectDatas.end(); ++itr) itr->second->transfer(oldEnuC, lonlatele);
    }
    return true;
  } else {
    return false;
  }
}

PRIORITY_TYPE txMap::MapData::priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr) {
  if (curPtr == NULL || cmpPtr == NULL) {
    return PRIORITY_NONE;
  } else if (curPtr->getJuncId() != cmpPtr->getJuncId()) {
    return PRIORITY_NONE;
  } else {
    mutexlock lck(juncDataMutex);
    if (juncDatas.find(curPtr->getJuncId()) == juncDatas.end()) {
      throw std::runtime_error("Priority cmp error, can not find specified junction");
    }
    txJunctionPtr juncPtr = juncDatas[curPtr->getJuncId()];

    if (!juncPtr->isInteraction(curPtr->getId(), cmpPtr->getId())) return PRIORITY_NONE;

    int r = juncPtr->priorityCmp(curPtr->getId(), cmpPtr->getId());
    if (r == 0) {
      return PRIORITY_SAME;
    } else if (r > 0) {
      return PRIORITY_HIGH;
    } else {
      return PRIORITY_LOW;
    }
  }
}

bool txMap::MapData::removeRoad(roadpkid roadId, bool related) {
  if (roadId == ROAD_PKID_INVALID) return false;

  std::vector<pkid_t> curLaneGeomIds;
  {
    mutexlock lck(roaddataMutex);
    if (roadDatas.find(roadId) == roadDatas.end()) return false;
    roadgeomIdEx.getData2Geom(roadId, curLaneGeomIds);
  }

  // remove related objects
  std::vector<objectpkid> relatedObjs;
  {
    mutexlock lck(roaddataMutex);
    if (roadRelObjects.find(roadId) != roadRelObjects.end()) {
      relatedObjs.assign(roadRelObjects[roadId].begin(), roadRelObjects[roadId].end());
      roadRelObjects.erase(roadId);
    }
  }
  removeObjects(relatedObjs);

  // remove related lanelinks
  if (false) {
    removeLinks(roadId, ROAD_PKID_INVALID);
    removeLinks(ROAD_PKID_INVALID, roadId);
  }
  // remove lane geom from rtree
  for (auto geomId : curLaneGeomIds) spIndex.remove(LANE_SP, geomId);

  // remove road info
  mutexlock lck(roaddataMutex);
  roadgeomIdEx.removeData2Geom(roadId);
  for (auto geomId : curLaneGeomIds) roadgeomIdEx.removeGeom2Data(geomId);

  roadDatas.erase(roadId);
  txlog::debug("txMap::MapData::removeRoad,roadDatas.size()=" + std::to_string(roadDatas.size()));
  return true;
}

bool txMap::MapData::removeRoads(const std::vector<roadpkid>& roadIds) {
  bool r = true;
  for (auto id : roadIds) r &= removeRoad(id);
  return r;
}

bool txMap::MapData::removeLaneLink(lanelinkpkid lanelinkid) {
  if (linkDatas.find(lanelinkid) == linkDatas.end()) return false;
  mutexlock lck(linkdataMutex);
  pkid_t curGeomId;
  linkgeomIdEx.getData2Geom(lanelinkid, curGeomId);
  // remove link geom from rtree
  spIndex.remove(LINK_SP, curGeomId);

  // remove link index
  txLaneLinkPtr ptr = linkDatas.find(lanelinkid)->second;
  if (!ptr) return false;
  roadpkid fid = ptr->fromRoadId();
  roadpkid tid = ptr->toRoadId();

  // remove toroad
  if (toLinkDatas.find(tid) == toLinkDatas.end() || toLinkDatas[tid].find(lanelinkid) == toLinkDatas[tid].end()) {
    return false;
  }
  toLinkDatas[tid].erase(lanelinkid);
  // remove tolanelink
  if (fromLinkDatas.find(fid) == fromLinkDatas.end() ||
      fromLinkDatas[fid].find(lanelinkid) == fromLinkDatas[fid].end()) {
    return false;
  }
  toLinkDatas[fid].erase(lanelinkid);
  // remove geom info
  linkgeomIdEx.removeData2Geom(lanelinkid);
  linkgeomIdEx.removeGeom2Data(curGeomId);
  // remove link ins
  linkDatas.erase(lanelinkid);
  return true;
}

// linkdataMutex only
bool txMap::MapData::removeLinks(roadpkid fromRoadId, roadpkid toRoadId) {
  if (fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID) {
    // remove all links
    mutexlock lck(linkdataMutex);
    linkDatas.clear();
    fromLinkDatas.clear();
    toLinkDatas.clear();
    linkgeomIdEx.clear();
    spIndex.removeAll(LINK_SP);
    return true;
  } else {
    std::vector<lanelinkpkid> removedLinkIds;
    std::vector<pkid_t> removedLinkGeomIds;
    {
      mutexlock lck(linkdataMutex);
      if (fromRoadId != ROAD_PKID_INVALID) {
        if (fromLinkDatas.find(fromRoadId) == fromLinkDatas.end()) return false;
        std::unordered_set<lanelinkpkid>& fromIds = fromLinkDatas[fromRoadId];
        if (toRoadId == ROAD_PKID_INVALID) {
          removedLinkIds.assign(fromIds.begin(), fromIds.end());
        } else {
          for (auto id : fromIds) {
            if (linkDatas[id]->toRoadId() == toRoadId) removedLinkIds.push_back(linkDatas[id]->getId());
          }
        }
      } else {
        // fromRoadId == ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID
        if (toLinkDatas.find(toRoadId) == toLinkDatas.end()) return false;
        std::unordered_set<lanelinkpkid>& toIds = toLinkDatas[toRoadId];
        removedLinkIds.assign(toIds.begin(), toIds.end());
      }
      if (removedLinkIds.empty()) return false;
      for (auto id : removedLinkIds) {
        pkid_t curGeomId;
        linkgeomIdEx.getData2Geom(id, curGeomId);
        removedLinkGeomIds.push_back(curGeomId);
      }
    }

    // remove link geom from rtree
    for (auto geomId : removedLinkGeomIds) spIndex.remove(LINK_SP, geomId);

    mutexlock lck(linkdataMutex);
    // remove link index
    if (fromRoadId != ROAD_PKID_INVALID) {
      fromLinkDatas[fromRoadId].clear();
    } else {
      for (auto id : removedLinkIds) fromLinkDatas[linkDatas[id]->fromRoadId()].erase(id);
    }

    if (toRoadId != ROAD_PKID_INVALID) {
      toLinkDatas[toRoadId].clear();
    } else {
      for (auto id : removedLinkIds) toLinkDatas[linkDatas[id]->toRoadId()].erase(id);
    }
    for (size_t i = 0; i < removedLinkIds.size(); ++i) {
      // remove geom info
      linkgeomIdEx.removeData2Geom(removedLinkIds[i]);
      linkgeomIdEx.removeGeom2Data(removedLinkGeomIds[i]);

      // remove link ins
      linkDatas.erase(removedLinkIds[i]);
    }
    return true;
  }
}

// objdataMutex only
bool txMap::MapData::removeObject(objectpkid objectId) {
  pkid_t objGeomId = PKID_INVALID;
  {
    mutexlock lck(objdataMutex);
    if (objectDatas.find(objectId) != objectDatas.end()) {
      objectgeomIdEx.getData2Geom(objectId, objGeomId);
    } else {
      return false;
    }
  }

  if (objGeomId != PKID_INVALID) {
    // remove obj geom from rtree first
    spIndex.remove(OBJ_SP, objGeomId);

    // remove obj geom info
    mutexlock lck(objdataMutex);
    objectgeomIdEx.removeData2Geom(objectId);
    objectgeomIdEx.removeGeom2Data(objGeomId);

    // remove obj related info
    txObjectPtr curObjPtr = objectDatas[objectId];

    std::vector<txLaneId> reliedIds;
    curObjPtr->getReliedLaneIds(reliedIds);
    for (auto& id : reliedIds) {
      if (roadRelObjects.find(id.roadId) != roadRelObjects.end()) roadRelObjects[id.roadId].erase(objectId);
    }

    // remove obj info
    objectDatas.erase(objectId);

    return true;
  } else {
    return false;
  }
}

bool txMap::MapData::removeObjects(const std::vector<objectpkid>& objectIds) {
  bool r = true;
  for (auto id : objectIds) r &= removeObject(id);
  return r;
}

bool txMap::MapData::removeJunction(junctionpkid jId) {
  mutexlock lck(juncDataMutex);
  if (juncDatas.find(jId) == juncDatas.end()) return false;
  juncDatas.erase(jId);
  return true;
}

txMap::txMap(CoordType type, const txPoint& centerLL) : instancePtr(new MapData(type, centerLL)) {}

txMap::~txMap() {}

// get info

bool txMap::nearestLane(const txPoint& loc, double radius, txLanePtr& lanePtr) {
  return instancePtr->nearestLane(loc, radius, lanePtr);
}

bool txMap::nextLanes(const txLaneId& curLaneId, txLanes& lanes) { return instancePtr->nextLanes(curLaneId, lanes); }

bool txMap::specLanes(const txLaneId& specLaneId, txLanes& lanes) { return instancePtr->specLanes(specLaneId, lanes); }

bool txMap::specBoundaries(const std::vector<laneboundarypkid>& ids, txLaneBoundaries& boundaries) {
  return instancePtr->specBoundaries(ids, boundaries);
}

bool txMap::specLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, txLaneLinks& links) {
  return instancePtr->specLinks(fromRoadId, toRoadId, links);
}

bool txMap::specLinks(const txLaneId& fromLaneId, const txLaneId& toLaneId, txLaneLinks& links) {
  return instancePtr->specLinks(fromLaneId, toLaneId, links);
}

bool txMap::specLinks(const std::vector<lanelinkpkid>& link_ids, txLaneLinks& links) {
  return instancePtr->specLinks(link_ids, links);
}
bool txMap::specRoadLink(const roadpkid roadid, txLaneLinkPtr& link) { return instancePtr->specRoadLink(roadid, link); }
bool txMap::specRoadLinks(const roadpkid roadid, txLaneLinks& links) {
  return instancePtr->specRoadLinks(roadid, links);
}
bool txMap::specJunction(const junctionpkid& jId, txJunctionPtr& juncPtr) {
  return instancePtr->specJunction(jId, juncPtr);
}

// bool txMap::searchLaneLinks( const PointVec& envelope, txLaneLinks& links )
// {
//   return instancePtr->searchLaneLinks( envelope, links );
// }

bool txMap::searchLanes(const PointVec& envelope, txLanes& lanes) { return instancePtr->searchLanes(envelope, lanes); }

bool txMap::searchBoundaries(const PointVec& envelope, txLaneBoundaries& boundaries) {
  return instancePtr->searchBoundaries(envelope, boundaries);
}

bool txMap::searchRoads(const PointVec& envelope, txRoads& roads) { return instancePtr->searchRoads(envelope, roads); }

bool txMap::getRoads(txRoads& roads) { return instancePtr->getRoads(roads); }

bool txMap::getLanes(txLanes& lanes) { return instancePtr->getLanes(lanes); }

bool txMap::getBoundaries(txLaneBoundaries& boundaries) { return instancePtr->getBoundaries(boundaries); }

bool txMap::getLaneLinks(txLaneLinks& links) { return instancePtr->getLaneLinks(links); }

bool txMap::getObjects(txObjects& objects) { return instancePtr->getObjects(objects); }

bool txMap::getObjects(OBJECT_TYPE type, txObjects& objects) { return instancePtr->getObjects(type, objects); }

bool txMap::searchObjects(const PointVec& envelope, const std::set<OBJECT_TYPE>& types, const txLaneId& relLaneId,
                          txObjects& objects) {
  return instancePtr->searchObjects(envelope, types, relLaneId, objects);
}

bool txMap::searchObjects(const PointVec& envelope, const std::set<OBJECT_SUB_TYPE>& types, const txLaneId& relLaneId,
                          txObjects& objects) {
  return instancePtr->searchObjects(envelope, types, relLaneId, objects);
}

// set info

bool txMap::insertRoad(const txRoadPtr roadPtr) { return instancePtr->insertRoad(roadPtr); }

bool txMap::insertRoads(const txRoads& roads) { return instancePtr->insertRoads(roads); }

bool txMap::insertLaneLink(const txLaneLinkPtr linkPtr) { return instancePtr->insertLaneLink(linkPtr); }

bool txMap::insertLaneLinks(const txLaneLinks& links) { return instancePtr->insertLaneLinks(links); }

bool txMap::insertObject(const txObjectPtr objectPtr) { return instancePtr->insertObject(objectPtr); }

bool txMap::insertObjects(const txObjects& objects) { return instancePtr->insertObjects(objects); }

bool txMap::insertJunction(const txJunctionPtr juncPtr) { return instancePtr->insertJunction(juncPtr); }

bool txMap::insertJunctions(const txJunctions& juncs) {
  bool r = false;
  for (auto juncPtr : juncs) r |= insertJunction(juncPtr);
  return r;
}
// update road
bool txMap::updateRoad(const txRoadPtr& road) { return instancePtr->updateRoad(road); }
// update roads
bool txMap::updateRoads(const txRoads& roads) {
  bool r = false;
  for (auto roadPtr : roads) {
    r |= updateRoad(roadPtr);
  }
  return r;
}
//
// update lanelinks
bool txMap::updateLaneLink(const txLaneLinkPtr& lanelink) { return instancePtr->updateLaneLink(lanelink); }

// update lanelinks
bool txMap::updateLaneLinks(const txLaneLinks& lanelinks) {
  bool r = false;
  for (auto lanelinkPtr : lanelinks) {
    r |= updateLaneLink(lanelinkPtr);
  }
  return r;
}

bool txMap::updateCenter(const txPoint& lonlatele) { return instancePtr->updateCenter(lonlatele); }

bool txMap::indertRefPoint(const hadmap::txPoint& point) {
  instancePtr->ref_point = point;
  return true;
}
// remove
bool txMap::removeRoad(roadpkid roadId) { return instancePtr->removeRoad(roadId); }

bool txMap::removeRoads(const std::vector<roadpkid>& roadIds) { return instancePtr->removeRoads(roadIds); }

bool txMap::removeLinks(const std::vector<lanelinkpkid>& lanelinkids) {
  bool r = false;
  for (auto it : lanelinkids) {
    r |= instancePtr->removeLaneLink(it);
  }
  return r;
}

bool txMap::removeLinks(roadpkid fromRoadId, roadpkid toRoadId) {
  return instancePtr->removeLinks(fromRoadId, toRoadId);
}

bool txMap::removeObject(objectpkid objectId) { return instancePtr->removeObject(objectId); }

bool txMap::removeObjects(const std::vector<objectpkid>& objectIds) { return instancePtr->removeObjects(objectIds); }

bool txMap::removeJunction(junctionpkid jId) { return instancePtr->removeJunction(jId); }

// interface
CoordType txMap::getGeomCoordType() { return instancePtr->coordType; }

txMapInterface::txProjectionConstPtr txMap::getProjection() { return instancePtr->getProjection(); }

txRoadPtr txMap::getRoadById(const roadpkid& roadId) {
  txRoadPtr roadPtr;
  if (instancePtr->getRoadById(roadId, roadPtr)) {
    return roadPtr;
  } else {
    return NULL;
  }
}

txLanePtr txMap::getLaneById(const txLaneId& laneId) {
  txLanes lanes;
  if (instancePtr->specLanes(laneId, lanes)) {
    return lanes.front();
  } else {
    return NULL;
  }
}

txLanes txMap::getLanes(const txPoint& pos, const double& radius) {
  double offset = radius;
  if (instancePtr->coordType == COORD_WGS84) {
    double x = radius, y = radius, z = 0;
    coord_trans_api::enu2lonlat(x, y, z, pos.x, pos.y, 0);
    offset = std::max(std::abs(x - pos.x), std::abs(y - pos.y));
  }
  txLanes lanes;
  PointVec envelope;
  envelope.push_back(txPoint(pos.x - offset, pos.y - offset, pos.z));
  envelope.push_back(txPoint(pos.x + offset, pos.y + offset, pos.z));
  if (instancePtr->searchLanes(envelope, lanes)) {
    return lanes;
  } else {
    return txLanes();
  }
}

txLanePtr txMap::nearestLane(const txPoint& pos, double radius) {
  txLanePtr lanePtr;
  if (instancePtr->nearestLane(pos, radius, lanePtr)) {
    return lanePtr;
  } else {
    return NULL;
  }
}

txLanePtr txMap::nearestLane(const txPoint& pos, double radius, const double& yaw) {
  double yawA = map_util::radian2angle(yaw);
  txLanePtr lanePtr;
  if (instancePtr->nearestLane(pos, radius, yawA, lanePtr)) {
    return lanePtr;
  } else {
    return NULL;
  }
}

txLaneLinkPtr txMap::nearestLaneLink(const txPoint& pos, double radius) {
  txLaneLinkPtr linkPtr;
  if (instancePtr->nearestLaneLink(pos, radius, linkPtr)) {
    return linkPtr;
  } else {
    return NULL;
  }
}

txLanes txMap::getNextLanes(const txLanePtr& lanePtr) {
  txLanes lanes;
  if (lanePtr == NULL || lanePtr->getGeometry() == NULL || lanePtr->getGeometry()->empty()) {
    return lanes;
  } else if (instancePtr->nextLanes(lanePtr->getTxLaneId(), lanes)) {
    return lanes;
  } else {
    return txLanes();
  }
}

txLanes txMap::getPreLanes(const txLanePtr& lanePtr) {
  txLanes lanes;
  if (lanePtr == NULL || lanePtr->getGeometry() == NULL || lanePtr->getGeometry()->empty()) {
    return lanes;
  } else if (instancePtr->preLanes(lanePtr->getTxLaneId(), lanes)) {
    return lanes;
  } else {
    return txLanes();
  }
}

txLanePtr txMap::getLeftLane(const txLanePtr& lanePtr) {
  if (lanePtr == NULL) return NULL;

  txLaneId curLaneId = lanePtr->getTxLaneId();
  if (curLaneId.laneId < 0) {
    curLaneId.laneId += 1;
    if (curLaneId.laneId == 0) curLaneId.laneId = 1;
  } else {
    curLaneId.laneId -= 1;
    if (curLaneId.laneId == 0) curLaneId.laneId = -1;
  }
  return getLaneById(curLaneId);
}

txLanePtr txMap::getRightLane(const txLanePtr& lanePtr) {
  if (lanePtr == NULL) return NULL;

  txLaneId curLaneId = lanePtr->getTxLaneId();
  if (curLaneId.laneId < 0) {
    curLaneId.laneId -= 1;
  } else {
    curLaneId.laneId += 1;
  }
  return getLaneById(curLaneId);
}

bool txMap::getBoundaries(const txPoint& pos, const double& yaw, const double& dis, txLaneBoundaries& out) {
  txLanePtr tag_lane_ptr = nearestLane(pos, 5.0, yaw);
  if (tag_lane_ptr == NULL) {
    return false;
  } else {
    double s, l;
    std::deque<txLaneBoundaries> boundaries_queue;
    if (!tag_lane_ptr->xy2sl(pos.x, pos.y, s, l)) {
      txPoint p;
      s = tag_lane_ptr->getGeometry()->getPassedDistance(pos, p, tag_lane_ptr->getGeometry()->getCoordType());
    }

    std::vector<std::pair<txLanePtr, double> > path;
    // generate front info
    double start = s;
    path.push_back(std::make_pair(tag_lane_ptr, dis));
    while (!path.empty()) {
      std::vector<std::pair<txLanePtr, double> > cur_path;
      txLaneBoundaries boundaries;
      for (auto& path_node : path) {
        txLaneBoundaries cur_boundaries;
        if (!path_node.first->getSection()->getRoad()->isJunction()) {
          if (dis < 0.0) {
            start = start < 0.0 ? path_node.first->getLength() : start;
            cur_boundaries = path_node.first->getSection()->getBoundariesByRange(start + path_node.second, start);
          } else {
            cur_boundaries = path_node.first->getSection()->getBoundariesByRange(start, start + path_node.second);
          }
        }
        boundaries.insert(boundaries.end(), cur_boundaries.begin(), cur_boundaries.end());
        double left = dis > 0.0 ? path_node.second - (path_node.first->getLength() - start) : start + path_node.second;
        if (dis * left > 0.0) {
          txLanes next_lanes = dis > 0.0 ? getNextLanes(path_node.first) : getPreLanes(path_node.first);
          std::unordered_set<roadpkid> rids;
          for (auto& cur_lane_ptr : next_lanes) {
            if (rids.find(cur_lane_ptr->getRoadId()) == rids.end()) {
              rids.insert(cur_lane_ptr->getRoadId());
              cur_path.push_back(std::make_pair(cur_lane_ptr, left));
            }
          }
        }
      }
      if (!boundaries.empty()) boundaries_queue.push_back(boundaries);
      path.swap(cur_path);
      start = dis > 0.0 ? 0.0 : -1.0;
    }
    if (boundaries_queue.empty()) return false;
    // generate out boundaries
    out.assign(boundaries_queue[0].begin(), boundaries_queue[0].end());
    for (size_t i = 1; i < boundaries_queue.size(); ++i) {
      txLaneBoundaries cur_out;
      for (auto& bound_ptr : out) {
        txPoint end_p = dis > 0.0 ? bound_ptr->getGeometry()->getEnd() : bound_ptr->getGeometry()->getStart();
        bool used = false;
        for (auto& next_bound_ptr : boundaries_queue[i]) {
          txPoint start_p =
              dis > 0.0 ? next_bound_ptr->getGeometry()->getStart() : next_bound_ptr->getGeometry()->getEnd();
          double offset = 1e-1;
          if (instancePtr->getCoordType() == COORD_WGS84) offset = 1e-6;
          if (fabs(start_p.x - end_p.x) < offset && fabs(start_p.y - end_p.y) < offset) {
            used = true;
            txLaneBoundaryPtr new_bound_ptr(new txLaneBoundary);
            new_bound_ptr->setLaneMark(next_bound_ptr->getLaneMark());
            PointVec front_pts, back_pts;
            dynamic_cast<const txLineCurve*>(bound_ptr->getGeometry())->getPoints(front_pts);
            dynamic_cast<const txLineCurve*>(next_bound_ptr->getGeometry())->getPoints(back_pts);

            LANE_MARK mark = bound_ptr->getLaneMark();
            LANE_MARK next_mark = next_bound_ptr->getLaneMark();
            if (dis > 0.0) {
              if (((mark & LANE_MARK_Broken) + (mark & LANE_MARK_Broken2) + (mark & LANE_MARK_Shielded) +
                       (mark & LANE_MARK_Absent) >
                   0) &&
                  ((mark & LANE_MARK_Solid) + (mark & LANE_MARK_Solid2) == 0)) {
                front_pts.assign(back_pts.begin(), back_pts.end());
              } else if (((next_mark & LANE_MARK_Broken) + (next_mark & LANE_MARK_Broken2) +
                              (next_mark & LANE_MARK_Shielded) + (next_mark & LANE_MARK_Absent) >
                          0) &&
                         ((next_mark & LANE_MARK_Solid) + (next_mark & LANE_MARK_Solid2) == 0)) {
                new_bound_ptr->setLaneMark(bound_ptr->getLaneMark());
              } else {
                front_pts.insert(front_pts.end(), back_pts.begin() + 1, back_pts.end());
                new_bound_ptr->setGeometry(front_pts, instancePtr->getCoordType());
              }
            } else {
              if (((mark & LANE_MARK_Broken) + (mark & LANE_MARK_Broken2) + (mark & LANE_MARK_Shielded) +
                       (mark & LANE_MARK_Absent) >
                   0) &&
                  ((mark & LANE_MARK_Solid) + (mark & LANE_MARK_Solid2) == 0)) {
              } else if (((next_mark & LANE_MARK_Broken) + (next_mark & LANE_MARK_Broken2) +
                              (next_mark & LANE_MARK_Shielded) + (next_mark & LANE_MARK_Absent) >
                          0) &&
                         ((next_mark & LANE_MARK_Solid) + (next_mark & LANE_MARK_Solid2) == 0)) {
                new_bound_ptr->setLaneMark(bound_ptr->getLaneMark());
                back_pts.assign(front_pts.begin(), front_pts.end());
              } else {
                back_pts.insert(back_pts.end(), front_pts.begin() + 1, front_pts.end());
              }
              new_bound_ptr->setGeometry(back_pts, instancePtr->getCoordType());
            }
            cur_out.push_back(new_bound_ptr);
          }
        }
        if (!used) {
          cur_out.push_back(bound_ptr);
        }
      }
      out.swap(cur_out);
    }

    for (auto itr = out.begin(); itr != out.end();) {
      LANE_MARK mark = (*itr)->getLaneMark();

      if (((mark & LANE_MARK_Broken) + (mark & LANE_MARK_Broken2) + (mark & LANE_MARK_Shielded) +
               (mark & LANE_MARK_Absent) >
           0) &&
          ((mark & LANE_MARK_Solid) + (mark & LANE_MARK_Solid2) == 0)) {
        itr = out.erase(itr);
      } else {
        itr++;
      }
    }

    return true;
  }
}

txLaneBoundaries txMap::getLaneBoundaries(const txPoint& pos, const double& yaw,
                                          const std::pair<double, double>& range) {
  txLaneBoundaries pre_bounds, next_bounds;
  if (range.first < -1.0) getBoundaries(pos, yaw, range.first, pre_bounds);
  if (range.second > 1.0) getBoundaries(pos, yaw, range.second, next_bounds);
  if (pre_bounds.empty()) {
    return next_bounds;
  } else if (next_bounds.empty()) {
    return pre_bounds;
  } else {
    txLaneBoundaries bounds;
    for (auto& pre_bound_ptr : pre_bounds) {
      txPoint end_p = pre_bound_ptr->getGeometry()->getEnd();
      bool used = false;
      for (auto& next_bound_ptr : next_bounds) {
        txPoint start_p = next_bound_ptr->getGeometry()->getStart();
        double offset = 1e-1;
        if (instancePtr->getCoordType() == COORD_WGS84) offset = 1e-6;
        if (fabs(start_p.x - end_p.x) < offset && fabs(start_p.y - end_p.y) < offset) {
          used = true;
          txLaneBoundaryPtr new_bound_ptr(new txLaneBoundary);
          PointVec front_pts, back_pts;
          dynamic_cast<const txLineCurve*>(pre_bound_ptr->getGeometry())->getPoints(front_pts);
          dynamic_cast<const txLineCurve*>(next_bound_ptr->getGeometry())->getPoints(back_pts);
          front_pts.insert(front_pts.end(), back_pts.begin() + 1, back_pts.end());
          new_bound_ptr->setGeometry(front_pts, instancePtr->getCoordType());
          bounds.push_back(new_bound_ptr);
        }
      }
    }
    return bounds;
  }
}

txObjectPtr txMap::getObject(const objectpkid& objId) {
  txObjectPtr objPtr;
  if (instancePtr->getObject(objId, objPtr)) {
    return objPtr;
  } else {
    return NULL;
  }
}

txObjectPtr txMap::getObject(const txPoint& pos, const OBJECT_TYPE& obj_type) {
  if (obj_type != OBJECT_TYPE_CrossWalk && obj_type != OBJECT_TYPE_DrivingArea &&
      obj_type != OBJECT_TYPE_ParkingSpace) {
    return NULL;
  }
  PointVec envelope(2, pos);
  double search_radius = 5.0;
  bool wgs84 = false;
  if (instancePtr->coordType == COORD_WGS84) {
    double x = search_radius, y = search_radius, z = 0;
    coord_trans_api::enu2lonlat(x, y, z, pos.x, pos.y, 0);
    search_radius = std::max(std::abs(x - pos.x), std::abs(y - pos.y));

    envelope[0].x -= search_radius;
    envelope[0].y -= search_radius;
    envelope[1].x += search_radius;
    envelope[1].y += search_radius;
    wgs84 = true;
  } else {
    envelope[0].x -= search_radius;
    envelope[0].y -= search_radius;
    envelope[1].x += search_radius;
    envelope[1].y += search_radius;
  }
  txObjects area_objs = getObjects(envelope, std::vector<OBJECT_TYPE>(1, obj_type));
  Point2d p(pos.x, pos.y);
  for (auto& obj_ptr : area_objs) {
    PointVec points;
    dynamic_cast<const txLineCurve*>(obj_ptr->getGeom()->getGeometry())->getPoints(points);
    Points2d pts(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
      pts[i].x = points[i].x;
      pts[i].y = points[i].y;
    }
    if (map_util::posInPolygon(p, pts, wgs84)) return obj_ptr;
  }
  return NULL;
}

txObjects txMap::getObjects(const PointVec& envelope, const std::vector<OBJECT_TYPE>& objTypes) {
  txObjects objects;
  std::set<OBJECT_TYPE> typeSets;
  for (auto& t : objTypes) typeSets.insert(t);
  if (instancePtr->searchObjects(envelope, typeSets, txLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID),
                                 objects)) {
    return objects;
  } else {
    return txObjects();
  }
}

txObjects txMap::getObjects(const PointVec& envelope, const double& yaw, const std::vector<OBJECT_TYPE>& objTypes) {
  txObjects objects = getObjects(envelope, objTypes);
  txObjects filter_objs;
  for (auto& obj_ptr : objects) {
    const txLineCurve* curve_ptr = dynamic_cast<const txLineCurve*>(obj_ptr->getGeom()->getGeometry());
    PointVec points;
    curve_ptr->getPoints(points);

    double y = map_util::radian2angle(yaw);
    hadmap::Point2d y_dir;
    map_util::yaw2dir(y, true, y_dir);
    switch (obj_ptr->getObjectType()) {
      case OBJECT_TYPE_TrafficSign: {
        txPoint normal;
        if (instancePtr->getCoordType() == COORD_WGS84) {
          coord_trans_api::lonlat2enu(points[0].x, points[0].y, points[0].z, points[1].x, points[1].y, points[1].z);
          coord_trans_api::lonlat2enu(points[2].x, points[2].y, points[2].z, points[1].x, points[1].y, points[1].z);
        } else {
          points[0].x -= points[1].x;
          points[0].y -= points[1].y;
          points[0].z -= points[1].z;
          points[2].x -= points[1].x;
          points[2].y -= points[1].y;
          points[2].z -= points[1].z;
        }
        normal.x = points[2].y * points[0].z - points[2].z * points[0].y;
        normal.y = points[2].z * points[0].x - points[2].x * points[0].z;
        normal.z = points[2].x * points[0].y - points[2].y * points[0].x;

        hadmap::Point2d n_dir(normal.x, normal.y);
        if (map_util::angle(y_dir, n_dir) > 100.0) filter_objs.push_back(obj_ptr);
      } break;
      case OBJECT_TYPE_RoadSign:
      case OBJECT_TYPE_Arrow:
      case OBJECT_TYPE_Text:
      case OBJECT_TYPE_RoadSign_Text:
      case OBJECT_TYPE_LaneBoundary:
      case OBJECT_TYPE_LineSegment_PL: {
        hadmap::Point2d dir;
        size_t front_index = 1;
        size_t back_index = 2;
#ifdef DATAMODEL_CLIENT
        front_index = 0;
        back_index = 1;
#endif
        if (OBJECT_TYPE_LineSegment_PL == obj_ptr->getObjectType()) {  // Only two points
          front_index = 1;
          back_index = 0;
        }
        if (instancePtr->getCoordType() == COORD_WGS84) {
          coord_trans_api::lonlat2enu(points[front_index].x, points[front_index].y, points[front_index].z,
                                      points[back_index].x, points[back_index].y, points[back_index].z);
        } else {
          points[front_index].x -= points[back_index].x;
          points[front_index].y -= points[back_index].y;
          points[front_index].z -= points[back_index].z;
        }
        dir.x = points[front_index].x;
        dir.y = points[front_index].y;

        if (map_util::angle(y_dir, dir) < 70.0) filter_objs.push_back(obj_ptr);
      } break;
      default:
        break;
    }
  }
  return filter_objs;
}

txObjects txMap::getObjects(const txPoint& pos, const double& yaw, const std::pair<double, double>& range,
                            const std::vector<OBJECT_TYPE>& objTypes) {
  Point2d dir;
  double ang = map_util::radian2angle(yaw);
  double left_ang = ang - 90.0 > 0.0 ? ang - 90.0 : ang + 270.0;
  map_util::yaw2dir(ang, true, dir);

  Point2d left_dir;
  map_util::yaw2dir(left_ang, true, left_dir);

  double radius = 30.0;
  txPoint pre_point(dir.x * range.first, dir.y * range.first, 0.0);
  txPoint next_point(dir.x * range.second, dir.y * range.second, 0.0);
  PointVec points;
  points.push_back(txPoint(pre_point.x + left_dir.x * radius, pre_point.y + left_dir.y * radius, 0.0));
  points.push_back(txPoint(pre_point.x - left_dir.x * radius, pre_point.y - left_dir.y * radius, 0.0));
  points.push_back(txPoint(next_point.x + left_dir.x * radius, next_point.y + left_dir.y * radius, 0.0));
  points.push_back(txPoint(next_point.x - left_dir.x * radius, next_point.y - left_dir.y * radius, 0.0));

  if (instancePtr->coordType == COORD_WGS84) {
    for (auto& point : points) coord_trans_api::enu2lonlat(point.x, point.y, point.z, pos.x, pos.y, pos.z);
  } else {
    for (auto& point : points) {
      point.x += pos.x;
      point.y += pos.y;
      point.z += pos.z;
    }
  }

  PointVec envelope(2, points[0]);
  for (auto& point : points) {
    envelope[0].x = std::min(envelope[0].x, point.x);
    envelope[0].y = std::min(envelope[0].y, point.y);
    envelope[1].x = std::max(envelope[1].x, point.x);
    envelope[1].y = std::max(envelope[1].y, point.y);
  }

  txObjects objs = getObjects(envelope, yaw, objTypes);
  const double HEIGHT_OFFSET = 5.0;
  for (auto itr = objs.begin(); itr != objs.end();) {
    txPoint p = (*itr)->getGeom()->getGeometry()->getStart();
    if (p.z > pos.z + HEIGHT_OFFSET || p.z < pos.z - HEIGHT_OFFSET) {
      itr = objs.erase(itr);
    } else {
      itr++;
    }
  }
  return objs;
}

txObjects txMap::getObjects(const std::vector<OBJECT_TYPE>& objTypes, const std::vector<txLaneId>& relIds) {
  txObjects objects;
  if (instancePtr->getObjects(relIds, objects)) {
    if (objTypes.empty()) return objects;
    std::set<OBJECT_TYPE> typeSets;
    for (auto& type : objTypes) typeSets.insert(type);
    for (auto itr = objects.begin(); itr != objects.end();) {
      if (typeSets.find((*itr)->getObjectType()) == typeSets.end()) {
        itr = objects.erase(itr);
      } else {
        ++itr;
      }
    }
    return objects;
  } else {
    return txObjects();
  }
}

txObjects txMap::getObjects(const std::vector<OBJECT_SUB_TYPE>& objTypes, const std::vector<txLaneId>& relIds) {
  txObjects objects;
  if (instancePtr->getObjects(relIds, objects)) {
    if (objTypes.empty()) return objects;
    std::set<OBJECT_SUB_TYPE> typeSets;
    for (auto& type : objTypes) typeSets.insert(type);
    for (auto itr = objects.begin(); itr != objects.end();) {
      if (typeSets.find((*itr)->getObjectSubType()) == typeSets.end()) {
        itr = objects.erase(itr);
      } else {
        ++itr;
      }
    }
    return objects;
  } else {
    return txObjects();
  }
}

txJunctionPtr txMap::getJunction(const junctionpkid& jId) {
  txJunctionPtr juncPtr;
  if (specJunction(jId, juncPtr)) {
    return juncPtr;
  } else {
    return NULL;
  }
}

bool txMap::getJunctions(txJunctions& juncs) { return instancePtr->getJunctions(juncs); }

txJunctions txMap::getJunctions() {
  txJunctions juncs;
  instancePtr->getJunctions(juncs);
  return juncs;
}

PRIORITY_TYPE txMap::priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr) {
  return instancePtr->priorityCmp(curPtr, cmpPtr);
}

txPoint txMap::getRefPoint() { return instancePtr->ref_point; }

txOdHeaderPtr txMap::getHeader() { return instancePtr->headerptr; }

void txMap::setHeader(const txOdHeaderPtr& header) { instancePtr->headerptr = header; }
}  // namespace hadmap
