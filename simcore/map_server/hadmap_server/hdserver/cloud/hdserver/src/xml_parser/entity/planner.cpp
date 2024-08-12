/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "planner.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

sControlPathPoint::sControlPathPoint() { Reset(); }

void sControlPathPoint::Reset() {
  m_dLon = 0.0;
  m_dLat = 0.0;
  m_dAlt = 0.0;
  m_dVelocity = 0.0;
  m_nGear = GEAR_INVALID;

  m_strLon = "";
  m_strLat = "";
  m_strAlt = "";
  m_strVelocity = "";
  m_strGear = "";
}

void sControlPathPoint::ConvertToStr() {
  boost::format precision8Num("%.8f");
  boost::format precision3Num("%.3f");
  boost::format precision2Num("%.2f");

  precision8Num % m_dLon;
  m_strLon = precision8Num.str();

  precision8Num.clear();
  precision8Num % m_dLat;
  m_strLat = precision8Num.str();

  precision3Num % m_dAlt;
  m_strAlt = precision3Num.str();

  precision2Num % m_dVelocity;
  m_strVelocity = precision2Num.str();

  m_strGear = GearTypeValue2Str(m_nGear);
}

void sControlPathPoint::ConvertToValue() {
  m_dLon = atof(m_strLon.c_str());
  m_dLat = atof(m_strLat.c_str());
  m_dAlt = atof(m_strAlt.c_str());
  m_dVelocity = atof(m_strVelocity.c_str());
  m_nGear = GearTypeStr2Value(m_strGear);
}

sControlPath::sControlPath() { Reset(); }

void sControlPath::Reset() {
  m_dSampleInterval = 10;
  m_strSampleInterval = "10";
  m_strPoints = "";
  m_vPoints.clear();
}

void sControlPath::pushPoint(sControlPathPoint& pp) { m_vPoints.push_back(pp); }

void sControlPath::ConvertToStr() {
  boost::format precision2Num("%.2f");
  precision2Num % m_dSampleInterval;
  m_strSampleInterval = precision2Num.str();
  m_strPoints = "";

  std::vector<sControlPathPoint>::iterator itr = m_vPoints.begin();
  int nSize = m_vPoints.size();
  int i = 0;
  for (; itr != m_vPoints.end(); ++itr) {
    itr->ConvertToStr();
    std::string strOnePoint =
        itr->m_strLon + "," + itr->m_strLat + "," + itr->m_strAlt + "," + itr->m_strVelocity + "," + itr->m_strGear;
    if (i + 1 != nSize) {
      strOnePoint.append(";");
    }

    m_strPoints.append(strOnePoint);
    i++;
  }
}

void sControlPath::ConvertToValue() {
  m_dSampleInterval = atof(m_strSampleInterval.c_str());
  m_vPoints.clear();
  std::vector<std::string> strs;
  CParseTools::SplitString(m_strPoints, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strOnePoint = strs[i];
    std::vector<std::string> strOnePointInfo;
    CParseTools::SplitString(strOnePoint, strOnePointInfo, ",");
    assert(strOnePointInfo.size() == 5);
    if (strOnePointInfo.size() != 5) {
      SYSTEM_LOGGER_ERROR("sim file control path point format error!");
      continue;
    }

    sControlPathPoint pp;
    pp.m_strLon = strOnePointInfo[0];
    pp.m_strLat = strOnePointInfo[1];
    pp.m_strAlt = strOnePointInfo[2];
    pp.m_strVelocity = strOnePointInfo[3];
    pp.m_strGear = strOnePointInfo[4];
    pp.ConvertToValue();

    m_vPoints.push_back(pp);
  }
}

CPlanner::CPlanner() { Reset(); }

CPlanner::CPlanner(CPlannerParam& pp) { *this = pp; }

CPlanner& CPlanner::operator=(CPlannerParam& pp) {
  m_ID = pp.m_ID;
  m_fStartVelocity = pp.m_fStartVelocity;
  m_fStartAngle = pp.m_fStartAngle;
  m_fMaxVelocity = pp.m_fMaxVelocity;
  m_dStartAlt = pp.m_dStartAlt;
  m_dEndAlt = pp.m_dEndAlt;
  m_nUnrealLevel = pp.m_nUnrealLevel;
  m_nType = pp.m_nType;

  m_strStartVelocity = pp.m_strStartVelocity;
  m_strTheta = pp.m_strTheta;
  m_strVelocityMax = pp.m_strVelocityMax;
  m_strStartAlt = pp.m_strStartAlt;
  m_strEndAlt = pp.m_strEndAlt;
  m_route = pp.m_route;
  m_strUnrealLevel = pp.m_strUnrealLevel;
  m_inputPath = pp.m_inputPath;
  m_strType = pp.m_strType;

  return (*this);
}

CPlanner& CPlanner::operator=(const CPlanner& other) {
  m_strStartVelocity = other.m_strStartVelocity;
  m_strTheta = other.m_strTheta;
  m_strVelocityMax = other.m_strVelocityMax;
  m_strUnrealLevel = other.m_strUnrealLevel;
  m_strStartAlt = other.m_strStartAlt;
  m_strEndAlt = other.m_strEndAlt;
  m_route = other.m_route;
  m_strType = other.m_strType;
  m_ID = other.m_ID;
  m_fStartVelocity = other.m_fStartVelocity;
  m_fStartAngle = other.m_fStartAngle;
  m_fMaxVelocity = other.m_fMaxVelocity;
  m_dStartAlt = other.m_dStartAlt;
  m_dEndAlt = other.m_dEndAlt;
  m_nUnrealLevel = other.m_nUnrealLevel;
  m_inputPath = other.m_inputPath;
  m_nType = other.m_nType;
  return (*this);
}

bool CPlanner::IsEmpty() {
  if (m_strStartVelocity.size() > 0) {
    return false;
  }
  if (m_strTheta.size() > 0) {
    return false;
  }
  if (m_strVelocityMax.size() > 0) {
    return false;
  }
  if (m_route.m_strID.size() > 0) {
    return false;
  }

  return true;
}

void CPlanner::Reset() {
  m_ID = 0;
  m_fStartVelocity = 0;
  m_fStartAngle = 0;
  m_fMaxVelocity = 0;
  m_dStartAlt = 0;
  m_dEndAlt = 0;
  m_strStartVelocity = "";
  m_strTheta = "";
  m_strVelocityMax = "";
  m_strStartAlt = "";
  m_strEndAlt = "";
  m_route.Reset();
  m_nUnrealLevel = 0;
  m_strUnrealLevel = "";
  m_nType = ET_SUV;
  m_strType = EgoTypeValue2Str(m_nType);
}

void CPlanner::ConvertToValue() {
  m_fStartVelocity = atof(m_strStartVelocity.c_str());
  m_fStartAngle = atof(m_strTheta.c_str());
  m_fMaxVelocity = atof(m_strVelocityMax.c_str());
  m_route.ConvertToValue();
  m_nUnrealLevel = atoi(m_strUnrealLevel.c_str());
  m_dStartAlt = atof(m_strStartAlt.c_str());
  m_dEndAlt = atof(m_strEndAlt.c_str());
  m_nType = EgoTypeStr2Value(m_strType);

  m_inputPath.ConvertToValue();
  m_controlPath.ConvertToValue();
}

void CPlanner::ConvertToStr() {
  m_strStartVelocity = CUtils::doubleToStringDot3(m_fStartVelocity);
  m_strTheta = CUtils::doubleToStringDot3(m_fStartAngle);
  m_strVelocityMax = CUtils::doubleToStringDot3(m_fMaxVelocity);
  // m_route
  m_strUnrealLevel = boost::lexical_cast<std::string>(m_nUnrealLevel);
  m_strStartAlt = std::to_string(m_dStartAlt);
  m_strEndAlt = std::to_string(m_dEndAlt);

  m_inputPath.ConvertToStr();
  m_controlPath.ConvertToStr();
  m_strType = EgoTypeValue2Str(m_nType);
}

CPlannerParam& CPlannerParam::operator=(const CPlanner& other) {
  /*
  m_ID = other.m_ID;
  m_strStartVelocity = other.m_strStartVelocity;
  m_fStartAngle = other.m_fStartAngle;
  m_fMaxVelocity = other.m_fMaxVelocity;
  m_dStartAlt = other.m_dStartAlt;
  m_dEndAlt = other.m_dEndAlt;
  m_nUnrealLevel = other.m_nUnrealLevel;

  m_fStartVelocity = other.m_fStartVelocity;
  m_strTheta = other.m_strTheta;
  m_strVelocityMax = other.m_strVelocityMax;
  m_strStartAlt = other.m_strStartAlt;
  m_strEndAlt = other.m_strEndAlt;
  m_route = other.m_route;
  m_strUnrealLevel = other.m_strUnrealLevel;
  m_inputPath = other.m_inputPath;
  */

  CPlanner::operator=(other);

  return (*this);
}
