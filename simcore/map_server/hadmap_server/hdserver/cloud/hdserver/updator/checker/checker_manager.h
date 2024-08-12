/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>

class CCheckerManager {
 public:
  static CCheckerManager& Instance();
  int Check(std::string strInstallDir, std::string strAppData);

 protected:
  CCheckerManager();
};
