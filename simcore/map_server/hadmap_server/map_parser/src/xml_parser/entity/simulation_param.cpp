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

CSimulationParam::CSimulationParam() {}

CSimulationParam::CSimulationParam(CSimulation& sim) { (*this) = sim; }

CSimulationParam& CSimulationParam::operator=(const CSimulation& sim) {
  if (this == &sim) {
    return (*this);
  }

  m_strScenePath = sim.m_strScenePath;
  m_strInfo = sim.m_strInfo;  // 工况
  m_mapFile = sim.m_mapFile;
  m_strTraffic = sim.m_strTraffic;
  m_strSensor = sim.m_strSensor;
  m_strEnvironment = sim.m_strEnvironment;
  m_strControlPath = sim.m_strControlPath;
  m_strGrading = sim.m_strGrading;
  m_generateInfo = sim.m_generateInfo;
  m_planner = sim.m_planner;
  m_rosbag = sim.m_rosbag;
  m_l3States = sim.m_l3States;
  m_strGeoFence = sim.m_strGeoFence;
  m_controlTrack = sim.m_controlTrack;

  return (*this);
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
  return 0;
}
