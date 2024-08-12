/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "checker_manager.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "../../src/database/db.h"
#include "../../src/engine/config.h"
#include "../../src/server_map_cache/map_data_cache.h"
#include "common/log/system_logger.h"

CCheckerManager::CCheckerManager() {}

CCheckerManager& CCheckerManager::Instance() {
  static CCheckerManager instance;
  return instance;
}

int CCheckerManager::Check(std::string strInstallDir, std::string strAppDir) {
  // SYSTEM_LOGGER_INFO("start check");
  std::cout << "start check" << std::endl;

  boost::filesystem::path pInstallDir = strInstallDir;
  boost::filesystem::path pAppDir = strAppDir;
  if (!boost::filesystem::exists(pInstallDir) || !boost::filesystem::exists(pAppDir)) {
    std::cout << "input directory not exist!" << std::endl;
    return -1;
  }

  boost::filesystem::path pScenario = pAppDir;
  pScenario /= "scenario";
  std::string strResourcePath = pScenario.string();
  CLog::Initialize(strResourcePath.c_str());

  SYSTEM_LOGGER_INFO("start check ");
  CEngineConfig::Instance().Init(strResourcePath.c_str(), strAppDir.c_str());

  // CMapDataCache::Instance().Init();
  // CDatabase::Instance().Init() ;

  SYSTEM_LOGGER_INFO("finish check");
  std::cout << "start check" << std::endl;

  return 0;
}
