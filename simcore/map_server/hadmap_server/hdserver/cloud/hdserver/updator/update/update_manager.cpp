/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "update_manager.h"
#include <boost/filesystem.hpp>
#include "../../src/database/db.h"
#include "common/log/system_logger.h"

CUpdatorManager::CUpdatorManager() {}

CUpdatorManager& CUpdatorManager::Instance() {
  static CUpdatorManager instance;
  return instance;
}

int CUpdatorManager::Update(std::string strInstallDir, std::string strAppDir) {
  SystemLogger::Info("start update hadmapserver");

  // update scenario
  CUpdatorManager::Instance().UpdateScenario();
  // update db
  CUpdatorManager::Instance().UpdateDB();
  // update hadmap
  CUpdatorManager::Instance().UpdateHadmap();
  // update config
  CUpdatorManager::Instance().UpdateConfig(strInstallDir, strAppDir);

  SystemLogger::Info("finish update hadmapserver");
  return 0;
}

void CUpdatorManager::UpdateScenario() {
  SystemLogger::Info("start update scenario");
  SystemLogger::Info("finish update scenario");
}

void CUpdatorManager::UpdateDB() {
  SystemLogger::Info("start update db");
  sTagDBVersion ver;
  CDatabase::Instance().DBVersion(ver);
  SystemLogger::Info("database version is : %d.%d.%d", ver.nMajor, ver.nMinor, ver.nRevision);

  bool bExist = CDatabase::Instance().TableExists(CDatabase::s_VERSION);
  if (!bExist) {
    CDatabase::Instance().CreateVersionTable();
  }
  SystemLogger::Info("finish update db");
}

void CUpdatorManager::UpdateHadmap() {
  SystemLogger::Info("start update hadmap");
  SystemLogger::Info("finish update hadmap");
}

void CUpdatorManager::UpdateConfig(std::string strInstallDir, std::string strAppDir) {
  SystemLogger::Info("start update Config");

  boost::filesystem::path pScenario = strAppDir;
  pScenario /= "scenario";
  boost::filesystem::path pGrading = pScenario;
  pGrading /= "grading.xml";

  boost::filesystem::path pInstalledScenario = strInstallDir;
  pInstalledScenario /= "scenario";
  boost::filesystem::path pInstalledGrading = pInstalledScenario;
  pInstalledGrading /= "grading.xml";

  if (!boost::filesystem::exists(pGrading)) {
    if (!boost::filesystem::exists(pInstalledGrading)) {
      SystemLogger::Error("installed grading not exist! path: %s", pInstalledGrading.string().c_str());
      return;
    }

    boost::filesystem::copy(pInstalledGrading, pGrading);
  }

  SystemLogger::Info("finish update Config");
}
