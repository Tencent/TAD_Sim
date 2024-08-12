/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include "base.h"

class CSensorRadar : public CSensorBase {
 public:
  CSensorRadar();
  void Reset();

  std::string m_strFrequency;
  std::string m_strDevice;
  // std::string m_strSaveData;
  bool m_bSaveData;
  std::string m_strF0GHz;
  std::string m_strPtDBm;
  std::string m_strGTDBi;
  std::string m_strGrDBi;
  std::string m_strTsK;
  std::string m_strFnDB;
  std::string m_strL0DB;
  std::string m_strSNRMinDB;
  std::string m_strRadarAngle;
  std::string m_strRM;
  std::string m_strRcs;
  std::string m_strWeather;
  std::string m_strTag;
  std::string m_strAnneTag;
  std::string m_strHWidth;
  std::string m_strVWidth;
  std::string m_strVFov;
  std::string m_strHFov;
  std::string m_strAntennaAnglePath1;
  std::string m_strAntennaAnglePath2;
  std::string m_strDelay;
};
