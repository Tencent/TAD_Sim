/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>

class CWeatherTime {
 public:
  CWeatherTime();
  void Reset();

  int64_t m_lTimeStamp;
  std::string m_strHour;
  std::string m_strMinute;
  std::string m_strSecond;
  std::string m_strMillisecond;
};
