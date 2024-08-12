/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/obstacle.h"
#include <cassert>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include "catalog.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"

CObstacle::CObstacle() {}

void CObstacle::Reset() {
  m_strName = "";

  m_strID = "";
  m_strRouteID = "";
  m_strLaneID = "";
  m_strStartShift = "";
  m_strOffset = "";
  m_strType = "";
  m_strDirection = "";
  m_strStartAngle = "";
  m_ID = 0;
  m_routeID = 0;
  m_laneID = 0;
  m_fStartShift = 0;
  m_fStartOffset = 0;
  m_fDirection = 0;
}

void CObstacle::ConvertToValue() {
  try {
    m_ID = std::stoi(m_strID);
    m_routeID = std::stoi(m_strRouteID);
  } catch (const std::exception& ex) {
    char err[512] = {0};
    snprintf(err, sizeof(err), "O:%s, ID:%s or RID:%s must be non-empty", m_strName.c_str(), m_strID.c_str(),
             m_strRouteID.c_str());
    SYSTEM_LOGGER_ERROR("%s", err);
    throw std::invalid_argument(err);
  }
  if (m_strLaneID.empty()) {
    m_laneID = 0;
  } else {
    m_laneID = std::atoi(m_strLaneID.c_str());
  }
  if (m_strType.empty()) {
    m_strType = "Stone_001";
  }
  m_fStartShift = atof(m_strStartShift.c_str());
  m_fStartOffset = atof(m_strOffset.c_str());
  m_fDirection = atof(m_strDirection.c_str());
}

bool CObstacle::GetDefaultObstacle(std::string name, std::string xoscpath) {
  if (!xoscpath.empty()) {
    CCatalogModelPtr tmp;
    int nRet = CCataLog::getCatalogName(xoscpath.c_str(), CATALOG_TYPE_MISCOBJECT, name, tmp);
    if (nRet == 0) {
      m_strType = tmp->getName();
      return true;
    }
  }
  return false;
}

void CObstacle::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  m_strRouteID = std::to_string(m_routeID);
  m_strLaneID = std::to_string(m_laneID);
  m_strStartShift = CUtils::doubleToStringDot3(m_fStartShift);
  m_strOffset = CUtils::doubleToStringDot3(m_fStartOffset);
  m_strDirection = CUtils::doubleToStringDot3(m_fDirection);
}
