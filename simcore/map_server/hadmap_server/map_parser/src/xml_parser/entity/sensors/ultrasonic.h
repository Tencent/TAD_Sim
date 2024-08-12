/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "base.h"

class CSensorUltrasonic : public CSensorBase {
 public:
  CSensorUltrasonic();
  void Reset();
  std::string m_strDevice;
  std::string m_strFrequency;        // 频率  [20000-9999999] Hz
  std::string m_strFOVHorizontal;    // 水平fov  (0-90) °
  std::string m_strFOVVertical;      // 垂直fov  (0-FOV_Horizontal) °
  std::string m_strDBmin;            // 有效分贝  [-1--10] dB  （负数）
  std::string m_strRadius;           // 发声半径  [0.001-0.3]m
  std::string m_strNoiseFactor;      // 噪声系数  (0.1-10]
  std::string m_strNoiseStd;         // 噪声方差  [0.1-10]
  std::string m_strAttachmentType;   // 附着物类型  [None,Water,Mud]
  std::string m_strAttachmentRange;  // 附着物程度  [0-1]
  std::string m_strDistance;         // 探测距离
};
