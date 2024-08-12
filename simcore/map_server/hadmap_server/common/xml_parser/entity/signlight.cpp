/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "signlight.h"
#include "common/utils/stringhelper.h"
CSignLight::CSignLight() { Reset(); }

void CSignLight::Reset() {
  m_strID = "";
  m_strRouteID = "";
  m_strStartShift = "0";
  m_strStartTime = "1";
  m_strOffset = "";
  m_strTimeGreen = "";
  m_strTimeYellow = "";
  m_strTimeRed = "";
  m_strDirection = "180";
  m_strLane = "ALL";
  m_strPhase = "A";
  m_strStatus = "Activated";
  m_strCompliance = "1.0";

  m_ID = 0;
  m_routeID = 0;
  m_fStartShift = 0.0f;
  m_fStartTime = 0.0f;
  m_fStartOffset = 0.0f;
  m_fTimeGreen = 0.0f;
  m_fTimeYellow = 0.0f;
  m_fTimeRed = 0.0f;
  m_fDirection = 0.0f;
  m_fCompliance = 1.0;
}

void CSignLight::ConvertToValue() {
  m_ID = atoi(m_strID.c_str());
  m_routeID = atoi(m_strRouteID.c_str());
  m_fStartShift = atof(m_strStartShift.c_str());
  m_fStartTime = atof(m_strStartTime.c_str());
  m_fStartOffset = atof(m_strOffset.c_str());
  m_fTimeGreen = atof(m_strTimeGreen.c_str());
  m_fTimeYellow = atof(m_strTimeYellow.c_str());
  m_fTimeRed = atof(m_strTimeRed.c_str());
  m_fDirection = atof(m_strDirection.c_str());
  m_fCompliance = atof(m_strCompliance.c_str());
}

void CSignLight::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  m_strRouteID = std::to_string(m_routeID);
  m_strStartShift = std::to_string(m_fStartShift);
  m_strStartTime = std::to_string(m_fStartTime);
  m_strOffset = std::to_string(m_fStartOffset);
  m_strTimeGreen = std::to_string(m_fTimeGreen);
  m_strTimeYellow = std::to_string(m_fTimeYellow);
  m_strTimeRed = std::to_string(m_fTimeRed);
  m_strDirection = std::to_string(m_fDirection);
  m_strCompliance = std::to_string(m_fCompliance);
}
void CSignLight::ConvertLaneId() {
  std::vector<std::string> strList = utils::Split(m_strLane, ';');
  for (auto& it : strList) {
    it = std::to_string(std::atoi(it.c_str()) * -1);
  }
  if (strList.size() > 0) {
    m_strLane = utils::MergeStringAddLink(strList, ";");
  }
}
