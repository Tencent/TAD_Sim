/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
// v1.0
class CWeather {
 public:
  CWeather();
  void Reset();
  int64_t m_lTimeStamp;
  std::string m_strVisibility;
  std::string m_strRainFall;
  std::string m_strSnowFall;
  std::string m_strWindSpeed;
  std::string m_strCloudDensity;
  std::string m_strTemperature;
};

// v2.0
class CWeatherV2 : public CWeather {
 public:
  void Reset();
  // bool Parse();

 public:
  std::string m_strCloudState;
  std::string m_strPrecipitationType;
  std::string m_strPrecipitationIntensity;
  std::string m_strVisualRange;
};
