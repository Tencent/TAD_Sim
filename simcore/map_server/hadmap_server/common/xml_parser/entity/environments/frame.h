/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/xml_parser/entity/environments/date.h"
#include "common/xml_parser/entity/environments/time.h"
#include "common/xml_parser/entity/environments/weather.h"

class CWeatherFrame {
 public:
  CWeatherFrame();
  void Reset();
  int64_t m_timestamp;
  CWeatherDate m_date;
  CWeatherTime m_time;
  CWeatherV2 m_weather;

 public:
  std::string toDateTime();
  std::string getCloudState();
  void setDateTime(std::string strDateTime);
  void setWeather(CWeatherV2 weather);
};
