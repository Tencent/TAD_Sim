/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/planner.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/utils/stringhelper.h"
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/entity/parser_tools.h"
sControlPathPoint::sControlPathPoint() { Reset(); }

void sControlPathPoint::Reset() {
  m_dLon = 0.0;
  m_dLat = 0.0;
  m_dAlt = 0.0;
  m_dVelocity = 0.0;
  m_nGear = GEAR_D;

  m_strLon = "";
  m_strLat = "";
  m_strAlt = "";
  m_strVelocity = "";
  m_strGear = "";
  m_accs = "null";
  m_heading = "null";
  m_frontwheel = "null";
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
  m_dLon = std::atof(m_strLon.c_str());
  m_dLat = std::atof(m_strLat.c_str());
  m_dAlt = std::atof(m_strAlt.c_str());
  m_dVelocity = std::atof(m_strVelocity.c_str());
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
    std::vector<std::string> slices{itr->m_strLon,  itr->m_strLat, itr->m_strAlt,  itr->m_strVelocity,
                                    itr->m_strGear, itr->m_accs,   itr->m_heading, itr->m_frontwheel};
    std::string strOnePoint = utils::Join(slices, ",");
    if (i + 1 != nSize) {
      strOnePoint.append(";");
    }

    m_strPoints.append(strOnePoint);
    i++;
  }
}

void sControlPath::ConvertToValue() {
  m_dSampleInterval = std::atof(m_strSampleInterval.c_str());
  m_vPoints.clear();
  std::vector<std::string> strs;
  CParseTools::SplitString(m_strPoints, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strOnePoint = strs[i];
    std::vector<std::string> strOnePointInfo;
    CParseTools::SplitString(strOnePoint, strOnePointInfo, ",");
    sControlPathPoint pp;
    if (strOnePointInfo.size() == 5) {
      pp.m_strLon = strOnePointInfo[0];
      pp.m_strLat = strOnePointInfo[1];
      pp.m_strAlt = strOnePointInfo[2];
      pp.m_strVelocity = strOnePointInfo[3];
      pp.m_strGear = strOnePointInfo[4];
      pp.m_accs = "null";
      pp.m_heading = "null";
      pp.m_frontwheel = "null";
      pp.ConvertToValue();
    } else if (strOnePointInfo.size() == 8) {
      pp.m_strLon = strOnePointInfo[0];
      pp.m_strLat = strOnePointInfo[1];
      pp.m_strAlt = strOnePointInfo[2];
      pp.m_strVelocity = strOnePointInfo[3];
      pp.m_strGear = strOnePointInfo[4];
      pp.m_accs = strOnePointInfo[5];
      pp.m_heading = strOnePointInfo[6];
      pp.m_frontwheel = strOnePointInfo[7];
      pp.ConvertToValue();
    } else {
      SYSTEM_LOGGER_ERROR("sim file control path point format error!");
      continue;
    }
    m_vPoints.push_back(pp);
  }
  // setControlPathPoint(m_strPoints);
}

void sControlPath::setControlPathPoint(std::string strPoints) {
  std::vector<std::string> strs;
  m_strPoints = strPoints;
  CParseTools::SplitString2(strPoints, strs, ";");
  for (int i = 0; i < strs.size(); i++) {
    std::vector<std::string> strKVvec;
    CParseTools::SplitString2(strs[i], strKVvec, ",");
    std::map<std::string, std::string> map_keyvalue;
    for (int j = 0; j < strKVvec.size(); j++) {
      std::vector<std::string> strK_V;
      CParseTools::SplitString2(strKVvec[j], strK_V, (":"));
      map_keyvalue.insert(std::make_pair(strK_V[0], strK_V[1]));
    }
    sControlPathPoint temp;
    auto itr = map_keyvalue.find("lon");
    if (itr != map_keyvalue.end()) temp.m_strLon = itr->second;

    itr = map_keyvalue.find("lat");
    if (itr != map_keyvalue.end()) temp.m_strLat = itr->second;

    itr = map_keyvalue.find("alt");
    if (itr != map_keyvalue.end()) temp.m_strAlt = itr->second;

    itr = map_keyvalue.find("speed_m_s");
    if (itr != map_keyvalue.end()) temp.m_strVelocity = itr->second;

    itr = map_keyvalue.find("gear");
    if (itr != map_keyvalue.end()) temp.m_strGear = itr->second;

    itr = map_keyvalue.find("accs");
    if (itr != map_keyvalue.end()) temp.m_accs = itr->second;

    itr = map_keyvalue.find("heading");
    if (itr != map_keyvalue.end()) temp.m_heading = itr->second;

    itr = map_keyvalue.find("frontwheel");
    if (itr != map_keyvalue.end()) temp.m_frontwheel = itr->second;

    m_vPoints.push_back(temp);
  }
  return;
}

void sControlPath::setControlPointLonLat(const sPath& inputpath) {
  int i = 0;
  for (auto it : inputpath.m_vPoints) {
    this->m_vPoints.at(i).m_strLon = it.m_strLon;
    this->m_vPoints.at(i).m_strLat = it.m_strLat;
    this->m_vPoints.at(i).m_strAlt = it.m_strAlt;
    i++;
  }
}

std::string sControlPath::getControlPath() {
  std::string controlPath = "";
  for (int i = 0; i < m_vPoints.size(); i++) {
    controlPath.append(m_vPoints[i].m_strLon);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_strLat);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_strAlt);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_strVelocity);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_strGear);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_accs);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_heading);
    controlPath.append(",");
    controlPath.append(m_vPoints[i].m_frontwheel);
    if (i < m_vPoints.size() - 1) {
      controlPath.append(";");
    }
  }
  return controlPath;
}
std::string sControlPath::getTrajectoryTracking() {
  std::string strTrajectoryTracking = "";
  for (int i = 0; i < m_vPoints.size(); i++) {
    strTrajectoryTracking.append("speed_m_s:");
    strTrajectoryTracking.append(m_vPoints[i].m_strVelocity);
    strTrajectoryTracking.append(",gear:");
    strTrajectoryTracking.append(m_vPoints[i].m_strGear);
    strTrajectoryTracking.append(",accs:");
    strTrajectoryTracking.append(m_vPoints[i].m_accs);
    strTrajectoryTracking.append(",heading:");
    strTrajectoryTracking.append(m_vPoints[i].m_heading);
    strTrajectoryTracking.append(",frontwheel:");
    strTrajectoryTracking.append(m_vPoints[i].m_frontwheel);
    if (i < m_vPoints.size() - 1) {
      strTrajectoryTracking.append(";");
    }
  }
  return strTrajectoryTracking;
}

CEgoInfo::CEgoInfo() { Reset(); }

bool CEgoInfo::IsEmpty() {
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

void CEgoInfo::Reset() {
  m_ID = 0;
  m_fStartVelocity = 0;
  m_fStartAngle = 0;
  m_fMaxVelocity = 10.0;
  m_dStartAlt = 0;
  m_dEndAlt = 0;
  m_strStartVelocity = "";
  m_strTheta = "0.0";
  m_strVelocityMax = "10.0";
  m_strStartAlt = "";
  m_strEndAlt = "";
  m_controlType = "none";
  m_controlLongitudinal = "true";
  m_controlLateral = "true";
  m_trajectoryEnabled = "false";
  m_strGroupName = "Ego_001";
  m_route.Reset();
  m_nUnrealLevel = 0;
  m_strUnrealLevel = "";
  m_nType = ET_SUV;
  m_accelerationMax = "12.0";
  m_decelerationMax = "10.0";
  m_strName = EgoTypeValue2Str(m_nType);
}

void CEgoInfo::ConvertToValue() {
  m_fStartVelocity = std::atof(m_strStartVelocity.c_str());
  m_fStartAngle = std::atof(m_strTheta.c_str());
  m_fMaxVelocity = std::atof(m_strVelocityMax.c_str());
  m_route.ConvertToValue();
  m_nUnrealLevel = atoi(m_strUnrealLevel.c_str());
  m_dStartAlt = std::atof(m_strStartAlt.c_str());
  m_dEndAlt = std::atof(m_strEndAlt.c_str());
  m_nType = EgoTypeStr2Value(m_strName);
  m_inputPath.ConvertToValue();
  // m_controlPath.ConvertToValue();
}

void CEgoInfo::ConvertToStr() {
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

void CEgoInfo::AdjustTypeAndName(const std::string& catalog_file, const char* raw_type, const char* raw_name) {
  m_strType.clear();
  m_strName.clear();
  thread_local std::map<std::string, std::string> raw2new_type_map = {
      {"suv", "car"},
      {"truck", "combination"},
      {"AIV_FullLoad_001", "car"},
      {"AIV_Empty_001", "car"},
      {"MPV_001", "car"},
      {"MiningDumpTruck_001", "car"},
      {"AIV_Empty_002", "car"},
      {"MIFA_01", "car"},
      {"AIV_V5_FullLoad_001", "car"},
      {"AIV_V5_Empty_001", "car"},
      {"AIV_FullLoad_002", "car"},
  };
  auto iter = raw2new_type_map.find(raw_type ? raw_type : "");
  m_strType = iter == raw2new_type_map.end() ? "car" : iter->second;
  m_strName = iter == raw2new_type_map.end() ? "suv" : iter->first;

  bool need_adjust_name = m_strName == "suv" || m_strName == "truck";
  if (!need_adjust_name || catalog_file.empty() || !raw_name || strlen(raw_name) == 0) {
    return;
  }
  if (CCataLog{}.findEgoModelFromName(catalog_file.c_str(), raw_name,
                                      m_strType == "car" ? VEHICLE_CAR : VEHICLE_TRUCK)) {
    m_strName = raw_name;
  }
}

CPlanner::CPlanner() { m_egos.clear(); }

void CPlanner::Reset() {}

void CPlanner::ConvertFirstEgoParam() {
  m_ID = m_egos.begin()->second.m_ID;
  m_fStartVelocity = m_egos.begin()->second.m_fStartVelocity;
  m_fStartAngle = m_egos.begin()->second.m_fStartAngle;
  m_fMaxVelocity = m_egos.begin()->second.m_fMaxVelocity;
  m_dStartAlt = m_egos.begin()->second.m_dStartAlt;
  m_dEndAlt = m_egos.begin()->second.m_dEndAlt;
  m_nType = m_egos.begin()->second.m_nType;
  m_strStartVelocity = m_egos.begin()->second.m_strStartVelocity;
  m_strTheta = m_egos.begin()->second.m_strTheta;
  m_strVelocityMax = m_egos.begin()->second.m_strVelocityMax;
  m_strStartAlt = m_egos.begin()->second.m_strStartAlt;
  m_strEndAlt = m_egos.begin()->second.m_strEndAlt;
  m_route = m_egos.begin()->second.m_route;
  m_strType = m_egos.begin()->second.m_strType;
  // m_ScenarioName = m_egos.begin()->second.m_ScenarioName;
  m_strName = m_egos.begin()->second.m_strName;
  m_sensorId = m_egos.begin()->second.m_sensorId;
  m_inputPath = m_egos.begin()->second.m_inputPath;
  m_controlPath = m_egos.begin()->second.m_controlPath;
  m_nUnrealLevel = m_egos.begin()->second.m_nUnrealLevel;
  m_strUnrealLevel = m_egos.begin()->second.m_strUnrealLevel;
  m_controlType = m_egos.begin()->second.m_controlType;
  m_controlLongitudinal = m_egos.begin()->second.m_controlLongitudinal;
  m_controlLateral = m_egos.begin()->second.m_controlLateral;
  m_accelerationMax = m_egos.begin()->second.m_accelerationMax;
  m_decelerationMax = m_egos.begin()->second.m_decelerationMax;
  m_trajectoryEnabled = m_egos.begin()->second.m_trajectoryEnabled;
  m_scenceEvents = m_egos.begin()->second.m_scenceEvents;
}

bool CPlanner::isContainsTrajectoryEnable() {
  for (auto it : m_egos) {
  }
  return true;
}

CPlannerParam& CPlannerParam::operator=(const CPlanner& other) {
  CPlanner::operator=(other);
  return (*this);
}

int CPlannerParam::GenerateSingleEgoParam() {
  int index = 0;
  std::map<std::string, CEgoInfo> tmp;
  for (auto& it : m_egos) {
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << (++index);
    tmp["Ego_" + oss.str()] = it.second;
  }
  m_egos = tmp;
  for (auto it : m_mapEgoGroup2Velocity) {
    std::string str_key = it.first;
    if (m_egos.find(str_key) != m_egos.end()) {
      auto& curEgo = m_egos.find(str_key)->second;
      int nVelocity = ComputeSingleNumber(str_key);
      curEgo.m_paramSceneStartV.clear();
      for (size_t i = 0; i <= nVelocity; i++) {
        float fVelocity = it.second.m_fStartVelocityStart + i * it.second.m_fStartVelocitySep;
        std::string veloc = utils::FloatPrecisionToString(fVelocity, 4);
        curEgo.m_paramSceneStartV.push_back(veloc);
      }
    }
  }
  std::vector<std::vector<std::string>> input;
  // 将所有主车值组合起来
  for (auto it : m_egos) {
    input.push_back(it.second.m_paramSceneStartV);
  }
  std::vector<std::string> currentSet;
  GenerateSets(input, currentSet, 0, m_paramsVec);
  return m_paramsVec.size();
}

int CPlannerParam::GenerateCount() {
  int nCount = 1;
  for (auto it : m_mapEgoGroup2Velocity) {
    std::string str_key = it.first;
    int nVelocity = ComputeSingleNumber(str_key);
    std::vector<std::string> curEgo;
    for (size_t i = 0; i <= nVelocity; i++) {
      float fVelocity = it.second.m_fStartVelocityStart + i * it.second.m_fStartVelocitySep;
      std::string veloc = utils::FloatPrecisionToString(fVelocity, 4);
      curEgo.push_back(veloc);
    }
    nCount *= curEgo.size();
  }
  return nCount;
}
