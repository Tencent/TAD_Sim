/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "traditional_lindar.h"

CSensorTraditionalLindar::CSensorTraditionalLindar() { Reset(); }

void CSensorTraditionalLindar::Reset() {
  CSensorBase::Reset();

  m_strFrequency = "0";
  m_strDevice = ".0";
  // m_strDrawPoint = "true";
  // m_strDrawRay = "true";
  m_bDrawPoint = true;
  m_bDrawRay = true;
  m_strModel = "RS32";
  m_strUChannels = "16";
  m_strURange = "0";
  m_strUHorizontalResolution = "0";
  m_strUUpperFov = "0";
  m_strULowerFov = "0";
  // m_strUSaveData = "true";
  m_bUSaveData = true;
}
