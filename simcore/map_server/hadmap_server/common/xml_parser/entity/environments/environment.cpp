/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/environments/environment.h"
#include <ctime>
#include <iomanip>
#include <vector>

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"

int CEnvironment::Parse(const char* strEnvironmentFile) {
  if (!strEnvironmentFile) return -1;

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strEnvironmentFile);

  if (!bRet) return -1;

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  if (boost::algorithm::iequals(strName, "Environment")) {
    TiXmlElement* elemTimeLine = xmlRoot->FirstChildElement("TimeLine");
    if (elemTimeLine) {
      int nRet = ParseWeathers(elemTimeLine, m_weathers);
      if (nRet != 0) {
        return -1;
      }
    }
    // convert v1.0 environment to environment 2
    for (auto& it : m_weathers) {
      double fRainy = std::atof(it.second.m_weather.m_strRainFall.c_str()) / 250.0;
      double fSnow = std::atof(it.second.m_weather.m_strSnowFall.c_str()) / 10.0;
      // Precipitation
      std::string precipitationType;
      std::string intensity;
      if (fRainy >= fSnow) {
        precipitationType = "rain";
        intensity = std::to_string(fRainy);
      } else {
        precipitationType = "snow";
        intensity = std::to_string(fSnow);
      }
      it.second.m_weather.m_strPrecipitationType = precipitationType;
      it.second.m_weather.m_strPrecipitationIntensity = intensity;
    }
  } else if (boost::algorithm::iequals(strName, "OpenSCENARIO")) {
    TiXmlElement* Catalog = xmlRoot->FirstChildElement("Catalog");
    if (!Catalog) return -1;
    int nRet = ParseWeathersV2(Catalog, m_weathers);
    if (nRet != 0) {
      return -1;
    }
  } else {
    return -1;
  }
  return 0;
}

CWeatherFrame* CEnvironment::findWeatherFrame(int64_t timeStamp) {
  Weathers::iterator itr = m_weathers.find(timeStamp);
  if (itr != m_weathers.end()) {
    return &(itr->second);
  }

  return nullptr;
}

CWeatherFrame* CEnvironment::getFirstFrame() {
  if (m_weathers.size() > 0) {
    return &(m_weathers.begin()->second);
  }

  return nullptr;
}

int CEnvironment::ParseTimeStamp(TiXmlElement* elem, int64_t& timeStamp) {
  if (!elem) return -1;
  const char* p = elem->Attribute("TimeStamp");
  if (p) {
    timeStamp = atoi(p);
  } else {
    timeStamp = 0;
  }

  return 0;
}

int CEnvironment::ParseOneDate(TiXmlElement* elemDate, CWeatherDate& d) {
  if (!elemDate) return -1;

  ParseTimeStamp(elemDate, d.m_lTimeStamp);

  const char* p = elemDate->Attribute("Year");
  if (p) d.m_strYear = p;

  p = elemDate->Attribute("Month");
  if (p) d.m_strMonth = p;

  p = elemDate->Attribute("Day");
  if (p) d.m_strDay = p;

  return 0;
}

int CEnvironment::ParseOneTime(TiXmlElement* elemTime, CWeatherTime& t) {
  if (!elemTime) return -1;

  ParseTimeStamp(elemTime, t.m_lTimeStamp);

  const char* p = elemTime->Attribute("Hour");
  if (p) t.m_strHour = p;

  p = elemTime->Attribute("Minute");
  if (p) t.m_strMinute = p;

  p = elemTime->Attribute("Second");
  if (p) t.m_strSecond = p;

  p = elemTime->Attribute("Millisecond");
  if (p) t.m_strMillisecond = p;

  return 0;
}

int CEnvironment::ParseOneWeather(TiXmlElement* elemWeather, CWeather& w) {
  if (!elemWeather) return -1;

  ParseTimeStamp(elemWeather, w.m_lTimeStamp);

  const char* p = elemWeather->Attribute("Visibility");
  if (p) w.m_strVisibility = p;

  p = elemWeather->Attribute("RainFall");
  if (p) w.m_strRainFall = p;

  p = elemWeather->Attribute("SnowFall");
  if (p) w.m_strSnowFall = p;

  p = elemWeather->Attribute("WindSpeed");
  if (p) w.m_strWindSpeed = p;

  p = elemWeather->Attribute("CloudDensity");
  if (p) w.m_strCloudDensity = p;
  p = elemWeather->Attribute("Temperature");
  if (p) {
    w.m_strTemperature = p;
  } else {
    w.m_strTemperature = 30.0;
  }
  return 0;
}

int CEnvironment::ParseWeathers(TiXmlElement* elemTimeline, Weathers& weathers) {
  if (!elemTimeline) return -1;

  weathers.clear();

  TiXmlElement* elemDate = elemTimeline->FirstChildElement("Date");

  while (elemDate) {
    int64_t timeStamp = 0;
    ParseTimeStamp(elemDate, timeStamp);

    CWeatherFrame* f = findWeatherFrame(timeStamp);

    if (!f) {
      CWeatherFrame frame;
      frame.m_timestamp = timeStamp;
      weathers.insert(std::make_pair(timeStamp, frame));
      f = findWeatherFrame(timeStamp);
    }

    int nRet = ParseOneDate(elemDate, f->m_date);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse weather timeline date error!");
      // assert(false);

      elemDate = elemDate->NextSiblingElement("Date");
      continue;
    }

    elemDate = elemDate->NextSiblingElement("Date");
  }

  TiXmlElement* elemTime = elemTimeline->FirstChildElement("Time");
  while (elemTime) {
    int64_t timeStamp = 0;
    ParseTimeStamp(elemTime, timeStamp);

    CWeatherFrame* f = findWeatherFrame(timeStamp);

    if (!f) {
      CWeatherFrame frame;
      frame.m_timestamp = timeStamp;
      weathers.insert(std::make_pair(timeStamp, frame));
      f = findWeatherFrame(timeStamp);
    }

    int nRet = ParseOneTime(elemTime, f->m_time);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse weather timeline time error!");
      assert(false);

      elemTime = elemTime->NextSiblingElement("Time");
      continue;
    }

    elemTime = elemTime->NextSiblingElement("Time");
  }

  TiXmlElement* elemWeather = elemTimeline->FirstChildElement("Weather");
  while (elemWeather) {
    int64_t timeStamp = 0;
    ParseTimeStamp(elemWeather, timeStamp);

    CWeatherFrame* f = findWeatherFrame(timeStamp);

    if (!f) {
      CWeatherFrame frame;
      frame.m_timestamp = timeStamp;
      weathers.insert(std::make_pair(timeStamp, frame));
      f = findWeatherFrame(timeStamp);
    }

    int nRet = ParseOneWeather(elemWeather, f->m_weather);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse weather timeline weather error!");
      assert(false);

      elemWeather = elemWeather->NextSiblingElement("Weather");
      continue;
    }

    elemWeather = elemWeather->NextSiblingElement("Weather");
  }
  return 0;
}

int CEnvironment::ParseWeathersV2(TiXmlElement* eleCatalog, Weathers& weathers) {
  weathers.clear();
  TiXmlElement* elemEnvironment = eleCatalog->FirstChildElement("Environment");
  while (elemEnvironment) {
    if (!elemEnvironment) return -1;
    if (!elemEnvironment->FirstChildElement("ParameterDeclarations")) {
      return -1;
    }
    CWeatherFrame tmp;
    // ParameterDeclarations
    TiXmlElement* ParameterDeclarations = elemEnvironment->FirstChildElement("ParameterDeclarations");
    TiXmlElement* ParameterDeclaration = ParameterDeclarations->FirstChildElement("ParameterDeclaration");
    uint64_t nTimeStamp = 0;
    while (ParameterDeclaration) {
      std::string strKey = ParameterDeclaration->Attribute("name");
      std::string strValue = ParameterDeclaration->Attribute("value");
      if (strKey == "TimeStamp") {
        nTimeStamp = std::atoi(strValue.c_str());
        tmp.m_timestamp = nTimeStamp;
        tmp.m_weather.m_lTimeStamp = nTimeStamp;
      } else if (strKey == "wind_speed") {
        tmp.m_weather.m_strWindSpeed = strValue;
      } else if (strKey == "tmperature") {
        tmp.m_weather.m_strTemperature = std::to_string(std::atof(strValue.c_str()) - 273.15);
      }
      ParameterDeclaration = ParameterDeclaration->NextSiblingElement("ParameterDeclaration");
    }
    // TimeOfDay
    TiXmlElement* _TimeOfDay = elemEnvironment->FirstChildElement("TimeOfDay");
    if (!_TimeOfDay) return -1;
    std::string strTime = _TimeOfDay->Attribute("dateTime");
    std::istringstream ss(strTime);
    std::vector<std::string> strVec;
    std::string item;
    while (std::getline(ss, item, '.')) {
      strVec.push_back(item);
    }
    if (strVec.size() < 2) {
      SYSTEM_LOGGER_ERROR("Failed to parse time");
      return -1;
    }
    // std::istringstream ssDate(strVec.at(0));
    // tm t = {};
    // ssDate >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");
    tmp.m_date.m_lTimeStamp = nTimeStamp;
    // tmp.m_date.m_strYear = std::to_string(t.tm_year + 1900);
    // tmp.m_date.m_strMonth = std::to_string(t.tm_mon + 1);
    // tmp.m_date.m_strDay = std::to_string(t.tm_mday);
    // tmp.m_time.m_strHour = std::to_string(t.tm_hour);
    // tmp.m_time.m_strMinute = std::to_string(t.tm_min);
    // tmp.m_time.m_strSecond = std::to_string(t.tm_sec);
    std::istringstream iss(strVec.at(0));

    std::vector<std::string> tokens;
    std::string token;

    while (std::getline(iss, token, 'T')) {
      tokens.push_back(token);
    }

    if (tokens.size() != 2) {
      SYSTEM_LOGGER_ERROR("Failed to parse time");
      return -1;
    }

    std::istringstream date_iss(tokens[0]);
    std::istringstream time_iss(tokens[1]);

    std::vector<int> date_parts;
    std::vector<int> time_parts;

    while (std::getline(date_iss, token, '-')) {
      date_parts.push_back(std::stoi(token));
    }

    while (std::getline(time_iss, token, ':')) {
      time_parts.push_back(std::stoi(token));
    }

    if (date_parts.size() != 3 || time_parts.size() != 3) {
      SYSTEM_LOGGER_ERROR("Failed to parse time");
      return -1;
    }
    tmp.m_date.m_strYear = std::to_string(date_parts.at(0));
    tmp.m_date.m_strMonth = std::to_string(date_parts.at(1));
    tmp.m_date.m_strDay = std::to_string(date_parts.at(2));
    tmp.m_time.m_strHour = std::to_string(time_parts.at(0));
    tmp.m_time.m_strMinute = std::to_string(time_parts.at(1));
    tmp.m_time.m_strSecond = std::to_string(time_parts.at(2));

    tmp.m_time.m_strMillisecond = strVec.at(1);
    // Weather
    TiXmlElement* _Weather = elemEnvironment->FirstChildElement("Weather");
    if (!_Weather) return -1;
    tmp.m_weather.m_strCloudState = _Weather->Attribute("cloudState");
    TiXmlElement* _Fog = _Weather->FirstChildElement("Fog");
    if (!_Fog) return -1;
    tmp.m_weather.m_strVisualRange = std::to_string(std::atof(_Fog->Attribute("visualRange")) / 1000.0);
    TiXmlElement* _Precipitation = _Weather->FirstChildElement("Precipitation");
    if (!_Precipitation) return -1;
    tmp.m_weather.m_strPrecipitationType = _Precipitation->Attribute("precipitationType");
    tmp.m_weather.m_strPrecipitationIntensity = _Precipitation->Attribute("intensity");

    weathers.insert(std::make_pair(nTimeStamp, tmp));
    elemEnvironment = elemEnvironment->NextSiblingElement("Environment");
  }
  return 0;
}

int CEnvironment::Save(const char* strEnvironmentFile, const char* version) {
  if (!strEnvironmentFile) return -1;
  TiXmlDocument doc;
  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  // v1.0
  if (std::string(version) == "1.0") {
    TiXmlElement* elemEnvironment = new TiXmlElement("Environment");
    elemEnvironment->SetAttribute("version", "1.0");

    CWeatherFrame* f = getFirstFrame();

    if (f) {
      int nRet = SaveOneWeather(elemEnvironment, f->m_weather, false);
      if (nRet) {
        // assert(false);
        return -1;
      }

      nRet = SaveOneDate(elemEnvironment, f->m_date, false);
      if (nRet) {
        // assert(false);
      }

      nRet = SaveOneTime(elemEnvironment, f->m_time, false);
      if (nRet) {
        // assert(false);
        return -1;
      }
    }
    int nRet = SaveWeathers(elemEnvironment, m_weathers);
    if (nRet) {
      // assert(false);
      return -1;
    }
    doc.LinkEndChild(dec);
    doc.LinkEndChild(elemEnvironment);
    doc.SaveFile(strEnvironmentFile);
  } else if (std::string(version) == "2.0") {
    // v2.0
    TiXmlElement* osc = new TiXmlElement("OpenSCENARIO");
    TiXmlElement* FileHeader = new TiXmlElement("FileHeader");
    FileHeader->SetAttribute("revMajor", 1);
    FileHeader->SetAttribute("revMinor", 0);
    char buf[64] = {0};
    std::time_t now = std::time(nullptr);
    std::size_t size = std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));
    FileHeader->SetAttribute("date", buf);
    FileHeader->SetAttribute("author", "TAD Sim");
    osc->LinkEndChild(FileHeader);
    // Catalog
    TiXmlElement* Catalog = new TiXmlElement("Catalog");
    int index = 0;
    for (auto it : m_weathers) {
      TiXmlElement* Environment = new TiXmlElement("Environment");
      Environment->SetAttribute("name", std ::string("Environment") + std::to_string(index));
      TiXmlElement* ParameterDeclarations = new TiXmlElement("ParameterDeclarations");

      SaveOneParamDeclaration(ParameterDeclarations, "TimeStamp", "integer", std::to_string(it.first));
      SaveOneParamDeclaration(ParameterDeclarations, "UsingSunByUser", "boolean", "false");
      SaveOneParamDeclaration(ParameterDeclarations, "wind_speed", "double", it.second.m_weather.m_strWindSpeed);
      SaveOneParamDeclaration(ParameterDeclarations, "tmperature", "double",
                              std::to_string(atof(it.second.m_weather.m_strTemperature.c_str()) + 273.15));
      Environment->LinkEndChild(ParameterDeclarations);
      // TimeOfDay
      TiXmlElement* _TimeOfDay = new TiXmlElement("TimeOfDay");
      _TimeOfDay->SetAttribute("animation", "false");

      int year = std::atoi(it.second.m_date.m_strYear.c_str());
      int month = std::atoi(it.second.m_date.m_strMonth.c_str());
      int day = std::atoi(it.second.m_date.m_strDay.c_str());
      int hour = std::atoi(it.second.m_time.m_strHour.c_str());
      int minute = std::atoi(it.second.m_time.m_strMinute.c_str());
      int second = std::atoi(it.second.m_time.m_strSecond.c_str());
      int millisecond = 500;

      tm t = {};
      t.tm_year = year - 1900;
      t.tm_mon = month - 1;
      t.tm_mday = day;
      t.tm_hour = hour;
      t.tm_min = minute;
      t.tm_sec = second;
      time_t tt = mktime(&t);
      char buf1[64] = {0};
      std::size_t size = std::strftime(buf1, sizeof(buf1), "%Y-%m-%dT%H:%M:%S", std::localtime(&tt));
      std::string _dateTime = buf1;
      std::stringstream ss;
      ss << std::setw(3) << std::setfill('0') << std::atoi(it.second.m_time.m_strMillisecond.c_str());
      _dateTime += ("." + ss.str());
      _TimeOfDay->SetAttribute("dateTime", _dateTime);
      Environment->LinkEndChild(_TimeOfDay);
      // Weather
      TiXmlElement* _Weather = new TiXmlElement("Weather");
      _Weather->SetAttribute("cloudState", it.second.m_weather.m_strCloudState);
      TiXmlElement* _Sun = new TiXmlElement("Sun");
      _Sun->SetAttribute("azimuth", 10000);
      _Sun->SetAttribute("elevation", 40);
      _Sun->SetAttribute("intensity", 20);
      _Weather->LinkEndChild(_Sun);
      TiXmlElement* _Fog = new TiXmlElement("Fog");
      _Fog->SetAttribute("visualRange", std::to_string(atof(it.second.m_weather.m_strVisualRange.c_str()) * 1000.0));
      _Weather->LinkEndChild(_Fog);
      TiXmlElement* _Precipitation = new TiXmlElement("Precipitation");
      _Precipitation->SetAttribute("intensity", it.second.m_weather.m_strPrecipitationIntensity);
      _Precipitation->SetAttribute("precipitationType", it.second.m_weather.m_strPrecipitationType);
      _Weather->LinkEndChild(_Precipitation);
      Environment->LinkEndChild(_Weather);
      // RoadCondition
      TiXmlElement* _RoadCondition = new TiXmlElement("RoadCondition");
      _RoadCondition->SetAttribute("frictionScaleFactor", 1);
      Environment->LinkEndChild(_RoadCondition);
      Catalog->LinkEndChild(Environment);
      index++;
    }
    osc->LinkEndChild(Catalog);
    doc.LinkEndChild(dec);
    doc.LinkEndChild(osc);
    doc.SaveFile(strEnvironmentFile);
  } else {
    return -1;
  }
  return 0;
}

int CEnvironment::SaveOneDate(TiXmlElement* elemParent, CWeatherDate& date, bool saveTimeStamp) {
  if (!elemParent) {
    return -1;
  }

  TiXmlElement* elemDate = new TiXmlElement("Date");

  if (saveTimeStamp) {
    elemDate->SetAttribute("TimeStamp", date.m_lTimeStamp);
  }

  if (date.m_strDay.size() > 0) {
    elemDate->SetAttribute("Year", date.m_strYear);
    elemDate->SetAttribute("Month", date.m_strMonth);
    elemDate->SetAttribute("Day", date.m_strDay);
    elemParent->LinkEndChild(elemDate);
  }

  return 0;
}

int CEnvironment::SaveOneTime(TiXmlElement* elemParent, CWeatherTime& time, bool saveTimeStamp) {
  if (!elemParent) {
    return -1;
  }

  TiXmlElement* elemTime = new TiXmlElement("Time");

  if (saveTimeStamp) {
    elemTime->SetAttribute("TimeStamp", time.m_lTimeStamp);
  }

  if (time.m_strHour.size() > 0) {
    elemTime->SetAttribute("Hour", time.m_strHour);
    elemTime->SetAttribute("Minute", time.m_strMinute);
    elemTime->SetAttribute("Second", time.m_strSecond);
    elemTime->SetAttribute("Millisecond", time.m_strMillisecond);
    elemParent->LinkEndChild(elemTime);
  }

  return 0;
}

int CEnvironment::SaveOneWeather(TiXmlElement* elemParent, CWeather& weather, bool saveTimeStamp) {
  if (!elemParent) {
    return -1;
  }

  TiXmlElement* elemWeather = new TiXmlElement("Weather");

  if (saveTimeStamp) {
    elemWeather->SetAttribute("TimeStamp", weather.m_lTimeStamp);
  }
  elemWeather->SetAttribute("Visibility", weather.m_strVisibility);
  elemWeather->SetAttribute("RainFall", weather.m_strRainFall);
  elemWeather->SetAttribute("SnowFall", weather.m_strSnowFall);
  elemWeather->SetAttribute("WindSpeed", weather.m_strWindSpeed);
  elemWeather->SetAttribute("CloudDensity", weather.m_strCloudDensity);
  elemWeather->SetAttribute("Temperature", weather.m_strTemperature);
  elemParent->LinkEndChild(elemWeather);

  return 0;
}

void CEnvironment::SaveOneParamDeclaration(TiXmlElement* elemparam, std::string name, std::string type,
                                           std::string value) {
  TiXmlElement* ParameterDeclaration = new TiXmlElement("ParameterDeclaration");
  ParameterDeclaration->SetAttribute("name", name);
  ParameterDeclaration->SetAttribute("parameterType", type);
  ParameterDeclaration->SetAttribute("value", value);
  elemparam->LinkEndChild(ParameterDeclaration);
  return;
}

int CEnvironment::SaveWeathers(TiXmlElement* elemParent, Weathers& weathers) {
  TiXmlElement* elemTimeLine = new TiXmlElement("TimeLine");

  Weathers::iterator itr = weathers.begin();
  for (; itr != weathers.end(); ++itr) {
    int nRet = SaveOneDate(elemTimeLine, itr->second.m_date, true);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one weather date error");
      // assert(false);
      continue;
    }

    nRet = SaveOneTime(elemTimeLine, itr->second.m_time, true);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one weather time error");
      // assert(false);
      continue;
    }

    nRet = SaveOneWeather(elemTimeLine, itr->second.m_weather, true);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one weather error");
      // assert(false);
      continue;
    }
  }

  elemParent->LinkEndChild(elemTimeLine);

  return 0;
}
