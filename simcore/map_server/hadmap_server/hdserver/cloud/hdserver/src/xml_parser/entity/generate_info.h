/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

class CGenerateInfo {
 public:
  CGenerateInfo();

  void Reset();

  std::string m_strOriginFile;

  std::string m_strGenerateInfo;
};
