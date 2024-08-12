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
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"
#include "common/xml_parser/xosc_replay/xosc_replay_converter.h"

int CParser::Parse(sTagSimuTraffic& simTrac, const char* strSimuFile, bool bLoadHadmap /* = true*/) {
  assert(false);
  //
  int nRet = simTrac.m_simulation.Parse(strSimuFile, bLoadHadmap);
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed:%d!", strSimuFile, nRet);
    return -1;
  }
  //// load map to convert
  ////
  std::string strTrafficFile = simTrac.m_simulation.m_strTraffic;
  nRet = simTrac.m_traffic.Parse(strTrafficFile.c_str());
  // convert route (laneid > 0)
  if (nRet) {
    XML_LOGGER_ERROR("traffic file %s parse failed!", strTrafficFile.c_str());
    return -1;
  }
  AdjustData(simTrac);
  return 0;
}

int CParser::Parse(sTagSimuTraffic& simTrac, const char* strSimuFile, const char* strDirScene,
                   bool bLoadHadmap /* = true*/, std::string strTrafficType) {
  boost::filesystem::path p = strSimuFile;
  std::string strExt = p.extension().string();
  if (boost::iequals(strExt, ".sim")) {
    return ParseSim(simTrac, strSimuFile, strDirScene, bLoadHadmap);
  } else if (boost::iequals(strExt, ".xosc")) {
    // if is xosc ,first to convert xosc to sim
    if (strTrafficType == "simrec") {
      SYSTEM_LOGGER_INFO("Simrec xosc = %s", strSimuFile);
      XOSCReplayConverter convert;
      utils::Status statu = convert.SerializeToSimrec(strSimuFile);
      std::string STATU = statu.ToString();
      std::string xosc_file_path = strSimuFile;
      xosc_file_path.replace(xosc_file_path.find_last_of("."), 5, "_convert.sim");
      SYSTEM_LOGGER_INFO("XOSC_FILE_PATH = %s file statu = %s", xosc_file_path.c_str(), STATU.c_str());
      simTrac.m_strType = "xosc";
      return ParseSim(simTrac, xosc_file_path.c_str(), strDirScene, bLoadHadmap);
    } else {
      return ParseXOSC(simTrac, strTrafficType, strSimuFile, strDirScene, bLoadHadmap);
    }
  } else {
    SYSTEM_LOGGER_ERROR("scene extension is error!");
    return -1;
  }
}

int CParser::ParseXOSC(sTagSimuTraffic& simTrac, std::string strTrafficType, const char* strXoscFile,
                       const char* strDirScene, bool bLoadHadmap /* = true*/) {
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
        return -3;
      }
      return nRet;
    }
  }
  return 0;
}

int CParser::AdjustData(sTagSimuTraffic& simTrac) {
  // boundingbox
  CCatalogModelPtr ptr = NULL;
  for (auto& it : simTrac.m_traffic.Vehicles()) {
    if (CCataLog::getCatalogName(simTrac.getCatalogDir().c_str(), CATALOG_TYPE_VEHICLE, it.second.m_strType, ptr) ==
        0) {
      it.second.m_boundingBox = ptr->getBoundingBox();
    }
  }
  for (auto& it : simTrac.m_traffic.Obstacles()) {
    if (CCataLog::getCatalogName(simTrac.getCatalogDir().c_str(), CATALOG_TYPE_MISCOBJECT, it.second.m_strType, ptr) ==
        0) {
      it.second.m_boundingBox = ptr->getBoundingBox();
    }
  }
  for (auto& it : simTrac.m_traffic.PedestriansV2()) {
    if (CCataLog::getCatalogName(simTrac.getCatalogDir().c_str(), CATALOG_TYPE_PEDESTRIAN, it.second.m_strType, ptr) ==
        0) {
      it.second.m_boundingBox = ptr->getBoundingBox();
    }
  }
  // double road
  int ret = InitMapQuery(simTrac.m_simulation);
  if (0 != ret) {
    return ret;
  }
  for (auto& itvehicle : simTrac.m_traffic.Vehicles()) {
    std::string strRouteid = itvehicle.second.m_strRouteID;
    if (simTrac.m_traffic.m_mapRoutes.find(strRouteid) != simTrac.m_traffic.m_mapRoutes.end()) {
      auto& itRoute = simTrac.m_traffic.m_mapRoutes.find(strRouteid)->second;
      hadmap::txLanePtr lanePtr = NULL;
      itRoute.ConvertToValue();
      int nQueRet = GetNearByLaneFromEnd(itRoute.m_dStartLon, itRoute.m_dStartLat, lanePtr);
      if (nQueRet == 0 && lanePtr != NULL) {
        if (lanePtr->getId() > 0) {
          itvehicle.second.m_laneID *= -1;
          itvehicle.second.m_fStartShift = lanePtr->getLength() - itvehicle.second.m_fStartShift;
          itvehicle.second.m_fStartOffset *= -1;
          itvehicle.second.ConvertToStr();
          itRoute.ModifyStart(lanePtr->getGeometry()->getStart().x, lanePtr->getGeometry()->getStart().y);
        }
      }
    }
  }
  for (auto& itObstacle : simTrac.m_traffic.Obstacles()) {
    std::string strRouteid = itObstacle.second.m_strRouteID;
    if (simTrac.m_traffic.m_mapRoutes.find(strRouteid) != simTrac.m_traffic.m_mapRoutes.end()) {
      auto& itRoute = simTrac.m_traffic.m_mapRoutes.find(strRouteid)->second;
      hadmap::txLanePtr lanePtr = NULL;
      itRoute.ConvertToValue();
      int nQueRet = GetNearByLaneFromEnd(itRoute.m_dStartLon, itRoute.m_dStartLat, lanePtr);
      if (nQueRet == 0 && lanePtr != NULL) {
        if (lanePtr->getId() > 0) {
          itObstacle.second.ConvertToValue();
          itObstacle.second.m_laneID *= -1;
          itObstacle.second.m_fStartShift = lanePtr->getLength() - itObstacle.second.m_fStartShift;
          itObstacle.second.m_fStartOffset *= -1;
          itObstacle.second.ConvertToStr();
          itRoute.ModifyStart(lanePtr->getGeometry()->getStart().x, lanePtr->getGeometry()->getStart().y);
        }
      }
    }
  }
  for (auto& itPedestrian : simTrac.m_traffic.PedestriansV2()) {
    std::string strRouteid = itPedestrian.second.m_strRouteID;

    if (simTrac.m_traffic.m_mapRoutes.find(strRouteid) != simTrac.m_traffic.m_mapRoutes.end()) {
      auto& itRoute = simTrac.m_traffic.m_mapRoutes.find(strRouteid)->second;
      hadmap::txLanePtr lanePtr = NULL;
      itRoute.ConvertToValue();
      int nQueRet = GetNearByLaneFromEnd(itRoute.m_dStartLon, itRoute.m_dStartLat, lanePtr);
      if (nQueRet == 0 && lanePtr != NULL) {
        if (lanePtr->getId() > 0) {
          itPedestrian.second.ConvertToValue();
          itPedestrian.second.m_laneID *= -1;
          itPedestrian.second.m_fStartShift = lanePtr->getLength() - itPedestrian.second.m_fStartShift;
          itPedestrian.second.m_fStartOffset *= -1;
          itPedestrian.second.ConvertToStr();
          itRoute.ModifyStart(lanePtr->getGeometry()->getStart().x, lanePtr->getGeometry()->getStart().y);
        }
      }
    }
  }
  for (auto& itLight : simTrac.m_traffic.Signlights()) {
    std::string strRouteid = itLight.second.m_strRouteID;

    if (simTrac.m_traffic.m_mapRoutes.find(strRouteid) != simTrac.m_traffic.m_mapRoutes.end()) {
      auto& itRoute = simTrac.m_traffic.m_mapRoutes.find(strRouteid)->second;
      hadmap::txLanePtr lanePtr = NULL;
      itRoute.ConvertToValue();
      int nQueRet = GetNearByLaneFromEnd(itRoute.m_dStartLon, itRoute.m_dStartLat, lanePtr);
      if (nQueRet == 0 && lanePtr != NULL) {
        if (lanePtr->getId() > 0) {
          itLight.second.ConvertLaneId();
        }
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
    XML_LOGGER_ERROR("simulation file %s parse failed:%d!", strSimuFile, nRet);
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
      return -1;
    }
  }

  // grading
  if (simTrac.m_simulation.m_strGrading.size() > 0) {
    boost::filesystem::path p = strDirScene;
    p /= simTrac.m_simulation.m_strGrading;

    std::string strGradingFile = p.lexically_normal().make_preferred().string();
    nRet = simTrac.m_grading.Parse(strGradingFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("grading file %s parse failed: %d!", strGradingFile.c_str(), nRet);
      return -1;
    }
  }

  AdjustData(simTrac);
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
