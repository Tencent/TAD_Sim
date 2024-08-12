/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/environments/weather.h"

CWeather::CWeather() { Reset(); }

void CWeather::Reset() { m_lTimeStamp = -1; }

///////////////////////////////////////////////////
// wheather v2.0 (osc1.0 catalog)
///////////////////////////////////////////////////
void CWeatherV2::Reset() {
  m_strCloudState = "cloudy";
  m_strPrecipitationType = "dry";
  m_strPrecipitationIntensity = "0.0";
  m_strVisualRange = "30";  // km
  m_strWindSpeed = "2";     // m/s
  m_strTemperature = "25";  // C
}
