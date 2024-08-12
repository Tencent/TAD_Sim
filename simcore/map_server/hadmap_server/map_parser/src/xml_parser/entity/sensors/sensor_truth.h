/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include "base.h"

class CSensorTruth : public CSensorBase {
 public:
  CSensorTruth();
  void Reset();

  // std::string m_strSaveData;
  bool m_bSaveData;
  std::string m_strDevice;
  std::string m_strVFov;
  std::string m_strHFov;
  std::string m_strDRange;
};
