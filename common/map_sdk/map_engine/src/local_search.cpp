// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "mapengine/local_search.h"
#include <inttypes.h>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>
#include "OpenDriveOutput.h"
#include "common/log.h"
#include "common/map_util.h"
#include "common/mapdata_trans.h"
#include "db_factory.h"
#include "mapengine/hadmap_codes.h"

namespace hadmap {
LocalSearch::LocalSearch(MAP_DATA_TYPE type, const std::string& path)
    : dbPath(path), dbPtr(NULL), dbType(type), optInfo("") {
  optInfo = std::string("Load DB Failed");

  if (dbType == SQLITE) {
    dbPtr = dbFactory::getInstance()->getSqliteOp(dbPath);
    if (dbPtr->connect() && dbPtr->initialize()) {
      optInfo = std::string("Load DB successful");
    } else {
      dbFactory::getInstance()->releaseDB(&dbPtr);
      dbPtr = NULL;
    }
  }
}

LocalSearch::~LocalSearch() {
  if (dbPtr == NULL) {
    optInfo = std::string("DB already released");
  } else {
    dbFactory::getInstance()->releaseDB(&dbPtr);
    dbPtr = NULL;
    optInfo = std::string("DB release successful");
  }
}

double calcLaneWidth(const hadmap::txCurve* leftB, const hadmap::txCurve* rightB) {
  if (leftB == NULL || rightB == NULL) return 3.75;

  hadmap::Points3d leftBoundaryGeom, rightBoundaryGeom;
  const hadmap::txLineCurve* leftCurvePtr = dynamic_cast<const hadmap::txLineCurve*>(leftB);
  for (size_t j = 0; j < leftCurvePtr->size(); ++j) {
    hadmap::txPoint tmpP = leftCurvePtr->getPoint(j);
    leftBoundaryGeom.push_back(hadmap::Point3d(tmpP.x, tmpP.y, tmpP.z));
  }
  const hadmap::txLineCurve* rightCurvePtr = dynamic_cast<const hadmap::txLineCurve*>(rightB);
  for (size_t j = 0; j < rightCurvePtr->size(); ++j) {
    hadmap::txPoint tmpP = rightCurvePtr->getPoint(j);
    rightBoundaryGeom.push_back(hadmap::Point3d(tmpP.x, tmpP.y, tmpP.z));
  }
  return map_util::calcLaneWidth(leftBoundaryGeom, rightBoundaryGeom, true);
}

bool LocalSearch::connSuccess() { return dbPtr != NULL; }

bool LocalSearch::saveMap(const int type, std::string filePath) {
  if (MAP_DATA_TYPE(type) == OPENDRIVE) {
    OpenDriveOutput outPut;
    hadmap::txRoads _roads;
    getRoads(true, _roads);
    if (_roads.size() > 0) {
      outPut.initFilePath(filePath.c_str());
      outPut.insertHeader(hadmap::txPoint());
      for (auto it : _roads) {
        // get fromroads and to roads
        hadmap::txLaneLinks links;
        getLaneLinks(it->getId(), ROAD_PKID_INVALID, links);
        if (links.size() > 0) {
          hadmap::txLaneLinkPtr lanelink = links.at(0);
          int junctionId = lanelink->getJunctionId();
          hadmap::txRoadLink _suclink;
          _suclink.Id = std::to_string(junctionId);
          _suclink.Type = "junction";
          it->setToLink(_suclink);
        }
        links.clear();
        getLaneLinks(ROAD_PKID_INVALID, it->getId(), links);
        if (links.size() > 0) {
          hadmap::txLaneLinkPtr lanelink = links.at(0);
          int junctionId = lanelink->getJunctionId();
          hadmap::txRoadLink _suclink;
          _suclink.Id = std::to_string(junctionId);
          _suclink.Type = "junction";
          it->setPreLink(_suclink);
        }
      }
      outPut.insertRoads(_roads);
    }
    // lanelinks
    hadmap::txLaneLinks links;
    roadpkid from_id = ROAD_PKID_INVALID;
    roadpkid to_id = ROAD_PKID_INVALID;
    getLaneLinks(from_id, to_id, links);
    outPut.insertLaneLinks(links);
    // objects
    hadmap::txObjects _objects;
    std::vector<txLaneId> laneids;
    std::vector<OBJECT_TYPE> types;
    hadmap::txObjects objects;
    getObjects(laneids, types, objects);
    outPut.insertObjects(objects, _roads);
  }
  return true;
}

std::string LocalSearch::getLastOptInfo() { return optInfo; }

int LocalSearch::getTopoLanes(txLanePtr curLanePtr, const txPoint& tagPoint, const int& topoFlag, txLanes& topoLanes) {
  PointVec envelope;
  double offset = 1.0 / 111000.0;
  envelope.push_back(txPoint(tagPoint.x - offset, tagPoint.y - offset, 0.0));
  envelope.push_back(txPoint(tagPoint.x + offset, tagPoint.y + offset, 0.0));

  txLanes lanes;
  if (TX_HADMAP_DATA_OK == getLanes(envelope, lanes)) {
    topoLanes.clear();
    for (auto& lanePtr : lanes) {
      if (lanePtr->getRoadId() == curLanePtr->getRoadId() && lanePtr->getSectionId() == curLanePtr->getSectionId()) {
        continue;
      }
      if (lanePtr->getGeometry() == NULL || lanePtr->getGeometry()->empty()) {
        continue;
      }
      txPoint startP;
      if (lanePtr->getId() > 0) {
        if (topoFlag == 1) {
          startP = lanePtr->getGeometry()->getEnd();
        } else {
          startP = lanePtr->getGeometry()->getStart();
        }
      } else {
        if (topoFlag == 1) {
          startP = lanePtr->getGeometry()->getStart();
        } else {
          startP = lanePtr->getGeometry()->getEnd();
        }
      }

      Point3d a(tagPoint.x, tagPoint.y, 0.0);
      Point3d b(startP.x, startP.y, 0.0);
      if (map_util::distanceBetweenPoints(a, b, true) < 0.5) topoLanes.push_back(lanePtr);
    }
  }
  return topoLanes.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int LocalSearch::getLane(const hadmap::txPoint& loc, double radius, hadmap::txLanePtr& lanePtr) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::Point2d locPoint(loc.x, loc.y);
  hadmap::tx_lane_t lane;
  int optCode = dbPtr->getLane(locPoint, lane, radius);
  if (HADMAP_DATA_OK == optCode) {
    mapdata_trans::TxLaneToHadLane(lane, lanePtr);
  }

  return optCode;
}

int LocalSearch::getLanes(const hadmap::txLaneId& laneId, hadmap::txLanes& lanes) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_lane_vec laneVec;
  int optCode = dbPtr->getLanes(laneId.roadId, laneId.sectionId, laneId.laneId, laneVec);
  if (HADMAP_DATA_OK == optCode) {
    lanes.clear();
    for (size_t i = 0; i < laneVec.size(); ++i) {
      hadmap::txLanePtr lanePtr;
      mapdata_trans::TxLaneToHadLane(laneVec[i], lanePtr);
      lanes.push_back(lanePtr);
    }
  }
  return optCode;
}

int LocalSearch::getLanes(const hadmap::PointVec& envelope, hadmap::txLanes& lanes) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::Points2d envelopeGeom;
  envelopeGeom.push_back(hadmap::Point2d(envelope[0].x, envelope[0].y));
  envelopeGeom.push_back(hadmap::Point2d(envelope[1].x, envelope[1].y));
  hadmap::tx_lane_vec laneVec;
  int optCode = dbPtr->getLanes(envelopeGeom, laneVec);
  if (HADMAP_DATA_OK == optCode) {
    lanes.clear();
    for (size_t i = 0; i < laneVec.size(); ++i) {
      hadmap::txLanePtr lanePtr;
      mapdata_trans::TxLaneToHadLane(laneVec[i], lanePtr);
      lanes.push_back(lanePtr);
    }
  }
  return optCode;
}

int LocalSearch::getNextLanes(txLanePtr curLanePtr, txLanes& nextLanes) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  if (curLanePtr->getGeometry() == NULL || curLanePtr->getGeometry()->empty()) return TX_HADMAP_PARAM_ERROR;

  txPoint endP;
  if (curLanePtr->getId() < 0) {
    endP = curLanePtr->getGeometry()->getEnd();
  } else {
    endP = curLanePtr->getGeometry()->getStart();
  }

  return getTopoLanes(curLanePtr, endP, 1, nextLanes);
}

int LocalSearch::getPrevLanes(txLanePtr curLanePtr, txLanes& prevLanes) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  if (curLanePtr->getGeometry() == NULL || curLanePtr->getGeometry()->empty()) return TX_HADMAP_PARAM_ERROR;

  txPoint startP;
  if (curLanePtr->getId() < 0) {
    startP = curLanePtr->getGeometry()->getStart();
  } else {
    startP = curLanePtr->getGeometry()->getEnd();
  }

  return getTopoLanes(curLanePtr, startP, -1, prevLanes);
}

int LocalSearch::getLeftLane(txLanePtr curLanePtr, txLanePtr& leftLanePtr) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneId curLaneId = curLanePtr->getTxLaneId();
  if (curLaneId.laneId > 0) {
    curLaneId.laneId -= 1;
    if (curLaneId.laneId == 0) curLaneId.laneId = -1;
  } else {
    curLaneId.laneId += 1;
    if (curLaneId.laneId == 0) curLaneId.laneId = 1;
  }

  txLanes lanes;
  if (TX_HADMAP_DATA_OK == getLanes(curLaneId, lanes)) {
    leftLanePtr = lanes.front();
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_DATA_EMPTY;
  }
}

int LocalSearch::getRightLane(txLanePtr curLanePtr, txLanePtr& rightLanePtr) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneId curLaneId = curLanePtr->getTxLaneId();
  if (curLaneId.laneId > 0) {
    curLaneId.laneId += 1;
  } else {
    curLaneId.laneId -= 1;
  }

  txLanes lanes;
  if (TX_HADMAP_DATA_OK == getLanes(curLaneId, lanes)) {
    rightLanePtr = lanes.front();
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_DATA_EMPTY;
  }
}

int LocalSearch::getBoundaryMaxId(laneboundarypkid& boundaryMaxId) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  int optCode = dbPtr->getLaneBoundaryMaxId(boundaryMaxId);
  return optCode;
}

int LocalSearch::getBoundaries(const std::vector<laneboundarypkid>& ids, hadmap::txLaneBoundaries& boundaries) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_laneboundary_vec boundaryVec;
  int optCode = dbPtr->getLaneBoundaries(ids, boundaryVec);
  if (HADMAP_DATA_OK == optCode) {
    boundaries.clear();
    for (size_t i = 0; i < boundaryVec.size(); ++i) {
      hadmap::txLaneBoundaryPtr boundaryPtr;
      mapdata_trans::TxBoundaryToHadBoundary(boundaryVec[i], boundaryPtr);
      boundaries.push_back(boundaryPtr);
    }
  }
  return optCode;
}

int LocalSearch::getSections(const hadmap::txSectionId& sectionId, hadmap::txSections& sections) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  if (sectionId.roadId == ROAD_PKID_INVALID) {
    optInfo = std::string("getSections by id error, road id error");
    return TX_HADMAP_PARAM_ERROR;
  }

  // get all lanes data by road id
  hadmap::tx_lane_vec lanes;
  int optCode = dbPtr->getLanes(sectionId.roadId, sectionId.sectionId, LANE_PKID_INVALID, lanes);
  if (HADMAP_DATA_OK == optCode) {
    sections.clear();
    std::set<laneboundarypkid> boundaryIds;
    for (size_t i = 0; i < lanes.size(); ++i) {
      boundaryIds.insert(lanes[i].left_boundary);
      boundaryIds.insert(lanes[i].right_boundary);
    }

    // get boundary data
    std::vector<laneboundarypkid> ids(boundaryIds.begin(), boundaryIds.end());
    hadmap::tx_laneboundary_vec boundaries;
    int optCode = dbPtr->getLaneBoundaries(ids, boundaries);
    if (HADMAP_DATA_OK == optCode) {
      std::map<laneboundarypkid, hadmap::txLaneBoundaryPtr> boundaryMap;
      for (size_t i = 0; i < boundaries.size(); ++i) {
        hadmap::txLaneBoundaryPtr boundaryPtr;
        mapdata_trans::TxBoundaryToHadBoundary(boundaries[i], boundaryPtr);
        boundaryMap.insert(std::make_pair(boundaries[i].pkid, boundaryPtr));
      }

      // set section data
      hadmap::txSectionPtr curSectionPtr;
      sectionpkid curSectionId = lanes[0].section_id;
      for (size_t i = 0; i < lanes.size(); ++i) {
        hadmap::txLanePtr curLanePtr;
        mapdata_trans::TxLaneToHadLane(lanes[i], curLanePtr);
        curLanePtr->setLeftBoundary(boundaryMap[curLanePtr->getLeftBoundaryId()]);
        curLanePtr->setRightBoundary(boundaryMap[curLanePtr->getRightBoundaryId()]);

        // calc lane width
        curLanePtr->setLaneWidth(calcLaneWidth(boundaryMap[curLanePtr->getLeftBoundaryId()]->getGeometry(),
                                               boundaryMap[curLanePtr->getRightBoundaryId()]->getGeometry()));

        if (!curSectionPtr) {
          curSectionPtr.reset(new hadmap::txSection);
          (*curSectionPtr).setRoadId(lanes[i].road_pkid).setId(lanes[i].section_id);
          curSectionId = lanes[i].section_id;
        }
        if (curSectionId == lanes[i].section_id) {
          curSectionPtr->add(curLanePtr);
        } else {
          sections.push_back(curSectionPtr);
          curSectionPtr.reset();
          i--;
        }
      }
      if (curSectionPtr) sections.push_back(curSectionPtr);
    }
  }
  return optCode;
}

int LocalSearch::getRoadMaxId(roadpkid& roadMaxId) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  int optCode = dbPtr->getRoadMaxId(roadMaxId);
  return optCode;
}

int LocalSearch::getRoad(const roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  if (roadId == ROAD_PKID_INVALID) {
    optInfo = std::string("getRoad by id error, road id error");
    return TX_HADMAP_HANDLE_ERROR;
  }

  hadmap::tx_road_t road;
  int optCode = dbPtr->getRoad(roadId, road);
  if (HADMAP_DATA_OK == optCode) {
    mapdata_trans::TxRoadToHadRoad(road, roadPtr);
    if (wholeData) {
      hadmap::txSections sections;
      getSections(hadmap::txSectionId(roadId, SECT_PKID_INVALID), sections);
      if (HADMAP_DATA_OK == optCode) {
        for (size_t i = 0; i < sections.size(); ++i) roadPtr->addSection(sections[i]);
      }
    }
  }
  return optCode;
}

int LocalSearch::getRoads(const hadmap::PointVec& envelope, const bool& wholeData, hadmap::txRoads& roads) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::Points2d envelopeGeom;
  envelopeGeom.push_back(hadmap::Point2d(envelope[0].x, envelope[0].y));
  envelopeGeom.push_back(hadmap::Point2d(envelope[1].x, envelope[1].y));

  roads.clear();
  hadmap::tx_road_vec roadVec;
  int optCode = dbPtr->getRoads(envelopeGeom, roadVec);
  if (HADMAP_DATA_OK == optCode) {
    for (size_t i = 0; i < roadVec.size(); ++i) {
      hadmap::txRoadPtr roadPtr;
      mapdata_trans::TxRoadToHadRoad(roadVec[i], roadPtr);
      if (wholeData) {
        hadmap::txSections sections;
        getSections(hadmap::txSectionId(roadPtr->getId(), SECT_PKID_INVALID), sections);
        if (HADMAP_DATA_OK == optCode) {
          for (size_t i = 0; i < sections.size(); ++i) roadPtr->addSection(sections[i]);
        } else {
          txlog::printf("%" PRIu64 " get sections error\n", roadPtr->getId());
        }
      }
      roads.push_back(roadPtr);
    }
  }
  return optCode;
}

int LocalSearch::getRoads(const std::vector<roadpkid>& rids, const bool& wholeData, hadmap::txRoads& roads) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  if (rids.empty()) return getRoads(wholeData, roads);

  roads.clear();
  for (auto& id : rids) {
    txRoadPtr roadPtr;
    if (TX_HADMAP_DATA_OK == getRoad(id, wholeData, roadPtr)) roads.push_back(roadPtr);
  }

  return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int LocalSearch::getRoads(const bool& wholeData, hadmap::txRoads& roads) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  roads.clear();
  hadmap::tx_road_vec roadVec;
  int optCode = dbPtr->getRoads(roadVec);
  if (HADMAP_DATA_OK == optCode) {
    for (size_t i = 0; i < roadVec.size(); ++i) {
      hadmap::txRoadPtr roadPtr;
      mapdata_trans::TxRoadToHadRoad(roadVec[i], roadPtr);
      if (wholeData) {
        hadmap::txSections sections;
        getSections(hadmap::txSectionId(roadPtr->getId(), SECT_PKID_INVALID), sections);
        if (HADMAP_DATA_OK == optCode) {
          for (size_t i = 0; i < sections.size(); ++i) roadPtr->addSection(sections[i]);
        }
      }
      roads.push_back(roadPtr);
    }
  }
  return optCode;
}

int LocalSearch::getJuncInfo(const roadpkid& entranceRoadId, const roadpkid& exitRoadId, hadmap::txRoads& roads,
                             hadmap::txLaneLinks& links) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  std::unordered_set<roadpkid> entranceIds;
  std::unordered_set<roadpkid> exitIds;
  if (entranceRoadId != ROAD_PKID_INVALID) {
    std::vector<std::pair<roadpkid, lanepkid> > curEntranceIds;
    std::vector<std::pair<roadpkid, lanepkid> > curExitIds;

    txLaneLinks entranceLinks;
    getLaneLinks(entranceRoadId, ROAD_PKID_INVALID, entranceLinks);
    curEntranceIds.push_back(std::make_pair(entranceRoadId, entranceLinks.front()->fromLaneId()));
    curExitIds.push_back(std::make_pair(entranceRoadId, entranceLinks.front()->fromLaneId() * -1));

    intersectionRoads(curEntranceIds, curExitIds, entranceIds, exitIds);
  }
  if (exitRoadId != ROAD_PKID_INVALID) {
    if (entranceRoadId != ROAD_PKID_INVALID) {
      if (entranceRoadId != exitRoadId && exitIds.find(exitRoadId) == exitIds.end()) return TX_HADMAP_PARAM_ERROR;
    } else {
      std::vector<std::pair<roadpkid, lanepkid> > curEntranceIds;
      std::vector<std::pair<roadpkid, lanepkid> > curExitIds;

      txLaneLinks exitLinks;
      getLaneLinks(ROAD_PKID_INVALID, exitRoadId, exitLinks);
      curExitIds.push_back(std::make_pair(exitRoadId, exitLinks.front()->toLaneId()));
      curEntranceIds.push_back(std::make_pair(exitRoadId, exitLinks.front()->toLaneId() * -1));

      intersectionRoads(curEntranceIds, curExitIds, entranceIds, exitIds);
    }
  }

  entranceIds.insert(exitIds.begin(), exitIds.end());
  std::vector<roadpkid> juncRIds(entranceIds.begin(), entranceIds.end());
  getRoads(juncRIds, true, roads);

  txLaneLinks relatedLinks;
  getLaneLinks(juncRIds, relatedLinks);
  links.clear();
  for (auto& linkPtr : relatedLinks) {
    if (entranceIds.find(linkPtr->fromRoadId()) != entranceIds.end() &&
        entranceIds.find(linkPtr->toRoadId()) != entranceIds.end())
      links.push_back(linkPtr);
  }

  return (roads.empty() || links.empty()) ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int LocalSearch::getJuncInfo(const lanelinkpkid& linkId, hadmap::txRoads& roads, hadmap::txLaneLinks& links) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  tx_lanelink_t curLink;
  if (HADMAP_DATA_OK == dbPtr->getLaneLink(linkId, curLink)) {
    roadpkid fromRId = curLink.from_road_pkid;
    lanepkid fromLId = curLink.from_lane_id;
    roadpkid toRId = curLink.to_road_pkid;
    lanepkid toLId = curLink.to_lane_id;

    std::vector<std::pair<roadpkid, lanepkid> > curEntranceIds;
    std::vector<std::pair<roadpkid, lanepkid> > curExitIds;
    curEntranceIds.push_back(std::make_pair(fromRId, fromLId));
    curExitIds.push_back(std::make_pair(fromRId, fromLId * -1));

    std::unordered_set<roadpkid> entranceIds;
    std::unordered_set<roadpkid> exitIds;
    intersectionRoads(curEntranceIds, curExitIds, entranceIds, exitIds);

    std::unordered_set<roadpkid> totalIds;
    totalIds.insert(exitIds.begin(), exitIds.end());
    totalIds.insert(entranceIds.begin(), entranceIds.end());
    std::vector<roadpkid> juncRIds(totalIds.begin(), totalIds.end());
    getRoads(juncRIds, true, roads);

    txLaneLinks relatedLinks;
    getLaneLinks(juncRIds, relatedLinks);
    links.clear();
    for (auto& linkPtr : relatedLinks) {
      if (entranceIds.find(linkPtr->fromRoadId()) != entranceIds.end() &&
          exitIds.find(linkPtr->toRoadId()) != exitIds.end())
        links.push_back(linkPtr);
    }

    return (roads.empty() || links.empty()) ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_PARAM_ERROR;
  }
}

void LocalSearch::intersectionRoads(const std::vector<std::pair<roadpkid, lanepkid> >& initEntranceIds,
                                    const std::vector<std::pair<roadpkid, lanepkid> >& initExitIds,
                                    std::unordered_set<roadpkid>& entranceIds, std::unordered_set<roadpkid>& exitIds) {
  std::vector<std::pair<roadpkid, lanepkid> > curEntranceIds(initEntranceIds.begin(), initEntranceIds.end());
  std::vector<std::pair<roadpkid, lanepkid> > curExitIds(initExitIds.begin(), initExitIds.end());

  entranceIds.clear();
  exitIds.clear();
  for (auto& p : curEntranceIds) entranceIds.insert(p.first);
  for (auto& p : curExitIds) exitIds.insert(p.first);

  while (!curEntranceIds.empty() || !curExitIds.empty()) {
    std::vector<std::pair<roadpkid, lanepkid> > nextEntranceIds;
    std::vector<std::pair<roadpkid, lanepkid> > nextExitIds;
    for (auto& p : curEntranceIds) {
      txLaneLinks links;
      if (TX_HADMAP_DATA_OK == getLaneLinks(p.first, ROAD_PKID_INVALID, links)) {
        for (auto& linkPtr : links) {
          if (linkPtr->fromLaneId() * p.second < 0) continue;
          if (exitIds.find(linkPtr->toRoadId()) == exitIds.end()) {
            exitIds.insert(linkPtr->toRoadId());
            nextExitIds.push_back(std::make_pair(linkPtr->toRoadId(), linkPtr->toLaneId()));
            nextEntranceIds.push_back(std::make_pair(linkPtr->toRoadId(), linkPtr->toLaneId() * -1));
          }
        }
      }
    }
    for (auto& p : curExitIds) {
      txLaneLinks links;
      if (TX_HADMAP_DATA_OK == getLaneLinks(ROAD_PKID_INVALID, p.first, links)) {
        for (auto& linkPtr : links) {
          if (linkPtr->toLaneId() * p.second < 0) continue;
          if (entranceIds.find(linkPtr->fromRoadId()) == entranceIds.end()) {
            entranceIds.insert(linkPtr->fromRoadId());
            nextEntranceIds.push_back(std::make_pair(linkPtr->fromRoadId(), linkPtr->fromLaneId()));
            nextExitIds.push_back(std::make_pair(linkPtr->fromRoadId(), linkPtr->fromLaneId() * -1));
          }
        }
      }
    }
    curEntranceIds.swap(nextEntranceIds);
    curExitIds.swap(nextExitIds);
  }
}

int LocalSearch::getLaneLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, hadmap::txLaneLinks& lanelinks) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_lanelink_vec lanelinkVec;
  int optCode = dbPtr->getLaneLinks(fromRoadId, SECT_PKID_INVALID, LANE_PKID_INVALID, toRoadId, SECT_PKID_INVALID,
                                    LANE_PKID_INVALID, lanelinkVec);
  if (HADMAP_DATA_OK == optCode) {
    lanelinks.clear();
    for (size_t i = 0; i < lanelinkVec.size(); ++i) {
      hadmap::txLaneLinkPtr lanelinkPtr;
      mapdata_trans::TxLaneLinkToHadLaneLink(lanelinkVec[i], lanelinkPtr);
      lanelinks.push_back(lanelinkPtr);
    }
  }
  return optCode;
}

int LocalSearch::getLaneLink(const hadmap::txPoint& loc, double radius, hadmap::txLaneLinkPtr& laneLinkPtr) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::Point2d locPoint(loc.x, loc.y);
  hadmap::tx_lanelink_t lanelink;
  int optCode = dbPtr->getLaneLink(locPoint, lanelink);
  if (HADMAP_DATA_OK == optCode) {
    mapdata_trans::TxLaneLinkToHadLaneLink(lanelink, laneLinkPtr);
  }

  return optCode;
}

int LocalSearch::getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_lanelink_t lanelink;
  int optCode = dbPtr->getLaneLink(lanelinkId, lanelink);

  if (HADMAP_DATA_OK == optCode) {
    mapdata_trans::TxLaneLinkToHadLaneLink(lanelink, lanelinkPtr);
  }

  return optCode;
}

int LocalSearch::getRoadLink(const hadmap::roadpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
  return HADMAP_DATA_EMPTY;
}

int LocalSearch::getPrevLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_lanelink_vec lanelinkVec;
  int optCode = dbPtr->getLaneLinks(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID, laneId.roadId,
                                    laneId.sectionId, laneId.laneId, lanelinkVec);
  if (HADMAP_DATA_OK == optCode) {
    lanelinks.clear();
    for (size_t i = 0; i < lanelinkVec.size(); ++i) {
      hadmap::txLaneLinkPtr lanelinkPtr;
      mapdata_trans::TxLaneLinkToHadLaneLink(lanelinkVec[i], lanelinkPtr);
      lanelinks.push_back(lanelinkPtr);
    }
  }
  return optCode;
}

int LocalSearch::getNextLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_lanelink_vec lanelinkVec;
  int optCode = dbPtr->getLaneLinks(laneId.roadId, laneId.sectionId, laneId.laneId, ROAD_PKID_INVALID,
                                    SECT_PKID_INVALID, LANE_PKID_INVALID, lanelinkVec);
  if (HADMAP_DATA_OK == optCode) {
    lanelinks.clear();
    for (size_t i = 0; i < lanelinkVec.size(); ++i) {
      hadmap::txLaneLinkPtr lanelinkPtr;
      mapdata_trans::TxLaneLinkToHadLaneLink(lanelinkVec[i], lanelinkPtr);
      lanelinks.push_back(lanelinkPtr);
    }
  }
  return optCode;
}

int LocalSearch::getLaneLinks(const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::Points2d envelopeGeom;
  envelopeGeom.push_back(hadmap::Point2d(envelope[0].x, envelope[0].y));
  envelopeGeom.push_back(hadmap::Point2d(envelope[1].x, envelope[1].y));

  txRoads roads;
  if (TX_HADMAP_DATA_OK == getRoads(envelope, false, roads)) {
    lanelinks.clear();
    tx_lanelink_vec llinks;
    if (TX_HADMAP_DATA_OK == dbPtr->getLaneLinks(llinks) && llinks.size() > 1000 && roads.size() > 20) {
      std::unordered_set<roadpkid> rIds;
      for (auto& roadPtr : roads) {
        rIds.insert(roadPtr->getId());
      }

      for (auto& linkPtr : llinks) {
        if (rIds.find(linkPtr.from_road_pkid) != rIds.end() || rIds.find(linkPtr.to_road_pkid) != rIds.end()) {
          hadmap::txLaneLinkPtr lanelinkPtr;
          mapdata_trans::TxLaneLinkToHadLaneLink(linkPtr, lanelinkPtr);
          lanelinks.push_back(lanelinkPtr);
          lanelinks.push_back(lanelinkPtr);
        }
      }

    } else {
      std::unordered_set<lanelinkpkid> linkIds;
      for (auto& roadPtr : roads) {
        txLaneLinks curLinks;
        if (TX_HADMAP_DATA_OK == getLaneLinks(roadPtr->getId(), ROAD_PKID_INVALID, curLinks)) {
          for (auto& linkPtr : curLinks) {
            if (linkIds.find(linkPtr->getId()) == linkIds.end()) {
              linkIds.insert(linkPtr->getId());
              lanelinks.push_back(linkPtr);
            }
          }
        }
        if (TX_HADMAP_DATA_OK == getLaneLinks(ROAD_PKID_INVALID, roadPtr->getId(), curLinks)) {
          for (auto& linkPtr : curLinks) {
            if (linkIds.find(linkPtr->getId()) == linkIds.end()) {
              linkIds.insert(linkPtr->getId());
              lanelinks.push_back(linkPtr);
            }
          }
        }
      }
    }
  }
  return lanelinks.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  /*
  lanelinks.clear();
  hadmap::tx_lanelink_vec lanelinkVec;
  int optCode = dbPtr->getLaneLinks(envelopeGeom, lanelinkVec);
  if (HADMAP_DATA_OK == optCode)
  {
          for (size_t i = 0; i < lanelinkVec.size(); ++i)
          {
                  hadmap::txLaneLinkPtr lanelinkPtr;
                  mapdata_trans::TxLaneLinkToHadLaneLink(lanelinkVec[i], lanelinkPtr);
                  lanelinks.push_back(lanelinkPtr);
          }
  }
  return optCode;
  */
}

int LocalSearch::getLaneLinks(const std::vector<roadpkid>& roadIds, txLaneLinks& links) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  std::unordered_set<lanelinkpkid> linkIdSets;
  links.clear();
  for (auto& rid : roadIds) {
    txLaneLinks curLinks;
    if (TX_HADMAP_DATA_OK == getLaneLinks(rid, ROAD_PKID_INVALID, curLinks)) {
      for (auto& curLinkPtr : curLinks) {
        if (linkIdSets.find(curLinkPtr->getId()) == linkIdSets.end()) {
          linkIdSets.insert(curLinkPtr->getId());
          links.push_back(curLinkPtr);
        }
      }
    }
    if (TX_HADMAP_DATA_OK == getLaneLinks(ROAD_PKID_INVALID, rid, curLinks)) {
      for (auto& curLinkPtr : curLinks) {
        if (linkIdSets.find(curLinkPtr->getId()) == linkIdSets.end()) {
          linkIdSets.insert(curLinkPtr->getId());
          links.push_back(curLinkPtr);
        }
      }
    }
  }
  return links.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int LocalSearch::getLaneLinkMaxId(lanelinkpkid& linkMaxId) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  int optCode = dbPtr->getLaneLinkMaxId(linkMaxId);
  return optCode;
}

void objDataTrans(hadmap::tx_object_vec& objVec, hadmap::tx_object_geom_vec& geomVec,
                  hadmap::tx_object_geom_rel_vec& geomRefVec, hadmap::txObjects& objects) {
  objects.clear();
  std::map<objectpkid, hadmap::txObjectPtr> objMap;
  for (size_t i = 0; i < objVec.size(); ++i) {
    objectpkid objPkid = objVec[i].pkid;
    if (objMap.find(objPkid) == objMap.end()) {
      hadmap::txObjectPtr curObjPtr;
      mapdata_trans::TxObjectToHadObject(objVec[i], curObjPtr);
      objMap.insert(std::make_pair(objPkid, curObjPtr));
    }
    for (int k = 0; k < geomRefVec.size(); k++) {
      if (geomRefVec.at(k).object_pkid == objPkid) {
        hadmap::txObjGeomPtr curGeomPtr;
        mapdata_trans::TxObjGeomToHadObjGeom(geomVec[k], curGeomPtr);
        objMap[objPkid]->addGeom(curGeomPtr);
      }
    }
  }
  for (auto itr = objMap.begin(); itr != objMap.end(); ++itr) objects.push_back(itr->second);
}

void objDataTrans(hadmap::tx_object_vec& objVec, hadmap::tx_object_geom_vec& geomVec,
                  hadmap::tx_object_geom_rel_vec& geomRefVec, hadmap::tx_object_rel_vec& relVec,
                  hadmap::txObjects& objects) {
  std::map<objectpkid, std::vector<txLaneId> > relMap;
  for (size_t i = 0; i < relVec.size(); ++i) {
    if (relMap.find(relVec[i].object_pkid) == relMap.end()) {
      relMap.insert(std::make_pair(relVec[i].object_pkid, std::vector<txLaneId>()));
    }
    relMap[relVec[i].object_pkid].push_back(txLaneId(relVec[i].road_pkid, relVec[i].section_id, relVec[i].lane_id));
  }
  objDataTrans(objVec, geomVec, geomRefVec, objects);
  for (auto& objPtr : objects) {
    if (relMap.find(objPtr->getId()) != relMap.end()) objPtr->setReliedLaneIds(relMap[objPtr->getId()]);
  }
}

int LocalSearch::getObjects(const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_TYPE>& types,
                            hadmap::txObjects& objects) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::Points2d envelopeGeom;
  envelopeGeom.push_back(hadmap::Point2d(envelope[0].x, envelope[0].y));
  envelopeGeom.push_back(hadmap::Point2d(envelope[1].x, envelope[1].y));
  hadmap::tx_object_vec objVec;

  hadmap::tx_object_geom_vec geomVec;
  hadmap::tx_object_rel_vec relVec;
  hadmap::tx_object_geom_rel_vec geomRefVec;
  int optCode = dbPtr->getObjects(envelopeGeom, types, objVec, geomVec, relVec);
  for (auto it : objVec) {
    hadmap::tx_object_geom_rel_vec tmp;
    dbPtr->getObjectRel(it.pkid, tmp);
    for (auto itref : tmp) geomRefVec.push_back(itref);
  }
  if (HADMAP_DATA_OK == optCode) {
    if (dbPtr->isTadsimSqlite()) {
      objDataTrans(objVec, geomVec, geomRefVec, relVec, objects);
      hadmap::tx_od_object_vec od_objVec;
      dbPtr->getOdObjects(objVec, od_objVec);
      std::map<objectpkid, txObjectPtr> map2ObjectPtr;
      for (auto it : objects) {
        map2ObjectPtr[it->getId()] = it;
      }
      for (auto it : od_objVec) {
        if (map2ObjectPtr.find(it.object_pkid) != map2ObjectPtr.end()) {
          // get yaw
          double x, y, yaw;
          if (it.roadid != -1) {
            hadmap::txRoadPtr roadPtr;
            getRoad(it.roadid, false, roadPtr);
            if (roadPtr && roadPtr->getGeometry()) roadPtr->getGeometry()->sl2xy(it.s, it.t, x, y, yaw);
          } else {
            hadmap::txLaneLinkPtr lanelink;
            getLaneLink(it.lanelinkid, lanelink);
            lanelink->getGeometry()->sl2xy(it.s, it.t, x, y, yaw);
          }
          double yawFinaly = it.hdg + yaw * M_PI / 180.0;
          if (it.orientation == 0) {
            yawFinaly += M_PI;
          }
          map2ObjectPtr.find(it.object_pkid)->second->setTxOdData(it);
          map2ObjectPtr.find(it.object_pkid)->second->setRPY(it.roll, it.pitch, yawFinaly);
        }
      }
    }
  }
  return optCode;
}

int LocalSearch::getObjects(const std::vector<hadmap::txLaneId>& laneIds, const std::vector<hadmap::OBJECT_TYPE>& types,
                            hadmap::txObjects& objects) {
  if (dbPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  hadmap::tx_object_vec objVec;
  hadmap::tx_object_geom_vec geomVec;
  hadmap::tx_object_geom_rel_vec geomRefVec;
  hadmap::txObjects curObjs;
  if (laneIds.empty()) {
    int optCode = dbPtr->getObjects(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID, types, objVec, geomVec);
    for (auto it : objVec) {
      hadmap::tx_object_geom_rel_vec tmp;
      dbPtr->getObjectRel(it.pkid, tmp);
      for (auto itref : tmp) geomRefVec.push_back(itref);
    }

    if (HADMAP_DATA_OK == optCode) {
      objDataTrans(objVec, geomVec, geomRefVec, objects);
      if (dbPtr->isTadsimSqlite()) {
        hadmap::tx_od_object_vec od_objVec;
        dbPtr->getOdObjects(objVec, od_objVec);
        std::map<objectpkid, txObjectPtr> map2ObjectPtr;
        for (auto it : objects) {
          map2ObjectPtr[it->getId()] = it;
        }
        for (auto it : od_objVec) {
          if (map2ObjectPtr.find(it.object_pkid) != map2ObjectPtr.end()) {
            double x = 0, y, yaw = 0;
            if (it.roadid != -1) {
              hadmap::txRoadPtr roadPtr;
              getRoad(it.roadid, false, roadPtr);
              if (roadPtr && roadPtr->getGeometry()) roadPtr->getGeometry()->sl2xy(it.s, it.t, x, y, yaw);
            } else {
              hadmap::txLaneLinkPtr lanelink;
              getLaneLink(it.lanelinkid, lanelink);
              lanelink->getGeometry()->sl2xy(it.s, it.t, x, y, yaw);
            }
            double yawFinaly = it.hdg + yaw * M_PI / 180.0;
            if (it.orientation == 0) {
              yawFinaly += M_PI;
            }
            map2ObjectPtr.find(it.object_pkid)->second->setTxOdData(it);
            map2ObjectPtr.find(it.object_pkid)->second->setRPY(it.roll, it.pitch, yawFinaly);
          }
        }
      }
    }
    return optCode;
  } else {
    for (size_t i = 0; i < laneIds.size(); ++i) {
      objVec.clear();
      geomVec.clear();
      curObjs.clear();
      roadpkid roadId = laneIds[i].roadId;
      sectionpkid sectionId = laneIds[i].sectionId;
      lanepkid laneId = laneIds[i].laneId;
      int optCode = dbPtr->getObjects(roadId, sectionId, laneId, types, objVec, geomVec);
      for (auto it : objVec) {
        hadmap::tx_object_geom_rel_vec tmp;
        dbPtr->getObjectRel(it.pkid, tmp);
        for (auto itref : tmp) geomRefVec.push_back(itref);
      }
      if (HADMAP_DATA_OK == optCode) {
        objDataTrans(objVec, geomVec, geomRefVec, curObjs);
        if (dbPtr->isTadsimSqlite()) {
          hadmap::tx_od_object_vec od_objVec;
          dbPtr->getOdObjects(objVec, od_objVec);
          std::map<objectpkid, txObjectPtr> map2ObjectPtr;
          for (auto it : objects) {
            map2ObjectPtr[it->getId()] = it;
          }
          for (auto it : od_objVec) {
            if (map2ObjectPtr.find(it.object_pkid) != map2ObjectPtr.end()) {
              double x, y, yaw;
              if (it.roadid != -1) {
                hadmap::txRoadPtr roadPtr;
                getRoad(it.roadid, false, roadPtr);
                roadPtr->getGeometry()->sl2xy(it.s, it.t, x, y, yaw);
              } else {
                hadmap::txLaneLinkPtr lanelink;
                getLaneLink(it.lanelinkid, lanelink);
                lanelink->getGeometry()->sl2xy(it.s, it.t, x, y, yaw);
              }
              double yawFinaly = it.hdg + yaw * M_PI / 180.0;
              if (it.orientation == 0) {
                yawFinaly += M_PI;
              }
              map2ObjectPtr.find(it.object_pkid)->second->setTxOdData(it);
              map2ObjectPtr.find(it.object_pkid)->second->setRPY(it.roll, it.pitch, yawFinaly);
            }
          }
        }
        objects.insert(objects.end(), curObjs.begin(), curObjs.end());
      }
    }
    if (objects.empty()) {
      return HADMAP_DATA_EMPTY;
    } else {
      return HADMAP_DATA_OK;
    }
  }
}

int LocalSearch::getObjects(const PointVec& envelope, const std::vector<OBJECT_SUB_TYPE>& types, txObjects& objects) {
  return TX_HADMAP_DATA_EMPTY;
}

int LocalSearch::getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_SUB_TYPE>& types,
                            txObjects& objects) {
  return TX_HADMAP_DATA_EMPTY;
}

int LocalSearch::getJunctions(hadmap::txJunctions& junctions) {
  roadpkid fromRoadId = ROAD_PKID_INVALID;
  roadpkid toRoadId = ROAD_PKID_INVALID;
  hadmap::txLaneLinks lanelinks;
  getLaneLinks(fromRoadId, toRoadId, lanelinks);
  std::map<junctionpkid, hadmap::txJunctionPtr> id2Junctions;
  for (auto it : lanelinks) {
    if (id2Junctions.find(it->getJunctionId()) != id2Junctions.end()) {
      id2Junctions.find(it->getJunctionId())->second->insertLaneLink(it->getId());
    } else {
      hadmap::txJunctionPtr ptr(new txJunction(it->getJunctionId()));
      ptr->insertLaneLink(it->getId());
      id2Junctions[it->getJunctionId()] = ptr;
    }
  }
  junctions.clear();
  for (auto it : id2Junctions) {
    junctions.push_back(it.second);
  }
  return HADMAP_DATA_OK;
}

int LocalSearch::getJunction(junctionpkid jid, hadmap::txJunctionPtr& junction) { return HADMAP_DATA_EMPTY; }

int LocalSearch::insertRoads(const hadmap::txRoads& roads) {
  tx_task_t task;
  task.pkid = 1;
  memcpy(task.name, kTadsimXodr, strlen(kTadsimXodr) + 1);
  task.srs = 4326;
  task.x = 0.0;
  task.y = 0.0;
  task.z = 0.0;
  task.heading = 0.0;
  task.pitch = 0.0;
  task.roll = 0.0;
  tx_task_vec t_tasks;
  t_tasks.push_back(task);
  dbPtr->insertTasks(t_tasks);

  hadmap::tx_road_vec t_roads;
  hadmap::tx_lane_vec t_lanes;
  hadmap::tx_laneboundary_vec t_laneboundarys;
  std::set<int> laneBoundaryid;
  for (auto it = roads.begin(); it != roads.end(); it++) {
    tx_road_t roat_t = (*it)->getTxData();
    roat_t.srs = 4326;
    roat_t.pkid = (*it)->getId();
    roat_t.section_num = (*it)->getSections().size();
    t_roads.push_back(roat_t);
    for (auto itSection : (*it)->getSections()) {
      for (auto itlane : (itSection)->getLanes()) {
        tx_lane_t tlane = (itlane)->getTxLaneData();
        tlane.srs = 4326;
        t_lanes.push_back(tlane);
        if (laneBoundaryid.find(itlane->getLeftBoundaryId()) == laneBoundaryid.end()) {
          tx_laneboundary_t boundary = itlane->getLeftBoundary()->getTxData();
          boundary.srs = 4326;
          laneBoundaryid.insert(itlane->getLeftBoundaryId());
          t_laneboundarys.push_back(boundary);
        }
        if (laneBoundaryid.find(itlane->getRightBoundaryId()) == laneBoundaryid.end()) {
          tx_laneboundary_t boundary = itlane->getRightBoundary()->getTxData();
          boundary.srs = 4326;
          laneBoundaryid.insert(itlane->getRightBoundaryId());
          t_laneboundarys.push_back(boundary);
        }
      }
    }
  }
  dbPtr->insertRoads(t_roads);
  dbPtr->insertLaneBoundaries(t_laneboundarys);
  dbPtr->insertLanes(t_lanes);
  return 0;
}

int LocalSearch::updateRoads(const hadmap::txRoads& roads) {
  std::set<int> laneBoundaryid;
  for (auto it = roads.begin(); it != roads.end(); it++) {
    // update road
    tx_road_t roat_t = (*it)->getTxData();
    roat_t.srs = 4326;
    roat_t.pkid = (*it)->getId();
    roat_t.section_num = (*it)->getSections().size();
    dbPtr->updateRoad(roat_t);
    hadmap::txRoadPtr roadPtr;
    getRoad(roat_t.pkid, true, roadPtr);

    int sectionNum = 0;
    // update lane and boundary
    if ((*it)->getSections().size() == roadPtr->getSections().size()) {
      for (auto itSection : (*it)->getSections()) {
        txLanes lanesNeedUpdate;
        txLanes lanesNeedDelete;
        txLanes lanesNeedInsert;
        int currentLaneSize = roadPtr->getSections().at(sectionNum)->getLanes().size();
        int updateLaneSize = itSection->getLanes().size();

        if (currentLaneSize > updateLaneSize) {
          lanesNeedUpdate.assign(itSection->getLanes().begin(), itSection->getLanes().end());
          lanesNeedDelete.assign(roadPtr->getSections().at(sectionNum)->getLanes().begin() + updateLaneSize,
                                 roadPtr->getSections().at(sectionNum)->getLanes().end());
        } else if (currentLaneSize < updateLaneSize) {
          lanesNeedUpdate.assign(itSection->getLanes().begin(), itSection->getLanes().begin() + currentLaneSize);
          lanesNeedInsert.assign(itSection->getLanes().begin() + currentLaneSize, itSection->getLanes().end());
        } else {
          lanesNeedUpdate.assign(itSection->getLanes().begin(), itSection->getLanes().end());
        }

        // need to update lane
        for (auto itlane : lanesNeedUpdate) {
          tx_lane_t tlane = (itlane)->getTxLaneData();
          tlane.srs = 4326;
          if (laneBoundaryid.find(itlane->getLeftBoundaryId()) == laneBoundaryid.end()) {
            tx_laneboundary_t boundary = itlane->getLeftBoundary()->getTxData();
            boundary.srs = 4326;
            laneBoundaryid.insert(itlane->getLeftBoundaryId());
            dbPtr->updateLaneBoundary(boundary);
          }
          if (laneBoundaryid.find(itlane->getRightBoundaryId()) == laneBoundaryid.end()) {
            tx_laneboundary_t boundary = itlane->getRightBoundary()->getTxData();
            boundary.srs = 4326;
            laneBoundaryid.insert(itlane->getRightBoundaryId());
            dbPtr->updateLaneBoundary(boundary);
          }
          dbPtr->updateLane(tlane);
        }
        // need to delete lane
        for (auto itlane : lanesNeedDelete) {
          tx_lane_t tlane = (itlane)->getTxLaneData();
          dbPtr->deleteLane(tlane.road_pkid, tlane.section_id, tlane.lane_id);
          if (laneBoundaryid.find(itlane->getLeftBoundaryId()) == laneBoundaryid.end()) {
            tx_laneboundary_t boundary = itlane->getLeftBoundary()->getTxData();
            boundary.srs = 4326;
            dbPtr->deleteLaneBoundary(boundary.pkid);
          }
          if (laneBoundaryid.find(itlane->getRightBoundaryId()) == laneBoundaryid.end()) {
            tx_laneboundary_t boundary = itlane->getRightBoundary()->getTxData();
            boundary.srs = 4326;
            dbPtr->deleteLaneBoundary(boundary.pkid);
          }
          dbPtr->deleteLane(tlane.road_pkid, tlane.section_id, tlane.lane_id);
        }
        hadmap::tx_laneboundary_vec boundaries;
        hadmap::tx_lane_vec lanes;
        // need to insert lane
        for (auto itlane : lanesNeedInsert) {
          tx_lane_t tlane = (itlane)->getTxLaneData();
          tlane.srs = 4326;
          if (laneBoundaryid.find(itlane->getLeftBoundaryId()) == laneBoundaryid.end()) {
            tx_laneboundary_t boundary = itlane->getLeftBoundary()->getTxData();
            boundary.srs = 4326;
            boundaries.push_back(boundary);
          }
          if (laneBoundaryid.find(itlane->getRightBoundaryId()) == laneBoundaryid.end()) {
            tx_laneboundary_t boundary = itlane->getRightBoundary()->getTxData();
            boundary.srs = 4326;
            boundaries.push_back(boundary);
          }
          lanes.push_back(tlane);
        }
        dbPtr->insertLaneBoundaries(boundaries);
        dbPtr->insertLanes(lanes);
        sectionNum++;
      }
    }
  }
  return true;
}

int LocalSearch::deleteRoads(const hadmap::txRoads& roads) {
  std::set<int> laneBoundaryid;
  for (auto it = roads.begin(); it != roads.end(); it++) {
    // delete lane and boundary
    for (auto itSection : (*it)->getSections()) {
      for (auto itlane : (itSection)->getLanes()) {
        dbPtr->deleteLane((*it)->getId(), itSection->getId(), itlane->getId());
        if (laneBoundaryid.find(itlane->getLeftBoundaryId()) == laneBoundaryid.end()) {
          laneBoundaryid.insert(itlane->getLeftBoundaryId());
          dbPtr->deleteLaneBoundary(itlane->getLeftBoundaryId());
        }
        if (laneBoundaryid.find(itlane->getRightBoundaryId()) == laneBoundaryid.end()) {
          laneBoundaryid.insert(itlane->getRightBoundaryId());
          dbPtr->deleteLaneBoundary(itlane->getRightBoundaryId());
        }
      }
    }
    // delete roads
    dbPtr->deleteRoad((*it)->getId());
  }
  return true;
}

int LocalSearch::insertLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  hadmap::tx_lanelink_vec t_lanelink;
  for (auto it = lanelinks.begin(); it != lanelinks.end(); it++) {
    t_lanelink.push_back((*it)->getTxData());
  }
  if (true == dbPtr->insertLaneLinks(t_lanelink)) {
    return TX_HADMAP_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int LocalSearch::updateLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  for (auto it = lanelinks.begin(); it != lanelinks.end(); it++) {
    tx_lanelink_t t_lanelink = (*it)->getTxData();
    dbPtr->updateLaneLink(t_lanelink);
  }
  return TX_HADMAP_OK;
}

int LocalSearch::deleteLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  hadmap::tx_lanelink_vec t_lanelink;
  for (auto it = lanelinks.begin(); it != lanelinks.end(); it++) {
    if ((*it) == nullptr) {
      return TX_HADMAP_DATA_ERROR;
    }
    dbPtr->deleteLaneLink((*it)->getId());
  }
  return TX_HADMAP_OK;
}
//
int LocalSearch::insertObjects(const hadmap::txObjects& objects) {
  hadmap::tx_object_vec t_object;
  hadmap::tx_od_object_vec t_od_object;
  hadmap::tx_object_geom_vec t_geoms_vecs;
  hadmap::tx_object_geom_rel_vec t_rel_geom_objects_vec;
  int _id = 0;
  int index = 0;
  for (auto it = objects.begin(); it != objects.end(); it++) {
    t_object.push_back((*it)->getTxData());
    t_od_object.push_back((*it)->getOdData());
    int nGeomSize = (*it)->getGeomSize();
    for (int i = 0; i < nGeomSize; i++) {
      auto itGeom = (*it)->getGeom(i);

      t_geoms_vecs.push_back(itGeom->getTxData());
    }
  }
  dbPtr->insertObjects(t_object);
  dbPtr->insertOdObjects(t_od_object);
  dbPtr->insertObjectGeoms(t_geoms_vecs);
  for (auto it = objects.begin(); it != objects.end(); it++) {
    int nGeomSize = (*it)->getGeomSize();
    for (int i = 0; i < nGeomSize; i++) {
      auto itGeom = (*it)->getGeom(i);
      tx_object_geom_rel_t object_rel;
      object_rel.geom_pkid = t_geoms_vecs.at(index).pkid;
      index++;
      object_rel.object_pkid = (*it)->getId();
      t_rel_geom_objects_vec.push_back(object_rel);
    }
  }
  dbPtr->insertObjectGeomRels(t_rel_geom_objects_vec);
  return TX_HADMAP_OK;
}

int LocalSearch::updateObjects(const hadmap::txObjects& objects) {
  deleteObjects(objects);
  return insertObjects(objects);
  // for (auto it : objects)
  // {
  //  hadmap::tx_object_geom_rel_vec t_rel_geom_objects_vec;
  //     hadmap::tx_od_object_t  object_od_data = it->getOdData();
  //  object_od_data.pkid = it->getId();
  //  dbPtr->updateOdObject(object_od_data);
  //  dbPtr->getObjectRel(it->getId(), t_rel_geom_objects_vec);
  //  if (t_rel_geom_objects_vec.size() > 0)
  //  {
  //    for (int i = 0; i < it->getGeomSize(); i++)
  //    {
  //      hadmap::tx_object_geom_t tmp = it->getGeom(i)->getTxData();
  //      tmp.pkid = t_rel_geom_objects_vec.at(0).geom_pkid;
  //      dbPtr->updateObjectGEOM(it->getGeom(i)->getTxData());
  //    }
  //  }
  // }
  // return TX_HADMAP_OK;
}

int LocalSearch::deleteObjects(const hadmap::txObjects& objects) {
  for (auto it : objects) {
    dbPtr->deleteObject(it->getId());
  }
  return TX_HADMAP_OK;
}

hadmap::txPoint LocalSearch::getRefPoint() { return hadmap::txPoint(); }

int LocalSearch::getHeader(hadmap::txOdHeaderPtr& headerPtr) { return HADMAP_DATA_EMPTY; }

}  // namespace hadmap
