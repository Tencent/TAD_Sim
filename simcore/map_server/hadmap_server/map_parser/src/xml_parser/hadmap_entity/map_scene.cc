/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_scene.h"
#include "cross.h"
#include "curve_road.h"
#include "entity_link.h"
#include "group_entity.h"
#include "straight_road.h"

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <iomanip>
#include <iostream>
#include "../../engine/config.h"
#include "../../src/server_map_cache/map_data_cache.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/mapfile.h"
#include "map_operator.h"
CMapScene::CMapScene() {
  m_dRefLon = 0.0;
  m_dRefLat = 0.0;
  m_dRefAlt = 0.0;
}

CMapScene::~CMapScene() {
  for (int i = 0; i < m_straight_roads.size(); ++i) {
    delete m_straight_roads[i];
  }
  m_straight_roads.clear();
  m_map_straight_roads.clear();

  for (int i = 0; i < m_groups.size(); ++i) {
    delete m_groups[i];
  }
  m_groups.clear();
}

void CMapScene::SetRef(double lon, double lat, double alt) {
  m_dRefLon = lon;
  m_dRefLat = lat;
  m_dRefAlt = alt;
}

void CMapScene::AddGroup(CGroupEntity* pGroup) { m_groups.push_back(pGroup); }

void CMapScene::AddStraight(CStraightRoad* pStraight) {
  // LOG(INFO) << "CMapScene::AddStraight";
  SYSTEM_LOGGER_INFO("CMapScene::AddStraight");
  CStraightRoad* pRoad = FindStraight(pStraight->m_id);
  if (pRoad != nullptr) {
    SYSTEM_LOGGER_INFO("straight road %d has added!", pStraight->m_id);
    return;
  }

  m_straight_roads.push_back(pStraight);
  m_map_straight_roads.insert(std::make_pair(pStraight->m_id, pStraight));
  m_map_elements.insert(std::make_pair(pStraight->m_id, pStraight));
}

void CMapScene::AddCurve(CCurveRoad* pCurve) {
  // LOG(INFO) << "CMapScene::AddStraight";
  SYSTEM_LOGGER_INFO("CMapScene::AddCurve");
  CCurveRoad* pRoad = FindCurve(pCurve->m_id);
  if (pRoad != nullptr) {
    SYSTEM_LOGGER_INFO("curve road %d has added!", pCurve->m_id);
    return;
  }

  m_curve_roads.push_back(pCurve);
  m_map_curve_roads.insert(std::make_pair(pCurve->m_id, pCurve));
  m_map_elements.insert(std::make_pair(pCurve->m_id, pCurve));
}

void CMapScene::AddCross(CCross* pCross) {
  // LOG(INFO) << "CMapScene::AddStraight";
  SYSTEM_LOGGER_INFO("CMapScene::AddCross");
  CCross* pRoad = FindCross(pCross->m_id);
  if (pRoad != nullptr) {
    SYSTEM_LOGGER_INFO("corss %d has added!", pCross->m_id);
    return;
  }

  m_crosses.push_back(pCross);
  m_map_crosses.insert(std::make_pair(pCross->m_id, pCross));
  m_map_elements.insert(std::make_pair(pCross->m_id, pCross));
}

CMapElement* CMapScene::FindElement(int elementID) {
  std::map<int, CMapElement*>::iterator itr = m_map_elements.find(elementID);
  if (itr == m_map_elements.end()) {
    return nullptr;
  }

  return itr->second;
}

CStraightRoad* CMapScene::FindStraight(int straightID) {
  std::map<int, CStraightRoad*>::iterator itr = m_map_straight_roads.find(straightID);
  if (itr == m_map_straight_roads.end()) {
    return nullptr;
  }

  return itr->second;
}

CCurveRoad* CMapScene::FindCurve(int curveID) {
  std::map<int, CCurveRoad*>::iterator itr = m_map_curve_roads.find(curveID);
  if (itr == m_map_curve_roads.end()) {
    return nullptr;
  }

  return itr->second;
}

CCross* CMapScene::FindCross(int crossID) {
  std::map<int, CCross*>::iterator itr = m_map_crosses.find(crossID);
  if (itr == m_map_crosses.end()) {
    return nullptr;
  }

  return itr->second;
}

int CMapScene::SaveToXml(const char* strSceneName) {
  // LOG(INFO) << "map scene save to xml";
  SYSTEM_LOGGER_INFO("map scene save to xml");

  if (!strSceneName) {
    return -1;
  }

  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemScene = new TiXmlElement("hadmap_scene");
  elemScene->SetAttribute("version", "1.0");

  TiXmlElement* elemSettings = new TiXmlElement("settings");
  {
    elemSettings->SetAttribute("lon", m_dRefLon);
    elemSettings->SetAttribute("lat", m_dRefLat);
    elemSettings->SetAttribute("alt", m_dRefAlt);
  }

  TiXmlElement* elemStraights = new TiXmlElement("straights");

  {
    boost::format vec3Fmt("%.3f, %.3f, %.3f");
    boost::format numFmt("%.3f");
    // boost::format matrix16Fmt("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%,
    // %16%");
    // LOG(INFO) << "CMapScene::SaveToXML()--m_straight_roads::size: " << m_straight_roads.size();
    std::vector<CStraightRoad*>::iterator itr = m_straight_roads.begin();
    for (; itr != m_straight_roads.end(); ++itr) {
      TiXmlElement* straight = new TiXmlElement("straight");
      straight->SetAttribute("id", (*itr)->m_id);
      straight->SetAttribute("name", (*itr)->m_name.c_str());
      straight->SetDoubleAttribute("height", (*itr)->m_height);
      straight->SetDoubleAttribute("length", (*itr)->m_length);
      straight->SetDoubleAttribute("width", (*itr)->m_width);
      straight->SetDoubleAttribute("maxSpeed", (*itr)->m_maxSpeed);
      straight->SetDoubleAttribute("laneWidth", (*itr)->m_widthOfLane);
      straight->SetAttribute("laneCount", (*itr)->m_numberOfLanes);
      straight->SetDoubleAttribute("roadThickness", (*itr)->m_roadThickness);
      straight->SetDoubleAttribute("roadFriction", (*itr)->m_roadFriction);
      straight->SetAttribute("roadLaneMark", (*itr)->m_roadLaneMark);

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_position[0] % (*itr)->m_position[1] % (*itr)->m_position[2];
      straight->SetAttribute("position", vec3Fmt.str().c_str());

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_rotation[0] % (*itr)->m_rotation[1] % (*itr)->m_rotation[2];
      straight->SetAttribute("rotation", vec3Fmt.str().c_str());

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_start[0] % (*itr)->m_start[1] % (*itr)->m_start[2];
      straight->SetAttribute("start", vec3Fmt.str().c_str());

      vec3Fmt % (*itr)->m_end[0] % (*itr)->m_end[1] % (*itr)->m_end[2];
      straight->SetAttribute("end", vec3Fmt.str().c_str());

      std::string strMat = "";
      for (int i = 0; i < 16; ++i) {
        numFmt.clear();
        numFmt % (*itr)->m_matrix[i];
        strMat.append(numFmt.str());
        if (i != 15) {
          strMat.append(", ");
        }
      }

      straight->SetAttribute("matrix", strMat.c_str());

      elemStraights->LinkEndChild(straight);
    }
  }

  TiXmlElement* elemCurves = new TiXmlElement("curves");
  {
    boost::format vec3Fmt("%.3f, %.3f, %.3f");
    boost::format numFmt("%.3f");
    // boost::format matrix16Fmt("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%,
    // %16%"); LOG(INFO) << "CMapScene::SaveToXML()--m_straight_roads::size: " << m_straight_roads.size();
    std::vector<CCurveRoad*>::iterator itr = m_curve_roads.begin();
    for (; itr != m_curve_roads.end(); ++itr) {
      TiXmlElement* curve = new TiXmlElement("curve");
      curve->SetAttribute("id", (*itr)->m_id);
      curve->SetAttribute("name", (*itr)->m_name.c_str());
      curve->SetDoubleAttribute("height", (*itr)->m_height);
      curve->SetDoubleAttribute("length", (*itr)->m_length);
      curve->SetDoubleAttribute("width", (*itr)->m_width);
      curve->SetDoubleAttribute("maxSpeed", (*itr)->m_maxSpeed);
      curve->SetDoubleAttribute("laneWidth", (*itr)->m_widthOfLane);
      curve->SetAttribute("laneCount", (*itr)->m_numberOfLanes);
      curve->SetDoubleAttribute("roadThickness", (*itr)->m_roadThickness);
      curve->SetDoubleAttribute("roadFriction", (*itr)->m_roadFriction);
      curve->SetAttribute("roadLaneMark", (*itr)->m_roadLaneMark);

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_position[0] % (*itr)->m_position[1] % (*itr)->m_position[2];
      curve->SetAttribute("position", vec3Fmt.str().c_str());

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_rotation[0] % (*itr)->m_rotation[1] % (*itr)->m_rotation[2];
      curve->SetAttribute("rotation", vec3Fmt.str().c_str());

      std::string strMat = "";
      for (int i = 0; i < 16; ++i) {
        numFmt.clear();
        numFmt % (*itr)->m_matrix[i];
        strMat.append(numFmt.str());
        if (i != 15) {
          strMat.append(", ");
        }
      }
      curve->SetAttribute("matrix", strMat.c_str());

      std::string strControlPoints = "";
      for (int i = 0; i < (*itr)->m_controlPoints.size(); ++i) {
        vec3Fmt.clear();
        vec3Fmt % (*itr)->m_controlPoints[i].X() % (*itr)->m_controlPoints[i].Y() % (*itr)->m_controlPoints[i].Z();
        strControlPoints.append(vec3Fmt.str());
        if (i != ((*itr)->m_controlPoints.size() - 1)) {
          strControlPoints.append(";");
        }
      }
      curve->SetAttribute("controlPoints", strControlPoints);

      elemCurves->LinkEndChild(curve);
    }
  }

  TiXmlElement* elemCrosses = new TiXmlElement("crosses");
  {
    boost::format vec3Fmt("%.3f, %.3f, %.3f");
    boost::format numFmt("%.3f");
    // boost::format matrix16Fmt("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%,
    // %16%"); LOG(INFO) << "CMapScene::SaveToXML()--m_straight_roads::size: " << m_straight_roads.size();
    std::vector<CCross*>::iterator itr = m_crosses.begin();
    for (; itr != m_crosses.end(); ++itr) {
      TiXmlElement* cross = new TiXmlElement("cross");
      cross->SetAttribute("id", (*itr)->m_id);
      cross->SetAttribute("name", (*itr)->m_name.c_str());
      cross->SetDoubleAttribute("height", (*itr)->m_height);
      cross->SetDoubleAttribute("length", (*itr)->m_length);
      cross->SetDoubleAttribute("width", (*itr)->m_width);
      cross->SetDoubleAttribute("maxSpeed", (*itr)->m_maxSpeed);
      /*cross->SetDoubleAttribute("laneWidth", (*itr)->m_widthOfLane);
      cross->SetAttribute("laneCount", (*itr)->m_numberOfLanes);
      cross->SetDoubleAttribute("roadThickness", (*itr)->m_roadThickness);
      cross->SetDoubleAttribute("roadFriction", (*itr)->m_roadFriction);
      cross->SetAttribute("roadLaneMark", (*itr)->m_roadLaneMark);*/

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_position[0] % (*itr)->m_position[1] % (*itr)->m_position[2];
      cross->SetAttribute("position", vec3Fmt.str().c_str());

      vec3Fmt.clear();
      vec3Fmt % (*itr)->m_rotation[0] % (*itr)->m_rotation[1] % (*itr)->m_rotation[2];
      cross->SetAttribute("rotation", vec3Fmt.str().c_str());

      /*vec3Fmt.clear();
      vec3Fmt % (*itr)->m_start[0] % (*itr)->m_start[1] % (*itr)->m_start[2];
      straight->SetAttribute("start", vec3Fmt.str().c_str());

      vec3Fmt % (*itr)->m_end[0] % (*itr)->m_end[1] % (*itr)->m_end[2];
      straight->SetAttribute("end", vec3Fmt.str().c_str());*/

      std::string strMat = "";
      for (int i = 0; i < 16; ++i) {
        numFmt.clear();
        numFmt % (*itr)->m_matrix[i];
        strMat.append(numFmt.str());
        if (i != 15) {
          strMat.append(", ");
        }
      }

      cross->SetAttribute("matrix", strMat.c_str());

      elemCrosses->LinkEndChild(cross);
    }
  }

  // groups
  TiXmlElement* elemGroups = new TiXmlElement("groups");
  {
    std::vector<CGroupEntity*>::iterator itr = m_groups.begin();
    for (; itr != m_groups.end(); ++itr) {
      TiXmlElement* group = new TiXmlElement("group");
      CGroupEntity::GroupChildren& child = (*itr)->Children();
      int nLen = child.size();
      if (nLen > 0) {
        boost::format numFmt("%.3f");
        std::string strChildren = "";
        for (int i = 0; i < nLen; ++i) {
          numFmt.clear();
          numFmt % child[i];
          strChildren.append(numFmt.str());
          if (i != (nLen - 1)) {
            strChildren.append(", ");
          }
        }

        group->SetAttribute("children", strChildren.c_str());
      }

      nLen = (*itr)->Links().size();
      if (nLen > 0) {
        for (int i = 0; i < nLen; ++i) {
          CLinkEntity* pLink = (*itr)->Links()[i];
          TiXmlElement* link = new TiXmlElement("link");

          link->SetAttribute("from_id", pLink->m_from_id);
          link->SetAttribute("from_end", pLink->m_from_end);
          link->SetAttribute("to_id", pLink->m_to_id);
          link->SetAttribute("to_end", pLink->m_to_end);

          group->LinkEndChild(link);
        }
      }
      elemGroups->LinkEndChild(group);
    }
  }

  elemScene->LinkEndChild(elemSettings);
  elemScene->LinkEndChild(elemStraights);
  elemScene->LinkEndChild(elemCrosses);
  elemScene->LinkEndChild(elemCurves);
  elemScene->LinkEndChild(elemGroups);

  doc.LinkEndChild(dec);
  doc.LinkEndChild(elemScene);

  // doc.SaveFile(strSceneName);

  // 生成文件名，并保存到相应目录
  // hadmap_scene/
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path initPath = strResourceDir;
  boost::filesystem::path hadmapScene = initPath;
  hadmapScene.append("hadmap_scene");
  boost::filesystem::path pSceneName = strSceneName;

  std::string strExt = pSceneName.extension().string();
  if (strExt.size() == 0 || !boost::algorithm::iequals(strExt, ".xml")) {
    std::string strScene = strSceneName;
    strScene.append(".xml");
    hadmapScene.append(strScene);
  }
  doc.SaveFile(hadmapScene.string().c_str());
  return 0;
}

int CMapScene::SaveToSqlite(const char* strSceneName) {
  // LOG(INFO) << "map scene save to sqlite";
  SYSTEM_LOGGER_INFO("map scene save to sqlite");

  // 生成文件名，并保存到相应目录
  // hadmap_editor/
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path initPath = strResourceDir;
  boost::filesystem::path hadmapEditor = initPath;
  hadmapEditor.append("hadmap_editor");
  boost::filesystem::path pSceneName = strSceneName;

  std::string strExt = pSceneName.extension().string();
  if (strExt.size() == 0 || !boost::algorithm::iequals(strExt, ".sqlite")) {
    std::string strScene = strSceneName;
    strScene.append(".sqlite");
    hadmapEditor.append(strScene);
  }

  if (boost::filesystem::exists(hadmapEditor)) {
    boost::filesystem::remove(hadmapEditor);
  }

  std::string strName = hadmapEditor.string();

  CMapOperator op;
  op.insertIntoSqlite(strName.c_str(), m_txroads, m_txLaneLinks);
  return CopyHadmap(strSceneName);
}

int CMapScene::CopyHadmap(const char* strFilePath) {
  std::string initPath = CEngineConfig::Instance().ResourceDir();

  boost::filesystem::path hadmapEditor = initPath;
  hadmapEditor.append("hadmap_editor");
  boost::filesystem::path pFilePath = strFilePath;
  std::string strFile = strFilePath;
  std::string strExt = pFilePath.extension().string();
  if (strExt.size() == 0 ||
      (!boost::algorithm::iequals(strExt, ".sqlite") && !boost::algorithm::iequals(strExt, ".xodr"))) {
    strFile = strFilePath;
    strFile.append(".sqlite");
  }
  hadmapEditor.append(strFile);

  boost::filesystem::path sourceFileDir = hadmapEditor;
  if (!boost::filesystem::exists(sourceFileDir)) {
    SYSTEM_LOGGER_ERROR("%s is not exist", sourceFileDir.string().c_str());
    assert(false);
    return -1;
  }

  boost::filesystem::path targetFile = initPath;
  targetFile.append("/hadmap/");
  targetFile.append(strFile);
  if (boost::filesystem::exists(targetFile)) {
    boost::filesystem::remove(targetFile);
  }
  boost::filesystem::copy_file(sourceFileDir, targetFile);
  // AddToConfigFile(targetFile.filename().string().c_str());
  AddToConfigFile(targetFile);
  return 0;
}

int CMapScene::AddToConfigFile(boost::filesystem::path& p) {
  // get hadmap parameter
  CMapFile mapfile;
  int nRet = mapfile.ParseMapV2(p.string().c_str(), false);
  if (nRet != 0) {
    SYSTEM_LOGGER_ERROR("open hadmap %s error", p.string().c_str());
    // return m_wstrMapFileFormatError.c_str();
  } else {
    std::string strName = p.filename().string();
    int nIndex = CEngineConfig::Instance().HadmapIndex(strName.c_str());
    // chanage attr 1
    tagHadmapAttr attr;
    attr.m_name = strName;
    attr.m_unRealLevelIndex = nIndex;
    attr.m_strUnrealLevelIndex = std::to_string(nIndex);
    attr.m_nPreset = 0;
    attr.m_strPreset = "0";
    attr.m_dRefLon = mapfile.m_center[0];
    attr.m_dRefLat = mapfile.m_center[1];
    attr.m_dRefAlt = mapfile.m_center[2];
    // attr.m_strRefAlt = std::to_string(attr.m_dRefAlt);
    // attr.m_strRefLon = std::to_string(attr.m_dRefLon);
    // attr.m_strRefLat = std::to_string(attr.m_dRefLat);
    std::string str;
    std::stringstream ss1, ss2, ss3;
    ss1 << std::fixed << std::setprecision(8) << attr.m_dRefAlt;
    attr.m_strRefAlt = ss1.str();

    ss2 << std::fixed << std::setprecision(8) << attr.m_dRefLon;
    attr.m_strRefLon = ss2.str();

    ss3 << std::fixed << std::setprecision(8) << attr.m_dRefLat;
    attr.m_strRefLat = ss3.str();
    CEngineConfig::Instance().UpdateHadmapAttr(strName, attr);
    std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(strName.c_str());
    CMapDataCache::Instance().RemoveHadmapCacheAll(wstrName.c_str());
  }
  return 0;
}

int CMapScene::AddToConfigFile(const char* strFileName) {
  tagHadmapAttr* pAttr = CEngineConfig::Instance().HadmapAttr(strFileName);
  if (pAttr) {
    /*
    pAttr->m_unRealLevelIndex = -1;
    pAttr->m_strUnrealLevelIndex = "-1";
    */
    pAttr->m_dRefAlt = m_dRefAlt;
    pAttr->m_dRefLon = m_dRefLon;
    pAttr->m_dRefLat = m_dRefLat;
    pAttr->m_strRefAlt = std::to_string(m_dRefAlt);
    pAttr->m_strRefLon = std::to_string(m_dRefLon);
    pAttr->m_strRefLat = std::to_string(m_dRefLat);
  } else {
    tagHadmapAttr attr;
    attr.m_name = strFileName;
    attr.m_unRealLevelIndex = 0;
    attr.m_strUnrealLevelIndex = "0";
    attr.m_nPreset = 0;
    attr.m_strPreset = "0";
    attr.m_dRefAlt = m_dRefAlt;
    attr.m_dRefLon = m_dRefLon;
    attr.m_dRefLat = m_dRefLat;
    // attr.m_strRefAlt = std::to_string(m_dRefAlt);
    // attr.m_strRefLon = std::to_string(m_dRefLon);
    // attr.m_strRefLat = std::to_string(m_dRefLat);
    std::string str;
    std::stringstream ss1, ss2, ss3;
    ss1 << std::fixed << std::setprecision(8) << attr.m_dRefAlt;
    attr.m_strRefAlt = ss1.str();

    ss2 << std::fixed << std::setprecision(8) << attr.m_dRefLon;
    attr.m_strRefLon = ss2.str();

    ss3 << std::fixed << std::setprecision(8) << attr.m_dRefLat;
    attr.m_strRefLat = ss3.str();
    attr.m_unRealLevelIndex = 6;
    CEngineConfig::Instance().HadmapAttrs().insert(std::make_pair(attr.m_name, attr));
    CEngineConfig::Instance().ComposeHadmapConfigStr();
    CEngineConfig::Instance().SaveToHadmapConfig();
  }

  return 0;
}

void ParseNumberArray(const char* p, double* arr, int arrLen) {
  std::vector<std::string> SplitVec;
  boost::algorithm::split(SplitVec, p, boost::algorithm::is_any_of(","), boost::algorithm::token_compress_on);
  if (SplitVec.size() != arrLen) {
    SYSTEM_LOGGER_ERROR("parse number array error!");
    assert(false);
  }
  for (int i = 0; i < SplitVec.size(); ++i) {
    arr[i] = atof(SplitVec[i].c_str());
  }
}

void ParsePoints(const char* p, std::vector<CVector3>& points) {
  std::vector<std::string> SplitVec;
  boost::algorithm::split(SplitVec, p, boost::algorithm::is_any_of(";"), boost::algorithm::token_compress_on);
  if (SplitVec.size() < 1) {
    SYSTEM_LOGGER_ERROR("parse curve control points error!");
    assert(false);
    return;
  }
  std::vector<std::string> pValueVec;
  for (int i = 0; i < SplitVec.size(); ++i) {
    boost::algorithm::split(pValueVec, SplitVec[i], boost::algorithm::is_any_of(","),
                            boost::algorithm::token_compress_on);

    CVector3 p;
    double dX = atof(pValueVec[0].c_str());
    double dY = atof(pValueVec[1].c_str());
    double dZ = atof(pValueVec[2].c_str());
    p.Set(dX, dY, dZ);
    points.push_back(p);
    pValueVec.clear();
  }
}

int CMapScene::ParseCrosses(TiXmlElement* elemCrosses, std::vector<CCross*>& crosses) {
  if (!elemCrosses) {
    return -1;
  }

  TiXmlElement* elemCross = elemCrosses->FirstChildElement("cross");
  while (elemCross) {
    CCross* pCross = new CCross();
    const char* p = elemCross->Attribute("id");
    if (p) {
      pCross->m_id = atoi(p);
    }

    p = elemCross->Attribute("name");
    if (p) {
      pCross->m_name = p;
    }

    p = elemCross->Attribute("height");
    if (p) {
      pCross->m_height = atof(p);
    }

    p = elemCross->Attribute("length");
    if (p) {
      pCross->m_length = atof(p);
    }

    p = elemCross->Attribute("width");
    if (p) {
      pCross->m_width = atof(p);
    }

    p = elemCross->Attribute("maxSpeed");
    if (p) {
      pCross->m_maxSpeed = atof(p);
    }

    /*p = elemCross->Attribute("laneWidth");
    if (p)
    {
            pCross->m_widthOfLane = atof(p);
    }

    p = elemCross->Attribute("laneCount");
    if (p)
    {
            pCross->m_numberOfLanes = atoi(p);
    }

    p = elemCross->Attribute("roadThickness");
    if (p)
    {
            pCross->m_roadThickness = atof(p);
    }

    p = elemCross->Attribute("roadFriction");
    if (p)
    {
            pCross->m_roadFriction = atof(p);
    }

    p = elemCross->Attribute("roadLaneMark");
    if (p)
    {
            pCross->m_roadLaneMark = atoi(p);
    }*/

    p = elemCross->Attribute("position");
    if (p) {
      ParseNumberArray(p, pCross->m_position, 3);
    }

    p = elemCross->Attribute("rotation");
    if (p) {
      ParseNumberArray(p, pCross->m_rotation, 3);
    }

    p = elemCross->Attribute("matrix");
    if (p) {
      ParseNumberArray(p, pCross->m_matrix, 16);
    }

    /*p = elemCross->Attribute("start");
    if (p)
    {
            ParseNumberArray(p, pCross->m_start, 3);
    }

    p = elemCross->Attribute("end");
    if (p)
    {
            ParseNumberArray(p, pCross->m_end, 3);
    }*/
    elemCross = elemCross->NextSiblingElement("cross");
    crosses.push_back(pCross);
  }

  return 0;
}

int CMapScene::ParseStraights(TiXmlElement* elemStraights, std::vector<CStraightRoad*>& straights) {
  if (!elemStraights) {
    return -1;
  }

  TiXmlElement* elemStraight = elemStraights->FirstChildElement("straight");
  while (elemStraight) {
    CStraightRoad* pStraight = new CStraightRoad();
    const char* p = elemStraight->Attribute("id");
    if (p) {
      pStraight->m_id = atoi(p);
    }

    p = elemStraight->Attribute("name");
    if (p) {
      pStraight->m_name = p;
    }

    p = elemStraight->Attribute("height");
    if (p) {
      pStraight->m_height = atof(p);
    }

    p = elemStraight->Attribute("length");
    if (p) {
      pStraight->m_length = atof(p);
    }

    p = elemStraight->Attribute("width");
    if (p) {
      pStraight->m_width = atof(p);
    }

    p = elemStraight->Attribute("maxSpeed");
    if (p) {
      pStraight->m_maxSpeed = atof(p);
    }

    p = elemStraight->Attribute("laneWidth");
    if (p) {
      pStraight->m_widthOfLane = atof(p);
    }

    p = elemStraight->Attribute("laneCount");
    if (p) {
      pStraight->m_numberOfLanes = atoi(p);
    }

    p = elemStraight->Attribute("roadThickness");
    if (p) {
      pStraight->m_roadThickness = atof(p);
    }

    p = elemStraight->Attribute("roadFriction");
    if (p) {
      pStraight->m_roadFriction = atof(p);
    }

    p = elemStraight->Attribute("roadLaneMark");
    if (p) {
      pStraight->m_roadLaneMark = atoi(p);
    }

    p = elemStraight->Attribute("position");
    if (p) {
      ParseNumberArray(p, pStraight->m_position, 3);
    }

    p = elemStraight->Attribute("rotation");
    if (p) {
      ParseNumberArray(p, pStraight->m_rotation, 3);
    }

    p = elemStraight->Attribute("matrix");
    if (p) {
      ParseNumberArray(p, pStraight->m_matrix, 16);
    }

    p = elemStraight->Attribute("start");
    if (p) {
      ParseNumberArray(p, pStraight->m_start, 3);
    }

    p = elemStraight->Attribute("end");
    if (p) {
      ParseNumberArray(p, pStraight->m_end, 3);
    }
    elemStraight = elemStraight->NextSiblingElement("straight");
    straights.push_back(pStraight);
  }

  return 0;
}

int CMapScene::ParseCurves(TiXmlElement* elemCurves, std::vector<CCurveRoad*>& curves) {
  if (!elemCurves) {
    return -1;
  }

  TiXmlElement* elemCurve = elemCurves->FirstChildElement("curve");
  while (elemCurve) {
    CCurveRoad* pCurve = new CCurveRoad();
    const char* p = elemCurve->Attribute("id");
    if (p) {
      pCurve->m_id = atoi(p);
    }

    p = elemCurve->Attribute("name");
    if (p) {
      pCurve->m_name = p;
    }

    p = elemCurve->Attribute("height");
    if (p) {
      pCurve->m_height = atof(p);
    }

    p = elemCurve->Attribute("length");
    if (p) {
      pCurve->m_length = atof(p);
    }

    p = elemCurve->Attribute("width");
    if (p) {
      pCurve->m_width = atof(p);
    }

    p = elemCurve->Attribute("maxSpeed");
    if (p) {
      pCurve->m_maxSpeed = atof(p);
    }

    p = elemCurve->Attribute("laneWidth");
    if (p) {
      pCurve->m_widthOfLane = atof(p);
    }

    p = elemCurve->Attribute("laneCount");
    if (p) {
      pCurve->m_numberOfLanes = atoi(p);
    }

    p = elemCurve->Attribute("roadThickness");
    if (p) {
      pCurve->m_roadThickness = atof(p);
    }

    p = elemCurve->Attribute("roadFriction");
    if (p) {
      pCurve->m_roadFriction = atof(p);
    }

    p = elemCurve->Attribute("roadLaneMark");
    if (p) {
      pCurve->m_roadLaneMark = atoi(p);
    }

    p = elemCurve->Attribute("position");
    if (p) {
      ParseNumberArray(p, pCurve->m_position, 3);
    }

    p = elemCurve->Attribute("rotation");
    if (p) {
      ParseNumberArray(p, pCurve->m_rotation, 3);
    }

    p = elemCurve->Attribute("matrix");
    if (p) {
      ParseNumberArray(p, pCurve->m_matrix, 16);
    }

    p = elemCurve->Attribute("controlPoints");
    if (p) {
      ParsePoints(p, pCurve->m_controlPoints);
    }

    elemCurve = elemCurve->NextSiblingElement("curve");
    curves.push_back(pCurve);
  }

  return 0;
}

int CMapScene::ParseGroups(TiXmlElement* elemGroups, std::vector<CGroupEntity*>& groups) {
  if (!elemGroups) {
    return -1;
  }

  TiXmlElement* elemGroup = elemGroups->FirstChildElement("group");
  while (elemGroup) {
    CGroupEntity* pGroup = new CGroupEntity();
    const char* p = elemGroup->Attribute("children");
    if (p) {
      std::vector<std::string> SplitVec;
      boost::algorithm::split(SplitVec, p, boost::algorithm::is_any_of(","), boost::algorithm::token_compress_on);
      for (int i = 0; i < SplitVec.size(); ++i) {
        pGroup->AddChild(atoi(SplitVec[i].c_str()));
      }
    }

    // TiXmlElement* elemLinks = elemGroup->FirstChildElement("links");
    {
      TiXmlElement* elemLink = elemGroup->FirstChildElement("link");
      while (elemLink) {
        CLinkEntity* pLink = new CLinkEntity();
        const char* p = elemLink->Attribute("from_id");
        if (p) {
          pLink->m_from_id = atoi(p);
        }
        p = elemLink->Attribute("from_end");
        if (p) {
          pLink->m_from_end = atoi(p);
        }
        p = elemLink->Attribute("to_id");
        if (p) {
          pLink->m_to_id = atoi(p);
        }
        p = elemLink->Attribute("to_end");
        if (p) {
          pLink->m_to_end = atoi(p);
        }
        elemLink = elemLink->NextSiblingElement("link");
        pGroup->AddLink(pLink);
      }
    }

    elemGroup = elemGroup->NextSiblingElement("group");
    groups.push_back(pGroup);
  }
  return 0;
}

int CMapScene::ParseFromXml(const char* strSceneName) {
  if (!strSceneName) {
    SYSTEM_LOGGER_ERROR("%s not exist", strSceneName);
    return -1;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strSceneName);

  if (!bRet) {
    SYSTEM_LOGGER_ERROR("xml format error!");
    return -1;
  }

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  // if (_stricmp(strName.c_str(), "simulation") != 0)
  if (!boost::algorithm::iequals(strName, "hadmap_scene")) return -1;

  // 解析地图参数数据
  TiXmlElement* elemSettings = xmlRoot->FirstChildElement("settings");
  {
    if (elemSettings) {
      const char* p = elemSettings->Attribute("lon");
      if (p) {
        m_dRefLon = atof(p);
      }

      p = elemSettings->Attribute("lat");
      if (p) {
        m_dRefLat = atof(p);
      }

      p = elemSettings->Attribute("alt");
      if (p) {
        m_dRefAlt = atof(p);
      }
    }
  }

  // 解析直路数据
  TiXmlElement* elemStraights = xmlRoot->FirstChildElement("straights");
  if (elemStraights) {
    int nRet = ParseStraights(elemStraights, m_straight_roads);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("parse element staights error!");
      return nRet;
    }
  }

  // 解析曲线数据
  TiXmlElement* elemCurves = xmlRoot->FirstChildElement("curves");
  if (elemCurves) {
    int nRet = ParseCurves(elemCurves, m_curve_roads);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("parse element curves error!");
      return nRet;
    }
  }

  // 解析路口数据
  TiXmlElement* elemCrosses = xmlRoot->FirstChildElement("crosses");
  if (elemCrosses) {
    int nRet = ParseCrosses(elemCrosses, m_crosses);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("parse element crosses error!");
      return nRet;
    }
  }

  // 解析分组数据
  TiXmlElement* elemGroups = xmlRoot->FirstChildElement("groups");
  if (elemGroups) {
    int nRet = ParseGroups(elemGroups, m_groups);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("parse element groups error!");
      return -1;
    }
  }
  return 0;
}

std::string CMapScene::ToJson() {
  std::string strRes = "";
  return strRes;
}
