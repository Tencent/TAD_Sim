/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "radar.h"

CSensorRadar::CSensorRadar() { Reset(); }

void CSensorRadar::Reset() {
  m_strFrequency = "0";
  m_strDevice = ".0";
  // m_strSaveData = "true";
  m_bSaveData = true;
  m_strF0GHz = "0";
  m_strPtDBm = "0";
  m_strGTDBi = "0";
  m_strGrDBi = "0";
  m_strTsK = "0";
  m_strFnDB = "0";
  m_strL0DB = "0";
  m_strSNRMinDB = "0";
  m_strRadarAngle = "0";
  m_strRM = "0";
  m_strRcs = "0";
  m_strWeather = "1";
  m_strTag = "0";
  m_strAnneTag = "0";
  m_strHWidth = "0";
  m_strVWidth = "0";
  m_strVFov = "0";
  m_strHFov = "0";
  m_strAntennaAnglePath1 = "";
  m_strAntennaAnglePath2 = "";
}
