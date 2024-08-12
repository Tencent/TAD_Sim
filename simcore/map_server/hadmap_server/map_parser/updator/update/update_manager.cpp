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
  SYSTEM_LOGGER_INFO("start update hadmapserver");

  // update scenario
  CUpdatorManager::Instance().UpdateScenario();
  // update db
  CUpdatorManager::Instance().UpdateDB();
  // update hadmap
  CUpdatorManager::Instance().UpdateHadmap();
  // update config
  CUpdatorManager::Instance().UpdateConfig(strInstallDir, strAppDir);

  SYSTEM_LOGGER_INFO("finish update hadmapserver");

  return 0;
}

void CUpdatorManager::UpdateScenario() {
  SYSTEM_LOGGER_INFO("start update scenario");
  SYSTEM_LOGGER_INFO("finish update scenario");
}

void CUpdatorManager::UpdateDB() {
  SYSTEM_LOGGER_INFO("start update db");
  sTagDBVersion ver;
  CDatabase::Instance().DBVersion(ver);
  SYSTEM_LOGGER_INFO("database version is : %d.%d.%d", ver.nMajor, ver.nMinor, ver.nRevision);

  bool bExist = CDatabase::Instance().TableExists("version");
  if (!bExist) {
    CDatabase::Instance().CreateVersionTable();
  }

  /*
   * 大版本升级，版本号为1.1.0，上一个版本号为1.0.0
   * 支持openscenario格式的读写，数据库添加了type字段用于进行格式上的区分
   */
  if (ver.nMajor == 1 && ver.nMinor < 1) {
    ver.nMajor = 1;
    ver.nMinor = 1;
    ver.nRevision = 0;

    CDatabase::Instance().UpdateScenarioFrom1To1_1();

    // 升级版本号
    CDatabase::Instance().UpdateVersion(ver);
  }

  SYSTEM_LOGGER_INFO("finish update db");
}

void CUpdatorManager::UpdateHadmap() {
  SYSTEM_LOGGER_INFO("start update hadmap");
  SYSTEM_LOGGER_INFO("finish update hadmap");
}

void CUpdatorManager::UpdateConfig(std::string strInstallDir, std::string strAppDir) {
  SYSTEM_LOGGER_INFO("start update Config");

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
      SYSTEM_LOGGER_ERROR("installed grading not exist! path: %s", pInstalledGrading.string().c_str());
      return;
    }

    boost::filesystem::copy(pInstalledGrading, pGrading);
  }

  SYSTEM_LOGGER_INFO("finish update Config");
}
