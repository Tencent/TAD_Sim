/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <functional>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>

#include "gflags/gflags.h"

namespace SIM {
namespace OSC {

DECLARE_string(vehicle_prefix);
DECLARE_string(dynamic_obstacle_prefix);
DECLARE_string(static_obstacle_prefix);
DECLARE_string(traffic_light_prefix);

// parent type
enum ObjectType {
  kNone = -1,
  kVehicle,
  kDynamicObstacle,
  kStaticObstacle,
  kTrafficLight,
};

// sub type
enum VehicleType {
  kCar = 0,
  kSedan = 1,
  kSuv = 2,
  kTruck = 4,
};
enum ObstacleType {
  // human = 0, child = 1, oldman = 2, woman = 3, girl = 4,
  // cat = 100, dog = 101, bike_001 = 201, elecBike_001 = 202, tricycle_001 = 203, tricycle_002 = 204, tricycle_003 =
  // 205,
  // moto_001 = 301, moto_002 = 302,
  // vendingCar_001 = 401
  kHuman = 0,
  kBike001 = 201,
  kMoto001 = 301,
  kCone = 3,
};

typedef std::pair<int, int> osc_type_t;

// {category, {first_type, second_type}}
extern std::map<std::string, osc_type_t> kCategory2Type;

extern std::map<osc_type_t, std::string> kType2Category;

extern std::unordered_map<int, std::string> kPrefixDict;

inline int64_t Name2Id(int type, const std::string& n) {
  try {
    auto p = kPrefixDict.at(type);
    auto pos = n.find(p);
    if (n.find(p) == 0) {
      return std::atoll(n.substr(p.size()).c_str());
    }
    std::smatch m;
    if (std::regex_search(n, m, std::regex("([^0-9]{0,})([0-9]{1,})")) && m.size() >= 3) {
      return std::atoll(m.str(2).c_str());
    }
    return std::atoll(n.c_str());
  } catch (...) {
  }
  return std::atoll(n.c_str());
}

inline std::string Id2Name(int type, const int64_t i) {
  try {
    std::string p = kPrefixDict.at(type);
    return p + std::to_string(i);
  } catch (...) {
  }
  return std::to_string(i);
}

}  // namespace OSC
}  // namespace SIM
