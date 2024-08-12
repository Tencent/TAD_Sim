/**
 * @file Hmap.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "hmap.h"
#include <boost/filesystem/path.hpp>
#include <iostream>
#include "mapengine/hadmap_codes.h"

/// @brief map handle
hadmap::txMapHandle *hMapHandle = nullptr;
/// @brief road information
hadmap::txRoads mapRoads;
/// @brief object information
hadmap::txObjects mapObjects;

/**
 * @brief load the map
 *
 * @param fname path to the map file
 * @return true on success
 * @return false on failure
 */
bool LoadMap(const std::string &fname) {
  using namespace hadmap;
  if (hMapHandle) {
    hadmapClose(&hMapHandle);
  }
  mapRoads.clear();
  mapObjects.clear();
  auto fext = boost::filesystem::path(fname).extension();
  auto ftype = NONE;
  if (fext == ".sqlite") {
    // hanle sqlite
    ftype = SQLITE;
  } else if (fext == ".db") {
    // hanle mysql
    ftype = MYSQL;
  } else if (fext == ".xodr") {
    // hanle opendrive
    ftype = OPENDRIVE;
  } else {
    std::cout << "Load map falid. Unsupport file type." << fname << std::endl;
    return false;
  }

  if (TX_HADMAP_HANDLE_OK != hadmapConnect(fname.c_str(), ftype, &hMapHandle)) {
    std::cout << "Load map falid." << std::endl;
    return false;
  }
  // get all roads
  getRoads(hMapHandle, true, mapRoads);
  // get all objects
  getObjects(hMapHandle, std::vector<hadmap::txLaneId>(), std::vector<hadmap::OBJECT_TYPE>(), mapObjects);
  std::cout << "Load map success" << std::endl;
  return true;
}

/**
 * @brief Get the Ego Lane object
 *
 * @param loc location of ego vehicle
 * @param lanePtr get the lane information
 * @return true on success
 * @return false on failure
 */
bool GetEgoLane(const hadmap::txPoint &loc, hadmap::txLanePtr &lanePtr) {
  static hadmap::txPoint ploc;
  static hadmap::txLanePtr rtLane;
  if (!hMapHandle) {
    return false;
  }

  // if the location is nearby, return the last handle lane
  if (std::abs(ploc.x - loc.x) < 1e-6 && std::abs(ploc.y - loc.y) < 1e-6 && std::abs(ploc.z - loc.z) < 0.1) {
    lanePtr = rtLane;
    return lanePtr.get() != nullptr;
  }
  // get the lane information
  if (hadmap::getLane(hMapHandle, loc, rtLane) != TX_HADMAP_DATA_OK) {
    return false;
  }
  lanePtr = rtLane;
  ploc = loc;
  return lanePtr.get() != nullptr;
}
