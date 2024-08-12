/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scene_catalog_data.h"

#include <google/protobuf/util/json_util.h>
#include <boost/filesystem.hpp>
#include "gflags/gflags.h"

#include "common/utils/base64.h"
#include "common/utils/misc.h"
#include "common/xml_parser/entity/catalog.h"
#include "engine/config.h"
#include "scene.pb.h"
#include "server_scene/scene_data_proxy.h"

DEFINE_string(cmd_insert, "INSERT", "");
DEFINE_string(cmd_delete, "DELETE", "");
DEFINE_string(cmd_update, "UPDATE", "");

DEFINE_string(type_dynamic, "DYNAMIC", "");
DEFINE_string(type_sensor, "SENSOR", "");
DEFINE_string(type_vehicle, "VEHICLE", "");
DEFINE_string(type_param, "Param", "");
DEFINE_string(type_device, "Device", "");
DEFINE_string(type_v2x, "V2X", "");
DEFINE_string(sensor_type_of_obu, "OBU", "");
DEFINE_string(sensor_group_name_prefix_of_obu, "OBU_", "");

std::set<std::string> SceneCatalogData::valid_types_ = {FLAGS_type_dynamic, FLAGS_type_sensor, FLAGS_type_vehicle,
                                                        FLAGS_type_param,   FLAGS_type_device, FLAGS_type_v2x};
std::set<std::string> SceneCatalogData::valid_cmds_ = {FLAGS_cmd_insert, FLAGS_cmd_delete, FLAGS_cmd_update};

SceneCatalogData::Options::Options(const char* param) {
  try {
    Json::Value root;
    root << param;
    is_lite = root.get("is_lite", false).asBool();
    type = root.get("type", "").asString();
    sub_type = root.get("subType", "").asString();
    secondary_sub_type = root.get("secondarySubType", "").asString();
    cmd = root.get("cmd", "").asString();
    bucket = root.get("bucket", CEngineConfig::Instance().EnvVar().m_strPrivateDefaultBucket).asString();
    key = root.get("key", "model/").asString();
    data = root.get("data", Json::Value(Json::arrayValue));
  } catch (...) {
  }
}

bool SceneCatalogData::Options::Valid() {
  if (type.empty() || cmd.empty() || bucket.empty() || data.empty()) {
    return false;
  }
  return true;
}

void SceneCatalogData::Initialize(const std::string& bucket, const std::string& key) {
  std::string cos_bucket = bucket.empty() ? CEngineConfig::Instance().EnvVar().m_strPrivateDefaultBucket : bucket;
  std::string cos_key = key.empty() ? "model/" : key;
  if (cos_bucket.empty() || cos_key.empty()) {
    return;
  }

  boost::filesystem::path res_dir(CEngineConfig::Instance().ResourceDir());
  for (auto f : {kDefaultXoscPath, kVehicleCatalogXoscPath, kSensorCatalogXoscPath, kPedestrianCatalogXoscPath,
                 kMiscObjectCatalogXoscPath}) {
    boost::filesystem::path file = (res_dir / f).string();
    boost::filesystem::create_directories(file.parent_path());
    CSceneDataProxy::Instance().Download(cos_bucket, cos_key + f, file.string(), false);
  }
}

utils::Status SceneCatalogData::GetCatalogImpl(const char* param, Json::Value& data) {
  SceneCatalogData::Options opts(param);

  Initialize(opts.bucket, opts.key);

  boost::filesystem::path res_dir(CEngineConfig::Instance().ResourceDir());

  std::string default_xosc = (res_dir / kDefaultXoscPath).string();
  std::string vehicle_catalog_xosc = (res_dir / kVehicleCatalogXoscPath).string();
  std::string sensor_catalog_xosc = (res_dir / kSensorCatalogXoscPath).string();
  std::string pedestrian_catalog_xosc = (res_dir / kPedestrianCatalogXoscPath).string();
  std::string miscobject_catalog_xosc = (res_dir / kMiscObjectCatalogXoscPath).string();

  if (opts.type == FLAGS_type_sensor || opts.type == FLAGS_type_v2x) {
    SensorV2 sensor;
    sensor.parseXML(sensor_catalog_xosc.c_str());
    sensor.toJson(data);

    std::set<std::string> valid_idxs;
    auto& sensors = data["sensors"];
    for (auto i = 0; sensors.isArray() && i < sensors.size();) {
      auto is_v2x =
          atoi(StringifyJsonValue(sensors[i].get("param", Json::Value(Json::objectValue)).get("isV2X", "0")).c_str());
      if ((opts.type == FLAGS_type_v2x && is_v2x != 1) || (opts.type == FLAGS_type_sensor && is_v2x == 1)) {
        sensors.removeIndex(i, nullptr);
      } else {
        valid_idxs.insert(sensors[i]["idx"].asString());
        i++;
      }
    }

    data["sensor_groups"] = Json::Value(Json::arrayValue);
    for (auto& sensor_group : sensor.getSensorGroup()) {
      auto filter = [&]() {
        for (auto& sensor : sensor_group.m_SensorVec) {
          if (valid_idxs.count(sensor.idx) <= 0) return true;
        }
        return sensor_group.m_SensorVec.empty();
      };
      if (filter()) continue;
      Json::Value one;
      sensor_group.toJson(one);
      data["sensor_groups"].append(one);
    }

    return utils::Status::OK();
  }

  if (opts.type == FLAGS_type_dynamic) {
    CCataLog catalog;
    catalog.parseDriverXML(default_xosc);
    for (auto& one : catalog.getDriveCatalogList()) {
      std::string file_name = std::string("dynamic_") + one.m_dynamicId + ".json";
      std::string dynamic_file = (res_dir / (kDynamicPath + file_name)).string();
      boost::filesystem::create_directories(boost::filesystem::path(dynamic_file).parent_path());
      CSceneDataProxy::Instance().Download(opts.bucket, opts.key + kDynamicPath, dynamic_file, false);
      if (!boost::filesystem::exists(dynamic_file)) {
        return utils::Status::Corruption("download dynamic file error", dynamic_file);
      }
      std::ifstream ifs(dynamic_file);
      if (ifs.is_open()) {
        std::stringstream ss;
        ss << ifs.rdbuf();
        data["dynamics"]["dynamicData"] << ss.str();
        ifs.close();
      }
    }
    return utils::Status::OK();
  }

  CCataLog{}.parseXml2Json(default_xosc, vehicle_catalog_xosc, pedestrian_catalog_xosc, miscobject_catalog_xosc, data);

  SensorV2 sensor;
  sensor.parseXML(sensor_catalog_xosc.c_str());
  for (auto& catalog : data.get("driver_catalog", Json::Value(Json::arrayValue))) {
    std::string sensor_name = catalog["sensor"]["groupName"].asString();
    sensor.toGroupJson(sensor_name, catalog["sensor"]);
  }

  // lite ver
  if (opts.is_lite) {
    auto to_physicle_common = [&](const Json::Value& catalog, sim_msg::PhysicleCommon* phy_common) {
      auto bounding_box = phy_common->mutable_bounding_box();
      auto center = bounding_box->mutable_center();
      auto _center = catalog.get("Center", Json::Value(Json::objectValue));
      center->set_x(std::atof(_center.get("x", "0.0").asCString()));
      center->set_y(std::atof(_center.get("y", "0.0").asCString()));
      center->set_z(std::atof(_center.get("z", "0.0").asCString()));
      auto _dimensions = catalog.get("Dimensions", Json::Value(Json::objectValue));
      bounding_box->set_higth(std::atof(_dimensions.get("height", "0.0").asCString()));
      bounding_box->set_length(std::atof(_dimensions.get("length", "0.0").asCString()));
      bounding_box->set_width(std::atof(_dimensions.get("width", "0.0").asCString()));
      phy_common->set_model_id(std::atoi(catalog.get("model_id", "-1").asCString()));
    };

    sim_msg::Scene scene;
    for (auto& vehicle : data.get("driver_catalog", {})) {
      auto ego = scene.mutable_egos()->Add();
      ego->set_name(vehicle.get("vehicle_name", "").asString());
      for (auto& catalog : vehicle.get("vehicle_param", {})) {
        auto phy = ego->add_physicles();
        to_physicle_common(catalog, phy->mutable_common());
        auto combination_offset = phy->mutable_geometory()->mutable_vehicle_coord()->mutable_combination_offset();
        combination_offset->set_x(catalog.get("offsetX", 0.0).asDouble());
        combination_offset->set_z(catalog.get("offsetZ", 0.0).asDouble());
      }
    }
    for (auto& vehicle : data.get("vehicle_catalog", {})) {
      auto veh = scene.mutable_vehicles()->Add();
      veh->set_name(vehicle.get("vehicle_name", "").asString());
      for (auto& catalog : vehicle.get("vehicle_param", {})) {
        to_physicle_common(catalog, veh->mutable_physicle()->mutable_common());
      }
    }
    for (auto& catalog : data.get("pedestrian_catalog", {})) {
      to_physicle_common(catalog, scene.mutable_vrus()->Add()->mutable_physicle()->mutable_common());
    }
    for (auto& catalog : data.get("miscobject_catalog", {})) {
      to_physicle_common(catalog, scene.mutable_miscs()->Add()->mutable_physicle());
    }

    std::string raw;
    std::string plaintext;
    scene.SerializeToString(&raw);
    google::protobuf::util::MessageToJsonString(scene, &plaintext, {});

    data.clear();
    data["plaintext"] = plaintext;
    data["raw"] = utils::Base64Encode(raw.begin(), raw.end());
  }

  return utils::Status::OK();
}

utils::Status SceneCatalogData::SetCatalogImpl(const char* param) {
  SceneCatalogData::Options opts(param);
  if (!opts.Valid()) {
    return utils::Status::InvalidArgument("invalid param", param);
  }

  if (valid_types_.count(opts.type) <= 0) {
    return utils::Status::InvalidArgument("invalid type", opts.type);
  }

  if (!opts.secondary_sub_type.empty() && valid_types_.count(opts.secondary_sub_type) <= 0) {
    return utils::Status::InvalidArgument("invalid secondary sub type", opts.secondary_sub_type);
  }

  if (valid_cmds_.count(opts.cmd) <= 0) {
    return utils::Status::InvalidArgument("invalid cmd", opts.cmd);
  }

  if (opts.type == FLAGS_type_vehicle) {
    return SetVehicleCatalog(param);
  } else if (opts.type == FLAGS_type_sensor || opts.type == FLAGS_type_v2x) {
    return SetSensorCatalog(param);
  } else {
    return SetDynamicCatalog(param);
  }

  return utils::Status::OK();
}

utils::Status SceneCatalogData::SetDynamicCatalog(const Options& opts) {
  boost::filesystem::path res_dir(CEngineConfig::Instance().ResourceDir());

  for (auto& one : opts.data) {
    std::string id = std::to_string(one.get("id", 0).asInt());
    std::string file_name = std::string("dynamic_") + id + ".json";
    std::string dynamic_file = (res_dir / (kDynamicPath + file_name)).string();
    boost::filesystem::create_directories(boost::filesystem::path(dynamic_file).parent_path());

    if (opts.cmd == FLAGS_cmd_delete) {
      CSceneDataProxy::Instance().Delete(opts.bucket, opts.key + kDynamicPath + file_name, dynamic_file);
      continue;
    }

    Json::Value config;
    if (misc::ParseJson(one.get("config", "").asString(), config, "SetDynamicCatalog") != 0) {
      return utils::Status::InvalidArgument("invalid config", JsonToString(one));
    }
    // write to file
    std::ofstream ofs(dynamic_file);
    Json::StreamWriterBuilder builder;
    builder["indentation"] = " ";
    builder["enableYAMLCompatibility"] = true;
    std::unique_ptr<Json::StreamWriter> const writer(builder.newStreamWriter());
    writer->write(config, &ofs);
    ofs.close();
    CSceneDataProxy::Instance().Upload(opts.bucket, opts.key + kDynamicPath + file_name, dynamic_file);
  }
  return utils::Status::OK();
}

utils::Status SceneCatalogData::SetSensorCatalog(const Options& opts) {
  boost::filesystem::path res_dir(CEngineConfig::Instance().ResourceDir());
  std::string sensor_catalog_xosc = (res_dir / kSensorCatalogXoscPath).string();
  boost::filesystem::create_directories(boost::filesystem::path(sensor_catalog_xosc).parent_path());

  CSceneDataProxy::Instance().Download(opts.bucket, opts.key + kSensorCatalogXoscPath, sensor_catalog_xosc, true);
  utils::Status status;
  for (auto& one : opts.data) {
    std::string idx = std::to_string(one.get("id", 0).asInt());
    Json::Value config(Json::objectValue);
    config["idx"] = idx;
    config["name"] = one.get("name", "");
    config["type"] = one.get("subType", "");

    std::string sensor_group_name;
    if (opts.type == FLAGS_type_v2x && config["type"].asString() == FLAGS_sensor_type_of_obu) {
      sensor_group_name = FLAGS_sensor_group_name_prefix_of_obu + idx;
    }

    // 1st delete
    SensorV2{}.deleteSensorDefine(sensor_catalog_xosc.c_str(), idx);
    if (opts.cmd == FLAGS_cmd_delete) {
      if (!sensor_group_name.empty()) {
        SensorV2{}.deleteGroupSensor(sensor_group_name, sensor_catalog_xosc);
      }
      continue;
    }
    // 2nd insert
    if (misc::ParseJson(one.get("config", "").asString(), config["param"], "SetSensorCatalog: ") != 0) {
      return utils::Status::InvalidArgument("invalid config", JsonToString(one));
    }
    SensorBaseV2 base;
    base.fromJson(config);
    if (!SensorV2{}.addSensorDefine(sensor_catalog_xosc.c_str(), base)) {
      return utils::Status::Corruption("sensor to xml error", JsonToString(config));
    }
    if (!sensor_group_name.empty()) {
      SensorGroup group;
      Sensor sensor;
      sensor.idx = idx;
      group.m_name = sensor_group_name;
      group.m_SensorVec.emplace_back(sensor);
      SensorV2{}.addGroupSensor(group, sensor_catalog_xosc);
    }
  }

  // upload
  CSceneDataProxy::Instance().Upload(opts.bucket, opts.key + kSensorCatalogXoscPath, sensor_catalog_xosc);
  return utils::Status::OK();
}

utils::Status SceneCatalogData::SetVehicleCatalog(const Options& opts) {
  boost::filesystem::path res_dir(CEngineConfig::Instance().ResourceDir());
  std::string default_xosc = (res_dir / kDefaultXoscPath).string();
  std::string sensor_catalog_xosc = (res_dir / kSensorCatalogXoscPath).string();
  boost::filesystem::create_directories(boost::filesystem::path(default_xosc).parent_path());
  boost::filesystem::create_directories(boost::filesystem::path(sensor_catalog_xosc).parent_path());

  CSceneDataProxy::Instance().Download(opts.bucket, opts.key + kDefaultXoscPath, default_xosc, true);
  CSceneDataProxy::Instance().Download(opts.bucket, opts.key + kSensorCatalogXoscPath, sensor_catalog_xosc, true);

  utils::Status status;

  for (auto& one : opts.data) {
    Json::Value config;
    if (misc::ParseJson(one.get("config", "").asString(), config, "SetVehicleCatalog: ") != 0) {
      return utils::Status::InvalidArgument("invalid record", JsonToString(one));
    }

    CCataLog catalog;
    catalog.parseDriverXML(default_xosc);
    std::string vehicle_name = config.get("vehicle_name", "").asString();
    std::string vehicle_category = config.get("vehicle_category", "").asString();
    VehicleType vehicle_type = vehicle_category == "car" ? VEHICLE_CAR : VEHICLE_TRUCK;
    bool has_vehicle = catalog.findCatalog(vehicle_name, vehicle_type);
    if (opts.cmd == FLAGS_cmd_delete) {
      // delete vehicle
      if (config.get("preset", "false").asString() == "true") {
        return utils::Status::Corruption("preset catalog, not allowed to delete", JsonToString(config));
      }
      if (has_vehicle && !catalog.deleteCatalog(config.get("vehicle_name", "").asString(), default_xosc)) {
        return utils::Status::Corruption("delete config", JsonToString(config));
      }
      continue;
    } else if (opts.cmd != FLAGS_cmd_insert && !has_vehicle) {
      return utils::Status::NotFound("not found vehicle name", vehicle_name);
    } else if (opts.cmd == FLAGS_cmd_insert && has_vehicle) {
      return utils::Status::Corruption("vehicle name duplicate", vehicle_name);
    }
    // update or insert vehicle
    CDriveCatalog ego_catalog;
    ego_catalog.fromJson(config);
    if (opts.cmd == FLAGS_cmd_update) {
      catalog.updateCatalog(ego_catalog, default_xosc);
    } else {
      if (!catalog.addCatalog(ego_catalog, default_xosc)) {
        return utils::Status::Corruption("insert config", JsonToString(config));
      }
    }
    // update sensor
    SensorV2 sensor;
    if (!sensor.addGroupSensor(ego_catalog.m_group, sensor_catalog_xosc)) {
      return utils::Status::Corruption("update sensor group");
    }
  }

  // upload
  CSceneDataProxy::Instance().Upload(opts.bucket, opts.key + kDefaultXoscPath, default_xosc);
  CSceneDataProxy::Instance().Upload(opts.bucket, opts.key + kSensorCatalogXoscPath, sensor_catalog_xosc);
  return status;
}
