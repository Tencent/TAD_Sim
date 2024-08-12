/**
 * @file RsuSensor.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "rsu_sensor.h"
#include <common/coord_trans.h>
#include <float.h>
#include <algorithm>
#include <iostream>
#include "../hmap.h"
#include "mapengine/hadmap_engine.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_junction.h"

/**
 * @brief Construct a new Rsu Sensor:: Rsu Sensor object
 *
 */
RsuSensor::RsuSensor() {}

/**
 * @brief Destroy the Rsu Sensor:: Rsu Sensor object
 *
 */
RsuSensor::~RsuSensor() {}

/**
 * @brief initialize the rsu sensor
 *
 * @return true on success
 * @return false
 */
bool RsuSensor::Init() {
  juncNearby.clear();
  for (const auto &juc : mapJunctions) {
    std::vector<hadmap::lanelinkpkid> lids;
    juc->getLaneLink(lids);
    if (!lids.empty()) {
      hadmap::txLaneLinkPtr link;
      hadmap::getLaneLink(hMapHandle, lids.front(), link);
      if (link.get() && link->getGeometry()) {
        hadmap::txPoint p = link->getGeometry()->getStart();
        coord_trans_api::lonlat2enu(p.x, p.y, p.z, position.x(), position.y(), position.z());
        double d = p.x * p.x + p.y * p.y;
        if (d < config.MapRadius * config.MapRadius) {
          juncNearby.push_back(juc->getId());
        }
      }
    }
  }
  return true;
}

/**
 * @brief filter the cars in the traffic
 *
 * @param traffic traffic message
 * @return std::vector<sim_msg::Car> of filtered cars
 */
std::vector<sim_msg::Car> RsuSensor::FilterCar(const sim_msg::Traffic &traffic) const {
  std::vector<sim_msg::Car> cars;
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d carpos(car.x(), car.y(), car.z());
    coord_trans_api::lonlat2enu(carpos.x(), carpos.y(), carpos.z(), map_ori.x(), map_ori.y(), map_ori.z());
    bool isvis = false;
    if (!isvis) {
      for (const auto &radar : radars) {
        if (radar.second->inFov(carpos)) {
          isvis = true;
          break;
        }
      }
    }
    if (!isvis) {
      for (const auto &lidar : lidars) {
        if (lidar.second->inFov(carpos)) {
          isvis = true;
          break;
        }
      }
    }
    if (!isvis) {
      for (const auto &camera : cameras) {
        if (camera.second->inFov(carpos)) {
          isvis = true;
          break;
        }
      }
    }
    if (isvis) {
      cars.push_back(car);
    }
  }
  return cars;
}

/**
 * @brief filter obstacles in the traffic
 *
 * @param traffic traffic message
 * @return std::vector<sim_msg::DynamicObstacle> of filtered obstacles
 */
std::vector<sim_msg::DynamicObstacle> RsuSensor::FilterObstacle(const sim_msg::Traffic &traffic) const {
  std::vector<sim_msg::DynamicObstacle> obs;
  for (const auto &obstacle : traffic.dynamicobstacles()) {
    Eigen::Vector3d pbjpos(obstacle.x(), obstacle.y(), obstacle.z());
    coord_trans_api::lonlat2enu(pbjpos.x(), pbjpos.y(), pbjpos.z(), map_ori.x(), map_ori.y(), map_ori.z());
    bool isvis = false;
    if (!isvis) {
      for (const auto &radar : radars) {
        if (radar.second->inFov(pbjpos)) {
          isvis = true;
          break;
        }
      }
    }
    if (!isvis) {
      for (const auto &lidar : lidars) {
        if (lidar.second->inFov(pbjpos)) {
          isvis = true;
          break;
        }
      }
    }
    if (!isvis) {
      for (const auto &camera : cameras) {
        if (camera.second->inFov(pbjpos)) {
          isvis = true;
          break;
        }
      }
    }
    if (isvis) {
      obs.push_back(obstacle);
    }
  }
  return obs;
}

/**
 * @brief filter cars in the traffic
 *
 * @param traffic message
 * @return std::vector<sim_msg::Car> of filtered cars
 */
std::vector<sim_msg::Car> RsuSensor::FilterRsuCars(const sim_msg::Traffic &traffic) const { return FilterCar(traffic); }

/**
 * @brief filter dynamic objects in the traffic
 *
 * @param traffic traffic message
 * @return std::vector<sim_msg::DynamicObstacle> of filtered dynamic objects
 */
std::vector<sim_msg::DynamicObstacle> RsuSensor::FilterRsuDynamicObjects(const sim_msg::Traffic &traffic) const {
  return FilterObstacle(traffic);
}

/**
 * @brief filter events. todo.
 *
 * @return std::vector<hadmap::objectpkid>
 */
std::vector<hadmap::objectpkid> RsuSensor::FilterRsuEvents() const {
  std::vector<hadmap::objectpkid> events;
  return events;
}

/**
 * @brief filter junctions.
 *
 * @return std::vector<hadmap::junctionpkid> of filtered junctions
 */
std::vector<hadmap::junctionpkid> RsuSensor::FilterRsuJuncs() const {
  std::set<hadmap::junctionpkid> juncs;
  juncs.insert(config.JunctionIDs.begin(), config.JunctionIDs.end());
  juncs.insert(juncNearby.begin(), juncNearby.end());
  return std::vector<hadmap::junctionpkid>(juncs.begin(), juncs.end());
}
