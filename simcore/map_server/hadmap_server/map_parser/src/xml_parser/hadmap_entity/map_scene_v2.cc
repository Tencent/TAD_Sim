/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_scene_v2.h"
#include "cross.h"
#include "curve_road.h"
#include "entity_link.h"
#include "group_entity.h"
#include "straight_road.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include "../../engine/config.h"
#include "common/log/system_logger.h"
#include "map_operator.h"

CMapSceneV2::CMapSceneV2() {}

CMapSceneV2::~CMapSceneV2() {}

void CMapSceneV2::SetName(std::string strName) { m_strName = strName; }

void CMapSceneV2::SetData(std::string strData) { m_strData = strData; }

int CMapSceneV2::SaveToJson(const char* strSceneName, const char* strData) {
  // LOG(INFO) << "map scene save to xml";
  SYSTEM_LOGGER_INFO("map scene save to json");

  if (!strSceneName || !strData) {
    return -1;
  }

  // hadmap_scene/
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path initPath = strResourceDir;
  boost::filesystem::path hadmapScene = initPath;
  hadmapScene.append("hadmap_scene");
  boost::filesystem::path pSceneName = strSceneName;

  std::string strExt = pSceneName.extension().string();
  if (strExt.size() == 0 || !boost::algorithm::iequals(strExt, ".json")) {
    std::string strScene = strSceneName;
    strScene.append(".json");
    hadmapScene.append(strScene);
  }
  std::ofstream file(hadmapScene.string());
  if (file.is_open()) {
    file << strData;
    file.close();
  }

  return 0;
}

std::string CMapSceneV2::ParseFromJson(const char* strSceneName) {
  std::fstream fs(strSceneName);

  std::string strData = "";
  std::string strLine;
  while (fs >> strLine) {
    strData.append(strLine);
  }

  return strData;
}