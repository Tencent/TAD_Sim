/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include "base.h"

class CSensorTraditionalLindar : public CSensorBase {
 public:
  CSensorTraditionalLindar();
  void Reset();

  std::string m_strFrequency;
  std::string m_strDevice;
  // std::string m_strDrawPoint;
  // std::string m_strDrawRay;
  bool m_bDrawPoint;
  bool m_bDrawRay;
  std::string m_strModel;
  std::string m_strUChannels;
  std::string m_strURange;
  std::string m_strUHorizontalResolution;
  std::string m_strUUpperFov;
  std::string m_strULowerFov;
  // std::string m_strUSaveData;
  bool m_bUSaveData;
};
