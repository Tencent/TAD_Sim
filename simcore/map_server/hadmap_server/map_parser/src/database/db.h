/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once

#include <soci/soci.h>
#include <set>
#include <string>
#include <vector>

struct sTagSearchScenarioParam;
struct sTagGetScenarioListParam;
struct sTagGetScenarioInfoListParam;
struct sTagGetScenarioSetListParam;
struct sTagCreateScenarioSetParam;
struct sTagUpdateScenarioSetParam;
struct sTagDeleteScenarioSetParam;

struct sTagEntityScenarioSet;
struct sTagEntityScenario;

class CScenarioSets;
class CScenarioEntities;

class CSimulation;

struct sTagDBVersion {
  int nMajor;
  int nMinor;
  int nRevision;
};

class CDatabase {
 public:
  void Init();

  // clear
  int ClearDB();
  bool GetIsPreset();
  void SetIsPreset(bool ispresent);
  // with logic
  int CreateOneScenario(sTagEntityScenario& scene, int nSetID = 1);
  int UpdateOneScenario(sTagEntityScenario& scene);
  int DeleteOneScenario(int64_t nID);
  int CreateOneScenarioSet(sTagEntityScenarioSet& scenarioSet);
  int UpdateOneScenarioSet(sTagEntityScenarioSet& scenarioSet);
  int DeleteOneScenarioSet(int64_t nID);

  // scene
  bool ScenarioExist(int64_t nID);
  bool ScenarioExist(const char* strFile);
  bool ScenarioExist(const char* strFile, const char* strExt);
  int64_t ScenarioCount();
  int ScenarioIDs(std::string& strIDS);
  int ScenarioList(sTagGetScenarioListParam& param, CScenarioEntities& scenarios);
  int ScenarioInfoList(sTagGetScenarioInfoListParam& param, CScenarioEntities& scenarios);
  int ScenarioSearch(sTagSearchScenarioParam& param, CScenarioEntities& scenarios);
  int Scenario(sTagEntityScenario& scene);
  int ScenarioByName(sTagEntityScenario& scene);
  int InsertScenario(sTagEntityScenario& scene);
  int InsertScenarios(std::vector<sTagEntityScenario>& scenes);
  int UpdateScenario(sTagEntityScenario& scene);
  int DeleteScenario(int64_t nID);

  int doUpdateScenario(sTagEntityScenario& scene, soci::connection_pool& pl);
  int doScenarioList(sTagGetScenarioListParam& param, CScenarioEntities& scenarios, soci::connection_pool& pl);
  //
  // scene set
  bool ScenarioSetExist(int64_t nID);
  bool ScenarioSetExist(std::string strSetName);
  int64_t ScenarioSetCount();
  int ScenarioSet(sTagEntityScenarioSet& scenarioSet);
  int ScenarioSetByName(sTagEntityScenarioSet& scenarioSet);
  int ScenarioSetsByScenario(int64_t nID, CScenarioSets& sets);
  int ScenarioSetList(sTagGetScenarioSetListParam& param, CScenarioSets& sets);
  int doScenarioSetList(sTagGetScenarioSetListParam& param, CScenarioSets& sets, soci::connection_pool& pl);
  int InsertScenarioSet(sTagCreateScenarioSetParam& param);
  int UpdateScenarioSet(sTagEntityScenarioSet& scenarioSet);
  int UpdateScenarioSet(sTagUpdateScenarioSetParam& param);
  int DeleteScenarioSet(sTagDeleteScenarioSetParam& param);
  int BatchInsertScenario(std::vector<sTagEntityScenario>& sceneV);

  // versions
  int DBVersion(sTagDBVersion& ver);
  int UpdateVersion(sTagDBVersion& ver);

  // data update
  int UpdateScenarioFrom1To1_1();

  // tables
  bool TableExists(std::string strTable);
  int CreateVersionTable();

  static CDatabase& Instance();

  int FixIDsStr(std::string strIn, std::string& strOut);
  int AddIDToIDs(std::string& strID, std::string& strIDs);
  int RemoveIDFromIDs(std::string& strIDs, std::string& strID);

  int dbBack(CScenarioEntities& scenarios, CScenarioSets& ssets);
  int dbPreset(CScenarioEntities& scenarios, CScenarioSets& ssets);

  int ConvertToIDs(std::string& strIDs, std::set<std::string>& ids);

 protected:
  int ConvertIDsToIDsStr(std::vector<std::string>& ids, std::string& strIDs);
  int ConvertIDsToIDsStr(std::set<std::string>& ids, std::string& strIDs);
  int Connect();
  int DeleteTables();
  int CreateTables();

  int PresetDatabase();

 protected:
  CDatabase();
  const std::string s_VERSION = "version";
  const std::string s_SCENARIO_SET = "scenesets";
  const std::string s_SCENARIO = "scenes";
  bool m_isPreset;
};
