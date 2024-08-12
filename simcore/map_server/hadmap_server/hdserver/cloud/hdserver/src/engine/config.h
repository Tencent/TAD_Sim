/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/
#pragma once

#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include "var/env.h"

struct tagConfigPlayer {
  int laneMarkEnable;
  float forwardLength;
  float forwardInterval;
  int messageFormat;  // 0 ÄÚ²¿¸ñÊ½£»1 OSI¸ñÊ½£»
  tagConfigPlayer() {
    laneMarkEnable = 0;
    forwardLength = 101;
    forwardInterval = 10;
    messageFormat = 0;
  }
};

struct tagTemplateItem {
  std::string strMapName;
  std::string strTemplate;
};

struct tagHadmapIndex {
  std::string strMapName;
  int nIndex;
};

struct tagConfigEditor {
  int saveXOSCEnable;
  int controlPathEnable;
  std::string currentTemplate;
  std::vector<tagTemplateItem> templates;
  std::vector<tagHadmapIndex> hadmapIndeies;

  tagConfigEditor() {
    saveXOSCEnable = 0;
    controlPathEnable = 0;
    currentTemplate = "";
  }
};

struct tagL3State {
  std::string strStateName;
};

struct tagL3StateType {
  std::string strStateTypeName;
  std::vector<tagL3State> states;
};

struct tagL3StateMachine {
  std::vector<tagL3StateType> stateTypes;
};

struct tagConfiguration {
  tagConfigPlayer playerConfig;
  tagConfigEditor editorConfig;
};

struct tagHadmapAttr {
  std::string m_name;
  double m_dRefLon;
  double m_dRefLat;
  double m_dRefAlt;
  int m_nPreset;
  int m_unRealLevelIndex;
  std::string m_strRefLon;
  std::string m_strRefLat;
  std::string m_strRefAlt;
  std::string m_strUnrealLevelIndex;
  std::string m_strPreset;
  std::string m_strTemplateName;
};

typedef std::map<std::string, tagHadmapAttr> HadmapAttrMap;

class TiXmlElement;

class CEngineConfig {
 public:
  static CEngineConfig& Instance();

  void Init(const char* strResourceDir, const char* strAppInstallDir, const std::string& xodr_plugin_path = "");
  bool Initialize(const std::string& resource_dir = ".", const std::string& xodr_plugin_path = "");

  bool DrawRoad() { return m_drawRoad; }
  bool DrawLane() { return m_drawLane; }
  bool DrawLaneBoundary() { return m_drawLaneBoundary; }
  bool DrawLaneLink() { return m_drawLaneLink; }
  bool DrawMapObject() { return m_drawMapObject; }
  bool DrawAabb() { return m_drawAabb; }
  bool DrawRoadMesh() { return m_drawRoadMesh; }
  bool DrawGrid() { return m_drawGrid; }
  CEnvVar& EnvVar() { return m_envVar; }

  void SetDrawRoad(bool bDraw);
  void SetDrawLane(bool bDraw);
  void SetDrawLaneBoundary(bool bDraw);
  void SetDrawLaneLink(bool bDraw);
  void SetDrawMapObject(bool bDraw);
  void SetDrawAabb(bool bDraw);
  void SetDrawRoadMesh(bool bDraw);
  void SetDrawGrid(bool bDraw);

  tagConfigEditor& EditorConfigure() { return m_config.editorConfig; }

  int UpdateHadmapAttr(std::string strHadmapName, tagHadmapAttr& attr);
  tagHadmapAttr* HadmapAttr(std::string strHadmapName);
  void RemoveHadmapAttr(std::string strHadmapName);
  HadmapAttrMap& HadmapAttrs() { return m_hadmapAttrs; }

  int LoadHadmapConfig();
  int SaveToHadmapConfig();
  int SaveHadmapConfig(std::string strHadmapPath, std::vector<tagHadmapAttr>& haVec);
  int ComposeHadmapConfigStr();
  const char* HadmapConfigStr() { return m_hadmapAttrsConfigStr.c_str(); }
  const wchar_t* HadmapConfigWStr() { return m_hadmapAttrsConfigWStr.c_str(); }

  int LoadConfig();
  int ComposeConfigStr();
  const char* ConfigStr() { return m_configStr.c_str(); }
  const wchar_t* ConfigWStr() { return m_configWStr.c_str(); }
  int HadmapIndex(const char* strMapName);

  const char* L3StateMachineStr() { return m_l3StateMachineStr.c_str(); }
  const wchar_t* L3StateMachineStrWStr() { return m_l3StateMachineWStr.c_str(); }

  std::string WStr2MBStr(const wchar_t* wstr);
  std::wstring MBStr2WStr(const char* str);

  bool GetProcessFilePath(std::string& strPath);
  const char* ImageFileDir() { return m_strProcessImageDir.c_str(); }
  const char* ResourceDir() { return m_strResourceDir.c_str(); }
  const char* AppInstallDir() { return m_strAppInstallDir.c_str(); }
  const char* HadmapDir() { return m_strHadmapDir.c_str(); }
  const char* GISImageDir() { return m_strGISImageDir.c_str(); }
  const char* GISModelDir() { return m_strGISModelDir.c_str(); }
  const char* SceneDir() { return m_strSceneDir.c_str(); }
  const char* SensorsDir() { return m_strSensorsDir.c_str(); }
  const char* GradingTemplateFile() { return m_strGradingTemplateFile.c_str(); }
  const char* getSceneDataDir() { return SceneDir(); }
  const char* getHadmapDataDir() { return HadmapDir(); }
  const char* getCataLogDir() { return m_strDefaultXosc.c_str(); }
  const char* getVehicle() { return m_strVehicleXosc.c_str(); }
  const char* getPedestrian() { return m_strPedestriantXosc.c_str(); }
  const char* getMisobject() { return m_strMiscobjecttXosc.c_str(); }
  const char* getSensorPath() { return m_strSensorData.c_str(); }
  const char* getDynamicPath() { return m_strDynamicDir.c_str(); }
  const char* getConfPath() { return m_strConf.c_str(); }
  std::string CatalogsDir() { return m_strResourceDir + "/Catalogs"; }

  int SaveToXodr(const char* strName, std::string& strJson);
  int ParseFromXodr(const char* strName, std::string& strJson);
  bool IsInitialized() { return initialized; }

 protected:
  int parseL3StateMachine(TiXmlElement* elemL3StateMachine);
  int ComposeL3StateMachineStr();

  CEngineConfig();
  CEngineConfig(const CEngineConfig&);

  bool m_drawRoad;
  bool m_drawLane;
  bool m_drawLaneBoundary;
  bool m_drawLaneLink;
  bool m_drawMapObject;
  bool m_drawAabb;
  bool m_drawRoadMesh;
  bool m_drawGrid;

  HadmapAttrMap m_hadmapAttrs;
  std::string m_hadmapAttrsConfigStr;
  std::wstring m_hadmapAttrsConfigWStr;

  std::string m_strProcessImageDir;
  std::string m_strResourceDir;
  std::string m_strAppInstallDir;
  std::string m_strDefaultXosc;  // 默认defaultxosc
  std::string m_strVehicleXosc;
  std::string m_strPedestriantXosc;
  std::string m_strMiscobjecttXosc;
  std::string m_strSensorData;  //
  std::string m_strDynamicDir;  //
  std::string m_strConf;        //

  std::string m_strHadmapDir;
  std::string m_strGISImageDir;
  std::string m_strGISModelDir;
  std::string m_strSceneDir;
  std::string m_strSensorsDir;
  std::string m_strGradingTemplateFile;

  tagConfiguration m_config;
  std::string m_configStr;
  std::wstring m_configWStr;

  tagL3StateMachine m_l3StateMachine;
  std::string m_l3StateMachineStr;
  std::wstring m_l3StateMachineWStr;
  CEnvVar m_envVar;

  bool initialized = false;
};
