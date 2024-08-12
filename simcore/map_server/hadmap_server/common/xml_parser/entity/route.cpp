/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/route.h"
#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "common/log/system_logger.h"
#include "common/utils/stringhelper.h"
#include "common/xml_parser/entity/parser_tools.h"
sPathPoint::sPathPoint() { Reset(); }
sPathPoint::sPathPoint(std::string slon, std::string slat, std::string salt) {
  Reset();
  m_strLon = slon;
  m_strLat = slat;
  m_strAlt = salt;
  ConvertToValue();
}
void sPathPoint::Reset() {
  m_dLon = 0.0;
  m_dLat = 0.0;
  m_dAlt = 0.0;

  m_strLon = "";
  m_strLat = "";
  m_strAlt = "";
}

void sPathPoint::ConvertToStr() {
  boost::format precision12Num("%.12f");
  boost::format precision3Num("%.3f");

  precision12Num % m_dLon;
  m_strLon = precision12Num.str();

  precision12Num.clear();
  precision12Num % m_dLat;
  m_strLat = precision12Num.str();

  precision3Num % m_dAlt;  // 高度信息保留小数点后三位
  m_strAlt = precision3Num.str();
}

void sPathPoint::ConvertToValue() {
  m_dLon = atof(m_strLon.c_str());
  m_dLat = atof(m_strLat.c_str());
  m_dAlt = atof(m_strAlt.c_str());
}

sPath::sPath() { Reset(); }

void sPath::Reset() {
  m_strPoints = "";
  m_vPoints.clear();
}

void sPath::pushPoint(double dLon, double dLat, double dAlt) {
  sPathPoint pp;
  pp.m_dLon = dLon;
  pp.m_dLat = dLat;
  pp.m_dAlt = dAlt;

  m_vPoints.push_back(pp);
  ConvertToStr();
}

void sPath::pushPoint(sPathPoint& pp) { m_vPoints.push_back(pp); }

void sPath::ConvertToStr() {
  m_strPoints = "";

  std::vector<sPathPoint>::iterator itr = m_vPoints.begin();
  int nSize = m_vPoints.size();
  int i = 0;
  for (; itr != m_vPoints.end(); ++itr) {
    itr->ConvertToStr();
    std::string strOnePoint = itr->m_strLon + "," + itr->m_strLat + "," + itr->m_strAlt;
    if (i + 1 != nSize) {
      strOnePoint.append(";");
    }
    m_strPoints.append(strOnePoint);
    i++;
  }
}

void sPath::ConvertToValue() {
  m_vPoints.clear();
  std::vector<std::string> strs;
  CParseTools::SplitString(m_strPoints, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strOnePoint = strs[i];
    std::vector<std::string> strOnePointInfo;
    CParseTools::SplitString(strOnePoint, strOnePointInfo, ",");
    assert(strOnePointInfo.size() == 3);
    if (strOnePointInfo.size() != 3) {
      SYSTEM_LOGGER_ERROR("sim file path point format error!");
      continue;
    }

    sPathPoint pp;
    pp.m_strLon = strOnePointInfo[0];
    pp.m_strLat = strOnePointInfo[1];
    pp.m_strAlt = strOnePointInfo[2];
    pp.ConvertToValue();

    m_vPoints.push_back(pp);
  }
}

CRoute::CRoute() { Reset(); }

CRoute& CRoute::operator=(const CRoute& other) {
  m_strID = other.m_strID;
  m_strType = other.m_strType;
  m_strStart = other.m_strStart;
  m_strStartLon = other.m_strStartLon;
  m_strStartLat = other.m_strStartLat;
  m_strStartAlt = other.m_strStartAlt;
  m_strMids = other.m_strMids;
  m_strMid = other.m_strMid;
  m_strMidLon = other.m_strMidLon;
  m_strMidLat = other.m_strMidLat;
  m_strMidAlt = other.m_strMidAlt;
  m_strEnd = other.m_strEnd;
  m_strEndLon = other.m_strEndLon;
  m_strEndLat = other.m_strEndLat;
  m_strEndAlt = other.m_strEndAlt;
  m_strRoadID = other.m_strRoadID;
  m_strSectionID = other.m_strSectionID;
  m_strInfo = other.m_strInfo;

  m_ID = other.m_ID;
  m_nType = other.m_nType;
  m_dStartLon = other.m_dStartLon;
  m_dStartLat = other.m_dStartLat;
  m_dMidLon = other.m_dMidLon;
  m_dMidLat = other.m_dMidLat;
  m_dEndLon = other.m_dEndLon;
  m_dEndLat = other.m_dEndLat;
  m_roadID = other.m_roadID;
  m_sectionID = other.m_sectionID;

  m_path = other.m_path;

  m_strControlPath = other.m_strControlPath;
  m_strTrajectoryTracking_XOSC = other.m_strTrajectoryTracking_XOSC;

  return (*this);
}

bool CRoute::operator<(const CRoute& other) {
  if (m_ID < other.m_ID) {
    return true;
  }

  return false;
}

void CRoute::Reset() {
  m_strID = "";
  m_strType = "";
  m_strStart = "";
  m_strStartLon = "";
  m_strStartLat = "";
  m_strStartAlt = "";
  m_strMids = "";
  m_strMid = "";
  m_strMidLon = "";
  m_strMidLat = "";
  m_strMidAlt = "";
  m_strEnd = "";
  m_strEndLon = "";
  m_strEndLat = "";
  m_strEndAlt = "";
  m_strRoadID = "";
  m_strSectionID = "";
  m_strInfo = "";

  m_ID = 0;
  m_nType = 0;
  m_dStartLon = 0;
  m_dStartLat = 0;
  m_dStartAlt = 0;
  m_dMidLon = -1;
  m_dMidLat = -1;
  m_dMidAlt = -1;
  m_dEndLon = -1;
  m_dEndLat = -1;
  m_dEndAlt = -1;
  m_roadID = 0;
  m_sectionID = 0;

  m_path.Reset();
}

void CRoute::ConvertToValue() {
  m_ID = std::stoi(m_strID.c_str());
  if (boost::algorithm::iequals(m_strType, "start_end")) {
    // route type start end
    m_nType = ROUTE_TYPE_START_END;
    if (!(m_strStartLon.size() > 0 && m_strStartLat.size() > 0) && m_strStart.size() > 0) {
      std::vector<std::string> _tmp = utils::Split(m_strStart, ',');
      if (_tmp.size() > 1) {
        m_strStartLon = _tmp.at(0);
        m_strStartLat = _tmp.at(1);
      }
      if (_tmp.size() > 2) {
        m_strStartAlt = _tmp.at(2);
      }
    }
    m_dStartLon = atof(m_strStartLon.c_str());
    m_dStartLat = atof(m_strStartLat.c_str());
    m_dStartAlt = atof(m_strStartAlt.c_str());
    if (m_strEndLon.size() > 0 && m_strEndLat.size() > 0) {
      m_dEndLon = atof(m_strEndLon.c_str());
      m_dEndLat = atof(m_strEndLat.c_str());
      m_dEndAlt = atof(m_strEndAlt.c_str());
    }
    if (m_strMidLon.size() > 0 && m_strMidLat.size() > 0) {
      m_dMidLon = atof(m_strMidLon.c_str());
      m_dMidLat = atof(m_strMidLat.c_str());
      m_dMidAlt = atof(m_strMidAlt.c_str());
    }
  } else if (boost::algorithm::iequals(m_strType, "roadID")) {
    m_nType = ROUTE_TYPE_ROAD_ID;
    m_roadID = std::stoi(m_strRoadID.c_str());
    m_sectionID = std::stoi(m_strSectionID.c_str());
  } else {
    assert(false);
  }
}

void CRoute::RemoveStrAlt() {
  std::vector<std::string> strs;
  if (m_strStart.size() > 0) {
    CParseTools::SplitString(m_strStart, strs, ",");
    assert(strs.size() == 2 || strs.size() == 3);
    m_strStart = strs[0] + "," + strs[1];
  }

  if (m_strEnd.size() > 0) {
    strs.clear();
    CParseTools::SplitString(m_strEnd, strs, ",");
    assert(strs.size() == 2 || strs.size() == 3);
    m_strEnd = strs[0] + "," + strs[1];
  }

  if (m_strMid.size() > 0) {
    strs.clear();
    CParseTools::SplitString(m_strMid, strs, ",");
    assert(strs.size() == 2 || strs.size() == 3);
    m_strMid = strs[0] + "," + strs[1];
  }

  if (m_strMids.size() > 0) {
    strs.clear();
    CParseTools::SplitString(m_strMids, strs, ";");

    std::string strMids = "";
    std::vector<std::string> subStrs;
    for (auto subStr : strs) {
      subStrs.clear();
      CParseTools::SplitString(subStr, subStrs, ",");
      assert(subStrs.size() == 2 || subStrs.size() == 3);
      std::string strPt = subStrs[0] + "," + subStrs[1] + ";";
      strMids += strPt;
    }
    m_strMids = strMids.substr(0, strMids.length() - 1);
  }
}

void CRoute::FromStr() {
  if (m_strStart.length() < 1) {
    return;
  }

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strStart, strs, ",");
  if (strs.size() != 3 && strs.size() != 2) {
    assert(false);
    SYSTEM_LOGGER_ERROR("traffic file start_end route error!");
  }

  m_strStartLon = strs[0];
  m_strStartLat = strs[1];
  if (strs.size() == 3) {
    m_strStartAlt = strs[2];
  }

  if (m_strEnd.length() < 1) {
    return;
  }

  strs.clear();
  CParseTools::SplitString(m_strEnd, strs, ",");
  if (strs.size() != 3 && strs.size() != 2) {
    assert(false);
    SYSTEM_LOGGER_ERROR("traffic file start_end route error!");
  }

  m_strEndLon = strs[0];
  m_strEndLat = strs[1];
  if (strs.size() == 3) {
    m_strEndAlt = strs[2];
  }
}

void CRoute::ExtractPath() {
  m_path.Reset();

  double dLon = atof(m_strStartLon.c_str());
  double dLat = atof(m_strStartLat.c_str());
  double dAlt = atof(m_strStartAlt.c_str());

  m_path.pushPoint(dLon, dLat, dAlt);

  if (m_strMidLon.size() > 0 && m_strMidLat.size() > 0) {
    dLon = atof(m_strMidLon.c_str());
    dLat = atof(m_strMidLat.c_str());
    dAlt = atof(m_strMidAlt.c_str());
    m_path.pushPoint(dLon, dLat, dAlt);

  } else if (m_strMids.size() > 0) {
    std::vector<std::string> strPoses;
    CParseTools::SplitString2(m_strMids, strPoses, ";");

    for (std::string strPos : strPoses) {
      std::vector<std::string> vPos;
      CParseTools::SplitString2(strPos, vPos, ",");
      dLon = atof(vPos[0].c_str());
      dLat = atof(vPos[1].c_str());
      dAlt = 0;

      if (vPos.size() == 3) {
        dAlt = atof(vPos[2].c_str());
      }

      m_path.pushPoint(dLon, dLat, dAlt);
    }
  }

  if (m_strEndLon.size() > 0 && m_strEndLat.size() > 0) {
    dLon = atof(m_strEndLon.c_str());
    dLat = atof(m_strEndLat.c_str());
    dAlt = atof(m_strEndAlt.c_str());
    m_path.pushPoint(dLon, dLat, dAlt);
  }
}

void CRoute::Set(uint64_t id) {
  m_ID = id;
  m_strID = std::to_string(id);
}

void CRoute::Set(double dStartLon, double dStartLat) {
  m_nType = ROUTE_TYPE_START_END;
  m_dStartLon = dStartLon;
  m_dStartLat = dStartLat;
  m_dMidLon = -1;
  m_dMidLat = -1;
  m_dEndLon = -1;
  m_dEndLat = -1;

  m_strType = "start_end";

  std::stringstream ss;
  ss << std::setprecision(20) << m_dStartLon;
  m_strStartLon = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dStartLat;
  m_strStartLat = ss.str();

  m_strMidLon = "";
  m_strMidLat = "";
  m_strEndLon = "";
  m_strEndLat = "";

  m_strStart = m_strStartLon + "," + m_strStartLat;
  m_strEnd = "";
  m_strMid = "";
}

void CRoute::Set(double dStartLon, double dStartLat, double dEndLon, double dEndLat) {
  m_nType = ROUTE_TYPE_START_END;
  m_dStartLon = dStartLon;
  m_dStartLat = dStartLat;
  m_dMidLon = -1;
  m_dMidLat = -1;
  m_dEndLon = dEndLon;
  m_dEndLat = dEndLat;

  m_strType = "start_end";

  std::stringstream ss;
  ss << std::setprecision(20) << m_dStartLon;
  m_strStartLon = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dStartLat;
  m_strStartLat = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dEndLon;
  m_strEndLon = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dEndLat;
  m_strEndLat = ss.str();

  m_strMidLon = "";
  m_strMidLat = "";

  m_strStart = m_strStartLon + "," + m_strStartLat;
  m_strEnd = m_strEndLon + "," + m_strEndLat;
  m_strMid = "";
}

void CRoute::Set(double dStartLon, double dStartLat, double dMidLon, double dMidLat, double dEndLon, double dEndLat) {
  m_nType = ROUTE_TYPE_START_END;
  m_dStartLon = dStartLon;
  m_dStartLat = dStartLat;
  m_dMidLon = dMidLon;
  m_dMidLat = dMidLat;
  m_dEndLon = dEndLon;
  m_dEndLat = dEndLat;

  m_strType = "start_end";
  std::stringstream ss;
  ss << std::setprecision(20) << m_dStartLon;
  m_strStartLon = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dStartLat;
  m_strStartLat = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dMidLon;
  m_strMidLon = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dMidLat;
  m_strMidLat = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dEndLon;
  m_strEndLon = ss.str();
  ss.str("");
  ss << std::setprecision(20) << m_dEndLat;
  m_strEndLat = ss.str();

  m_strStart = m_strStartLon + "," + m_strStartLat;
  m_strEnd = m_strEndLon + "," + m_strEndLat;
  m_strMid = m_strMidLon + "," + m_strMidLat;
}

void CRoute::Set(roadpkid roadID, sectionpkid sectionID) {
  m_nType = ROUTE_TYPE_ROAD_ID;
  m_roadID = roadID;
  m_sectionID = sectionID;

  m_strType = "roadID";
  m_strRoadID = std::to_string(m_roadID);
  m_strSectionID = std::to_string(m_sectionID);
}

void CRoute::ModifyStart(double dStartLon, double dStartLat) {
  m_dStartLon = dStartLon;
  m_dStartLat = dStartLat;
  std::stringstream ss;
  ss << std::setprecision(12) << m_dStartLon;
  m_strStartLon = ss.str();
  ss.str("");
  ss << std::setprecision(12) << m_dStartLat;
  m_strStartLat = ss.str();
  m_strStart = m_strStartLon + "," + m_strStartLat;
}

void CRoute::Set(std::string strInfo) { m_strInfo = strInfo; }

std::string CRoute::getControlPath() {
  std::string controlPath = "";
  for (int i = 0; i < m_ControlPathVer.size(); i++) {
    controlPath.append("lon:");
    controlPath.append(m_ControlPathVer[i].strLon);
    controlPath.append(",lat:");
    controlPath.append(m_ControlPathVer[i].strLat);
    controlPath.append(",alt:");
    controlPath.append(m_ControlPathVer[i].strAlt);
    controlPath.append(",speed_m_s:");
    controlPath.append(m_ControlPathVer[i].strSpeed_m_s);
    controlPath.append(",gear:");
    controlPath.append(m_ControlPathVer[i].strGear);
    if (i < m_ControlPathVer.size() - 1) {
      controlPath.append(";");
    }
  }
  return controlPath;
}

std::string CRoute::getTrajectoryTracking() {
  std::string strTrajectoryTracking = "";
  for (int i = 0; i < m_ControlPathVer.size(); i++) {
    strTrajectoryTracking.append("speed_m_s:");
    strTrajectoryTracking.append(m_ControlPathVer[i].strSpeed_m_s);
    strTrajectoryTracking.append(",gear:");
    strTrajectoryTracking.append(m_ControlPathVer[i].strGear);
    if (i < m_ControlPathVer.size() - 1) {
      strTrajectoryTracking.append(";");
    }
  }
  return strTrajectoryTracking;
}

void CRoute::SegmentString() {
  if (m_strControlPath.empty()) return;
  m_ControlPathVer.clear();
  std::vector<std::string> strConPathvec;
  boost::algorithm::split(strConPathvec, m_strControlPath, boost::algorithm::is_any_of(";"));

  for (int i = 0; i < strConPathvec.size(); i++) {
    std::vector<std::string> strKVvec;
    boost::algorithm::split(strKVvec, strConPathvec[i], boost::algorithm::is_any_of(","));

    std::map<std::string, std::string> map_keyvalue;
    for (int j = 0; j < strKVvec.size(); j++) {
      std::vector<std::string> strK_V;
      boost::algorithm::split(strK_V, strKVvec[j], boost::algorithm::is_any_of(":"));

      map_keyvalue.insert(std::make_pair(strK_V[0], strK_V[1]));
    }

    sRouteControlPath temp;
    auto itr = map_keyvalue.find("lon");
    if (itr != map_keyvalue.end()) temp.strLon = itr->second;

    itr = map_keyvalue.find("lat");
    if (itr != map_keyvalue.end()) temp.strLat = itr->second;

    itr = map_keyvalue.find("alt");
    if (itr != map_keyvalue.end()) temp.strAlt = itr->second;

    itr = map_keyvalue.find("speed_m_s");
    if (itr != map_keyvalue.end()) temp.strSpeed_m_s = itr->second;

    itr = map_keyvalue.find("gear");
    if (itr != map_keyvalue.end()) temp.strGear = itr->second;

    m_ControlPathVer.push_back(temp);
  }
}

void CRoute::MergeString() {
  m_strControlPath = "";
  m_strTrajectoryTracking_XOSC = "";

  for (int i = 0; i < m_ControlPathVer.size(); i++) {
    m_strControlPath.append("lon:");
    m_strControlPath.append(m_ControlPathVer[i].strLon);
    m_strControlPath.append(",lat:");
    m_strControlPath.append(m_ControlPathVer[i].strLat);
    m_strControlPath.append(",alt:");
    m_strControlPath.append(m_ControlPathVer[i].strAlt);
    m_strControlPath.append(",speed_m_s:");
    m_strControlPath.append(m_ControlPathVer[i].strSpeed_m_s);
    m_strControlPath.append(",gear:");
    m_strControlPath.append(m_ControlPathVer[i].strGear);

    m_strTrajectoryTracking_XOSC.append("speed_m_s:");
    m_strTrajectoryTracking_XOSC.append(m_ControlPathVer[i].strSpeed_m_s);
    m_strTrajectoryTracking_XOSC.append(",gear:");
    m_strTrajectoryTracking_XOSC.append(m_ControlPathVer[i].strGear);

    if (i < m_ControlPathVer.size() - 1) {
      m_strControlPath.append(";");

      m_strTrajectoryTracking_XOSC.append(";");
    }
  }
}

void CRoute::MergeXOSCString() {
  std::vector<std::string> strConPathvec;
  boost::algorithm::split(strConPathvec, m_strTrajectoryTracking_XOSC, boost::algorithm::is_any_of(";"));

  m_strControlPath = "";

  for (int i = 0; i < m_path.m_vPoints.size(); i++) {
    m_strControlPath.append("lon:");
    m_strControlPath.append(m_path.m_vPoints[i].m_strLon);
    m_strControlPath.append(",lat:");
    m_strControlPath.append(m_path.m_vPoints[i].m_strLat);
    m_strControlPath.append(",alt:");
    m_strControlPath.append(m_path.m_vPoints[i].m_strAlt);

    m_strControlPath.append(",");
    m_strControlPath.append(strConPathvec[i]);

    if (i < m_path.m_vPoints.size() - 1) {
      m_strControlPath.append(";");
    }
  }
}
