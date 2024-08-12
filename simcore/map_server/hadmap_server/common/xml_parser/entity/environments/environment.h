/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <map>
#include "frame.h"
#ifndef ENVIRONMENT_VERSION
#  define ENVIRONMENT_VERSION "2.0"
#endif
class TiXmlElement;

class CEnvironment {
 public:
  typedef std::map<int64_t, CWeatherFrame> Weathers;

  Weathers& WeatherData() { return m_weathers; }

  int Parse(const char* strEnvironmentFile);
  int Save(const char* strEnvironmentFile, const char* version = ENVIRONMENT_VERSION);

 protected:
  int ParseWeathers(TiXmlElement* elemTimeline, Weathers& weathers);
  int ParseWeathersV2(TiXmlElement* elemTimeline, Weathers& weathers);

  CWeatherFrame* findWeatherFrame(int64_t timeStamp);
  CWeatherFrame* getFirstFrame();

  // v1.0
  int ParseTimeStamp(TiXmlElement* elem, int64_t& timeStamp);
  int ParseOneDate(TiXmlElement* elemDate, CWeatherDate& date);
  int ParseOneTime(TiXmlElement* elemTime, CWeatherTime& time);
  int ParseOneWeather(TiXmlElement* elemWeather, CWeather& weather);

  int SaveWeathers(TiXmlElement* elemTimeline, Weathers& weathers);
  int SaveOneDate(TiXmlElement* elemDate, CWeatherDate& date, bool bSaveStampTime);
  int SaveOneTime(TiXmlElement* elemTime, CWeatherTime& time, bool bSaveStampTime);
  int SaveOneWeather(TiXmlElement* elemWeather, CWeather& weather, bool bSaveStampTime);
  // v2.0
  void SaveOneParamDeclaration(TiXmlElement* elemparam, std::string name, std::string type, std::string value);
  Weathers m_weathers;
};
