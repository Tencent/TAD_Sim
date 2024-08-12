/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include <iostream>
#include "./backup/backup_manager.h"
#include "./checker/checker_manager.h"
#include "./update/update_manager.h"
// #include "../src/server_map_cache/map_data_cache.h"
// #include "../src/server_map_cache/map_data_proxy.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "please input app install_dir app_dir" << std::endl;
    return 0;
  }

  std::string strInstallDir = argv[1];
  std::string strAppDir = argv[2];

  std::cout << "updator start!" << std::endl;

  std::cout << "install dir is : " << strInstallDir << std::endl;
  std::cout << "application dir is : " << strAppDir << std::endl;

  int nRet = CCheckerManager::Instance().Check(strInstallDir, strAppDir);
  if (nRet != 0) {
    std::cout << "resource check failed!" << std::endl;
    return -1;
  }

  /*
  sTagServiceHadmapInfo info;
  info.m_strLastModifiedTime = "Tue, 08 Dec 2020 11:28:11 GMT";
  info.ToTimeStamp();
  info.ToTimeStamp();
  CMapDataCache::Instance().LoadHadMap(L"sihuan.sqlite");
  CMapDataCache::Instance().LoadHadMap(L"sihuan.sqlite");
  */

  nRet = CBackupManager::Instance().Backup(strInstallDir, strAppDir);
  if (nRet != 0) {
    std::cout << "resource backup failed!" << std::endl;
    return -1;
  }

  nRet = CUpdatorManager::Instance().Update(strInstallDir, strAppDir);
  if (nRet != 0) {
    std::cout << "resource update failed!" << std::endl;
    return -1;
  }

  return 0;
}
