/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/pedestrian.h"

tagDirection::tagDirection() { Reset(); }

void tagDirection::Reset() {
  m_strDir = "";
  m_strDirDuration = "";
  m_strDirVelocity = "";

  m_fDir = 0.0f;
  m_fDirDuration = 0.0f;
  m_fDirVelocity = 0.0f;
}

void tagDirection::ConvertToValue() {
  m_fDir = atof(m_strDir.c_str());
  m_fDirDuration = atof(m_strDirDuration.c_str());
  m_fDirVelocity = atof(m_strDirVelocity.c_str());
}

void tagDirection::ConvertToStr() {
  m_strDir = std::to_string(m_fDir);
  m_strDirDuration = std::to_string(m_fDirDuration);
  m_strDirVelocity = std::to_string(m_fDirVelocity);
}

CPedestrian::CPedestrian() {}

void CPedestrian::Reset() {
  m_strName = "";

  m_strID = "";
  m_strRouteID = "";
  m_strLaneID = "";
  m_strStartShift = "";
  m_strStartTime = "";
  m_strEndTime = "";
  m_strOffset = "";
  m_strEventId = "";
  m_ID = 0;
  m_routeID = 0;
  m_laneID = 0;
  m_fStartShift = 0;
  m_fStartTime = 0;
  m_fEndTime = -1;
  m_fStartOffset = 0;

  m_directions.clear();
  m_events.Reset();
}

void CPedestrian::ConvertToValue() {
  m_ID = std::atoi(m_strID.c_str());
  m_routeID = std::atoi(m_strRouteID.c_str());
  if (m_strLaneID.empty()) {
    m_laneID = 0;
  } else {
    m_laneID = std::atoi(m_strLaneID.c_str());
  }
  m_fStartShift = atof(m_strStartShift.c_str());
  m_fStartTime = atof(m_strStartTime.c_str());
  if (m_strEndTime.size() > 0) {
    m_fEndTime = atof(m_strEndTime.c_str());
  } else {
    m_fEndTime = -1;
  }

  m_fStartOffset = atof(m_strOffset.c_str());

  std::vector<tagDirection>::iterator itr = m_directions.begin();
  for (; itr != m_directions.end(); ++itr) {
    itr->ConvertToValue();
  }

  m_events.ConvertToValue();
}

void CPedestrian::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  m_strRouteID = std::to_string(m_routeID);
  m_strLaneID = std::to_string(m_laneID);
  m_strStartShift = std::to_string(m_fStartShift);
  m_strStartTime = std::to_string(m_fStartTime);
  if (m_fEndTime < 0) {
    m_strEndTime = "";
  } else {
    m_strEndTime = std::to_string(m_fEndTime);
  }
  m_strOffset = std::to_string(m_fStartOffset);

  std::vector<tagDirection>::iterator itr = m_directions.begin();
  for (; itr != m_directions.end(); ++itr) {
    itr->ConvertToStr();
  }

  m_events.ConvertToStr();
}
