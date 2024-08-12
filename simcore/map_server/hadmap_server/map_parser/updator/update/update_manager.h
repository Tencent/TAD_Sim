/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once
#include <string>

class CUpdatorManager {
 public:
  static CUpdatorManager& Instance();
  int Update(std::string strInstallDir, std::string strAppDir);

 protected:
  CUpdatorManager();
  void UpdateScenario();
  void UpdateHadmap();
  void UpdateConfig(std::string strInstallDir, std::string strAppDir);
  void UpdateDB();
};
