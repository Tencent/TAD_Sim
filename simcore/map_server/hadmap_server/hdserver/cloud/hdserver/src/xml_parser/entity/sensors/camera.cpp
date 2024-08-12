/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "camera.h"

CSensorCamera::CSensorCamera() { Reset(); }

void CSensorCamera::Reset() {
  CSensorBase::Reset();

  m_strFrequency = "0";
  m_strDevice = ".0";
  m_nIntrinsicParamType = 0;
  m_bSaveData = true;
  m_strDisplayMode = "Color";
  m_strResHorizontal = "0";
  m_strResVertical = "0";
  m_strBlurIntensity = "0";
  m_strMotionBlurAmount = "0";
  m_strVignetteIntensity = "0";
  m_strNoiseIntensity = "Low";
  m_strDistortionParamters = "0";
  m_strIntrinsicMatrix = "0";
  m_strFOVHorizontal = "0";
  m_strFOVVertical = "0";
  m_strCCDWidth = "0";
  m_strCCDHeight = "0";
  m_strFocalLength = "0";
}
