/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/environments/frame.h"
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

CWeatherFrame::CWeatherFrame() { Reset(); }

void CWeatherFrame::Reset() {
  m_timestamp = 0;
  m_date.Reset();
  m_time.Reset();
  m_weather.Reset();
}

std::string CWeatherFrame::toDateTime() {
  std::string year = m_date.m_strYear;
  // year-month-day
  std::string month = m_date.m_strMonth;
  if (month.size() < 2) month = "0" + month;
  std::string day = m_date.m_strDay;
  if (day.size() < 2) day = "0" + day;
  std::string date = year + "-" + month + "-" + day;
  // hour: minute: second
  std::string hour = m_time.m_strHour;
  if (hour.size() < 2) {
    hour = "0" + hour;
  }
  std::string minute = m_time.m_strMinute;
  if (minute.size() < 2) {
    minute = "0" + minute;
  }
  std::string second = m_time.m_strSecond;
  if (second.size() < 2) {
    second = "0" + second;
  }
  std::string time = hour + ":" + minute + ":" + second;
  return std::string(date + "T" + time);
}

std::string CWeatherFrame::getCloudState() { return this->m_weather.m_strCloudState; }

void CWeatherFrame::setDateTime(std::string strDateTime) {
  std::vector<std::string> strDate_Time;
  boost::algorithm::split(strDate_Time, strDateTime, boost::algorithm::is_any_of("T"));
  this->m_date.Reset();
  this->m_time.Reset();
  if (strDate_Time.size() < 2) {
    return;
  }
  std::string dateStr = strDate_Time.at(0);
  std::vector<std::string> year_month_day;
  boost::algorithm::split(year_month_day, dateStr, boost::algorithm::is_any_of("-"));
  this->m_date.m_strYear = year_month_day.at(0);
  this->m_date.m_strMonth = year_month_day.at(1);
  this->m_date.m_strDay = year_month_day.at(2);

  std::string timeStr = strDate_Time.at(1);
  std::vector<std::string> hour_minute_seccond;
  boost::algorithm::split(hour_minute_seccond, timeStr, boost::algorithm::is_any_of(":"));
  this->m_time.m_strHour = hour_minute_seccond.at(0);
  this->m_time.m_strMinute = hour_minute_seccond.at(1);
  this->m_time.m_strSecond = hour_minute_seccond.at(2);
}

void CWeatherFrame::setWeather(CWeatherV2 weather) { this->m_weather = weather; }
