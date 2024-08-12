/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "fisheye.h"

CSensorFisheye::CSensorFisheye() { Reset(); }

void CSensorFisheye::Reset() {
  CSensorBase::Reset();

  m_strDevice = ".0";
  m_strFrequency = "50";           // 频率  [0-1000]
  m_bSaveData = true;              // 保存文件
  m_strDisplayMode = "Color";      // 色彩模式 Color | Gray
  m_strResHorizontal = "1080";     // 水平分辨率 [1-4096]
  m_strResVertical = "1080";       // 垂直分辨率  [1-4096]
  m_strBlurIntensity = "0";        // 模糊值 [0, 1]
  m_strMotionBlurAmount = "0";     // 运动模糊 [0, 1]
  m_strVignetteIntensity = "0";    // 光晕强度 [0, 1]
  m_strNoiseIntensity = "Low";     // 噪声强度 Low | Middle | High | Off
  m_strDistortionParamters = "4";  // 畸变参数k1~k4
  m_strIntrinsicMatrix = "0";      // 内参矩阵 matrix 3*3

  m_nIntrinsicParamType = 0;
}
