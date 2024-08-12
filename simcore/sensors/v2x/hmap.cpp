/**
 * @file hmap.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "hmap.h"
#include <iostream>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "mapengine/hadmap_codes.h"

/// @brief map handle
hadmap::txMapHandle *hMapHandle = nullptr;
/// @brief road information
hadmap::txRoads mapRoads;
/// @brief object information
hadmap::txObjects mapObjects;
hadmap::txJunctions mapJunctions;

/**
 * @brief Get the Map Type object
 *
 * @param map_path path of the map
 * @return hadmap::MAP_DATA_TYPE
 */
hadmap::MAP_DATA_TYPE GetMapType(const std::string &map_path) {
  boost::filesystem::path p(map_path);
  std::string ext = p.extension().string();
  boost::to_lower(ext);
  if (ext == ".sqlite") return hadmap::SQLITE;
  if (ext == ".xodr") return hadmap::OPENDRIVE;
  LOG(ERROR) << "unknown map file type(" << ext << "): " << map_path;
  throw std::runtime_error(std::string("unknown map file type: ") + map_path);
}

/**
 * @brief check handle code
 *
 * @param optCode
 */
void checkHandleCode(int optCode) {
  if (optCode == TX_HADMAP_HANDLE_ERROR || optCode == TX_HADMAP_DATA_ERROR || optCode == TX_HADMAP_PARAM_ERROR) {
    std::stringstream ss;
    ss << "hadmap engine operation failed(" << optCode << "): ";
    if (hMapHandle) ss << hadmap::getLastOptInfo(hMapHandle);
    throw std::runtime_error(ss.str());
  }
}

/**
 * @brief reset map handle
 *
 */
void resetMapHandle() {
  mapRoads.clear();
  mapObjects.clear();
  if (hMapHandle) checkHandleCode(hadmap::hadmapClose(&hMapHandle));
}

/**
 * @param fname path to the map file
 *
 * @return true on success
 * @return false on failure
 */
bool LoadMap(const std::string &mapPath) {
  resetMapHandle();
  int ret = hadmap::hadmapConnect(mapPath.c_str(), GetMapType(mapPath), &hMapHandle);
  checkHandleCode(ret);
  // get all roads
  getRoads(hMapHandle, true, mapRoads);
  // get all objects
  getObjects(hMapHandle, std::vector<hadmap::txLaneId>(), std::vector<hadmap::OBJECT_TYPE>(), mapObjects);
  hadmap::getJunctions(hMapHandle, mapJunctions);
  LOG(INFO) << "Load map success. road has " << mapRoads.size() << ", object has " << mapObjects.size()
            << ", junction has " << mapJunctions.size();
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
