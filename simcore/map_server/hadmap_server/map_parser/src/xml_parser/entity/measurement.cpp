/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "measurement.h"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <iomanip>
#include <sstream>
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

sTagMeasurementPoint::sTagMeasurementPoint() { Reset(); }

void sTagMeasurementPoint::Reset() {
  dLon = 0;
  dLat = 0;
  dAlt = 0;
}

CMeasurement::CMeasurement() { Reset(); }

CMeasurement& CMeasurement::operator=(const CMeasurement& other) {
  m_strID = other.m_strID;
  m_strPositions = other.m_strPositions;

  m_ID = other.m_ID;
  m_nodes = other.m_nodes;

  return (*this);
}

bool CMeasurement::operator<(const CMeasurement& other) {
  if (m_ID < other.m_ID) {
    return true;
  }

  return false;
}

void CMeasurement::Reset() {
  m_strID = "";
  m_strPositions = "";

  m_ID = 0;
  m_nodes.clear();
}

void CMeasurement::ConvertToValue() {
  m_ID = atoi(m_strID.c_str());
  std::vector<std::string> strs;
  CParseTools::SplitString(m_strPositions, strs, ";");

  for (int i = 0; i < strs.size(); ++i) {
    std::string strOneNode = strs[i];
    std::vector<std::string> strCoords;
    CParseTools::SplitString(strOneNode, strCoords, ",");

    assert(strCoords.size() == 3);
    if (strCoords.size() == 3) {
      sTagMeasurementPoint mp;
      mp.dLon = atof(strCoords[0].c_str());
      mp.dLat = atof(strCoords[1].c_str());
      mp.dAlt = atof(strCoords[2].c_str());

      m_nodes.push_back(mp);
    } else {
      SYSTEM_LOGGER_ERROR("project file measurement node error!");
      continue;
    }
  }
}

void CMeasurement::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  m_strPositions = "";
  MeasurementNodes::iterator itr = m_nodes.begin();
  int nSize = m_nodes.size();
  int nCount = 0;
  for (; itr != m_nodes.end(); ++itr) {
    std::string strLon = FormatString((*itr).dLon);
    std::string strLat = FormatString((*itr).dLat);
    std::string strAlt = FormatString((*itr).dAlt);
    std::string strOneNode = strLon + "," + strLat + "," + strAlt;
    if (nCount > 0) {
      m_strPositions.append(";");
    }

    m_strPositions.append(strOneNode);

    nCount++;
  }
}

void CMeasurement::Set(uint64_t id) {
  m_ID = id;
  m_strID = std::to_string(id);
}

void CMeasurement::Set(std::string strPositions) { m_strPositions = strPositions; }

std::string CMeasurement::FormatString(double value) {
  std::stringstream ss;
  ss << std::setprecision(10) << value;
  std::string str = ss.str();
  return str;
}
