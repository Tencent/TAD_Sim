/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <json/json.h>
#include <boost/filesystem.hpp>
#include <string>

#include "common/map_data/map_data_predef.h"
#include "common/utils/misc.h"
#include "server_map_cache/map_data_cache.h"

namespace adapter {

inline std::string ScenarioOptionsAdapter(const char* param) {
  Json::Value root = StringToJson(param);

  // gen cos key by version
  if (root["key"].isNull() && root["version"].isString()) {
    if (root["mapName"].empty()) {
      root["mapName"] = "map.xodr";
    }
    root["key"] = GenSceneEditorCosKeyByVersion(root["version"].asString());
    root["mapUrl"] =
        GenMapEditorCosKeyByVersion(root.get("mapName", "map.xodr").asString(), root["version"].asString());
  }

  // reset filename by adsUrl/mapUrl
  auto reset_filename = [&](const char* target, const char* source) {
    try {
      std::string path = root.get(source, "").asString();
      std::string filename = boost::filesystem::path(path).filename().string();
      if (filename.empty()) {
        return;
      }
      root[target] = filename;
    } catch (const std::exception& ex) {
    } catch (...) {
    }
  };
  reset_filename("name", "adsUrl");
  reset_filename("mapName", "mapUrl");

  // replace_space
  auto replace_space = [&](const char* target) {
    std::string data = root.get(target, "").asString();
    auto pos = data.find(" ");
    if (pos != std::string::npos) {
      data = data.substr(0, pos) + "+" + data.substr(pos + 1);
      root[target] = data;
    }
  };
  replace_space("name");

  return JsonToString(root);
}

inline std::string HadmapOptionsAdapter(const std::string& name, const std::string& path) {
  if (path.empty()) {
    return name;
  }
  // coskey -> name
  std::string filename = boost::filesystem::path(path).filename().string();
  return filename.empty() ? name : filename;
}

inline std::string HadmapOptionsAdapter(const char* param) {
  if (!param) {
    return {};
  }

  // for getallroaddata/getalllanedata/...
  std::string raw_name = param;
  raw_name = raw_name.substr(raw_name.find(VERSION_NAME_DELIM) + 1);
  boost::filesystem::path p(raw_name);
  if (!p.extension().empty()) {
    return param;
  }
  for (auto& kv : kHadmapTypeDict) {
    std::string name = std::string(param) + kv.first;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring name_w = converter.from_bytes(name);
    if (CMapDataCache::Instance().HadmapCache(name_w.c_str())) {
      return name;
    }
  }
  return param;
}

inline std::string HadmapOptionsAdapter(Json::Value root) {
  // for hadmap
  std::string name = root.get("name", "").asString();
  std::string path = root.get("cosKey", "").asString();
  std::string filename = HadmapOptionsAdapter(name, path);
  root["name"] = filename;
  return JsonToString(root);
}

inline std::string HadmapOptionsAdapter(const std::string& param) { return HadmapOptionsAdapter(param.c_str()); }

}  // namespace adapter
