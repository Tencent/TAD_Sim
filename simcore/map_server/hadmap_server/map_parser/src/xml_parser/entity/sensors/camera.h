/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "base.h"

class CSensorCamera : public CSensorBase {
 public:
  CSensorCamera();
  void Reset();

  std::string m_strFrequency;
  std::string m_strDevice;
  int m_nIntrinsicParamType;
  bool m_bSaveData;
  std::string m_strDisplayMode;
  std::string m_strResHorizontal;
  std::string m_strResVertical;
  std::string m_strBlurIntensity;
  std::string m_strMotionBlurAmount;
  std::string m_strVignetteIntensity;
  std::string m_strNoiseIntensity;
  std::string m_strDistortionParamters;
  std::string m_strIntrinsicMatrix;
  std::string m_strFOVHorizontal;
  std::string m_strFOVVertical;
  std::string m_strCCDWidth;
  std::string m_strCCDHeight;
  std::string m_strFocalLength;
};
