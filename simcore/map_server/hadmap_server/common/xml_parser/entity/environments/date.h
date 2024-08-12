/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>

class CWeatherDate {
 public:
  CWeatherDate();
  void Reset();

  int64_t m_lTimeStamp;
  std::string m_strYear;
  std::string m_strMonth;
  std::string m_strDay;
};
