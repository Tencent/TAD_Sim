/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/environments/time.h"

CWeatherTime::CWeatherTime() { Reset(); }

void CWeatherTime::Reset() {
  m_lTimeStamp = -1;
  m_strHour = "8";
  m_strMinute = "0";
  m_strSecond = "0";
  m_strMillisecond = "0";
}
