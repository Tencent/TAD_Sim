/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>

class CBackupManager {
 public:
  static CBackupManager& Instance();
  int Backup(std::string strInstallDir, std::string strAppDir);
  int Rollback(std::string strInstallDir, std::string strAppDir);

 protected:
  CBackupManager();
};
