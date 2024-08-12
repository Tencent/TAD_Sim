/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "parser.h"
#include <fstream>
#include "common/log/xml_logger.h"
// #include<direct.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"

int CParser::Parse(sTagSimuTraffic& simTrac, const char* strSimuFile, bool bLoadHadmap /* = true*/) {
  assert(false);

  int nRet = simTrac.m_simulation.Parse(strSimuFile, bLoadHadmap);
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed!", strSimuFile);
    return -1;
  }

  std::string strTrafficFile = simTrac.m_simulation.m_strTraffic;
  nRet = simTrac.m_traffic.Parse(strTrafficFile.c_str());
  if (nRet) {
    XML_LOGGER_ERROR("traffic file %s parse failed!", strTrafficFile.c_str());
    return -1;
  }

  return 0;
}

int CParser::Parse(sTagSimuTraffic& simTrac, const char* strSimuFile, const char* strDirScene,
                   bool bLoadHadmap /* = true*/) {
  boost::filesystem::path p = strSimuFile;
  std::string strExt = p.extension().string();

  if (boost::iequals(strExt, ".sim")) {
    return ParseSim(simTrac, strSimuFile, strDirScene, bLoadHadmap);
  } else if (boost::iequals(strExt, ".xosc")) {
    return ParseXOSC(simTrac, strSimuFile, strDirScene, bLoadHadmap);
  } else {
    SYSTEM_LOGGER_ERROR("scene extension is error!");
    return -1;
  }
}

int CParser::ParseXOSC(sTagSimuTraffic& simTrac, const char* strXoscFile, const char* strDirScene,
                       bool bLoadHadmap /* = true*/) {
  XOSCReader_1_0_v4 reader;
  simTrac.m_strType = "xosc";
  int nRet = reader.ParseXOSC(strXoscFile, simTrac);
  if (nRet != 0) {
    return nRet;
  }

  if (bLoadHadmap) {
    CMapFile& mapfile = simTrac.m_simulation.m_mapFile;
    if (mapfile.m_strMapfile.length() > 0) {
      nRet = mapfile.ParseMapV2(mapfile.m_strMapfile.c_str());
      if (nRet != 0) {
        return -1;
      }
    }
  }

  return 0;
}

int CParser::ParseSim(sTagSimuTraffic& simTrac, const char* strSimuFile, const char* strDirScene,
                      bool bLoadHadmap /* = true*/) {
  // project
  {
    boost::filesystem::path project = strDirScene;
    boost::filesystem::path projectName = strSimuFile;
    std::string strProjName = projectName.stem().string();
    strProjName.append(".prj");
    project /= strProjName;
    if (boost::filesystem::exists(project)) {
      simTrac.m_project.Parse(project.lexically_normal().make_preferred().string().c_str(), false);
    }
  }

  // simulation
  int nRet = simTrac.m_simulation.Parse(strSimuFile, bLoadHadmap);
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed!", strSimuFile);
    return -1;
  }

  // traffic
  if (simTrac.m_simulation.m_strTraffic.size() > 0) {
    boost::filesystem::path p = strDirScene;
    p /= simTrac.m_simulation.m_strTraffic;

    std::string strTrafficFile = p.lexically_normal().make_preferred().string();
    std::string strExt = p.extension().string();
    if (boost::algorithm::iequals(".xml", strExt)) {
      simTrac.m_traffic.SetTrafficFileType(TRAFFIC_TYPE_XML);
      nRet = simTrac.m_traffic.Parse(strTrafficFile.c_str());
      if (nRet) {
        XML_LOGGER_ERROR("traffic file %s parse failed!", strTrafficFile.c_str());
        return -1;
      }
    } else if (boost::algorithm::iequals(".xosc", strExt)) {
      simTrac.m_traffic.SetTrafficFileType(TRAFFIC_TYPE_XOSC);
    } else {
      simTrac.m_traffic.SetTrafficFileType(TRAFFIC_TYPE_INVALID);
    }
  }

  // sensors
  if (simTrac.m_simulation.m_strSensor.size() > 0) {
    boost::filesystem::path p = strDirScene;
    p /= simTrac.m_simulation.m_strSensor;

    std::string strSensorFile = p.lexically_normal().make_preferred().string();
    nRet = simTrac.m_sensors.Parse(strSensorFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("sensor file %s parse failed!", strSensorFile.c_str());
      return -1;
    }
  }

  // environment
  if (simTrac.m_simulation.m_strEnvironment.size() > 0) {
    boost::filesystem::path p = strDirScene;
    p /= simTrac.m_simulation.m_strEnvironment;

    std::string strEnvironmentFile = p.lexically_normal().make_preferred().string();
    nRet = simTrac.m_environment.Parse(strEnvironmentFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("environment file %s parse failed!", strEnvironmentFile.c_str());
      simTrac.m_simulation.m_strEnvironment = "";
      // return -1;
    }
  }

  // grading
  if (simTrac.m_simulation.m_strGrading.size() > 0) {
    boost::filesystem::path p = strDirScene;
    p /= simTrac.m_simulation.m_strGrading;

    std::string strGradingFile = p.lexically_normal().make_preferred().string();
    nRet = simTrac.m_grading.Parse(strGradingFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("grading file %s parse failed!", strGradingFile.c_str());
      simTrac.m_simulation.m_strGrading = "";
    }
  }

  return 0;
}

bool CParser::DirectoryExist(const char* strPath) {
  std::fstream _file;
  _file.open(strPath, std::ios::in);
  if (!_file) {
    return false;
  }

  return true;
}

bool CParser::CreateDir(const char* strPath) {
  boost::filesystem::path p = strPath;
  if (!boost::filesystem::exists(p) || !boost::filesystem::is_directory(p)) {
    boost::filesystem::create_directory(p);
  }
  return true;
}
