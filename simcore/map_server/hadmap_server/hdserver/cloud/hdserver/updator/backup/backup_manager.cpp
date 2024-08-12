/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "backup_manager.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "./directory_utility.h"
#include "common/log/system_logger.h"

CBackupManager::CBackupManager() {}

CBackupManager& CBackupManager::Instance() {
  static CBackupManager instance;
  return instance;
}

int CBackupManager::Backup(std::string strInstallDir, std::string strAppDir) {
  SystemLogger::Info("start backup");

  boost::filesystem::path pInstallDir = strInstallDir;
  boost::filesystem::path pAppDir = strAppDir;
  boost::filesystem::path pScenarioDir = strAppDir;
  pScenarioDir /= "scenario";

  boost::filesystem::path pScenarioBackupDir = strAppDir;
  pScenarioBackupDir /= "scenario_bak";

  if (boost::filesystem::exists(pScenarioBackupDir)) {
    CDirectoryUtility::RemoveDir(pScenarioBackupDir);
  }

  CDirectoryUtility::CopyDir(pScenarioDir, pScenarioBackupDir);

  SystemLogger::Info("finish backup");

  return 0;
}

int CBackupManager::Rollback(std::string strInstallDir, std::string strAppDir) {
  SystemLogger::Info("start rollback");

  SystemLogger::Info("finish rollback");

  return 0;
}
