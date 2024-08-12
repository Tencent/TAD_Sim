/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/
#include "config.h"

#include <cassert>
#include <iomanip>
#include <mutex>
#include <sstream>

#ifdef _WINDOWS
#  include <Psapi.h>
#  include <tchar.h>
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#ifndef UPGRADE_MAP_EDITOR
#  include "tx_od_plugin.h"
#endif

#include <json/value.h>
#include <json/writer.h>
#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/shared_ptr.hpp>
#include "common/log/system_logger.h"
#include "common/utils/misc.h"

CEngineConfig::CEngineConfig() {
  m_drawLane = true;
  m_drawLaneBoundary = true;
  m_drawRoad = false;
  m_drawLaneLink = true;
  m_drawMapObject = true;
  m_drawGrid = true;
  m_drawAabb = false;
  m_hadmapAttrsConfigStr = "";
  m_hadmapAttrsConfigWStr = L"";
}

CEngineConfig& CEngineConfig::Instance() {
  static CEngineConfig instance;
  return instance;
}

void CEngineConfig::Init(const char* strResourceDir, const char* strAppInstallDir,
                         const std::string& xodr_plugin_path) {
  // std::locale utf8LC = std::locale("utf-8");
  /*
  try{
          std::locale utf8LC = std::locale(".UTF8");
          std::locale oldLC = std::locale::global(utf8LC);

          SYSTEM_LOGGER_INFO("old locale is %s, now set to %s"
                  , oldLC.name().c_str(), utf8LC.name().c_str());
  }
  catch (std::exception& e) {
          SYSTEM_LOGGER_ERROR(e.what());
  }
  */
  /*
  std::locale lc;
  SYSTEM_LOGGER_INFO("current locale is %s",lc.name().c_str());

  char strA[] = u8"你";
  wchar_t strB[] = L"你";
  char strC[] = "你";
  char16_t strD[] = u"你";
  char32_t strE[] = U"你";

  size_t a = sizeof(strA);
  size_t b = sizeof(strB);
  size_t c = sizeof(strC);

  SYSTEM_LOGGER_INFO("a is %d, b is %d, c is %d !", a, b, c);

  SYSTEM_LOGGER_INFO("a is %s, b is %s, c is %s !"
          , strA, strB, strC);
  */
  SYSTEM_LOGGER_INFO("start engine config init!");
  std::string strResourceRootDir = strResourceDir;
  SYSTEM_LOGGER_INFO("resource dir is %s .", strResourceDir);
  if (boost::algorithm::iequals(".", strResourceDir)) {
    strResourceRootDir = boost::filesystem::initial_path().string();
  }

  m_strResourceDir = strResourceRootDir;
  m_strAppInstallDir = strAppInstallDir;
  // catalog
  m_strDefaultXosc = strResourceRootDir + "/Catalogs/Vehicles/default.xosc";
  m_strSensorData = strResourceRootDir + "/Catalogs/Sensors/SensorCatalog.xosc";
  m_strDynamicDir = strResourceRootDir + "/Catalogs/Dynamics/";
  m_strVehicleXosc = strResourceRootDir + "/Catalogs/Vehicles/VehicleCatalog.xosc";
  m_strPedestriantXosc = strResourceRootDir + "/Catalogs/Pedestrians/PedestrianCatalog.xosc";
  m_strMiscobjecttXosc = strResourceRootDir + "/Catalogs/MiscObjects/MiscObjectCatalog.xosc";
  m_strConf = strResourceRootDir + "/Catalogs/tadsim_setting.conf";

  GetProcessFilePath(m_strProcessImageDir);
  SYSTEM_LOGGER_INFO("app install dir is %s .", strAppInstallDir);
  SYSTEM_LOGGER_INFO("image dir is %s .", m_strProcessImageDir.c_str());

  LoadHadmapConfig();
  LoadConfig();

  boost::filesystem::path hadmapSceneDir = strResourceRootDir;
  hadmapSceneDir.append("/hadmap_scene");
  if (!boost::filesystem::exists(hadmapSceneDir) || !boost::filesystem::is_directory(hadmapSceneDir)) {
    boost::filesystem::create_directory(hadmapSceneDir);
  }
  /*
  boost::filesystem::path hadmapGeometryDir = strResourceRootDir;
  hadmapSceneDir.append("/hadmap_geometry");
  if (!boost::filesystem::exists(hadmapGeometryDir) || !boost::filesystem::is_directory(hadmapGeometryDir)) {
    boost::filesystem::create_directory(hadmapGeometryDir);
  }
  */

  boost::filesystem::path hadmapEditorDir = strResourceRootDir;
  hadmapEditorDir.append("/hadmap_editor");
  if (!boost::filesystem::exists(hadmapEditorDir) || !boost::filesystem::is_directory(hadmapEditorDir)) {
    boost::filesystem::create_directory(hadmapEditorDir);
  }

  boost::filesystem::path sceneDirPath = strResourceRootDir;
  sceneDirPath.append("/scene/");
  boost::filesystem::create_directory(sceneDirPath);
  m_strSceneDir = sceneDirPath.string();

  boost::filesystem::path hadmapDirPath = strResourceRootDir;
  hadmapDirPath.append("/hadmap/");
  boost::filesystem::create_directory(hadmapDirPath);
  m_strHadmapDir = hadmapDirPath.string();

  boost::filesystem::path gisImageDirPath = strResourceRootDir;
  gisImageDirPath.append("/gis_images/");
  if (!boost::filesystem::exists(gisImageDirPath) || !boost::filesystem::is_directory(gisImageDirPath)) {
    boost::filesystem::create_directory(gisImageDirPath);
  }
  m_strGISImageDir = gisImageDirPath.string();

  boost::filesystem::path gisModelDirPath = strResourceRootDir;
  gisModelDirPath.append("/gis_models/");
  if (!boost::filesystem::exists(gisModelDirPath) || !boost::filesystem::is_directory(gisModelDirPath)) {
    boost::filesystem::create_directory(gisModelDirPath);
  }
  m_strGISModelDir = gisModelDirPath.string();

  boost::filesystem::path sensorsDirPath = strResourceRootDir;
  sensorsDirPath.append("/sensors/");
  if (!boost::filesystem::exists(sensorsDirPath) || !boost::filesystem::is_directory(sensorsDirPath)) {
    boost::filesystem::create_directory(sensorsDirPath);
  }
  m_strSensorsDir = sensorsDirPath.string();

  boost::filesystem::path gradingTemplatePath = strResourceDir;
  gradingTemplatePath /= "grading.xml";
  m_strGradingTemplateFile = gradingTemplatePath.string();

  m_envVar.ParseFromEnv();
  initialized = true;
  std::cout << "host is: " << m_envVar.m_strHost << std::endl;
  /*
  boost::filesystem::path xodrPluginPath = boost::filesystem::current_path() /
  boost::filesystem::path("service/txSimService/OpenDrivePlugin"); SYSTEM_LOGGER_INFO("Load plugin from %s",
  xodrPluginPath.string().c_str());

  typedef boost::shared_ptr<txOpenDrivePlugin>(PluginCreate)();
  boost::function<PluginCreate> pluginCreate;
  try {
          pluginCreate = boost::dll::import_alias<PluginCreate>(xodrPluginPath, "create_plugin"
                  , boost::dll::load_mode::append_decorations);
          m_xodrPlugin = pluginCreate();
  } catch (boost::system::system_error& err) {

          SYSTEM_LOGGER_ERROR("Cannot load plugin from ", xodrPluginPath.string().c_str());
          SYSTEM_LOGGER_ERROR("load plugin error: ", err.what());
  }
  */
}

bool CEngineConfig::Initialize(const std::string& resource_dir, const std::string& xodr_plugin_path) {
  m_strResourceDir = resource_dir;
  if (boost::algorithm::iequals(".", m_strResourceDir)) {
    m_strResourceDir = boost::filesystem::initial_path().string();
  }

  m_envVar.ParseFromEnv();
  SYSTEM_LOGGER_INFO("cos env host is: %s", m_envVar.m_strHost.c_str());
  return true;
}

void CEngineConfig::SetDrawLane(bool bDraw) { m_drawLane = bDraw; }

void CEngineConfig::SetDrawRoad(bool bDraw) { m_drawRoad = bDraw; }

void CEngineConfig::SetDrawLaneBoundary(bool bDraw) { m_drawLaneBoundary = bDraw; }

void CEngineConfig::SetDrawLaneLink(bool bDraw) { m_drawLaneLink = bDraw; }

void CEngineConfig::SetDrawMapObject(bool bDraw) { m_drawMapObject = bDraw; }

void CEngineConfig::SetDrawAabb(bool bDraw) { m_drawAabb = bDraw; }

void CEngineConfig::SetDrawGrid(bool bDraw) { m_drawGrid = bDraw; }

void CEngineConfig::SetDrawRoadMesh(bool bDraw) { m_drawRoadMesh = bDraw; }

int CEngineConfig::UpdateHadmapAttr(std::string strHadmapName, tagHadmapAttr& attr) {
  HadmapAttrMap::iterator itr = m_hadmapAttrs.find(strHadmapName);
  if (itr != m_hadmapAttrs.end()) {
    itr->second = attr;
  } else {
    m_hadmapAttrs.insert(std::make_pair(attr.m_name, attr));
  }

  ComposeHadmapConfigStr();
  return SaveToHadmapConfig();
}

void CEngineConfig::RemoveHadmapAttr(std::string strHadmapName) {
  HadmapAttrMap::iterator itr = m_hadmapAttrs.find(strHadmapName);
  if (itr != m_hadmapAttrs.end()) {
    m_hadmapAttrs.erase(strHadmapName);
  }
}

tagHadmapAttr* CEngineConfig::HadmapAttr(std::string strHadmapName) {
  strHadmapName = strHadmapName.substr(strHadmapName.find(VERSION_NAME_DELIM) + 1);
  HadmapAttrMap::iterator itr = m_hadmapAttrs.find(strHadmapName);
  if (itr != m_hadmapAttrs.end()) {
    return (&itr->second);
  }

  return NULL;
}

int CEngineConfig::LoadHadmapConfig() {
  // std::string strConfigPath = ImageFileDir();
  std::string strConfigPath = ResourceDir();
  // std::string strConfigPath = "";
  strConfigPath.append("/hadmap_config.xml");

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strConfigPath);

  if (!bRet) return -1;

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  // if (_stricmp(strName.c_str(), "config") != 0)
  if (!boost::algorithm::iequals(strName, "config")) return -1;

  m_hadmapAttrs.clear();
  TiXmlElement* elemHadmap = xmlRoot->FirstChildElement("hadmap");

  while (elemHadmap) {
    tagHadmapAttr attr;
    const char* p = elemHadmap->Attribute("lon");
    if (p) {
      attr.m_strRefLon = p;
      attr.m_dRefLon = atof(p);
    }

    p = elemHadmap->Attribute("lat");
    if (p) {
      attr.m_strRefLat = p;
      attr.m_dRefLat = atof(p);
    }

    p = elemHadmap->Attribute("alt");
    if (p) {
      attr.m_strRefAlt = p;
      attr.m_dRefAlt = atof(p);
    }

    p = elemHadmap->Attribute("unrealLevelIndex");
    if (p) {
      attr.m_strUnrealLevelIndex = p;
      attr.m_unRealLevelIndex = atoi(p);
    }

    p = elemHadmap->Attribute("preset");
    if (p) {
      attr.m_strPreset = p;
      attr.m_nPreset = atoi(p);
    }

    p = elemHadmap->Attribute("templateName");
    if (p) {
      attr.m_strTemplateName = p;
    }

    p = elemHadmap->GetText();
    if (p) {
      attr.m_name = p;
    }

    HadmapAttrMap::iterator itr = m_hadmapAttrs.find(p);
    if (itr != m_hadmapAttrs.end()) {
      SYSTEM_LOGGER_ERROR("hadmap config file exist repeat hadmap name items");
      assert(false);
      continue;
    }

    m_hadmapAttrs.insert(std::make_pair(attr.m_name, attr));

    elemHadmap = elemHadmap->NextSiblingElement("hadmap");
  }

  // 生成hadmap熟悉字符串
  ComposeHadmapConfigStr();

  return 0;
}

int CEngineConfig::SaveToHadmapConfig() {
  std::string strConfigPath = ResourceDir();
  // std::string strConfigPath = "";
  strConfigPath.append("/hadmap_config.xml");

  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemConfig = new TiXmlElement("config");

  std::map<std::string, tagHadmapAttr>::iterator itr = m_hadmapAttrs.begin();
  for (; itr != m_hadmapAttrs.end(); ++itr) {
    TiXmlElement* elemHadmap = new TiXmlElement("hadmap");

    elemHadmap->SetAttribute("lon", itr->second.m_strRefLon);
    elemHadmap->SetAttribute("lat", itr->second.m_strRefLat);
    elemHadmap->SetAttribute("alt", itr->second.m_strRefAlt);
    elemHadmap->SetAttribute("unrealLevelIndex", itr->second.m_strUnrealLevelIndex);
    elemHadmap->SetAttribute("preset", itr->second.m_strPreset);
    TiXmlText* txtHadmap = new TiXmlText(itr->second.m_name);

    elemHadmap->LinkEndChild(txtHadmap);

    elemConfig->LinkEndChild(elemHadmap);
  }

  doc.LinkEndChild(dec);
  doc.LinkEndChild(elemConfig);

  doc.SaveFile(strConfigPath.c_str());
  return 0;
}

int CEngineConfig::SaveHadmapConfig(std::string strHadmapPath, std::vector<tagHadmapAttr>& haVec) {
  if (!strHadmapPath.size()) return -1;
  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemConfig = new TiXmlElement("config");

  std::vector<tagHadmapAttr>::iterator itr = haVec.begin();
  for (; itr != haVec.end(); ++itr) {
    TiXmlElement* elemHadmap = new TiXmlElement("hadmap");

    elemHadmap->SetAttribute("lon", itr->m_strRefLon);
    elemHadmap->SetAttribute("lat", itr->m_strRefLat);
    elemHadmap->SetAttribute("alt", itr->m_strRefAlt);
    elemHadmap->SetAttribute("preset", itr->m_strPreset);
    TiXmlText* txtHadmap = new TiXmlText(itr->m_name);

    elemHadmap->LinkEndChild(txtHadmap);

    elemConfig->LinkEndChild(elemHadmap);
  }

  doc.LinkEndChild(dec);
  doc.LinkEndChild(elemConfig);

  doc.SaveFile(strHadmapPath);
  return 0;
}

int CEngineConfig::ComposeHadmapConfigStr() {
  Json::Value root;

  if (m_hadmapAttrs.size() > 0) {
    root["code"] = 0;
    Json::Value maps;
    int nCount = 0;
    HadmapAttrMap::iterator itr = m_hadmapAttrs.begin();
    for (; itr != m_hadmapAttrs.end(); ++itr) {
      Json::Value item;
      item["id"] = nCount++;
      item["name"] = itr->second.m_name;
      item["reflon"] = itr->second.m_dRefLon;
      item["reflat"] = itr->second.m_strRefLat;
      item["refalt"] = itr->second.m_strRefAlt;
      item["unreallevelindex"] = itr->second.m_strUnrealLevelIndex;
      item["preset"] = itr->second.m_strPreset;
      maps.append(item);
    }

    root["maps"] = maps;
    root["message"] = "ok";
  } else {
    root["code"] = -1;
    root["message"] = "server error: no hadmap data";
  }

  m_hadmapAttrsConfigStr = root.toStyledString();
  m_hadmapAttrsConfigWStr = MBStr2WStr(m_hadmapAttrsConfigStr.c_str());

  return 0;
}

int CEngineConfig::ComposeConfigStr() {
  Json::Value root;

  Json::Value player;
  player["lanemarkenable"] = m_config.playerConfig.laneMarkEnable;
  player["forwardlength"] = m_config.playerConfig.forwardLength;
  player["forwardinterval"] = m_config.playerConfig.forwardInterval;
  player["messageformat"] = m_config.playerConfig.messageFormat;

  root["player"] = player;

  Json::Value editor;

  // control path
  editor["controlPathEnable"] = m_config.editorConfig.controlPathEnable;

  // template
  Json::Value tpl;
  // template current setting
  tpl["current"] = m_config.editorConfig.currentTemplate;
  // template items
  Json::Value tplItems;
  std::vector<tagTemplateItem>::iterator itr = m_config.editorConfig.templates.begin();
  for (; itr != m_config.editorConfig.templates.end(); ++itr) {
    Json::Value item;
    item["map"] = itr->strMapName;
    item["tpl"] = itr->strTemplate;
    tplItems.append(item);
  }
  tpl["items"] = tplItems;

  editor["template"] = tpl;

  // map indexes
  Json::Value mapIndexes;

  // hadmap index items
  Json::Value hadmapIndexItems;
  std::vector<tagHadmapIndex>::iterator it = m_config.editorConfig.hadmapIndeies.begin();
  for (; it != m_config.editorConfig.hadmapIndeies.end(); ++it) {
    Json::Value item;
    item["map"] = it->strMapName;
    item["index"] = it->nIndex;
    hadmapIndexItems.append(item);
  }

  editor["mapindex"] = mapIndexes;

  root["editor"] = editor;

  m_configStr = root.toStyledString();
  m_configWStr = MBStr2WStr(m_configStr.c_str());

  return 0;
}

int CEngineConfig::HadmapIndex(const char* strMapName) {
  std::vector<tagHadmapIndex>::iterator itr = m_config.editorConfig.hadmapIndeies.begin();
  for (; itr != m_config.editorConfig.hadmapIndeies.end(); ++itr) {
    if (boost::iequals(strMapName, itr->strMapName)) {
      return itr->nIndex;
    }
  }

  return 0;
}

int CEngineConfig::parseL3StateMachine(TiXmlElement* elemL3StateMachine) {
  if (!elemL3StateMachine) {
    return -1;
  }

  m_l3StateMachine.stateTypes.clear();

  TiXmlElement* elemType = elemL3StateMachine->FirstChildElement("type");
  while (elemType) {
    tagL3StateType stateType;
    const char* p = elemType->Attribute("name");
    if (p) {
      stateType.strStateTypeName = p;
    }

    TiXmlElement* elemState = elemType->FirstChildElement("state");
    while (elemState) {
      tagL3State state;
      const char* p = elemState->Attribute("name");
      if (p) {
        state.strStateName = p;
      }
      stateType.states.push_back(state);
      elemState = elemState->NextSiblingElement("state");
    }

    m_l3StateMachine.stateTypes.push_back(stateType);
    elemType = elemType->NextSiblingElement("type");
  }

  return 0;
}

int CEngineConfig::ComposeL3StateMachineStr() {
  Json::Value root;

  Json::Value l3statemachine;
  for (int i = 0; i < m_l3StateMachine.stateTypes.size(); ++i) {
    Json::Value states;
    for (int j = 0; j < m_l3StateMachine.stateTypes[i].states.size(); ++j) {
      states.append(m_l3StateMachine.stateTypes[i].states[j].strStateName);
    }
    l3statemachine[m_l3StateMachine.stateTypes[i].strStateTypeName] = states;
  }

  root["l3statemachine"] = l3statemachine;

  m_l3StateMachineStr = root.toStyledString();
  m_l3StateMachineWStr = MBStr2WStr(m_l3StateMachineStr.c_str());

  return 0;
}

int CEngineConfig::LoadConfig() {
  // std::string strConfigPath = ImageFileDir();
  std::string strConfigPath = ResourceDir();
  // std::string strConfigPath = "";
  strConfigPath.append("/config.xml");

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strConfigPath);

  if (!bRet) return -1;

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  if (!boost::algorithm::iequals(strName, "config")) return -1;

  TiXmlElement* elemPlayer = xmlRoot->FirstChildElement("player");

  // player config
  if (elemPlayer) {
    // lane mark
    TiXmlElement* elemLaneMark = elemPlayer->FirstChildElement("lanemark");
    if (elemLaneMark) {
      const char* p = elemLaneMark->Attribute("enable");
      if (p) {
        m_config.playerConfig.laneMarkEnable = atoi(p);
      }

      p = elemLaneMark->Attribute("forwardlength");
      if (p) {
        m_config.playerConfig.forwardLength = atof(p);
      }

      p = elemLaneMark->Attribute("forwardinterval");
      if (p) {
        m_config.playerConfig.forwardInterval = atof(p);
      }
    }

    // message format
    TiXmlElement* elemMessageFormat = elemPlayer->FirstChildElement("messagefromat");
    if (elemMessageFormat) {
      const char* p = elemMessageFormat->Attribute("format");
      if (p) {
        m_config.playerConfig.messageFormat = atoi(p);
      }
    }
  }

  // editor config
  TiXmlElement* elemEditor = xmlRoot->FirstChildElement("editor");
  if (elemEditor) {
    // openscenario
    TiXmlElement* elemSaveXOSC = elemEditor->FirstChildElement("saveXOSC");
    if (elemSaveXOSC) {
      const char* p = elemSaveXOSC->Attribute("enable");
      if (p) {
        m_config.editorConfig.saveXOSCEnable = atoi(p);
      }
    }

    // enable Control path function
    TiXmlElement* elemEnableControlPath = elemEditor->FirstChildElement("controlPath");
    if (elemEnableControlPath) {
      const char* p = elemEnableControlPath->Attribute("enable");
      if (p) {
        m_config.editorConfig.controlPathEnable = atoi(p);
      }
    }

    // template setting
    TiXmlElement* elemTemplateSetting = elemEditor->FirstChildElement("templateSetting");
    if (elemTemplateSetting) {
      const char* p = elemTemplateSetting->Attribute("current");
      if (p) {
        m_config.editorConfig.currentTemplate = p;
      }

      TiXmlElement* elemTemplateItem = elemTemplateSetting->FirstChildElement("template");
      while (elemTemplateItem) {
        tagTemplateItem item;
        p = elemTemplateItem->Attribute("map");
        if (p) {
          item.strMapName = p;
        }
        p = elemTemplateItem->Attribute("tpl");
        if (p) {
          item.strTemplate = p;
        }

        m_config.editorConfig.templates.push_back(item);

        elemTemplateItem = elemTemplateItem->NextSiblingElement("template");
      }
    }

    // hadmap index setting
    TiXmlElement* elemHadmapIndexSetting = elemEditor->FirstChildElement("hadmapIndexSetting");
    if (elemHadmapIndexSetting) {
      TiXmlElement* elemHadmapIndexItem = elemHadmapIndexSetting->FirstChildElement("map");
      while (elemHadmapIndexItem) {
        tagHadmapIndex item;
        const char* p = elemHadmapIndexItem->Attribute("name");
        if (p) {
          item.strMapName = p;
        }
        p = elemHadmapIndexItem->Attribute("id");
        if (p) {
          item.nIndex = atoi(p);
        }

        m_config.editorConfig.hadmapIndeies.push_back(item);

        elemHadmapIndexItem = elemHadmapIndexItem->NextSiblingElement("map");
      }
    }
  }

  ComposeConfigStr();

  TiXmlElement* elemL3StateMachine = xmlRoot->FirstChildElement("l3_state_machine");

  if (elemL3StateMachine) {
    int ret = parseL3StateMachine(elemL3StateMachine);
    if (ret != 0) {
      return -1;
    }

    ComposeL3StateMachineStr();
  }
  return 0;
}

#ifdef _WINDOWS
bool CEngineConfig::GetProcessFilePath(std::string& strPath) {
  HANDLE hProcess = GetCurrentProcess();

  strPath = "";

  std::wstring szFilePath = _T("");

  TCHAR tsFileDosPath[MAX_PATH + 1];
  ZeroMemory(tsFileDosPath, sizeof(TCHAR) * (MAX_PATH + 1));
  if (0 == GetProcessImageFileName(hProcess, tsFileDosPath, MAX_PATH + 1)) {
    return false;
  }

  // 获取Logic Drive String长度
  UINT uiLen = GetLogicalDriveStrings(0, NULL);
  if (0 == uiLen) {
    return false;
  }

  PTSTR pLogicDriveString = new TCHAR[uiLen + 1];
  ZeroMemory(pLogicDriveString, uiLen + 1);
  uiLen = GetLogicalDriveStrings(uiLen, pLogicDriveString);
  if (0 == uiLen) {
    delete[] pLogicDriveString;
    return false;
  }

  TCHAR szDrive[3] = TEXT(" :");
  PTSTR pDosDriveName = new TCHAR[MAX_PATH];
  PTSTR pLogicIndex = pLogicDriveString;

  do {
    szDrive[0] = *pLogicIndex;
    uiLen = QueryDosDevice(szDrive, pDosDriveName, MAX_PATH);
    if (0 == uiLen) {
      if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
        break;
      }

      delete[] pDosDriveName;
      pDosDriveName = new TCHAR[uiLen + 1];
      uiLen = QueryDosDevice(szDrive, pDosDriveName, uiLen + 1);
      if (0 == uiLen) {
        break;
      }
    }

    uiLen = _tcslen(pDosDriveName);
    if (0 == _tcsnicmp(tsFileDosPath, pDosDriveName, uiLen)) {
      wchar_t buf[1024];
      swprintf(buf, 1024, L"%s%s", szDrive, tsFileDosPath + uiLen);
      wchar_t* pstr = buf;
      szFilePath = std::wstring(pstr);

      break;
    }

    while (*pLogicIndex++) {
    }
  } while (*pLogicIndex) {
  }

  delete[] pLogicDriveString;
  delete[] pDosDriveName;

  strPath = WStr2MBStr(szFilePath.c_str());

  boost::filesystem::path p = strPath;
  strPath = p.parent_path().string();

  return true;
}
#else
bool CEngineConfig::GetProcessFilePath(std::string& strPath) {
  try {
    char link[100];
    char path[100];

    strPath = "";
    sprintf(link, "/proc/%d/exe", getpid());
    int i = readlink(link, path, sizeof(path));
    path[i] = '\0';

    boost::filesystem::path p = path;
    boost::filesystem::path pP = p.parent_path();
    // strPath = "/home/hughhwang/workspace/hadmap_server/src";
    strPath = pP.string();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("GetProcessFilePath! %s", e->what());
  }

  return true;
}
#endif
std::string CEngineConfig::WStr2MBStr(const wchar_t* wstr) {
  std::string strRes = "";
  try {
    strRes = boost::locale::conv::utf_to_utf<char>(wstr);
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("WStr2MBStr error: %s ", e.what());
  }

  return strRes;

  setlocale(LC_ALL, "");
  int nLen = wcstombs(nullptr, wstr, 0);
  char* pStrTmp = new char[nLen + 1];
  memset(pStrTmp, 0, nLen + 1);
  int nLen2 = wcstombs(pStrTmp, wstr, nLen);
  std::string strMB = pStrTmp;
  delete[] pStrTmp;

  return strMB;
}

std::mutex s_mutex;
std::wstring CEngineConfig::MBStr2WStr(const char* str) {
  std::unique_lock<std::mutex> ul(s_mutex);

  std::wstring wstr = L"";
  try {
    // wstr = boost::locale::conv::to_utf<wchar_t>(str, "gbk");
    wstr = boost::locale::conv::to_utf<wchar_t>(str, "utf8");
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("MBStr2WStr! %s", e->what());
  }

  return wstr;
  // SYSTEM_LOGGER_INFO("MBStr2WStr start ");
  int nLen = mbstowcs(nullptr, str, 0);
  // SYSTEM_LOGGER_INFO("MBStr2WStr 1 %d", nLen);
  wchar_t* pWstrTmp = new wchar_t[nLen + 1];
  wmemset(pWstrTmp, 0, nLen + 1);
  // SYSTEM_LOGGER_INFO("MBStr2WStr 2 %p ", pWstrTmp);
  int nLen2 = mbstowcs(pWstrTmp, str, nLen);
  std::wstring strW = pWstrTmp;
  // SYSTEM_LOGGER_INFO("MBStr2WStr 3 ");
  delete[] pWstrTmp;

  // SYSTEM_LOGGER_INFO("MBStr2WStr end ");
  return strW;
}

int CEngineConfig::SaveToXodr(const char* strName, std::string& strJson) {
  /*
  int nRet = m_xodrPlugin->save_opendrive_from_json_string(strName, strJson);
  if (nRet != 1)
  {
          return -1;
  }
  return 0;
  */
#ifndef UPGRADE_MAP_EDITOR
  boost::filesystem::path appInstallPath = AppInstallDir();
#  ifdef _WIN32
  boost::filesystem::path xodrPluginPath =
      appInstallPath / boost::filesystem::path("service/txSimService/OpenDrivePlugin");
#  else
  boost::filesystem::path xodrPluginPath = appInstallPath / boost::filesystem::path("service/simdeps/OpenDrivePlugin");
#  endif

  SYSTEM_LOGGER_INFO("Load plugin from %s", xodrPluginPath.string().c_str());

  typedef boost::shared_ptr<txOpenDrivePlugin>(PluginCreate)();
  boost::function<PluginCreate> pluginCreate;
  int nRet = 0;
  try {
    pluginCreate = boost::dll::import_alias<PluginCreate>(xodrPluginPath, "create_plugin",
                                                          boost::dll::load_mode::append_decorations);
    SYSTEM_LOGGER_INFO("Load plugin from %s succeed!", xodrPluginPath.string().c_str());
    auto xodrPlugin = pluginCreate();
    SYSTEM_LOGGER_INFO("start save open drive %s!", strName);
    nRet = xodrPlugin->save_opendrive_from_json_string(strName, strJson);
  } catch (boost::system::system_error& err) {
    SYSTEM_LOGGER_ERROR("Cannot load plugin from ", xodrPluginPath.string().c_str());
    SYSTEM_LOGGER_ERROR("load plugin error: ", err.what());
  }

  if (nRet != 1) {
    SYSTEM_LOGGER_INFO("save hadmap %s failed", strName);
    return -1;
  }
  SYSTEM_LOGGER_INFO("save hadmap %s success", strName);
#endif
  return 0;
}

int CEngineConfig::ParseFromXodr(const char* strName, std::string& strJson) {
  /*
  int nRet = m_xodrPlugin->parse_opendrive_to_json(strName, strJson);
  if (nRet != 1)
  {
          return -1;
  }
  return 0;
  */
#ifndef UPGRADE_MAP_EDITOR
  boost::filesystem::path appInstallPath = AppInstallDir();
#  ifdef _WIN32
  boost::filesystem::path xodrPluginPath =
      appInstallPath / boost::filesystem::path("service/txSimService/OpenDrivePlugin");
#  else
  boost::filesystem::path xodrPluginPath = appInstallPath / boost::filesystem::path("service/simdeps/OpenDrivePlugin");
#  endif
  SYSTEM_LOGGER_INFO("Load plugin from %s", xodrPluginPath.string().c_str());

  typedef boost::shared_ptr<txOpenDrivePlugin>(PluginCreate)();
  boost::function<PluginCreate> pluginCreate;
  try {
    pluginCreate = boost::dll::import_alias<PluginCreate>(xodrPluginPath, "create_plugin",
                                                          boost::dll::load_mode::append_decorations);
    SYSTEM_LOGGER_INFO("Load plugin from %s succeed!", xodrPluginPath.string().c_str());
  } catch (boost::system::system_error& err) {
    SYSTEM_LOGGER_ERROR("Cannot load plugin from ", xodrPluginPath.string().c_str());
    SYSTEM_LOGGER_ERROR("load plugin error: ", err.what());
  }

  auto xodrPlugin = pluginCreate();
  SYSTEM_LOGGER_INFO("start load open drive %s!", strName);
  int nRet = xodrPlugin->parse_opendrive_to_json(strName, strJson);
  if (nRet != 1) {
    SYSTEM_LOGGER_INFO("parse hadmap %s failed", strName);
    return -1;
  }
  SYSTEM_LOGGER_INFO("parse hadmap %s success", strName);
#endif
  return 0;
}
