/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/type_adapter.h"

namespace SIM {
namespace OSC {

DEFINE_string(vehicle_prefix, "V_", "");
DEFINE_string(dynamic_obstacle_prefix, "P_", "");
DEFINE_string(static_obstacle_prefix, "O_", "");
DEFINE_string(traffic_light_prefix, "", "");

std::map<std::string, osc_type_t> kCategory2Type = {
    {"bicycle", {ObjectType::kDynamicObstacle, ObstacleType::kBike001}},
    {"bike", {ObjectType::kDynamicObstacle, ObstacleType::kBike001}},
    {"car", {ObjectType::kVehicle, VehicleType::kSuv}},
    {"car|suv", {ObjectType::kVehicle, VehicleType::kSuv}},
    {"car|Sedan", {ObjectType::kVehicle, VehicleType::kSedan}},
    {"car|SUV", {ObjectType::kVehicle, VehicleType::kSuv}},
    {"car|truck", {ObjectType::kVehicle, VehicleType::kTruck}},
    {"car|Truck", {ObjectType::kVehicle, VehicleType::kTruck}},
    {"human", {ObjectType::kDynamicObstacle, ObstacleType::kHuman}},
    {"moto_001", {ObjectType::kDynamicObstacle, ObstacleType::kMoto001}},
    {"pedestrian", {ObjectType::kDynamicObstacle, ObstacleType::kHuman}},
    {"pedestrian|moto_001", {ObjectType::kDynamicObstacle, ObstacleType::kMoto001}},
    {"obstacle|Cone", {ObjectType::kStaticObstacle, ObstacleType::kCone}},
    {"Sedan", {ObjectType::kVehicle, VehicleType::kSedan}},
    {"suv", {ObjectType::kVehicle, VehicleType::kSuv}},
    {"SUV", {ObjectType::kVehicle, VehicleType::kSuv}},
    {"Truck", {ObjectType::kVehicle, VehicleType::kTruck}},
    {"truck", {ObjectType::kVehicle, VehicleType::kTruck}},
};

std::map<osc_type_t, std::string> kType2Category = []() {
  std::map<osc_type_t, std::string> tmp;
  for (auto& kv : kCategory2Type) {
    tmp[kv.second] = kv.first;
  }
  return tmp;
}();

std::unordered_map<int, std::string> kPrefixDict = {
    {ObjectType::kVehicle, FLAGS_vehicle_prefix},
    {ObjectType::kDynamicObstacle, FLAGS_dynamic_obstacle_prefix},
    {ObjectType::kStaticObstacle, FLAGS_static_obstacle_prefix},
    {ObjectType::kTrafficLight, FLAGS_traffic_light_prefix},
};

}  // namespace OSC
}  // namespace SIM
