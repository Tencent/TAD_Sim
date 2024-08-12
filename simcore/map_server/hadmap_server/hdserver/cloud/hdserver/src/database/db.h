/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

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
  int64_t ScenarioCount();
  int ScenarioIDs(std::string& strIDS);
  int ScenarioList(sTagGetScenarioListParam& param, CScenarioEntities& scenarios);
  int ScenarioInfoList(sTagGetScenarioInfoListParam& param, CScenarioEntities& scenarios);
  int ScenarioSearch(sTagSearchScenarioParam& param, CScenarioEntities& scenarios);
  int Scenario(sTagEntityScenario& scene);
  int ScenarioByName(sTagEntityScenario& scene);
  int InsertScenario(sTagEntityScenario& scene);
  int UpdateScenario(sTagEntityScenario& scene);
  int DeleteScenario(int64_t nID);

  // scene set
  bool ScenarioSetExist(int64_t nID);
  int64_t ScenarioSetCount();
  int ScenarioSet(sTagEntityScenarioSet& scenarioSet);
  int ScenarioSetsByScenario(int64_t nID, CScenarioSets& sets);
  int ScenarioSetList(sTagGetScenarioSetListParam& param, CScenarioSets& sets);
  int InsertScenarioSet(sTagCreateScenarioSetParam& param);
  int UpdateScenarioSet(sTagEntityScenarioSet& scenarioSet);
  int UpdateScenarioSet(sTagUpdateScenarioSetParam& param);
  int DeleteScenarioSet(sTagDeleteScenarioSetParam& param);

  // versions
  int DBVersion(sTagDBVersion& ver);
  int UpdateVersion(sTagDBVersion& ver);

  // tables
  bool TableExists(std::string strTable);
  int CreateVersionTable();

  static CDatabase& Instance();

 protected:
  int FixIDsStr(std::string strIn, std::string& strOut);
  int AddIDToIDs(std::string& strID, std::string& strIDs);
  int RemoveIDFromIDs(std::string& strIDs, std::string& strID);
  int ConvertToIDs(std::string& strIDs, std::set<std::string>& ids);
  int ConvertIDsToIDsStr(std::vector<std::string>& ids, std::string& strIDs);
  int ConvertIDsToIDsStr(std::set<std::string>& ids, std::string& strIDs);
  int Connect();
  int DeleteTables();
  int CreateTables();

 protected:
  CDatabase();
  const std::string s_VERSION = "version";
  const std::string s_SCENARIO_SET = "scenesets";
  const std::string s_SCENARIO = "scenes";
};
