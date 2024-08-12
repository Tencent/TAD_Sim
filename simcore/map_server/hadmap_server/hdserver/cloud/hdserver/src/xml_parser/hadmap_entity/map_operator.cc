/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_operator.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

using namespace hadmap;

int CMapOperator::openDB(dbOperation** db_ptr, const char* db_file) {
  (*db_ptr) = dbFactory::getInstance()->getSqliteOp(db_file);
  if (!(*db_ptr)->connect()) {
    printf("#ERROR: open db failed (%s)\n", db_file);
    return -1;
  }
  if (!(*db_ptr)->initialize()) {
    printf("#ERROR: init db failed (%s)\n", db_file);
    return -1;
  }
  return 0;
}

int CMapOperator::closeDB(dbOperation** db_ptr) {
  int r = dbFactory::getInstance()->releaseDB(db_ptr);
  (*db_ptr) = NULL;
  return r;
}

void CMapOperator::insertIntoSqlite(const char* strName, hadmap::txRoads& roads, hadmap::txLaneLinks& links) {
  dbOperation* db_ptr;
  if (openDB(&db_ptr, strName) != 0) return;

  std::unordered_map<roadpkid, roadpkid> _road_map;
  for (auto _road_ptr : roads) {
    std::cout << "Insert Road " << _road_ptr->getId() << std::endl;
    tx_road_t _cur_road;
    mapdata_trans::HadRoadToTxRoad(_road_ptr, _cur_road);
    _cur_road.task_pkid = 1;
    _cur_road.srs = 4326;
    tx_road_vec _cur_roads(1, _cur_road);
    db_ptr->insertRoads(_cur_roads);

    _road_map[_road_ptr->getId()] = _cur_roads.front().pkid;

    std::unordered_map<laneboundarypkid, laneboundarypkid> _bound_id_map;
    tx_lane_vec _cur_lanes;

    txSections& _sections = _road_ptr->getSections();
    for (auto _sec_ptr : _sections) {
      txLanes& _lanes = _sec_ptr->getLanes();
      for (auto _lane_ptr : _lanes) {
        tx_lane_t _cur_lane;
        mapdata_trans::HadLaneToTxLane(_lane_ptr, _cur_lane);
        _cur_lane.srs = 4326;
        _cur_lane.road_pkid = _cur_roads.front().pkid;
        _cur_lanes.push_back(_cur_lane);

        if (_bound_id_map.find(_lane_ptr->getLeftBoundaryId()) == _bound_id_map.end()) {
          tx_laneboundary_t _cur_bound;
          mapdata_trans::HadBoundaryToTxBoundary(_lane_ptr->getLeftBoundary(), _cur_bound);
          _cur_bound.srs = 4326;
          tx_laneboundary_vec _bounds(1, _cur_bound);
          db_ptr->insertLaneBoundaries(_bounds);
          _bound_id_map.insert(std::make_pair(_lane_ptr->getLeftBoundaryId(), _bounds.front().pkid));
        }

        if (_bound_id_map.find(_lane_ptr->getRightBoundaryId()) == _bound_id_map.end()) {
          tx_laneboundary_t _cur_bound;
          mapdata_trans::HadBoundaryToTxBoundary(_lane_ptr->getRightBoundary(), _cur_bound);
          _cur_bound.srs = 4326;
          tx_laneboundary_vec _bounds(1, _cur_bound);
          db_ptr->insertLaneBoundaries(_bounds);
          _bound_id_map.insert(std::make_pair(_lane_ptr->getRightBoundaryId(), _bounds.front().pkid));
        }
      }
    }

    for (auto& _cur_lane : _cur_lanes) {
      _cur_lane.left_boundary = _bound_id_map[_cur_lane.left_boundary];
      _cur_lane.right_boundary = _bound_id_map[_cur_lane.right_boundary];
    }
    db_ptr->insertLanes(_cur_lanes);
  }

  tx_lanelink_vec _links;
  for (auto& _cur_link : links) {
    tx_lanelink_t _link;
    mapdata_trans::HadLaneLinkToTxLaneLink(_cur_link, _link);

    _link.from_road_pkid = _road_map[_link.from_road_pkid];
    _link.to_road_pkid = _road_map[_link.to_road_pkid];

    _links.push_back(_link);
  }

  db_ptr->insertLaneLinks(_links);

  closeDB(&db_ptr);
}