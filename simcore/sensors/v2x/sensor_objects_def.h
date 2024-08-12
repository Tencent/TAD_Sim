// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//

#pragma once
#include <types/map_defs.h>
#include <vector>
#include "location.pb.h"
#include "traffic.pb.h"

struct RsuLocConfig {
  int id;
  double lon;
  double lat;
};

struct Position {
  double lat;
  double lng;
  Position() {
    lat = 118.5043414;
    lng = -0.0028034;
  }
  explicit Position(int index) {
    lat = 118.5043414 + 0.00005 * index;
    lng = -0.0028034 - 0.003 * index;
  }
};

struct SimSensorObjects {
  uint32_t RsuId = 0;
  double timestamp = 0;
  std::vector<sim_msg::Car> SimCars{};
  std::vector<sim_msg::DynamicObstacle> SimDynamicObstacles{};
  SimSensorObjects() {}
  explicit SimSensorObjects(uint32_t id) { RsuId = id; }
};

struct SimTrafficLights {
  uint32_t RsuId = 0;
  double timestamp = 0;
  std::vector<sim_msg::TrafficLight> SimLightLists{};
  std::vector<hadmap::junctionpkid> SimJuncs{};
  SimTrafficLights() {}
  explicit SimTrafficLights(uint32_t id) { RsuId = id; }
};

struct SimTrafficEvents {
  uint32_t RsuId = 0;
  double timestamp = 0;
  std::vector<hadmap::objectpkid> SimEventLists{};
  SimTrafficEvents() {}
  explicit SimTrafficEvents(uint32_t id) { RsuId = id; }
};

struct SimTrafficMaps {
  uint32_t RsuId = 0;
  double timestamp = 0;
  std::vector<hadmap::junctionpkid> SimJunctions{};
  SimTrafficMaps() {}
  explicit SimTrafficMaps(uint32_t id) { RsuId = id; }
};

struct SimRsuSensors {
  std::map<int, SimSensorObjects> sim_sensor_objects;
  std::map<int, SimTrafficLights> sim_traffic_lights;
  std::map<int, SimTrafficEvents> sim_traffic_events;
  std::map<int, SimTrafficMaps> sim_traffic_maps;
  std::map<int, Position> sim_rsu_posotion;
};

struct ObuMountConfig {
  int id = 0;
};

struct SimObus {
  uint32_t ObuId = 0;
  double timestamp = 0;
  sim_msg::Car CarObu;
};

struct SimObuSensors {
  std::map<int, SimObus> sim_obus;
};
