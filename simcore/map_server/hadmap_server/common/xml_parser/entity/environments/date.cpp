/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/environments/date.h"

CWeatherDate::CWeatherDate() { Reset(); }

void CWeatherDate::Reset() {
  m_lTimeStamp = -1;
  m_strYear = "1970";
  m_strMonth = "1";
  m_strDay = "1";
}
