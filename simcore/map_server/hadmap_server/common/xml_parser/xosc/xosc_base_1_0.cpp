/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include <fstream>

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "common/engine/entity/hadmap.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/route.h"
#include "common/xml_parser/entity/traffic.h"
#include "common/xml_parser/xosc/xosc_base_1_0.h"
#include "engine/config.h"
#include "server_map_cache/map_data_cache.h"
#include "xml_parser/entity/simulation.h"

#include <mapengine/hadmap_engine.h>

std::set<std::string> XOSCBase_1_0::m_set_PedestrianType{
    "human",
    "oldman",
    "child",
    "girl",
    "woman",
    "cat",
    "dog",
    "bike_001",
    "elecBike_001",
    "tricycle_001",
    "tricycle_002",
    "vendingCar_001",
    "moto_001",
    "moto_002",
    "Port_Crane_001",
    "Port_Crane_002",
    "Port_Crane_002_0_0",
    "Port_Crane_002_0_2",
    "Port_Crane_002_0_5",
    "Port_Crane_002_1_0",
    "Port_Crane_002_5_0",
    "Port_Crane_003",
    "Port_Crane_004",
    "Honda_CreaScoopy_AF55",
    "HarleyDavidson_VRod_2003",
    "Aprilia_SR50R_Euro4",
    "Suzuki_GSXR1000_2010",
    "Honda_Monkey125_2017",
    "Giant_EscapeR3_2018",
    "Giant_SCR2_2017",
    "Maruishi_Bicycle_26Inch",
    "Xidesheng_AD350_2020",
    "Woman002",
    "Woman003",
    "Woman004",
    "Woman005",
    "Woman006",
    "Woman007",
    "Man003",
    "Man004",
    "Man005",
    "Man006",
    "Oldman002",
    "Oldman003",
    "Mobike_Classic2",
    "Man002",
};

XOSCBase_1_0::XOSCBase_1_0() {
  m_coordnator_str = "";
  m_dWest = 0.0;
  m_dEast = 0.0;
  m_dNorth = 0.0;
  m_dSourth = 0.0;
  m_query = nullptr;
  m_delete_query = false;
#ifdef PROJ_NEW_API
  m_C = proj_context_create();
#else
  pj_map_[s_Lonlat_Coordinator_Str] = pj_init_plus(s_Lonlat_Coordinator_Str);
#endif
}

XOSCBase_1_0::~XOSCBase_1_0() {
  if (m_delete_query) {
    if (m_query) {
      SYSTEM_LOGGER_INFO("delete a map query!");

      if (m_query->Initialized()) {
        m_query->Deinit();
      }
      delete m_query;
      m_query = nullptr;
    }
    m_delete_query = false;
  }
#ifdef PROJ_NEW_API
  if (m_PJ) {
    proj_destroy(m_PJ);
  }
  if (m_PJ2) {
    proj_destroy(m_PJ2);
  }
  if (m_C) {
    proj_context_destroy(m_C);
  }
#else
  for (auto& kv : pj_map_) {
    if (kv.second) {
      pj_free(kv.second);
      kv.second = nullptr;
    }
  }
#endif
}

void XOSCBase_1_0::ParameterDeclarationPreprocess(const std::string& source) {
  std::map<std::string, std::string> dict;
  xercesc::SAX2XMLReader* parser = xercesc::XMLReaderFactory::createXMLReader();
  ParameterDeclarationHandler handler(&dict);
  parser->setContentHandler(&handler);
  parser->setErrorHandler(&handler);
  try {
    parser->parse(source.c_str());
  } catch (const std::runtime_error& ex) {
  } catch (...) {
  }
  if (dict.empty()) {
    return;
  }
  // backup
  std::string backup = source + ".bak";
  boost::filesystem::copy_file(source, backup, BOOST_COPY_OPTION);
  // replace
  std::fstream ifs(backup, std::ios::in);
  std::fstream ofs(source, std::ios::out);
  if (ifs.is_open() && ofs.is_open()) {
    for (std::string line; std::getline(ifs, line);) {
      for (auto& kv : dict) {
        boost::replace_all(line, kv.first, kv.second);
      }
      ofs << line << std::endl;
    }
    ifs.close();
    ofs.close();
  }
  if (boost::filesystem::exists(boost::filesystem::path(backup))) {
    boost::filesystem::remove(boost::filesystem::path(backup));
  }

  delete parser;
}

bool XOSCBase_1_0::IsLatLongPj(const std::string& s) {
  if (s.empty()) {
    return false;
  }
  thread_local std::unordered_map<std::string, bool> cache;
  if (cache.find(s) != cache.end()) {
    return cache.at(s);
  }
  // auto pj = pj_init_plus(s.c_str());
  // cache[s] = pj_is_latlong(pj);
  // pj_free(pj);
  return cache.at(s);
}

int XOSCBase_1_0::CanSaveXOSC(CSimulation& simulation) {
  boost::filesystem::path p = simulation.m_mapFile.m_strMapfile;
  std::string strExt = p.extension().string();
  if (boost::iequals(strExt, ".xodr") || boost::iequals(strExt, ".xml") || boost::iequals(strExt, ".sqlite")) {
    return 0;
  }
  return -1;
}

int XOSCBase_1_0::InitMapQuery(CSimulation& simulation, bool enable_load_hadmap) {
  boost::filesystem::path p = simulation.m_mapFile.m_strMapfile;
  std::string strMapName = computeMapCompleteName(p.filename().string(), simulation.m_mapFile.m_strVersion);
  std::string strExt = p.extension().string();
  if (std::string(strExt) == std::string(".xodr")) {
    m_mapType = MAP_XODR;
  } else if (std::string(strExt) == std::string(".sqlite")) {
    m_mapType = MAP_SQLITE;
  }
  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName.c_str());
  CHadmap* pHadmap = CMapDataCache::Instance().HadmapCache(wstrMapName.c_str());
  if (pHadmap) {
    SYSTEM_LOGGER_INFO("Map exist in cache: %s, ", strMapName.c_str());
  } else if (enable_load_hadmap) {
    CMapDataCache::Instance().LoadHadMap(wstrMapName.c_str());
    pHadmap = CMapDataCache::Instance().HadmapCache(wstrMapName.c_str());
  }
  if (pHadmap) {
    CMapQuery& query = pHadmap->MapQuery();
    m_query = &query;
    m_delete_query = false;
  }

  auto pAttr = CEngineConfig::Instance().HadmapAttr(strMapName);

  if (!pHadmap && !pAttr) {
    SYSTEM_LOGGER_ERROR("InitMapQuery:hadmap not exist local, %s", strMapName.c_str());
    return -1;
  }
  if (pAttr) {
    simulation.m_mapFile.m_strLon = pAttr->m_strRefLon;
    simulation.m_mapFile.m_strLat = pAttr->m_strRefLat;
    simulation.m_mapFile.m_strAlt = pAttr->m_strRefAlt;
    simulation.m_mapFile.m_strUnrealLevelIndex = pAttr->m_strUnrealLevelIndex;
  } else if (pHadmap) {
    auto ref = pHadmap->RefPoint();
    simulation.m_mapFile.m_strLon = std::to_string(ref.X());
    simulation.m_mapFile.m_strLat = std::to_string(ref.Y());
    simulation.m_mapFile.m_strAlt = std::to_string(ref.Z());
  }

  return 0;
}

const char* XOSCBase_1_0::XODRCoordinator(CSimulation& simulation) {
  boost::filesystem::path pm = simulation.m_mapFile.m_strMapfile;
  std::string strMapName = computeMapCompleteName(pm.filename().string(), simulation.m_mapFile.m_strVersion);

  std::string strExt = pm.extension().string();
  if (boost::iequals(strExt, ".sqlite")) {
    m_coordnator_str.clear();
    return nullptr;
  }

  return XODRCoordinator(strMapName);
}

const char* XOSCBase_1_0::XODRCoordinator(std::string strMapName) {
  // boost::filesystem::path initPath = CEngineConfig::Instance().ResourceDir();
  // boost::filesystem::path hadmapPath;
  // hadmapPath.append("hadmap");

  std::string srcPath = CEngineConfig::Instance().getHadmapDataDir();
  auto mapAttr = CEngineConfig::Instance().HadmapAttrs().find(strMapName.c_str());
  if (mapAttr != CEngineConfig::Instance().HadmapAttrs().end() && mapAttr->second.m_strPreset == "") {
    srcPath = CEngineConfig::Instance().HadmapDir();
  }
  boost::filesystem::path hadmapPath = srcPath;
  boost::filesystem::path filePath = hadmapPath.append(strMapName);
  std::string strMapFile = filePath.string();

  if (strMapFile.length() < 1) {
    SYSTEM_LOGGER_ERROR("map file name is null");
    return nullptr;
  }
  boost::filesystem::path pathMapFile = strMapFile;
  if (!boost::filesystem::exists(pathMapFile)) {
    SYSTEM_LOGGER_ERROR("map file %s not exist!", strMapFile.c_str());
    return nullptr;
  }
  std::string strExt = pathMapFile.extension().string();
  if (kHadmapTypeDict.count(strExt) <= 0) {
    SYSTEM_LOGGER_ERROR("unknown map type: %s!", strMapFile.c_str());
    return nullptr;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strMapFile);

  if (!bRet) {
    SYSTEM_LOGGER_ERROR("load xodr file %s failed!", strMapFile.c_str());
    return nullptr;
  }

  TiXmlElement* odr = doc.FirstChildElement("OpenDRIVE");
  if (!odr) {
    SYSTEM_LOGGER_ERROR("no opendriver node in file %s!", strMapFile.c_str());
    return nullptr;
  }

  TiXmlElement* header = odr->FirstChildElement("header");
  if (!header) {
    SYSTEM_LOGGER_ERROR("no header node in file %s!", strMapFile.c_str());
    return nullptr;
  }
  const char* p = header->Attribute("north");
  if (p) {
    m_dNorth = atof(p);
  }

  p = header->Attribute("south");
  if (p) {
    m_dSourth = atof(p);
  }

  p = header->Attribute("west");
  if (p) {
    m_dWest = atof(p);
  }

  p = header->Attribute("east");
  if (p) {
    m_dEast = atof(p);
  }

  TiXmlElement* geoRef = header->FirstChildElement("geoReference");
  if (!geoRef) {
    SYSTEM_LOGGER_ERROR("no gerReference node in file %s, use web mectr", strMapFile.c_str());
    return nullptr;
  }

  p = geoRef->Attribute("originLat");
  if (p) {
    // cdata
  } else {
    const TiXmlNode* child = geoRef->FirstChild();
    if (child) {
      const TiXmlText* childText = child->ToText();
      if (childText) {
        SYSTEM_LOGGER_INFO("use file coordinator in file %s", strMapFile.c_str());
        SYSTEM_LOGGER_INFO("coordinator is: %s", childText->Value());
        m_coordnator_str = childText->Value();
        return childText->Value();
      }
    }
  }

  SYSTEM_LOGGER_ERROR("no gerReference node in file %s, use web mectr", strMapFile.c_str());
  return nullptr;
}

const char* XOSCBase_1_0::XODRCoordinatorCloud(std::string strMapFile) {
  boost::filesystem::path pathMapFile = strMapFile;
  if (!boost::filesystem::exists(pathMapFile)) {
    SYSTEM_LOGGER_ERROR("map file %s not exist!", strMapFile.c_str());
    return nullptr;
  }
  std::string strExt = pathMapFile.extension().string();
  if (!boost::algorithm::iequals(".xodr", strExt)) {
    SYSTEM_LOGGER_ERROR("unknown map type: %s!", strMapFile.c_str());
    return nullptr;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strMapFile);

  if (!bRet) {
    SYSTEM_LOGGER_ERROR("load xodr file %s failed!", strMapFile.c_str());
    return nullptr;
  }

  TiXmlElement* odr = doc.FirstChildElement("OpenDRIVE");
  if (!odr) {
    SYSTEM_LOGGER_ERROR("no opendriver node in file %s!", strMapFile.c_str());
    return nullptr;
  }

  TiXmlElement* header = odr->FirstChildElement("header");
  if (!header) {
    SYSTEM_LOGGER_ERROR("no header node in file %s!", strMapFile.c_str());
    return nullptr;
  }
  const char* p = header->Attribute("north");
  if (p) {
    m_dNorth = atof(p);
  }

  p = header->Attribute("south");
  if (p) {
    m_dSourth = atof(p);
  }

  p = header->Attribute("west");
  if (p) {
    m_dWest = atof(p);
  }

  p = header->Attribute("east");
  if (p) {
    m_dEast = atof(p);
  }

  TiXmlElement* geoRef = header->FirstChildElement("geoReference");
  if (!geoRef) {
    SYSTEM_LOGGER_ERROR("no gerReference node in file %s, use web mectr", strMapFile.c_str());
    return nullptr;
  }

  p = geoRef->Attribute("originLat");
  if (p) {
    // cdata
  } else {
    const TiXmlNode* child = geoRef->FirstChild();
    if (child) {
      const TiXmlText* childText = child->ToText();
      if (childText) {
        SYSTEM_LOGGER_INFO("use file coordinator in file %s", strMapFile.c_str());
        SYSTEM_LOGGER_INFO("coordinator is: %s", childText->Value());
        m_coordnator_str = childText->Value();
        return childText->Value();
      }
    }
  }

  SYSTEM_LOGGER_ERROR("no gerReference node in file %s, use web mectr", strMapFile.c_str());
  return nullptr;
}

#ifdef PROJ_NEW_API
const char* XOSCBase_1_0::ConvertLonLat2XODR(double& lon, double& lat) {
  if (m_mapType != MAP_XODR) return "";
  if (m_coordnator_str.length() > 0) {
    if (!m_PJ) {
      m_PJ = proj_create_crs_to_crs(m_C, s_Lonlat_Coordinator_Str, m_coordnator_str.c_str(), NULL);
    }
    if (m_PJ) {
      PJ_COORD a = proj_coord(lon, lat, 0, 0);
      PJ_COORD b = proj_trans(m_PJ, PJ_FWD, a);
      lon = b.xyz.x;
      lat = b.xyz.y;
    } else {
      if (!m_query) {
        return "";
      }
      m_query->LonlatToMercator(lon, lat);
    }
  } else {
    if (!m_query) {
      return "";
    }
    m_query->LonlatToMercator(lon, lat);
  }
  lon -= m_dWest;
  lat -= m_dSourth;
  return "";
}
#else
const char* XOSCBase_1_0::ConvertLonLat2XODR(double& lon, double& lat) {
  if (m_mapType != MAP_XODR) return "";
  if (IsLatLongPj(m_actualMap)) return "";
  double llon = lon;
  double llat = lat;
  if (m_coordnator_str.length() > 0) {
    if (pj_map_.count(m_coordnator_str) <= 0) {
      pj_map_[m_coordnator_str] = pj_init_plus(m_coordnator_str.c_str());
    }
    projPJ pj_xodr = pj_map_.at(m_coordnator_str);
    projPJ pj_lonlat = pj_map_.at(s_Lonlat_Coordinator_Str);

    lon *= DEG_TO_RAD;
    lat *= DEG_TO_RAD;

    int code = pj_transform(pj_lonlat, pj_xodr, 1, 1, &lon, &lat, NULL);
    if (code != 0) {
      lon = llon;
      lat = llat;
      if (!m_query) {
        return "";
      }
      m_query->LonlatToMercator(lon, lat);
    }
  } else {
    if (!m_query) {
      return "";
    }
    m_query->LonlatToMercator(lon, lat);
  }

  lon -= m_dWest;
  lat -= m_dSourth;

  return "";
}
#endif

#ifdef PROJ_NEW_API
const char* XOSCBase_1_0::ConvertXODR2LonLat(double& x, double& y) {
  if (m_mapType != MAP_XODR) return "";
  x += m_dWest;
  y += m_dSourth;
  if (m_coordnator_str.length() > 0) {
    if (!m_PJ2) {
      m_PJ2 = proj_create_crs_to_crs(m_C, m_coordnator_str.c_str(), s_Lonlat_Coordinator_Str, NULL);
    }
    if (m_PJ2) {
      PJ_COORD a = proj_coord(x, y, 0, 0);
      PJ_COORD b = proj_trans(m_PJ2, PJ_FWD, a);
      x = b.xyz.x;
      y = b.xyz.y;
    } else {
      if (!m_query) {
        return "";
      }
      m_query->MercatorToLonlat(x, y);
    }
  } else {
    if (!m_query) {
      return "";
    }
    m_query->MercatorToLonlat(x, y);
  }
  return "";
}
#else
const char* XOSCBase_1_0::ConvertXODR2LonLat(double& x, double& y) {
  if (m_mapType != MAP_XODR) return "";
  if (IsLatLongPj(m_actualMap)) return "";

  if (m_coordnator_str.length() > 0) {
    if (pj_map_.count(m_coordnator_str) <= 0) {
      pj_map_[m_coordnator_str] = pj_init_plus(m_coordnator_str.c_str());
    }
    projPJ pj_xodr = pj_map_.at(m_coordnator_str);
    projPJ pj_lonlat = pj_map_.at(s_Lonlat_Coordinator_Str);

    x += m_dWest;
    y += m_dSourth;
    double lx = x;
    double ly = y;
    int code = pj_transform(pj_xodr, pj_lonlat, 1, 1, &x, &y, NULL);
    if (code == 0) {
      x *= RAD_TO_DEG;
      y *= RAD_TO_DEG;
    } else {
      x = lx;
      y = ly;
      if (!m_query) {
        return "";
      }
      m_query->MercatorToLonlat(x, y);
    }
  } else {
    x += m_dWest;
    y += m_dSourth;
    if (!m_query) {
      return "";
    }
    m_query->MercatorToLonlat(x, y);
  }
  return "";
}
#endif
int XOSCBase_1_0::ConvertLonLat2SimPosition(double dLon, double dLat, sTagQueryInfoResult& result) {
  if (m_query && m_query->Initialized()) {
    lanelinkpkid lanelinkID = 0;
    roadpkid roadID = 0;
    sectionpkid secID = 0;
    lanepkid laneID = 0;
    double dLaneLon = -999;
    double dLaneLat = -999;
    double dLaneAlt = -999;
    double dLaneDist = 0;
    double dLaneWidth = 0;
    double dLaneOffset = 0;
    double dLanelinkLon = -999;
    double dLanelinkLat = -999;
    double dLanelinkAlt = -999;
    double dLanelinkDist = 0;
    double dLanelinkOffset = 0;
    double dLaneYaw = 0;
    int nRetLane = m_query->QuerySection(dLon, dLat, roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dLaneDist,
                                         dLaneWidth, dLaneOffset, dLaneYaw);

    int nRetLanelink = m_query->QueryLaneLink(dLon, dLat, lanelinkID, dLanelinkLon, dLanelinkLat, dLanelinkAlt,
                                              dLanelinkDist, dLanelinkOffset, dLaneYaw);

    result.dsrcLon = dLon;
    result.dsrcLat = dLat;

    if (nRetLane == 0) {
      double ret = 0;
      if (m_mapType != MAP_XODR) {
        ret = 0.1;
      }
      int nRet = m_query->QueryLanePosByEnu(dLon, dLat, ret, 0, dLaneLon, dLaneLat, dLaneAlt);
      if (nRet != 0) {
        SYSTEM_LOGGER_ERROR("GetLonLat error!");
        return -1;
      }
    }

    if (nRetLane != 0 && nRetLanelink != 0) {
      SYSTEM_LOGGER_ERROR("(%f, %f) no lane and lanelink!", dLon, dLat);
      return -1;

    } else if (nRetLane != 0 && nRetLanelink == 0) {
      result.strType = "lanelink";
      result.roadID = roadID;
      result.lanelinkID = lanelinkID;
      result.dLon = dLanelinkLon;
      result.dLat = dLanelinkLat;
      result.dAlt = dLanelinkAlt;
      result.dShift = dLanelinkDist;
      result.dOffset = dLanelinkOffset;

    } else if (nRetLane == 0 && nRetLanelink != 0) {
      result.strType = "lane";
      result.roadID = roadID;
      result.laneID = laneID;
      result.dLon = dLaneLon;
      result.dLat = dLaneLat;
      result.dAlt = dLaneAlt;
      result.dShift = dLaneDist;
      result.dOffset = dLaneOffset;

    } else if (nRetLane == 0 && nRetLanelink == 0) {
      double dAbsLanelinkOffset = dLanelinkOffset > 0 ? dLanelinkOffset : -1 * dLanelinkOffset;
      double dAbsLaneOffset = dLaneOffset > 0 ? dLaneOffset : -1 * dLaneOffset;

      if (dAbsLaneOffset <= dAbsLanelinkOffset || dLaneDist < 0.01) {
        result.strType = "lane";
        result.roadID = roadID;
        result.laneID = laneID;
        result.dLon = dLaneLon;
        result.dLat = dLaneLat;
        result.dAlt = dLaneAlt;
        result.dShift = dLaneDist;
        result.dOffset = dLaneOffset;

      } else {
        result.strType = "lanelink";
        result.lanelinkID = lanelinkID;
        result.dLon = dLanelinkLon;
        result.dLat = dLanelinkLat;
        result.dAlt = dLanelinkAlt;
        result.dShift = dLanelinkDist;
        result.dOffset = dLanelinkOffset;
      }
    }

    return 0;
  }

  return -1;
}

int XOSCBase_1_0::ConvertLonLat2SimPosition(roadpkid roadID, /*sectionpkid secID,*/ lanepkid laneID, double dShift,
                                            double dOffset, sTagQueryInfoResult& result) {
  if (m_query && m_query->Initialized()) {
    sectionpkid secID;
    hadmap::txLanes pLanesAll;
    if (laneID > 0) {
      roadID = 1000000000 + roadID;
      laneID = -laneID;
      hadmap::txRoadPtr _road;
      hadmap::getRoad(m_query->GettxMapHandle(), roadID, false, _road);
      if (dShift > -9999) dShift = _road->getLength() - dShift;
      dOffset = -dOffset;
    }
    if (dShift == -9999) {
      dShift = 0.1;
    }
    const hadmap::txLaneId tempid(roadID, SECT_PKID_INVALID, laneID);
    const hadmap::roadpkid roadid = roadID;
    hadmap::txRoadPtr roadPtr;
    hadmap::getRoad(m_query->GettxMapHandle(), roadid, true, roadPtr);
    double _shift = 0;
    if (roadPtr) {
      hadmap::txSections _sections = roadPtr->getSections();
      double Len = 0;
      int index = 0;
      for (auto it : _sections) {
        _shift = dShift - Len;
        Len += it->getLength();
        if ((Len - dShift) >= 0.000001) {
          secID = it->getId();
          break;
        } else {
          if (index == _sections.size() - 1) secID = it->getId();
          continue;
        }
        index++;
      }
    } else {
      return -1;
    }
    result.sectionID = secID;
    dShift = _shift;
    hadmap::txLaneLinkPtr lanelink;

    double dLon = -999;
    double dLat = -999;
    double dAlt = -999;
    double dWidth = -999;
    double yaw;
    int nRet = m_query->QueryLonLat(roadID, secID, laneID, dShift, dOffset, dLon, dLat, dAlt, dWidth, yaw);
    result.yaw = yaw;
    if (nRet == 0) {
      result.strType = "lane";
      result.roadID = roadID;
      result.laneID = laneID;
      result.dsrcLon = dLon;
      result.dsrcLat = dLat;
      result.dLon = dLon;
      result.dLat = dLat;
      result.dAlt = dAlt;
      result.dShift = dShift;
      result.dOffset = dOffset;
      result.yaw = yaw;
      return 0;
    } else {
      return -1;
    }
  }

  return -1;
}

int XOSCBase_1_0::ConvertLonLat2SimPosition(roadpkid roadID, sectionpkid secID, lanepkid laneID, double dShift,
                                            double dOffset, sTagQueryInfoResult& result) {
  if (m_query && m_query->Initialized()) {
    hadmap::txLanes pLanesAll;
    if (laneID > 0) {
      roadID = 1000000000 + roadID;
      laneID = -laneID;
      hadmap::txRoadPtr _road;
      hadmap::getRoad(m_query->GettxMapHandle(), roadID, false, _road);
      if (dShift > -9999) dShift = _road->getLength() - dShift;
      dOffset = -dOffset;
    }
    if (dShift == -9999) {
      dShift = 0.1;
    }

    double dLon = -999;
    double dLat = -999;
    double dAlt = -999;
    double dWidth = -999;
    double yaw;
    int nRet = m_query->QueryLonLat(roadID, secID, laneID, dShift, dOffset, dLon, dLat, dAlt, dWidth, yaw);
    result.yaw = yaw;
    if (nRet == 0) {
      result.strType = "lane";
      result.roadID = roadID;
      result.laneID = laneID;
      result.dsrcLon = dLon;
      result.dsrcLat = dLat;
      result.dLon = dLon;
      result.dLat = dLat;
      result.dAlt = dAlt;
      result.dShift = dShift;
      result.dOffset = dOffset;
      result.yaw = yaw;
      return 0;
    } else {
      return -1;
    }
  }

  return -1;
}

int XOSCBase_1_0::ConvertLonLat2SimPosition(roadpkid roadID, double dShift, double dOffset,
                                            sTagQueryInfoResult& result) {
  if (m_query == NULL || m_query->GettxMapHandle() == NULL) return -1;
  hadmap::txRoadPtr pRoad;
  int ret = hadmap::getRoad(m_query->GettxMapHandle(), roadID, true, pRoad);
  const hadmap::txCurve* pCurve = pRoad->getGeometry();

  double x, y, s, l, yaw;

  pCurve->sl2xy(dShift, dOffset, x, y, yaw);

  return ConvertLonLat2SimPosition(x, y, result);
}

int XOSCBase_1_0::ConvertSimPosition2LonLat(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset,
                                            double& dFinalLon, double& dFinalLat) {
  if (m_query == NULL || m_query->GettxMapHandle() == NULL) return -1;
  dFinalLon = -999;
  dFinalLat = -999;
  double dFinalAlt = -999;
  double dFinalDist = -999;
  double dWidth = -1;
  double dFinalYaw;
  int nRet = m_query->QueryLonLatByPoint(dLon, dLat, laneID, dShift, dOffset, dFinalLon, dFinalLat, dFinalAlt, dWidth,
                                         dFinalDist, dFinalYaw, true);
  if (nRet < 0) {
    SYSTEM_LOGGER_ERROR("convert position lon lat error");
    return -1;
  }

  return 0;
}

int XOSCBase_1_0::ConvertSimPosition2Local(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset,
                                           double& dX, double& dY) {
  dX = -999;
  dY = -999;
  double dZ = -999;
  int nRet = ConvertSimPosition2LonLat(dLat, dLat, laneID, dShift, dOffset, dX, dY);
  if (nRet < 0) {
    SYSTEM_LOGGER_ERROR("convert position lon lat error");
    return -1;
  }

  nRet = m_query->LonlatToLocal(dX, dY, dZ);
  if (nRet < 0) {
    SYSTEM_LOGGER_ERROR("convert position to local error");
    return -1;
  }

  return 0;
}

int XOSCBase_1_0::ConvertSimPosition2Local(double dLon, double dLat, std::string strlaneID, std::string strShift,
                                           std::string strOffset, double& dX, double& dY) {
  dX = -999;
  dY = -999;
  double dZ = -999;
  int nRet = ConvertSimPosition2LonLat(dLat, dLat, stoi(strlaneID), stod(strShift), stod(strOffset), dX, dY);
  if (nRet < 0) {
    SYSTEM_LOGGER_ERROR("convert position lon lat error");
    return -1;
  }

  nRet = m_query->LonlatToLocal(dX, dY, dZ);
  if (nRet < 0) {
    SYSTEM_LOGGER_ERROR("convert position to local error");
    return -1;
  }

  return 0;
}

int XOSCBase_1_0::ConvertLonLat2Local(double& dX, double& dY) {
  double dZ = -999;
  int nRet = m_query->LonlatToLocal(dX, dY, dZ);
  if (nRet < 0) {
    SYSTEM_LOGGER_ERROR("convert position to local error");
    return -1;
  }

  return 0;
}

int XOSCBase_1_0::ExtractPosition(CTraffic& traffic, std::string strRouteID, lanepkid laneID, double dShift,
                                  double dOffset, double& dFinalLon, double& dFinalLat, double& dFinalYaw,
                                  bool isLonLat) {
  CTraffic::RouteMap& mapRoutes = traffic.Routes();
  CTraffic::RouteMap::iterator rItr = mapRoutes.find(strRouteID);
  if (rItr == mapRoutes.end()) {
    assert(false);
    return false;
  }

  return ExtractPosition(rItr->second, laneID, dShift, dOffset, dFinalLon, dFinalLat, dFinalYaw, isLonLat);
}

int XOSCBase_1_0::ExtractPosition(CTraffic& traffic, std::string strRouteID, std::string strlaneID,
                                  std::string strShift, std::string strOffset, double& dFinalLon, double& dFinalLat,
                                  double& dFinalYaw, bool isLonLat) {
  CTraffic::RouteMap& mapRoutes = traffic.Routes();
  CTraffic::RouteMap::iterator rItr = mapRoutes.find(strRouteID);
  if (rItr == mapRoutes.end()) {
    assert(false);
    return false;
  }

  return ExtractPosition(rItr->second, stoi(strlaneID), stod(strShift), stod(strOffset), dFinalLon, dFinalLat,
                         dFinalYaw, isLonLat);
}

int XOSCBase_1_0::ExtractPosition(CRoute& r, lanepkid laneID, double dShift, double dOffset, double& dFinalLon,
                                  double& dFinalLat, double& dFinalYaw, bool isLonLat) {
  r.FromStr();
  r.ConvertToValue();
  r.ExtractPath();

  // ------------ position ---------------
  double dLon = r.m_dStartLon;
  double dLat = r.m_dStartLat;
  dFinalLon = -999;
  dFinalLat = -999;
  double dFinalAlt = -999;
  double dFinalDist = -999;
  double dWidth = -1;
  lanelinkpkid lanelinkid;
  int nRet = m_query->QueryLonLatByPoint(dLon, dLat, laneID, dShift, dOffset, dFinalLon, dFinalLat, dFinalAlt, dWidth,
                                         dFinalDist, dFinalYaw, true);
  if (nRet >= 0) {
    if (!isLonLat) ConvertLonLat2XODR(dFinalLon, dFinalLat);
    return 0;
  }
  int nRetLink =
      m_query->QueryLaneLink(dLon, dLat, lanelinkid, dFinalLon, dFinalLat, dFinalAlt, dWidth, dFinalDist, dFinalYaw);
  if (nRetLink >= 0) {
    if (!isLonLat) ConvertLonLat2XODR(dFinalLon, dFinalLat);
    return 0;
  }
  SYSTEM_LOGGER_ERROR("convert route %s position error", r.m_strID.c_str());
  return -1;
}

int XOSCBase_1_0::ExtractPosition(CRoute& r, std::string strlaneID, std::string strShift, std::string strOffset,
                                  double& dFinalLon, double& dFinalLat, double& dFinalYaw, bool isLonLat) {
  r.FromStr();
  r.ConvertToValue();
  r.ExtractPath();

  // ------------ position ---------------
  double dLon = r.m_dStartLon;
  double dLat = r.m_dStartLat;
  dFinalLon = -999;
  dFinalLat = -999;
  double dFinalAlt = -999;
  double dFinalDist = -999;
  double dWidth = -1;
  int nRetLane = m_query->QueryLonLatByPoint(dLon, dLat, stoi(strlaneID), stod(strShift), stod(strOffset), dFinalLon,
                                             dFinalLat, dFinalAlt, dWidth, dFinalDist, dFinalYaw, true);
  if (nRetLane < 0) {
    int nRetLaneLink = m_query->QueryLonLatByPointOnLanelink(
        dLon, dLat, stoi(strlaneID), stod(strShift), stod(strOffset), dFinalLon, dFinalLat, dFinalAlt, dFinalDist);
    if (nRetLaneLink < 0) {
      SYSTEM_LOGGER_ERROR("convert route %s position error", r.m_strID.c_str());
      return -1;
    }
  }
  if (!isLonLat) ConvertLonLat2XODR(dFinalLon, dFinalLat);
  return 0;
}

int XOSCBase_1_0::GetYaw(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset, double& dFinalYaw) {
  return 0;
}

int XOSCBase_1_0::GetNearByLaneFromStart(double dLon, double dLat, hadmap::txLanePtr& pLane) {
  int nRet = m_query->GetNearByLaneFromOldStart(dLon, dLat, pLane);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }
  return 0;
}

int XOSCBase_1_0::GetNearByLaneFromEnd(double dLon, double dLat, hadmap::txLanePtr& pLane) {
  int nRet = m_query->GetNearbyLane(dLon, dLat, pLane);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }
  return 0;
}

int XOSCBase_1_0::GetTrafficLightInfo(const std::string strId, std::string& routeId, std::string& phase,
                                      std::string& lane, std::string& junction, std::string& roadId,
                                      hadmap::txPoint& endp) {
  return m_query->GetTrafficLights(strId, routeId, phase, lane, junction, roadId, endp);
}

std::string XOSCBase_1_0::DoubleToString(double& DD, int nlen) {
  std::string str;
  std::stringstream ss;
  ss << std::fixed << std::setprecision(nlen) << DD;
  str = ss.str();
  return str;
}
