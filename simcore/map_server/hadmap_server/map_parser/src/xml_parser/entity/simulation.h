/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

#include "common/xml_parser/entity/mapfile.h"
#include "common/xml_parser/entity/planner.h"
#include "control_track.h"
#include "generate_info.h"
#include "l3_states.h"
#include "rosbag.h"

class TiXmlElement;
class CSimulationParam;

class CSimulation {
  using SceneEventMap = std::map<std::string, SceneEvent>;

 public:
  CSimulation();
  explicit CSimulation(CSimulationParam& sp);
  virtual int Parse(const char* strFileName, bool bLoadHadmap = true);
  int Save(const char* strFileName);

  int ParseHadmapAttr(const char* strFileName);

  int SaveSceneEvents(TiXmlElement* SceneEventsElem, SceneEventMap& mapSceneEvent);

  int SaveOneSceneEvent(TiXmlElement* SceneEventElem, SceneEvent& se);

  int ParseSceneEvents(TiXmlElement* SceneEventsElem, std::map<std::string, SceneEvent>& mapSceneEvents);

  int ParseOneSceneEvent(TiXmlElement* elemSceneEvent, SceneEvent& se, std::string version);

 public:
  int SetScenePath(const char* strScenePath);
  int SetMapFile(const CMapFile& mapfile);
  int SetTrafficFile(const char* strTrafficFileName);
  int SetSensorFile(const char* strSensorFileName);
  int SetEnvironmentFile(const char* strEnvironmentFileName);
  int SetControlPathFile(const char* strControlPathFileName);
  int SetGradingFile(const char* strGradingFileName);
  int SetPlanner(const CPlanner& planner);
  int SetRosbag(const CRosbag& rosbag);
  int ChangeTrafficName(const char* strFileName, const char* strTrafficFileName);

 protected:
  int ParseMapfile(TiXmlElement* elemMapfile, CMapFile& mapfile);
  int ParseTraffic(TiXmlElement* elemTraffic, std::string& strTraffic);
  int ParseSensor(TiXmlElement* elemSensor, std::string& strSensor);
  int ParseEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment);
  int ParseControlPath(TiXmlElement* elemControlPath, std::string& strControlPath);
  int ParseGrading(TiXmlElement* elemGrading, std::string& strGrading);
  int ParsePlanner(TiXmlElement* elemPlanner, CPlanner& planner);
  int ParsePlannerV2(TiXmlElement* elemPlanner, CPlanner& planner);
  int ParseRosbag(TiXmlElement* elemRosbag, CRosbag& rosbag);
  int ParseL3States(TiXmlElement* elemL3States, CL3States& l3States);
  int ParseGeoFence(TiXmlElement* elemGeoFence, std::string& strGeoFence);
  int ParseGenerateInfo(TiXmlElement* elemGenerateInfo, CGenerateInfo& strGenerateInfo);
  int ParseInfo(TiXmlElement* elemInfo, std::string& strInfo);
  int ParseSimrecDataSource(TiXmlElement* elemInfo, std::string& strSource);

  int SaveGenerateInfo(TiXmlElement* elemGenerateInfo, CGenerateInfo& strGenerateInfo);
  int SaveMapfile(TiXmlElement* elemMapfile, CMapFile& mapfile);
  int SaveTraffic(TiXmlElement* elemTraffic, std::string& strTraffic);
  int SaveSensor(TiXmlElement* elemSensor, std::string& strSensor);
  int SaveEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment);
  int SaveControlPath(TiXmlElement* elemControlPath, std::string& strControlPath);
  int SaveGrading(TiXmlElement* elemGrading, std::string& strGrading);
  int SavePlanner(TiXmlElement* elemPlanner, CPlanner& planner);
  int SaveRosbag(TiXmlElement* elemRosbag, CRosbag& rosbag);
  int SaveL3States(TiXmlElement* elemL3States, CL3States& l3States);
  int SaveGeoFence(TiXmlElement* elemGeoFence, std::string& strGeoFence);
  int SaveInfo(TiXmlElement* elemInfo, std::string& strInfo);

 public:
  std::string m_strScenePath;
  std::string m_strInfo;  // 工况
  CMapFile m_mapFile;
  std::string m_strTraffic;
  std::string m_strSensor;
  std::string m_strEnvironment;
  std::string m_strControlPath;
  std::string m_strGrading;
  CGenerateInfo m_generateInfo;
  CPlanner m_planner;
  CRosbag m_rosbag;
  CL3States m_l3States;
  std::string m_strGeoFence;
  std::vector<CControlTrack> m_controlTrack;

  // the record id of the scene which this simulation belongs to
  int64_t m_nID;
  std::string m_strDataSource;
  struct ParameterDeclaration {
    std::string name;
    std::string parameterType;
    std::string value;
  };
  std::vector<ParameterDeclaration> m_parameterDeclarations;
};
