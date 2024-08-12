/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <set>
#include <string>

#include <json/json.h>
#include "common/utils/status.h"

class SceneCatalogData {
 public:
  struct Options {
    explicit Options(const char* param);
    bool Valid();
    bool is_lite;
    std::string type;
    std::string sub_type;
    std::string secondary_sub_type;
    std::string cmd;
    std::string bucket;
    std::string key;
    Json::Value data;
  };

  utils::Status GetCatalogImpl(const char* param, Json::Value& data);

  utils::Status SetCatalogImpl(const char* param);

  static void Initialize(const std::string& bucket = {}, const std::string& key = "model/");

 private:
  utils::Status SetDynamicCatalog(const Options& param);
  utils::Status SetSensorCatalog(const Options& param);
  utils::Status SetVehicleCatalog(const Options& param);

 private:
  static std::set<std::string> valid_types_;
  static std::set<std::string> valid_cmds_;
  static constexpr const char* kDynamicPath = "/Catalogs/Dynamics/";
  static constexpr const char* kDefaultXoscPath = "/Catalogs/Vehicles/default.xosc";
  static constexpr const char* kVehicleCatalogXoscPath = "/Catalogs/Vehicles/VehicleCatalog.xosc";
  static constexpr const char* kSensorCatalogXoscPath = "/Catalogs/Sensors/SensorCatalog.xosc";
  static constexpr const char* kPedestrianCatalogXoscPath = "/Catalogs/Pedestrians/PedestrianCatalog.xosc";
  static constexpr const char* kMiscObjectCatalogXoscPath = "/Catalogs/MiscObjects/MiscObjectCatalog.xosc";
};
