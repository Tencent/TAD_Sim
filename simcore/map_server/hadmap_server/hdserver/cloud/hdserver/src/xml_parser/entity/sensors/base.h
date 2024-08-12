/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>

class CSensorBase {
 public:
  CSensorBase();
  void Reset();

  std::string m_strID;
  bool m_bEnabled;
  std::string m_strLocationX;
  std::string m_strLocationY;
  std::string m_strLocationZ;
  std::string m_strRotationX;
  std::string m_strRotationY;
  std::string m_strRotationZ;
  std::string m_strInstallSlot;
};
