// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "mapengine/hadmap_engine.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/search_factory.h"
#include "mapengine/search_interface.h"

#include <algorithm>
#include <ctime>
#include <exception>
#include <functional>
#include <map>
#include <set>
#include "structs/hadmap_curve.h"

namespace hadmap {
struct txMapHandle {
  MAP_DATA_TYPE dataType;
  ISearchPtr searchPtr;

  txMapHandle() : dataType(NONE) {}
};

// A helper struct to check if the txMapHandle is valid
struct handleSafeDetect {
  // Constructor that checks if the txMapHandle is valid
  // @param pHandle: The pointer to txMapHandle
  // @param info: The error message
  handleSafeDetect(txMapHandle* pHandle, const std::string& info) {
    if (NULL == pHandle) throw std::runtime_error(info + ", txMapHandle is Empty");
    if (NULL == pHandle->searchPtr) throw std::runtime_error(info + ", searchPtr is Empty");
    if (pHandle->searchPtr->connSuccess() == false) throw std::runtime_error(info + ", search conn error");
  }
};

// A helper struct to check if the envelope is valid
struct EnvelopeSafeDetect {
  // Constructor that checks if the envelope is valid
  // @param pHandle: The pointer to txMapHandle
  // @param envelope: The envelope to check
  // @param funcName: The function name for error message
  EnvelopeSafeDetect(txMapHandle* pHandle, const hadmap::PointVec& envelope, const std::string& funcName) {
    if (envelope[0].x > envelope[1].x || envelope[0].y > envelope[1].y) {
      throw std::runtime_error(funcName + " by envelope error, envelope error");
    }
  }
};

int hadmapConnect(const char* fileInfo, const MAP_DATA_TYPE& type, txMapHandle** ppHandle) {
  if ((*ppHandle) != NULL) delete (*ppHandle);
  (*ppHandle) = new txMapHandle;
  (*ppHandle)->dataType = type;
  (*ppHandle)->searchPtr = SearchFactory::getInstance()->getSearchInterface(type, fileInfo, true);
  if ((*ppHandle)->searchPtr->connSuccess()) {
    return TX_HADMAP_HANDLE_OK;
  } else {
    delete (*ppHandle);
    (*ppHandle) = NULL;
    return TX_HADMAP_HANDLE_ERROR;
  }
}

int hadmapConnect(const char* fileInfo, const MAP_DATA_TYPE& type, txMapHandle** ppHandle, bool doubleRoad) {
  if ((*ppHandle) != NULL) delete (*ppHandle);
  (*ppHandle) = new txMapHandle;
  (*ppHandle)->dataType = type;
  (*ppHandle)->searchPtr = SearchFactory::getInstance()->getSearchInterface(type, fileInfo, doubleRoad);
  if ((*ppHandle)->searchPtr->connSuccess()) {
    return TX_HADMAP_HANDLE_OK;
  } else {
    delete (*ppHandle);
    (*ppHandle) = NULL;
    return TX_HADMAP_HANDLE_ERROR;
  }
}

int hadmapConnect(
    const char* fileInfo,
    std::vector<std::tuple<hadmap::txPoint /*left_bottom*/, hadmap::txPoint /*right_top*/>> map_range_list,
    const MAP_DATA_TYPE& type, txMapHandle** ppHandle) {
  if ((*ppHandle) != NULL) delete (*ppHandle);
  (*ppHandle) = new txMapHandle;
  (*ppHandle)->dataType = type;
  (*ppHandle)->searchPtr = SearchFactory::getInstance()->getSearchAreaInterface(type, fileInfo, map_range_list);
  if ((*ppHandle)->searchPtr->connSuccess()) {
    return TX_HADMAP_HANDLE_OK;
  } else {
    delete (*ppHandle);
    (*ppHandle) = NULL;
    return TX_HADMAP_HANDLE_ERROR;
  }
}

TXSIMMAP_API int hadmapSave(txMapHandle* ppHandle) {
  if ((ppHandle) == NULL) {
    return TX_HADMAP_HANDLE_ERROR;
  }
  if (true == ppHandle->searchPtr->save()) return 0;
  return -1;
}
int hadmapClose(txMapHandle** ppHandle) {
  if (!ppHandle || !(*ppHandle)) {
    return TX_HADMAP_HANDLE_OK;
  }
  delete (*ppHandle);
  (*ppHandle) = NULL;
  return TX_HADMAP_HANDLE_OK;
}

int hadmapOutput(const char* fileInfo, const MAP_DATA_TYPE& type, txMapHandle* ppHandle) {
  if ((ppHandle) == NULL) {
    return TX_HADMAP_HANDLE_ERROR;
  }
  if (true == ppHandle->searchPtr->saveMap(type, fileInfo)) return 0;
  return -1;
}

std::string version() { return "HAD Map Engine ver 1.0.1"; }

std::string getLastOptInfo(txMapHandle* pHandle) {
  if (NULL == pHandle) throw std::runtime_error("getLastOptInfo error, pHandle is empty");
  return pHandle->searchPtr->getLastOptInfo();
}

int getLane(txMapHandle* pHandle, const hadmap::txPoint& loc, hadmap::txLanePtr& lanePtr, double radius) {
  handleSafeDetect det(pHandle, "getLane by loc");

  return pHandle->searchPtr->getLane(loc, radius, lanePtr);
}

int getLane(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLanePtr& lanePtr) {
  handleSafeDetect det(pHandle, "getLane by id");

  txLanes lanes;
  int code = pHandle->searchPtr->getLanes(laneId, lanes);
  if (TX_HADMAP_DATA_OK == code) lanePtr = lanes.front();
  return code;
}

int getLanes(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLanes& lanes) {
  handleSafeDetect det(pHandle, "getLanes by id");

  return pHandle->searchPtr->getLanes(laneId, lanes);
}

int getLanes(txMapHandle* pHandle, const hadmap::PointVec& envelope, hadmap::txLanes& lanes) {
  handleSafeDetect det(pHandle, "getLanes by envelope");
  EnvelopeSafeDetect envelopeDet(pHandle, envelope, "getLanes");

  return pHandle->searchPtr->getLanes(envelope, lanes);
}

int getLanes(txMapHandle* pHandle, const hadmap::txPoint& center, const double& radius, hadmap::txLanes& lanes) {
  handleSafeDetect det(pHandle, "getLanes by center & radius");

  double deg = radius / 111300;
  hadmap::PointVec envelope;
  envelope.push_back(hadmap::txPoint(center.x - deg, center.y - deg, center.z));
  envelope.push_back(hadmap::txPoint(center.x + deg, center.y + deg, center.z));
  return getLanes(pHandle, envelope, lanes);
}

int getNextLanes(txMapHandle* pHandle, txLanePtr curLanePtr, txLanes& nextLanes) {
  handleSafeDetect det(pHandle, "getNextLanes");
  if (curLanePtr == NULL) return TX_HADMAP_PARAM_ERROR;
  return pHandle->searchPtr->getNextLanes(curLanePtr, nextLanes);
}

int getPrevLanes(txMapHandle* pHandle, txLanePtr curLanePtr, txLanes& prevLanes) {
  handleSafeDetect det(pHandle, "getPrevLanes");
  if (curLanePtr == NULL) return TX_HADMAP_PARAM_ERROR;
  return pHandle->searchPtr->getPrevLanes(curLanePtr, prevLanes);
}

int getLeftLane(txMapHandle* pHandle, txLanePtr curLanePtr, txLanePtr& leftLanePtr) {
  handleSafeDetect det(pHandle, "getLeftLane");
  if (curLanePtr == NULL) return TX_HADMAP_PARAM_ERROR;
  return pHandle->searchPtr->getLeftLane(curLanePtr, leftLanePtr);
}

int getRightLane(txMapHandle* pHandle, txLanePtr curLanePtr, txLanePtr& rightLanePtr) {
  handleSafeDetect det(pHandle, "getRightLane");
  if (curLanePtr == NULL) return TX_HADMAP_PARAM_ERROR;
  return pHandle->searchPtr->getRightLane(curLanePtr, rightLanePtr);
}

int getBoundaryMaxId(txMapHandle* pHandle, laneboundarypkid& boundaryMaxId) {
  handleSafeDetect det(pHandle, "getBoundaryMaxId");
  return pHandle->searchPtr->getBoundaryMaxId(boundaryMaxId);
}

int getBoundary(txMapHandle* pHandle, const laneboundarypkid& boundaryId, hadmap::txLaneBoundaryPtr& boundaryPtr) {
  handleSafeDetect det(pHandle, "getBoundary by id");

  txLaneBoundaries boundaries;
  int code = pHandle->searchPtr->getBoundaries(std::vector<laneboundarypkid>(1, boundaryId), boundaries);
  if (TX_HADMAP_DATA_OK == code) boundaryPtr = boundaries.front();
  return code;
}

int getBoundaries(txMapHandle* pHandle, const std::vector<laneboundarypkid>& ids,
                  hadmap::txLaneBoundaries& boundaries) {
  handleSafeDetect det(pHandle, "getBoundaries by ids");

  return pHandle->searchPtr->getBoundaries(ids, boundaries);
}

int getSections(txMapHandle* pHandle, const hadmap::txSectionId& sectionId, hadmap::txSections& sections) {
  handleSafeDetect det(pHandle, "getSections by id");

  return pHandle->searchPtr->getSections(sectionId, sections);
}

int getRoadMaxId(txMapHandle* pHandle, roadpkid& roadMaxId) {
  handleSafeDetect det(pHandle, "getRoadMaxId");

  return pHandle->searchPtr->getRoadMaxId(roadMaxId);
}

int getRoad(txMapHandle* pHandle, const roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr) {
  handleSafeDetect det(pHandle, "getRoad by id");

  return pHandle->searchPtr->getRoad(roadId, wholeData, roadPtr);
}

int getRoads(txMapHandle* pHandle, const hadmap::PointVec& envelope, const bool& wholeData, hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "getRoads by envelope");
  EnvelopeSafeDetect envelopeDet(pHandle, envelope, "getRoads");

  return pHandle->searchPtr->getRoads(envelope, wholeData, roads);
}

int getRoads(txMapHandle* pHandle, const std::vector<roadpkid>& rids, const bool& wholeData, hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "getRoads by ids");

  return pHandle->searchPtr->getRoads(rids, wholeData, roads);
}

int getRoads(txMapHandle* pHandle, const bool& wholeData, hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "getRoads All");

  return pHandle->searchPtr->getRoads(wholeData, roads);
}

int getJuncInfo(txMapHandle* pHandle, const roadpkid& entranceRoadId, const roadpkid& exitRoadId,
                hadmap::txRoads& roads, hadmap::txLaneLinks& links) {
  handleSafeDetect det(pHandle, "getRoadsByJunc");

  if (entranceRoadId == ROAD_PKID_INVALID && exitRoadId == ROAD_PKID_INVALID) return TX_HADMAP_PARAM_ERROR;

  return pHandle->searchPtr->getJuncInfo(entranceRoadId, exitRoadId, roads, links);
}

int getJuncInfo(txMapHandle* pHandle, const lanelinkpkid& linkId, hadmap::txRoads& roads, hadmap::txLaneLinks& links) {
  handleSafeDetect det(pHandle, "getRoadsByJunc by id");

  if (linkId == LINK_PKID_INVALID) return TX_HADMAP_PARAM_ERROR;

  return pHandle->searchPtr->getJuncInfo(linkId, roads, links);
}

int getLaneLink(txMapHandle* pHandle, const hadmap::txPoint& loc, hadmap::txLaneLinkPtr& lanelinkPtr, double radius) {
  handleSafeDetect det(pHandle, "getLaneLink by loc");
  return pHandle->searchPtr->getLaneLink(loc, radius, lanelinkPtr);
}

int getRoadLaneLink(txMapHandle* pHandle, const hadmap::roadpkid& roadpkid, hadmap::txLaneLinkPtr& lanelinkPtr) {
  handleSafeDetect det(pHandle, "getLaneLink by roadid");
  return pHandle->searchPtr->getRoadLink(roadpkid, lanelinkPtr);
  return 0;
}

int getLaneLink(txMapHandle* pHandle, const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
  handleSafeDetect det(pHandle, "getLaneLink by id");
  return pHandle->searchPtr->getLaneLink(lanelinkId, lanelinkPtr);
}

int getPrevLaneLinks(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "getPrevLaneLink by laneId");
  return pHandle->searchPtr->getPrevLaneLinks(laneId, lanelinks);
}

int getNextLaneLinks(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "getNextLaneLink by laneId");
  return pHandle->searchPtr->getNextLaneLinks(laneId, lanelinks);
}

int getLaneLinks(txMapHandle* pHandle, const roadpkid& fromRoadId, const roadpkid& toRoadId,
                 hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "getLaneLinks by id");

  return pHandle->searchPtr->getLaneLinks(fromRoadId, toRoadId, lanelinks);
}

int getLaneLinks(txMapHandle* pHandle, const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "getLaneLinks by envelope");
  EnvelopeSafeDetect envelopeDet(pHandle, envelope, "getLaneLinks");

  return pHandle->searchPtr->getLaneLinks(envelope, lanelinks);
}

int getLaneLinks(txMapHandle* pHandle, const std::vector<roadpkid>& roadIds, hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "getLaneLinks by envelope");
  if (roadIds.empty()) return TX_HADMAP_PARAM_ERROR;
  return pHandle->searchPtr->getLaneLinks(roadIds, lanelinks);
}

int getLaneLinkMaxId(txMapHandle* pHandle, lanelinkpkid& linkMaxId) {
  handleSafeDetect det(pHandle, "getLaneLinkMaxId");

  return pHandle->searchPtr->getLaneLinkMaxId(linkMaxId);
}

int getObjects(txMapHandle* pHandle, const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_TYPE>& types,
               hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "getObjects by envelope");
  EnvelopeSafeDetect envelopeDet(pHandle, envelope, "getObjects");

  return pHandle->searchPtr->getObjects(envelope, types, objects);
}

int getObjects(txMapHandle* pHandle, const std::vector<hadmap::txLaneId>& laneIds,
               const std::vector<hadmap::OBJECT_TYPE>& types, hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "getObjects by relations");
  return pHandle->searchPtr->getObjects(laneIds, types, objects);
}

int getSubObjects(txMapHandle* pHandle, const hadmap::PointVec& envelope,
                  const std::vector<hadmap::OBJECT_SUB_TYPE>& subtypes, hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "getObjects by relations");

  return pHandle->searchPtr->getObjects(envelope, subtypes, objects);
  return 0;
}

int getSubObjects(txMapHandle* pHandle, const std::vector<hadmap::txLaneId>& laneIds,
                  const std::vector<hadmap::OBJECT_SUB_TYPE>& subtypes, hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "getObjects by relations");

  return pHandle->searchPtr->getObjects(laneIds, subtypes, objects);
}

// get all junction
int getJunctions(txMapHandle* pHandle, hadmap::txJunctions& junctions) {
  handleSafeDetect det(pHandle, "getJunctions by relations");
  return pHandle->searchPtr->getJunctions(junctions);
}
int getJunction(txMapHandle* pHandle, junctionpkid jid, hadmap::txJunctionPtr& junction) {
  handleSafeDetect det(pHandle, "getJunction by id");
  return pHandle->searchPtr->getJunction(jid, junction);
}
int getForwardPoints(txMapHandle* pHandle, const hadmap::txPoint& p, double dis,
                     std::vector<std::pair<double, hadmap::PointVec>>& pointArray, double sampleDis,
                     std::vector<std::vector<hadmap::txLaneId>>* laneIds) {
  std::vector<std::vector<hadmap::txPoint>> paths;
  std::function<void(hadmap::txLaneLinkPtr link, double s0, double s1, std::vector<hadmap::txPoint> path,
                     std::vector<hadmap::txLaneId> laneids)>
      findlink;
  std::function<void(hadmap::txLanePtr lane, double s0, double s1, std::vector<hadmap::txPoint> path,
                     std::vector<hadmap::txLaneId> laneids)>
      findlane;
  findlink = [&](hadmap::txLaneLinkPtr link, double s0, double s1, std::vector<hadmap::txPoint> path,
                 std::vector<hadmap::txLaneId> laneids) {
    auto geo = dynamic_cast<const hadmap::txLineCurve*>(link->getGeometry());
    laneids.push_back(hadmap::txLaneId(link->getId(), SECT_PKID_INVALID, LANE_PKID_INVALID));
    double len = 0;
    if (geo) {
      len = geo->getLength();
      if (std::abs(s1) <= len) {
        hadmap::PointVec points;
        if (s1 < 0) {
          s0 += len;
          s1 += len;
          geo->cut(s1, s0, points);
          std::reverse(points.begin(), points.end());
        } else {
          geo->cut(s0, s1, points);
        }
        path.insert(path.end(), points.begin(), points.end());
        paths.push_back(path);
        if (laneIds) {
          laneIds->push_back(laneids);
        }
        return;
      }
    }
    if (s1 > 0) {
      if (geo) {
        hadmap::PointVec points;
        if (s0 < 0.0001) {
          geo->getPoints(points);
        } else {
          geo->cut(s0, -1, points);
        }
        path.insert(path.end(), points.begin(), points.end());
      }
      s0 = 0;
      s1 -= len;
      hadmap::txLanePtr lane;
      hadmap::getLane(pHandle, link->toTxLaneId(), lane);
      if (lane) {
        findlane(lane, s0, s1, path, laneids);
      }
    } else {
      if (geo) {
        hadmap::PointVec points;
        if (s0 > -0.0001) {
          geo->getPoints(points);
        } else {
          geo->cut(0, s0 + len, points);
        }
        std::reverse(points.begin(), points.end());
        path.insert(path.end(), points.begin(), points.end());
      }
      s0 = 0;
      s1 += len;
      hadmap::txLanePtr lane;
      hadmap::getLane(pHandle, link->fromTxLaneId(), lane);
      if (lane) {
        findlane(lane, s0, s1, path, laneids);
      }
    }
  };
  findlane = [&](hadmap::txLanePtr lane, double s0, double s1, std::vector<hadmap::txPoint> path,
                 std::vector<hadmap::txLaneId> laneids) {
    double len = lane->getLength();
    laneids.push_back(lane->getTxLaneId());
    auto geo = dynamic_cast<const hadmap::txLineCurve*>(lane->getGeometry());
    if (std::abs(s1) <= len) {
      hadmap::PointVec points;
      if (geo) {
        if (s1 < 0) {
          s0 += len;
          s1 += len;
          geo->cut(s1, s0, points);
          std::reverse(points.begin(), points.end());
        } else {
          geo->cut(s0, s1, points);
        }
        path.insert(path.end(), points.begin(), points.end());
      }
      paths.push_back(path);
      if (laneIds) {
        laneIds->push_back(laneids);
      }
      return;
    }
    if (s1 > 0) {
      if (geo) {
        hadmap::PointVec points;
        if (s0 < 0.0001) {
          geo->getPoints(points);
        } else {
          geo->cut(s0, -1, points);
        }
        path.insert(path.end(), points.begin(), points.end());
      }
      s0 = 0;
      s1 -= len;

      hadmap::txLanes nextLanes;
      hadmap::getNextLanes(pHandle, lane, nextLanes);
      if (nextLanes.empty()) {
        hadmap::txLaneLinks lanelinks;
        hadmap::getNextLaneLinks(pHandle, lane->getTxLaneId(), lanelinks);
        if (lanelinks.empty()) {
          paths.push_back(path);
          if (laneIds) {
            laneIds->push_back(laneids);
          }
          return;
        }
        for (auto nl : lanelinks) {
          findlink(nl, s0, s1, path, laneids);
        }
      } else {
        for (auto nl : nextLanes) {
          findlane(nl, s0, s1, path, laneids);
        }
      }
    } else {
      if (geo) {
        hadmap::PointVec points;
        if (s0 > -0.0001) {
          geo->getPoints(points);
        } else {
          geo->cut(0, s0 + len, points);
        }
        std::reverse(points.begin(), points.end());
        path.insert(path.end(), points.begin(), points.end());
      }
      s0 = 0;
      s1 += len;
      hadmap::txLanes prevLanes;
      hadmap::getPrevLanes(pHandle, lane, prevLanes);
      if (prevLanes.empty()) {
        hadmap::txLaneLinks lanelinks;
        hadmap::getPrevLaneLinks(pHandle, lane->getTxLaneId(), lanelinks);
        if (lanelinks.empty()) {
          paths.push_back(path);
          if (laneIds) {
            laneIds->push_back(laneids);
          }
          return;
        }
        for (auto nl : lanelinks) {
          findlink(nl, s0, s1, path, laneids);
        }
      } else {
        for (auto nl : prevLanes) {
          findlane(nl, s0, s1, path, laneids);
        }
      }
    }
  };

  double s, l, yaw;
  hadmap::txLanePtr lane;
  hadmap::getLane(pHandle, p, lane);
  if (lane && lane->xy2sl(p.x, p.y, s, l, yaw)) {
    if (dis < 0) {
      double len = lane->getLength();
      s = s - len;
    }
    std::vector<hadmap::txPoint> path;
    std::vector<hadmap::txLaneId> laneids;
    findlane(lane, s, s + dis, path, laneids);
  } else {
    return TX_HADMAP_DATA_EMPTY;
  }

  pointArray.clear();
  for (const auto& path : paths) {
    hadmap::PointVec pts = path;
    hadmap::txLineCurve linecurve(hadmap::COORD_WGS84);
    linecurve.setCoords(path);
    if (sampleDis > 1e-6) {
      linecurve.sample(sampleDis, pts);
    }
    if (!pts.empty()) {
      pointArray.push_back(std::make_pair(linecurve.getLength(), pts));
    }
  }
  return TX_HADMAP_DATA_OK;
}
int insertRoads(txMapHandle* pHandle, const hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "insertRoads");
  return pHandle->searchPtr->insertRoads(roads);
}
int upDateRoads(txMapHandle* pHandle, const hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "updateRoads");
  return pHandle->searchPtr->updateRoads(roads);
}

int updateRoads(txMapHandle* pHandle, const hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "updateRoads");
  return pHandle->searchPtr->updateRoads(roads);
}

int deleteRoads(txMapHandle* pHandle, const hadmap::txRoads& roads) {
  handleSafeDetect det(pHandle, "deleteRoads");
  return pHandle->searchPtr->deleteRoads(roads);
}

int insertLaneLinks(txMapHandle* pHandle, const hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "insertLaneLinks");
  return pHandle->searchPtr->insertLaneLinks(lanelinks);
}

int updateLaneLinks(txMapHandle* pHandle, const hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "updateLaneLinks");
  return pHandle->searchPtr->updateLaneLinks(lanelinks);
}

int deleteLaneLinks(txMapHandle* pHandle, const hadmap::txLaneLinks& lanelinks) {
  handleSafeDetect det(pHandle, "deleteLaneLinks");
  return pHandle->searchPtr->deleteLaneLinks(lanelinks);
}

int insertObjects(txMapHandle* pHandle, const hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "insertObjects");
  return pHandle->searchPtr->insertObjects(objects);
}

int updateObjects(txMapHandle* pHandle, const hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "updateObjects");
  return pHandle->searchPtr->updateObjects(objects);
}

int deleteObjects(txMapHandle* pHandle, const hadmap::txObjects& objects) {
  handleSafeDetect det(pHandle, "updateObjects");
  return pHandle->searchPtr->deleteObjects(objects);
  return 0;
}

hadmap::txPoint getRefPoint(txMapHandle* pHandle) {
  handleSafeDetect det(pHandle, "getLaneLink by id");
  return pHandle->searchPtr->getRefPoint();
}
int getHeader(txMapHandle* pHandle, txOdHeaderPtr& headerPtr) {
  handleSafeDetect det(pHandle, "get header");
  return pHandle->searchPtr->getHeader(headerPtr);
}

}  // namespace hadmap
