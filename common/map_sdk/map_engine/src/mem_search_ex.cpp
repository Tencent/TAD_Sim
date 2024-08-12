// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "mapengine/mem_search_ex.h"
#include "ParseInterface.h"
#include "common/log.h"
#include "mapengine/remote_search.h"

namespace hadmap {
CloudSearch::CloudSearch(MAP_DATA_TYPE type, const std::string& envelope) : MemSearch(type) {
  // parse envelope
  size_t splitIndex = envelope.find(';');
  std::string lbP = envelope.substr(0, splitIndex);
  std::string rtP = envelope.substr(splitIndex + 1);
  size_t fIndex = lbP.find(',');
  size_t bIndex = rtP.find(',');

  double lbLon = std::stod(lbP.substr(0, fIndex));
  double lbLat = std::stod(lbP.substr(fIndex + 1));
  double rtLon = std::stod(rtP.substr(0, bIndex));
  double rtLat = std::stod(rtP.substr(bIndex + 1));

  curEnvelope.push_back(txPoint(lbLon, lbLat, 0.0));
  curEnvelope.push_back(txPoint(rtLon, rtLat, 0.0));

  init();
}

CloudSearch::~CloudSearch() {}

void CloudSearch::init() {
  if (curEnvelope[0].x > curEnvelope[1].x || curEnvelope[0].y > curEnvelope[1].y) return;

  mapPtr.reset(new txMap(COORD_WGS84));

  RemoteSearch rs(REMOTE, "yby.sqlite");
  txRoads roads;
  if (rs.getRoads(curEnvelope, true, roads)) {
    txLaneLinks links;
    rs.getLaneLinks(curEnvelope, links);

    mapPtr->insertRoads(roads);
    mapPtr->insertLaneLinks(links);

    txObjects objects;
    rs.getObjects(curEnvelope, std::vector<OBJECT_TYPE>(1, OBJECT_TYPE_Stop), objects);
    mapPtr->insertObjects(objects);
  } else {
    mapPtr.reset();
  }
}

OpenDriveSearch::OpenDriveSearch(MAP_DATA_TYPE type, const std::string& path, bool doubleRoad) : MemSearch(type) {
  txRoads _roads;
  txLaneLinks _links;
  txObjects objs;
  txJunctions juncs;
  txOdHeaderPtr header;
  txPoint point_ref;
  if (parseOpenDrive(path, _roads, _links, objs, juncs, header, point_ref, doubleRoad)) {
    txlog::printf("odr parse finish.\n");
    mapPtr.reset(new txMap(COORD_WGS84));
    mapPtr->insertRoads(_roads);
    mapPtr->insertLaneLinks(_links);
    mapPtr->insertObjects(objs);
    mapPtr->insertJunctions(juncs);
    mapPtr->setHeader(header);
    mapPtr->indertRefPoint(point_ref);
  } else {
    mapPtr.reset();
  }
  this->initFile(path);
}
OpenDriveSearch::OpenDriveSearch(MAP_DATA_TYPE type, const std::string& path,
                                 std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>> map_range_list)
    : MemSearch(type) {
  txRoads _roads;
  txLaneLinks _links;
  txObjects objs;
  txJunctions juncs;
  txOdHeaderPtr header;
  txPoint point_ref;
  if (parseOpenDriveArea(path, _roads, _links, objs, juncs, map_range_list, header, point_ref)) {
    mapPtr.reset(new txMap(COORD_WGS84));
    mapPtr->insertRoads(_roads);
    mapPtr->insertLaneLinks(_links);
    mapPtr->insertObjects(objs);
    mapPtr->insertJunctions(juncs);
    mapPtr->setHeader(header);
    mapPtr->indertRefPoint(point_ref);
    txlog::printf("odr parse finish.\n");
  } else {
    mapPtr.reset(new txMap(COORD_WGS84));
  }
  this->initFile(path);
}

OpenDriveSearch::~OpenDriveSearch() {}

void OpenDriveSearch::init() {}
}  // namespace hadmap
