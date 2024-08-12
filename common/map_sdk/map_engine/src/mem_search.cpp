// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "mapengine/mem_search.h"
#include <iostream>
#include "common/map_util.h"
#include "db_factory.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/remote_search.h"
#include "sqlite_operation.h"

namespace hadmap {
#define COPY_LANE(origin_ptr) std::make_shared<txLane>(txLane(*origin_ptr))

#define COPY_BOUNDARY(origin_ptr) std::make_shared<txLaneBoundary>(txLaneBoundary(*origin_ptr))

#define COPY_SECTION(origin_ptr) std::make_shared<txSection>(txSection(*origin_ptr))

#define COPY_ROAD(origin_ptr) std::make_shared<txRoad>(txRoad(*origin_ptr))

#define COPY_LINK(origin_ptr) std::make_shared<txLaneLink>(txLaneLink(*origin_ptr))

#define COPY_JUNCTION(origin_ptr) std::make_shared<txJunction>(txJunction(*origin_ptr))

MemSearch::MemSearch(MAP_DATA_TYPE type) : dataType(type), optInfo("MemSearch"), mapPtr(NULL) {}

MemSearch::~MemSearch() {}

bool MemSearch::connSuccess() { return mapPtr != NULL; }

bool MemSearch::initFile(const std::string& filePath) {
  if (mOutPtr == NULL) {
    mOutPtr = std::make_shared<hadmap::OpenDriveOutput>();
    mOutPtr->initFilePath(filePath.c_str());
  }
  mapfilePath = filePath;
  return true;
}

bool MemSearch::saveMap(const int type, std::string filePath) {
  if (MAP_DATA_TYPE(type) == SQLITE) {
    dbOperation* dbPtr = dbFactory::getInstance()->getSqliteOp(filePath);
    if (dbPtr == nullptr) {
      return false;
    }
    if (dbPtr->connect() && dbPtr->initialize()) {
      optInfo = std::string("Load DB successful");
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
      hadmap::txPoint point = mapPtr->getRefPoint();
      task.longitude_ref = point.x;
      task.latitude_ref = point.y;
      tx_task_vec t_tasks;
      // save boundarys;
      std::cout << "save task" << std::endl;
      t_tasks.push_back(task);
      dbPtr->insertTasks(t_tasks);
      std::cout << "save boundarys" << std::endl;
      txLaneBoundaries boundarys;
      bool bRet = mapPtr->getBoundaries(boundarys);
      std::cout << "get boundarys" << std::endl;
      hadmap::tx_laneboundary_vec t_boundary;
      if (bRet) {
        for (auto it = boundarys.begin(); it != boundarys.end(); it++) {
          if ((*it) == NULL) {
            std::cout << "NULL " << std::endl;
            continue;
          }
          tx_laneboundary_t boundary = (*it)->getTxData();
          boundary.srs = 4326;
          t_boundary.push_back(boundary);
        }
        dbPtr->insertLaneBoundaries(t_boundary);
      }
      std::cout << "save roads" << std::endl;
      // save roads
      txRoads roads;
      bRet = mapPtr->getRoads(roads);
      hadmap::tx_road_vec t_roads;
      if (bRet) {
        for (auto it = roads.begin(); it != roads.end(); it++) {
          tx_road_t roat_t = (*it)->getTxData();
          roat_t.srs = 4326;
          roat_t.pkid = (*it)->getId();
          roat_t.section_num = (*it)->getSections().size();
          t_roads.push_back(roat_t);
        }
        dbPtr->insertRoads(t_roads);
      }
      std::cout << "save lanes" << std::endl;
      // save lanes
      txLanes lanes;
      hadmap::tx_lane_vec t_lanes;
      bRet = mapPtr->getLanes(lanes);
      if (bRet) {
        for (auto it = lanes.begin(); it != lanes.end(); it++) {
          tx_lane_t lane = (*it)->getTxLaneData();
          lane.srs = 4326;
          t_lanes.push_back(lane);
        }
        dbPtr->insertLanes(t_lanes);
      }
      std::cout << "save lanelinks" << std::endl;
      // save laneLink
      txLaneLinks lanelinks;
      bRet = mapPtr->getLaneLinks(lanelinks);
      if (bRet) {
        hadmap::tx_lanelink_vec t_lanelink;
        for (auto it = lanelinks.begin(); it != lanelinks.end(); it++) {
          t_lanelink.push_back((*it)->getTxData());
        }
        dbPtr->insertLaneLinks(t_lanelink);
      }
      std::cout << "save objects" << std::endl;
      // save objects
      txObjects objects;
      bRet = mapPtr->getObjects(objects);
      if (bRet) {
        hadmap::tx_object_vec t_object;
        hadmap::tx_od_object_vec t_od_object;
        hadmap::tx_object_geom_vec t_geoms_vecs;
        hadmap::tx_object_geom_rel_vec t_rel_geom_objects_vec;
        int _id = 0;
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
            object_rel.pkid = _id++;
            object_rel.geom_pkid = itGeom->getId();
            object_rel.object_pkid = (*it)->getId();
            t_rel_geom_objects_vec.push_back(object_rel);
          }
        }
        dbPtr->insertObjectGeomRels(t_rel_geom_objects_vec);
      }
      return true;
    } else {
      dbFactory::getInstance()->releaseDB(&dbPtr);
      dbPtr = NULL;
      return false;
    }
  } else if (MAP_DATA_TYPE(type) == OPENDRIVE) {
    initFile(filePath);
    OpenDriveOutput outPut;
    hadmap::txRoads _roads;
    getRoads(true, _roads);
    if (_roads.size() > 0) {
      outPut.initFilePath(filePath.c_str());
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
    if (links.size() > 0) {
      outPut.insertLaneLinks(links);
    }
    // objects
    hadmap::txObjects _objects;
    std::vector<txLaneId> laneids;
    std::vector<OBJECT_TYPE> types;
    hadmap::txObjects objects;
    if (objects.size() > 0) {
      getObjects(laneids, types, objects);
      outPut.insertObjects(objects, _roads);
    }
    outPut.saveFile(filePath.c_str());
    return true;
  }
}

bool MemSearch::save() {
  if (mOutPtr->getIsInit()) {
    mOutPtr->saveFile(mapfilePath.c_str());
    return true;
  }
  return false;
}

std::string MemSearch::getLastOptInfo() { return optInfo; }

int MemSearch::getLane(const txPoint& loc, double radius, txLanePtr& lanePtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanePtr mem_lane_ptr = mapPtr->nearestLane(loc, radius);
  if (mem_lane_ptr == NULL) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    lanePtr = COPY_LANE(mem_lane_ptr);
    lanePtr->setLeftBoundary(COPY_BOUNDARY(mem_lane_ptr->getLeftBoundary()));
    lanePtr->setRightBoundary(COPY_BOUNDARY(mem_lane_ptr->getRightBoundary()));
    return TX_HADMAP_DATA_OK;
    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::getLanes(const txLaneId& laneId, txLanes& lanes) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanes mem_lanes;
  if (laneId.roadId == ROAD_PKID_INVALID && laneId.sectionId == SECT_PKID_INVALID &&
      laneId.laneId == LANE_PKID_INVALID) {
    mapPtr->getLanes(mem_lanes);
  } else {
    mapPtr->specLanes(laneId, mem_lanes);
  }

  if (mem_lanes.empty()) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    lanes.clear();
    for (auto& lane_ptr : mem_lanes) {
      lanes.push_back(COPY_LANE(lane_ptr));
      lanes.back()->setLeftBoundary(COPY_BOUNDARY(lane_ptr->getLeftBoundary()));
      lanes.back()->setRightBoundary(COPY_BOUNDARY(lane_ptr->getRightBoundary()));
    }

    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::getLanes(const PointVec& envelope, txLanes& lanes) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanes mem_lanes;
  mapPtr->searchLanes(envelope, mem_lanes);

  lanes.clear();
  for (auto& lane_ptr : mem_lanes) {
    lanes.push_back(COPY_LANE(lane_ptr));
    lanes.back()->setLeftBoundary(COPY_BOUNDARY(lane_ptr->getLeftBoundary()));
    lanes.back()->setRightBoundary(COPY_BOUNDARY(lane_ptr->getRightBoundary()));
  }

  return lanes.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getNextLanes(txLanePtr curLanePtr, txLanes& nextLanes) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanes mem_next_lanes = mapPtr->getNextLanes(curLanePtr);

  if (mem_next_lanes.empty()) return TX_HADMAP_DATA_EMPTY;

  // consider the effect of links
  txPoint endP;
  if (curLanePtr->getId() > 0) {
    endP = curLanePtr->getGeometry()->getStart();
  } else {
    endP = curLanePtr->getGeometry()->getEnd();
  }
  nextLanes.clear();
  for (auto& lane_ptr : mem_next_lanes) {
    txPoint startP;
    if (lane_ptr->getId() > 0) {
      startP = lane_ptr->getGeometry()->getEnd();
    } else {
      startP = lane_ptr->getGeometry()->getStart();
    }

    Point3d a(endP.x, endP.y, 0.0);
    Point3d b(startP.x, startP.y, 0.0);
    if (map_util::distanceBetweenPoints(a, b, true) < 0.5) nextLanes.push_back(COPY_LANE(lane_ptr));
  }
  return nextLanes.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getPrevLanes(txLanePtr curLanePtr, txLanes& prevLanes) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanes mem_pre_lanes = mapPtr->getPreLanes(curLanePtr);

  if (mem_pre_lanes.empty()) return TX_HADMAP_DATA_EMPTY;

  txPoint startP;
  if (curLanePtr->getId() > 0) {
    startP = curLanePtr->getGeometry()->getEnd();
  } else {
    startP = curLanePtr->getGeometry()->getStart();
  }

  prevLanes.clear();
  for (auto& lane_ptr : mem_pre_lanes) {
    txPoint endP;
    if (lane_ptr->getId() > 0) {
      endP = lane_ptr->getGeometry()->getStart();
    } else {
      endP = lane_ptr->getGeometry()->getEnd();
    }
    Point3d a(endP.x, endP.y, 0.0);
    Point3d b(startP.x, startP.y, 0.0);
    if (map_util::distanceBetweenPoints(a, b, true) < 0.5) prevLanes.push_back(COPY_LANE(lane_ptr));
  }

  return prevLanes.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getLeftLane(txLanePtr curLanePtr, txLanePtr& leftLanePtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanePtr mem_left_lane_ptr = mapPtr->getLeftLane(curLanePtr);
  if (mem_left_lane_ptr == NULL) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    leftLanePtr = COPY_LANE(mem_left_lane_ptr);
    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::getRightLane(txLanePtr curLanePtr, txLanePtr& rightLanePtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLanePtr mem_right_lane_ptr = mapPtr->getRightLane(curLanePtr);
  if (mem_right_lane_ptr == NULL) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    rightLanePtr = COPY_LANE(mem_right_lane_ptr);
    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::getBoundaryMaxId(laneboundarypkid& boundaryMaxId) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::getBoundaries(const std::vector<laneboundarypkid>& ids, txLaneBoundaries& boundaries) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneBoundaries mem_boundaries;
  mapPtr->specBoundaries(ids, mem_boundaries);

  boundaries.clear();
  for (auto& boundary_ptr : mem_boundaries) boundaries.push_back(COPY_BOUNDARY(boundary_ptr));

  return boundaries.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getSections(const txSectionId& sectionId, txSections& sections) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txRoadPtr roadPtr = mapPtr->getRoadById(sectionId.roadId);
  if (roadPtr == NULL) return TX_HADMAP_DATA_ERROR;

  sections.clear();
  if (sectionId.sectionId == SECT_PKID_INVALID) {
    txSections mem_sections = roadPtr->getSections();
    for (auto& sec_ptr : mem_sections) {
      sections.push_back(COPY_SECTION(sec_ptr));
      sections.back()->bindSectionPtr();
    }
  } else {
    txSections& curSections = roadPtr->getSections();
    for (auto& curSecPtr : curSections) {
      if (curSecPtr->getId() == sectionId.sectionId) {
        sections.push_back(COPY_SECTION(curSecPtr));
        sections.back()->bindSectionPtr();
        break;
      }
    }
  }

  return sections.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getRoadMaxId(roadpkid& roadMaxId) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::getRoad(const roadpkid& roadId, const bool& wholeData, txRoadPtr& roadPtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txRoadPtr mem_road_ptr = mapPtr->getRoadById(roadId);
  if (mem_road_ptr == NULL) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    roadPtr = COPY_ROAD(mem_road_ptr);
    roadPtr->bindRoadPtr();
    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::getRoads(const PointVec& envelope, const bool& wholeData, txRoads& roads) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txRoads mem_roads;
  mapPtr->searchRoads(envelope, mem_roads);

  roads.clear();
  for (auto& road_ptr : mem_roads) {
    roads.push_back(COPY_ROAD(road_ptr));
    roads.back()->bindRoadPtr();
  }
  return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getRoads(const std::vector<roadpkid>& rids, const bool& wholeData, txRoads& roads) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  if (rids.empty()) return getRoads(wholeData, roads);

  roads.clear();
  for (auto& id : rids) {
    txRoadPtr roadPtr;
    if (TX_HADMAP_DATA_OK == getRoad(id, wholeData, roadPtr)) roads.push_back(roadPtr);
  }

  return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getRoads(const bool& wholeData, txRoads& roads) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txRoads mem_roads;
  mapPtr->getRoads(mem_roads);

  roads.clear();
  for (auto& road_ptr : mem_roads) {
    roads.push_back(COPY_ROAD(road_ptr));
    roads.back()->bindRoadPtr();
  }
  return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getJuncInfo(const roadpkid& entranceRoadId, const roadpkid& exitRoadId, hadmap::txRoads& roads,
                           hadmap::txLaneLinks& links) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  std::unordered_set<roadpkid> entranceIds;
  std::unordered_set<roadpkid> exitIds;
  if (entranceRoadId != ROAD_PKID_INVALID) {
    std::vector<std::pair<roadpkid, lanepkid> > curEntranceIds;
    std::vector<std::pair<roadpkid, lanepkid> > curExitIds;

    txLaneLinks entranceLinks;
    getLaneLinks(entranceRoadId, ROAD_PKID_INVALID, entranceLinks);
    curEntranceIds.push_back(std::make_pair(entranceRoadId, entranceLinks.front()->fromLaneId()));

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

int MemSearch::getJuncInfo(const lanelinkpkid& linkId, txRoads& roads, txLaneLinks& links) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;
  txLaneLinks junc_links;
  if (mapPtr->specLinks(std::vector<lanelinkpkid>(1, linkId), junc_links)) {
    roadpkid fromRId = junc_links.front()->fromRoadId();
    lanepkid fromLId = junc_links.front()->fromLaneId();
    roadpkid toRId = junc_links.front()->toRoadId();
    lanepkid toLId = junc_links.front()->toLaneId();

    std::vector<std::pair<roadpkid, lanepkid> > curEntranceIds;
    std::vector<std::pair<roadpkid, lanepkid> > curExitIds;
    curEntranceIds.push_back(std::make_pair(fromRId, fromLId));
    curExitIds.push_back(std::make_pair(fromRId, fromLId * -1));

    std::unordered_set<roadpkid> entranceIds;
    std::unordered_set<roadpkid> exitIds;
    intersectionRoads(curEntranceIds, curExitIds, entranceIds, exitIds);

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
  } else {
    return TX_HADMAP_PARAM_ERROR;
  }
}

void MemSearch::intersectionRoads(const std::vector<std::pair<roadpkid, lanepkid> >& initEntranceIds,
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
          }
        }
      }
    }
    curEntranceIds.swap(nextEntranceIds);
    curExitIds.swap(nextExitIds);
  }
}
// search lane link by loc (if has geom)
int MemSearch::getLaneLink(const hadmap::txPoint& loc, double radius, hadmap::txLaneLinkPtr& laneLinkPtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinkPtr mem_link_ptr = mapPtr->nearestLaneLink(loc, radius);
  if (mem_link_ptr == NULL) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    laneLinkPtr = COPY_LINK(mem_link_ptr);
    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinks links;
  if (mapPtr->specLinks(std::vector<lanelinkpkid>(1, lanelinkId), links)) {
    lanelinkPtr = links[0];
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_PARAM_ERROR;
  }
}
int MemSearch::getRoadLink(const hadmap::roadpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;
  txLaneLinks links;
  if (mapPtr->specRoadLink(lanelinkId, lanelinkPtr)) {
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_PARAM_ERROR;
  }
}
int MemSearch::getPrevLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinks links;
  hadmap::txLaneId fromLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID);

  if (mapPtr->specLinks(fromLaneId, laneId, lanelinks)) {
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_PARAM_ERROR;
  }
}

int MemSearch::getNextLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinks links;
  hadmap::txLaneId toLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID);

  if (mapPtr->specLinks(laneId, toLaneId, lanelinks)) {
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_PARAM_ERROR;
  }
}

int MemSearch::getLaneLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, txLaneLinks& links) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinks mem_links;
  if (fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID) {
    mapPtr->getLaneLinks(mem_links);
  } else {
    mapPtr->specLinks(fromRoadId, toRoadId, mem_links);
  }

  links.clear();
  for (auto& link_ptr : mem_links) links.push_back(COPY_LINK(link_ptr));

  return links.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getLaneLinks(const PointVec& envelope, txLaneLinks& links) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinks mem_links;
  txRoads roads;
  if (TX_HADMAP_DATA_OK == getRoads(envelope, true, roads)) {
    std::unordered_set<lanelinkpkid> ids;
    for (auto& roadPtr : roads) {
      txLaneLinks curLinks;
      if (TX_HADMAP_DATA_OK == getLaneLinks(roadPtr->getId(), ROAD_PKID_INVALID, curLinks)) {
        for (auto& linkPtr : curLinks) {
          if (ids.find(linkPtr->getId()) == ids.end()) {
            ids.insert(linkPtr->getId());
            mem_links.push_back(linkPtr);
          }
        }
      }
      if (TX_HADMAP_DATA_OK == getLaneLinks(ROAD_PKID_INVALID, roadPtr->getId(), curLinks)) {
        for (auto& linkPtr : curLinks) {
          if (ids.find(linkPtr->getId()) == ids.end()) {
            ids.insert(linkPtr->getId());
            mem_links.push_back(linkPtr);
          }
        }
      }
    }
  }

  links.clear();
  for (auto& link_ptr : mem_links) links.push_back(COPY_LINK(link_ptr));

  return links.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getLaneLinks(const std::vector<roadpkid>& roadIds, txLaneLinks& links) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txLaneLinks mem_links;
  std::unordered_set<lanelinkpkid> ids;
  for (auto& rid : roadIds) {
    txLaneLinks curLinks;
    if (TX_HADMAP_DATA_OK == getLaneLinks(rid, ROAD_PKID_INVALID, curLinks)) {
      for (auto& linkPtr : curLinks) {
        if (ids.find(linkPtr->getId()) == ids.end()) {
          ids.insert(linkPtr->getId());
          mem_links.push_back(linkPtr);
        }
      }
    }
    if (TX_HADMAP_DATA_OK == getLaneLinks(ROAD_PKID_INVALID, rid, curLinks)) {
      for (auto& linkPtr : curLinks) {
        if (ids.find(linkPtr->getId()) == ids.end()) {
          ids.insert(linkPtr->getId());
          mem_links.push_back(linkPtr);
        }
      }
    }
  }

  links.clear();
  for (auto& link_ptr : mem_links) links.push_back(COPY_LINK(link_ptr));

  return links.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getLaneLinkMaxId(lanelinkpkid& linkMaxId) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;
  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::getObjects(const PointVec& envelope, const std::vector<OBJECT_TYPE>& types, txObjects& objects) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  std::set<OBJECT_TYPE> typeSet;
  for (auto t : types) typeSet.insert(t);
  mapPtr->searchObjects(envelope, typeSet, txLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID), objects);

  return objects.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_TYPE>& types,
                          txObjects& objects) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;
  hadmap::tx_object_vec objVec;
  hadmap::tx_object_geom_vec geomVec;
  hadmap::txObjects curObjs;
  if (laneIds.empty()) {
    if (mapPtr->getObjects(objects)) {
      if (!types.empty()) {
        std::unordered_set<int> searchtype;
        for (auto t : types) {
          searchtype.insert(t);
        }
        for (auto it = objects.begin(); it != objects.end();) {
          if (searchtype.find((*it)->getObjectType()) == searchtype.end()) {
            it = objects.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
  } else {
    objects = mapPtr->getObjects(types, laneIds);
  }
  if (!objects.empty()) return TX_HADMAP_DATA_OK;
  return TX_HADMAP_DATA_EMPTY;
}

int MemSearch::getObjects(const PointVec& envelope, const std::vector<OBJECT_SUB_TYPE>& types, txObjects& objects) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  std::set<OBJECT_SUB_TYPE> typeSet;
  for (auto t : types) typeSet.insert(t);
  mapPtr->searchObjects(envelope, typeSet, txLaneId(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID), objects);

  return objects.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_SUB_TYPE>& types,
                          txObjects& objects) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;
  hadmap::tx_object_vec objVec;
  hadmap::tx_object_geom_vec geomVec;
  hadmap::txObjects curObjs;
  if (laneIds.empty()) {
    if (mapPtr->getObjects(objects)) {
      if (!types.empty()) {
        std::unordered_set<int> searchtype;
        for (auto t : types) {
          searchtype.insert(t);
        }
        for (auto it = objects.begin(); it != objects.end();) {
          if (searchtype.find((*it)->getObjectSubType()) == searchtype.end()) {
            it = objects.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
  } else {
    objects = mapPtr->getObjects(types, laneIds);
  }
  if (!objects.empty()) return TX_HADMAP_DATA_OK;
  return TX_HADMAP_DATA_EMPTY;
}

int MemSearch::getJunctions(hadmap::txJunctions& junctions) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txJunctions mem_juntions;
  mapPtr->getJunctions(mem_juntions);

  junctions.clear();
  for (auto& junc_ptr : mem_juntions) {
    junctions.push_back(COPY_JUNCTION(junc_ptr));
    // junctions.back()->bindRoadPtr();
  }
  return junctions.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int MemSearch::getJunction(junctionpkid jid, hadmap::txJunctionPtr& junction) {
  if (mapPtr == NULL) return TX_HADMAP_HANDLE_ERROR;

  txJunctions mem_juntions;
  mapPtr->getJunctions(mem_juntions);

  for (auto& junc_ptr : mem_juntions) {
    if (junc_ptr->getId() == jid) {
      junction = COPY_JUNCTION(junc_ptr);
      return TX_HADMAP_DATA_OK;
    }
  }
  return TX_HADMAP_DATA_EMPTY;
}

int MemSearch::insertRoads(const hadmap::txRoads& roads) {
  if (true == mapPtr->insertRoads(roads)) {
    mOutPtr->insertRoads(roads);
    return TX_HADMAP_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::updateRoads(const hadmap::txRoads& roads) {
  if (true == mapPtr->updateRoads(roads)) {
    mOutPtr->updateRoads(roads);
    return TX_HADMAP_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::deleteRoads(const hadmap::txRoads& roads) {
  std::vector<roadpkid> roadids;
  for (auto it : roads) {
    mOutPtr->deleteRoads(roads);
    roadids.push_back(it->getId());
  }
  return mapPtr->removeRoads(roadids);
}

int MemSearch::insertLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  for (auto& it : lanelinks) {
    std::vector<lanelinkpkid> _tmp;
    _tmp.push_back(it->getId());
    hadmap::txLaneLinks tmplanelinks;
    mapPtr->specLinks(_tmp, tmplanelinks);
    if (tmplanelinks.size() > 0) {
      it->setOdrRoadId(tmplanelinks.front()->getOdrRoadId());
    }
  }
  if (true == mapPtr->insertLaneLinks(lanelinks)) {
    mOutPtr->insertLaneLinks(lanelinks);
    return TX_HADMAP_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::updateLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  deleteLaneLinks(lanelinks);
  insertLaneLinks(lanelinks);
  return TX_HADMAP_OK;
}

int MemSearch::deleteLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  std::vector<lanelinkpkid> ids;
  for (auto it : lanelinks) {
    ids.push_back(it->getId());
  }
  std::map<roadpkid, hadmap::txLaneLinks> road2Lanelinks;
  for (auto it : lanelinks) {
    hadmap::txLaneLinks tmp;
    mapPtr->specRoadLinks(it->getOdrRoadId(), tmp);
    road2Lanelinks[it->getOdrRoadId()] = tmp;
  }
  for (auto it : road2Lanelinks) {
    if (it.second.size() > 1) {
      mOutPtr->specLaneLinks(it.first, it.second);
      mapPtr->updateLaneLinks(it.second);
    }
  }
  for (auto& it : lanelinks) {
    std::vector<lanelinkpkid> _tmp;
    _tmp.push_back(it->getId());
    hadmap::txLaneLinks tmplanelinks;
    mapPtr->specLinks(_tmp, tmplanelinks);
    if (tmplanelinks.size() > 0) {
      it->setOdrRoadId(tmplanelinks.front()->getOdrRoadId());
    }
  }
  if (true == mapPtr->removeLinks(ids)) {
    mOutPtr->deleteLanelinks(lanelinks);
    return TX_HADMAP_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

hadmap::txPoint MemSearch::getRefPoint() {
  if (mapPtr == NULL) return hadmap::txPoint();
  return mapPtr->getRefPoint();
}

int MemSearch::getHeader(hadmap::txOdHeaderPtr& header) {
  header = mapPtr->getHeader();
  if (header == NULL) {
    return TX_HADMAP_DATA_EMPTY;
  } else {
    return TX_HADMAP_DATA_OK;
  }
}

int MemSearch::insertObjects(const hadmap::txObjects& objects) {
  if (true == mapPtr->insertObjects(objects)) {
    hadmap::txRoads roads;
    getRoads(false, roads);
    mOutPtr->insertObjects(objects, roads);
    return TX_HADMAP_DATA_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::updateObjects(const hadmap::txObjects& objects) {
  std::vector<objectpkid> objectIds;
  for (auto it : objects) {
    objectIds.push_back(it->getId());
  }
  if (true == mapPtr->removeObjects(objectIds) && true == mapPtr->insertObjects(objects)) {
    hadmap::txRoads roads;
    getRoads(false, roads);
    mOutPtr->updateObjects(objects, roads);
    return TX_HADMAP_DATA_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int MemSearch::deleteObjects(const hadmap::txObjects& objects) {
  std::vector<objectpkid> objectIds;
  for (auto it : objects) {
    objectIds.push_back(it->getId());
  }
  if (true == mapPtr->removeObjects(objectIds)) {
    hadmap::txRoads roads;
    getRoads(false, roads);
    mOutPtr->deleteObjects(objects);
    return TX_HADMAP_DATA_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

}  // namespace hadmap
