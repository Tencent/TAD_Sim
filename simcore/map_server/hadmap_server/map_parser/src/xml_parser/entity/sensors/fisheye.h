/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "base.h"

class CSensorFisheye : public CSensorBase {
 public:
  CSensorFisheye();
  void Reset();

  std::string m_strFrequency;  // 频率  [0-1000]

  bool m_bSaveData;                      // 保存文件
  std::string m_strDisplayMode;          // 色彩模式 Color | Gray
  std::string m_strResHorizontal;        // 水平分辨率 [1-4096]
  std::string m_strResVertical;          // 垂直分辨率  [1-4096]
  std::string m_strBlurIntensity;        // 模糊值 [0, 1]
  std::string m_strMotionBlurAmount;     // 运动模糊 [0, 1]
  std::string m_strVignetteIntensity;    // 光晕强度 [0, 1]
  std::string m_strNoiseIntensity;       // 噪声强度 Low | Middle | High | Off
  std::string m_strDistortionParamters;  // 畸变参数k1~k4
  std::string m_strIntrinsicMatrix;      // 内参矩阵 matrix 3*3

  std::string m_strDevice;
  int m_nIntrinsicParamType;  // 内参形式，0：矩阵 只有0
};
