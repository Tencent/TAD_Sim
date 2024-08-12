/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "simulation_param.h"

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cstring>
#include <iostream>
#include <utility>
#include "common/log/xml_logger.h"
#include "common/xml_parser/entity/parser_tools.h"
#include "common/xml_parser/entity/traffic.h"

int CSimulationParam::Parse(const char* strFileName) {
  int nRet = CSimulation::Parse(strFileName, false);
  if (nRet != 0) {
    return nRet;
  }

  m_planner.ConvertToValue();

  return 0;
}

int CSimulationParam::ParseTemplate(const char* strFileName) {
  if (!strFileName) {
    XML_LOGGER_ERROR("sim file name null");
    return -1;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFileName);

  if (!bRet) {
    XML_LOGGER_ERROR("sim format error!");
    return -1;
  }

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  // if (_stricmp(strName.c_str(), "simulation") != 0)
  if (!boost::algorithm::iequals(strName, "simulation")) return -1;

  TiXmlElement* elemMapFile = xmlRoot->FirstChildElement("mapfile");
  int nRet = ParseMapfile(elemMapFile, m_mapFile);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemTraffic = xmlRoot->FirstChildElement("traffic");
  nRet = ParseTraffic(elemTraffic, m_strTraffic);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemPlanner = xmlRoot->FirstChildElement("planner");
  nRet = ParsePlanner(elemPlanner, m_plannerparam);
  if (nRet != 0) {
    return -1;
  }

  XML_LOGGER_INFO("Parse sim file : %s succeed!", strFileName);

  return 0;
}

int CSimulationParam::ParsePlanner(TiXmlElement* elemPlanner, CPlannerParam& planner) {
  if (!elemPlanner) return -1;

  TiXmlElement* elemRoute = elemPlanner->FirstChildElement("route");
  int nRet = CTraffic::ParseOneRoute(elemRoute, planner.m_route);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemStartV = elemPlanner->FirstChildElement("start_v");
  if (!elemStartV) {
    return -1;
  }

  const char* p = elemStartV->GetText();
  if (p) {
    planner.m_strStartVelocity = p;

    std::vector<std::string> strs;
    std::string strSep = ",";
    CParseTools::SplitString(planner.m_strStartVelocity, strs, strSep);

    if (strs.size() != 3) {
      assert(false);
    }

    planner.m_fStartVelocityStart = atof(strs[0].c_str());
    planner.m_fStartVelocityEnd = atof(strs[1].c_str());
    planner.m_fStartVelocitySep = atof(strs[2].c_str());
  }

  TiXmlElement* elemTheta = elemPlanner->FirstChildElement("theta");
  if (!elemTheta) {
    return -1;
  }

  p = elemTheta->GetText();
  if (p) planner.m_strTheta = p;

  TiXmlElement* elemVelocityMax = elemPlanner->FirstChildElement("Velocity_Max");
  if (!elemVelocityMax) {
    return -1;
  }

  p = elemVelocityMax->GetText();
  if (p) planner.m_strVelocityMax = p;

  return 0;
}
