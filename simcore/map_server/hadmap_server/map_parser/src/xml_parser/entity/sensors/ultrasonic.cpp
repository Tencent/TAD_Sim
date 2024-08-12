/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "ultrasonic.h"

CSensorUltrasonic::CSensorUltrasonic() { Reset(); }

void CSensorUltrasonic::Reset() {
  CSensorBase::Reset();

  m_strDevice = ".0";
  m_strFrequency = "40000";      // 频率  [20000-9999999] Hz
  m_strFOVHorizontal = "60";     // 水平fov  (0-90) °
  m_strFOVVertical = "30";       // 垂直fov  (0-FOV_Horizontal) °
  m_strDBmin = "-6";             // 有效分贝  [-1--10] dB  （负数）
  m_strRadius = "0.01";          // 发声半径  [0.001-0.3]m
  m_strNoiseFactor = "5";        // 噪声系数  (0.1-10]
  m_strNoiseStd = "5";           // 噪声方差  [0.1-10]
  m_strAttachmentType = "None";  // 附着物类型  [None,Water,Mud]
  m_strAttachmentRange = "0.5";  // 附着物程度  [0-1]
  m_strDistance = "300";
}
