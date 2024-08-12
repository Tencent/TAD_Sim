/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "db.h"
#include <soci/soci.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include "../engine/config.h"
#include "../xml_parser/entity/scenario_set_params.h"
#include "../xml_parser/entity/simulation.h"
#include "./entity/scenario.h"
#include "./entity/scenario_set.h"
#include "common/log/system_logger.h"
#include "synchronizer.h"

int pool_size = 3;
soci::connection_pool pool(pool_size);
CDatabase::CDatabase() {
  Connect();
  CreateTables();
}

CDatabase& CDatabase::Instance() {
  static CDatabase db;

  return db;
}

void CDatabase::Init() {
  if (ScenarioCount() == 0) {
    CSynchronizer::Instance().SyncDiskToDB();
  }
}

bool CDatabase::TableExists(std::string strTable) {
  try {
    soci::session sql(pool);

    int nTableExist = 0;
    std::string strSql = R"(select count(*) from sqlite_master where type="table" and name=")" + s_VERSION + R"(")";
    sql << strSql, soci::into(nTableExist);

    if (nTableExist > 0) {
      return true;
    }
    return false;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("TableExist error : %s", e.what());
  }

  return false;
}

int CDatabase::CreateVersionTable() {
  try {
    soci::session sql(pool);

    // table version
    sql << "create table if not exists " + s_VERSION +
               "("
               "major integer, "
               "minor integer, "
               "revision integer) ";
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("CreateVersionTable: %s", e.what());
  }

  return -1;
}

int CDatabase::DeleteTables() {
  try {
    soci::session sql(pool);
    // table scene
    sql << "drop table if exists " + s_SCENARIO;
    // table simu scene set
    sql << "drop table if exists " + s_SCENARIO_SET;
    // table version
    sql << "drop table if exists " + s_VERSION;
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_ERROR("ex: %s", e.what());
  }

  return 0;
}

int CDatabase::Connect() {
  std::string strResouceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path p = strResouceDir;
  p /= "simu_scene.sqlite";

  try {
    std::string strConnectString = p.string();
    for (int i = 0; i < pool_size; ++i) {
      soci::session& sql = pool.at(i);
      sql.open(strConnectString);
    }
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}

int CDatabase::CreateTables() {
  try {
    soci::session sql(pool);
    // table scene
    sql << "create table if not exists " + s_SCENARIO +
               "("
               "id integer primary key autoincrement, "
               "name varchar(256) unique, "
               "map varchar(256) , "
               "path varchar(1024), "
               "info varchar(256), "
               "label varchar(1024), "
               "sets varchar(1024), "
               "content text )";
    // "ctm datetime) ";

    // table simu scene set
    sql << "create table if not exists " + s_SCENARIO_SET +
               "("
               "id integer primary key autoincrement, "
               "name varchar(256) not null, "
               "label varchar(1024), "
               "content text) ";

    // table version
    CreateVersionTable();
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_ERROR("ex: %s", e.what());
  }

  return 0;
}

int CDatabase::ClearDB() {
  DeleteTables();
  CreateTables();

  return 0;
}

int CDatabase::DBVersion(sTagDBVersion& ver) {
  ver.nMajor = 1;
  ver.nMinor = 0;
  ver.nRevision = 0;

  try {
    if (TableExists(s_VERSION)) {
      soci::session sql(pool);
      std::string strSql = "select * from " + s_VERSION;
      sql << strSql, soci::into(ver.nMajor), soci::into(ver.nMinor), soci::into(ver.nRevision);
    }

    std::string strVersion =
        std::to_string(ver.nMajor) + "." + std::to_string(ver.nMinor) + "." + std::to_string(ver.nRevision);
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("DBVersion error %s", e.what());
  }

  return -1;
}

int CDatabase::UpdateVersion(sTagDBVersion& ver) {
  try {
    int nRet = CreateVersionTable();
    if (nRet != 0) {
      return nRet;
    }

    soci::session sql(pool);
    sql << "insert into " + s_VERSION +
               " (major, minor, revision) "
               " values(:major, :minor, :revision) ",
        soci::use(ver.nMajor), soci::use(ver.nMinor), soci::use(ver.nRevision);

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("UpdateVersion error: %s", e.what());
  }

  return -1;
}

int CDatabase::CreateOneScenario(sTagEntityScenario& scene, int nSetID /* = 1*/) {
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);

    std::string strSetID = std::to_string(nSetID);
    scene.m_strSets = "," + strSetID + ",";
    // insert scenario
    int nRet = InsertScenario(scene);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Insert Scenario failed!");
      return -1;
    }

    // get default scenarioset
    sTagEntityScenarioSet scenarioSet;
    scenarioSet.m_nID = nSetID;
    nRet = ScenarioSet(scenarioSet);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Get ScenarioSet failed!");
      return -1;
    }

    // update scenes id
    std::string strNewID = std::to_string(scene.m_nID) + ",";
    if (scenarioSet.m_strScenes.size() > 0) {
      scenarioSet.m_strScenes += strNewID;
    } else {
      scenarioSet.m_strScenes = "," + strNewID;
    }

    // update scenario set

    nRet = UpdateScenarioSet(scenarioSet);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Update ScenarioSet failed!");
      return -1;
    }

    tr.commit();
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("CreateOneScenario, %s", e.what());
  }
  return -1;
}

int CDatabase::UpdateOneScenario(sTagEntityScenario& scene) {
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);

    sTagEntityScenario oldScenario;
    oldScenario.m_nID = scene.m_nID;
    Scenario(oldScenario);
    scene.m_strSets = oldScenario.m_strSets;

    // update scenario
    int nRet = UpdateScenario(scene);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Update Scenario failed!");
      return -1;
    }

    tr.commit();
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("UpdateOneScenario, %s", e.what());
  }
  return -1;
}

int CDatabase::DeleteOneScenario(int64_t nID) {
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);

    // get scenariosets
    CScenarioSets sets;
    int nRet = ScenarioSetsByScenario(nID, sets);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Get ScenarioSets failed!");
      return -1;
    }

    // delete scenario
    nRet = DeleteScenario(nID);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Delete Scenario failed!");
      return -1;
    }

    std::string strID = std::to_string(nID);

    // update scenario set
    ScenarioSets& entities = sets.Sets();
    ScenarioSets::iterator itr = entities.begin();
    for (; itr != entities.end(); ++itr) {
      // update scenes id
      RemoveIDFromIDs(itr->m_strScenes, strID);
      nRet = UpdateScenarioSet((*itr));
      if (nRet != 0) {
        SYSTEM_LOGGER_ERROR("Update ScenarioSet failed!");
        return -1;
      }
    }

    tr.commit();
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("DeleteOneScenario, %s", e.what());
  }
  return -1;
}

int CDatabase::CreateOneScenarioSet(sTagEntityScenarioSet& scenarioSet) {
  FixIDsStr(scenarioSet.m_strScenes, scenarioSet.m_strScenes);
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);

    // create scenario set
    sTagCreateScenarioSetParam param;
    param.m_nID = scenarioSet.m_nID;
    param.m_strName = scenarioSet.m_strName;
    param.m_strLabel = scenarioSet.m_strLabel;
    param.m_strScenes = scenarioSet.m_strScenes;
    InsertScenarioSet(param);

    if (scenarioSet.m_strScenes.size() > 0) {
      sTagEntityScenarioSet defaultSet;
      defaultSet.m_nID = 1;
      ScenarioSet(defaultSet);
      std::set<std::string> addedSceneIDs;
      std::set<std::string> defaultSceneIDs;
      ConvertToIDs(scenarioSet.m_strScenes, addedSceneIDs);
      ConvertToIDs(defaultSet.m_strScenes, defaultSceneIDs);

      // get added and removed scenes
      std::vector<std::string> remainedIDs;
      std::set_difference(defaultSceneIDs.begin(), defaultSceneIDs.end(), addedSceneIDs.begin(), addedSceneIDs.end(),
                          std::back_inserter(remainedIDs));
      ConvertIDsToIDsStr(remainedIDs, defaultSet.m_strScenes);
      if (remainedIDs.size() > 0) {
        std::string strRemainedIDs = boost::algorithm::join(remainedIDs, ",");
        defaultSet.m_strScenes = "," + strRemainedIDs + ",";
      } else {
        defaultSet.m_strScenes = "";
      }

      sTagEntityScenario scenario;
      std::string strSetID = std::to_string(param.m_nID);
      std::string strDefaultID = std::to_string(1);

      // update add scenario
      std::set<std::string>::iterator itr = addedSceneIDs.begin();
      for (; itr != addedSceneIDs.end(); ++itr) {
        scenario.m_nID = atoi(itr->c_str());
        Scenario(scenario);
        RemoveIDFromIDs(scenario.m_strSets, strDefaultID);
        AddIDToIDs(strSetID, scenario.m_strSets);
        UpdateScenario(scenario);
      }

      UpdateScenarioSet(defaultSet);
    }

    tr.commit();

    scenarioSet.m_nID = param.m_nID;
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("CreateOneScenarioSet, %s", e.what());
  }
  return -1;
}

int CDatabase::UpdateOneScenarioSet(sTagEntityScenarioSet& scenarioSet) {
  FixIDsStr(scenarioSet.m_strScenes, scenarioSet.m_strScenes);
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);
    // get scenario set in db
    sTagEntityScenarioSet oldSet;
    oldSet.m_nID = scenarioSet.m_nID;
    int nRet = ScenarioSet(oldSet);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Get ScenarioSets %ld failed!", oldSet.m_nID);
      return -1;
    }

    std::set<std::string> oldSceneIDs, newSceneIDs;
    ConvertToIDs(oldSet.m_strScenes, oldSceneIDs);
    ConvertToIDs(scenarioSet.m_strScenes, newSceneIDs);

    // get added and removed scenes
    std::vector<std::string> addedSceneIDs;
    std::vector<std::string> removedSceneIDs;
    std::set_difference(newSceneIDs.begin(), newSceneIDs.end(), oldSceneIDs.begin(), oldSceneIDs.end(),
                        std::back_inserter(addedSceneIDs));
    std::set_difference(oldSceneIDs.begin(), oldSceneIDs.end(), newSceneIDs.begin(), newSceneIDs.end(),
                        std::back_inserter(removedSceneIDs));

    sTagEntityScenario scenario;
    std::string strSetID = std::to_string(scenarioSet.m_nID);

    std::set<std::string> addedToDefaultSet;
    // update removed scenario
    std::vector<std::string>::iterator itr = removedSceneIDs.begin();
    for (; itr != removedSceneIDs.end(); ++itr) {
      scenario.m_nID = atoi(itr->c_str());
      Scenario(scenario);
      RemoveIDFromIDs(scenario.m_strSets, strSetID);
      if (scenario.m_strSets.size() == 0) {
        std::string strScenarioID = std::to_string(scenario.m_nID);
        addedToDefaultSet.insert(strScenarioID);
        scenario.m_strSets = "," + strScenarioID + ",";
      }
      UpdateScenario(scenario);
    }

    std::set<std::string> removedFromDefaultSet;
    // update add scenario
    std::set<std::string> ownedSets;
    itr = addedSceneIDs.begin();
    for (; itr != addedSceneIDs.end(); ++itr) {
      scenario.m_nID = atoi(itr->c_str());
      Scenario(scenario);
      ownedSets.clear();
      ConvertToIDs(scenario.m_strSets, ownedSets);
      std::string strID = (*ownedSets.begin());
      if (ownedSets.size() == 1 && boost::algorithm::equals(strID, "1")) {
        std::string strScenarioID = std::to_string(scenario.m_nID);
        removedFromDefaultSet.insert(strScenarioID);
        scenario.m_strSets = "";
      }
      AddIDToIDs(strSetID, scenario.m_strSets);
      UpdateScenario(scenario);
    }

    // update scenario set
    UpdateScenarioSet(scenarioSet);

    if (addedToDefaultSet.size() > 0 || removedFromDefaultSet.size() > 0) {
      sTagEntityScenarioSet defaultSet;
      defaultSet.m_nID = 1;
      ScenarioSet(defaultSet);
      std::set<std::string> defaultScenes;
      ConvertToIDs(defaultSet.m_strScenes, defaultScenes);

      std::set<std::string> removedDefalut;
      set_difference(defaultScenes.begin(), defaultScenes.end(), removedFromDefaultSet.begin(),
                     removedFromDefaultSet.end(), std::inserter(removedDefalut, removedDefalut.begin()));

      std::set<std::string> finalDefalut;
      set_union(removedDefalut.begin(), removedDefalut.end(), addedToDefaultSet.begin(), addedToDefaultSet.end(),
                std::inserter(finalDefalut, finalDefalut.begin()));

      ConvertIDsToIDsStr(finalDefalut, defaultSet.m_strScenes);
      UpdateScenarioSet(defaultSet);
    }

    tr.commit();

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("UpdateOneScenarioSet, %s", e.what());
  }
  return -1;
}

int CDatabase::DeleteOneScenarioSet(int64_t nID) {
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);
    // get scenario set in db
    sTagEntityScenarioSet oldSet;
    oldSet.m_nID = nID;
    int nRet = ScenarioSet(oldSet);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("Get ScenarioSets %ld failed!", oldSet.m_nID);
      return -1;
    }

    std::set<std::string> removedSceneIDs;
    ConvertToIDs(oldSet.m_strScenes, removedSceneIDs);

    sTagEntityScenario scenario;
    std::string strSetID = std::to_string(nID);
    std::string strDefaultSetID = std::to_string(1);
    std::vector<std::string> backToDefaultIDs;

    std::set<std::string> addedToDefaultSet;
    // update removed scenario
    std::set<std::string>::iterator itr = removedSceneIDs.begin();
    for (; itr != removedSceneIDs.end(); ++itr) {
      scenario.m_nID = atoi(itr->c_str());
      Scenario(scenario);
      RemoveIDFromIDs(scenario.m_strSets, strSetID);
      if (scenario.m_strSets.size() == 0) {
        std::string strScenarioID = std::to_string(scenario.m_nID);
        addedToDefaultSet.insert(strScenarioID);
        scenario.m_strSets = "," + strDefaultSetID + ",";
      }
      UpdateScenario(scenario);
    }
    sTagDeleteScenarioSetParam param;
    param.m_nID = nID;
    DeleteScenarioSet(param);

    if (addedToDefaultSet.size() > 0) {
      sTagEntityScenarioSet defaultSet;
      defaultSet.m_nID = 1;
      ScenarioSet(defaultSet);
      std::set<std::string> defaultScenes;
      ConvertToIDs(defaultSet.m_strScenes, defaultScenes);

      std::set<std::string> finalDefalut;
      set_union(defaultScenes.begin(), defaultScenes.end(), addedToDefaultSet.begin(), addedToDefaultSet.end(),
                std::inserter(finalDefalut, finalDefalut.begin()));

      ConvertIDsToIDsStr(finalDefalut, defaultSet.m_strScenes);
      UpdateScenarioSet(defaultSet);
    }

    tr.commit();

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("DeleteOneScenarioSet, %s", e.what());
  }
  return -1;
}

bool CDatabase::ScenarioExist(int64_t nID) {
  try {
    soci::session sql(pool);
    sql << "select * from " + s_SCENARIO + " where id = :nID", soci::use(nID);

    bool bRet = sql.got_data();

    SYSTEM_LOGGER_DEBUG("Scenario Exist for %ld  is %d", nID, bRet);

    return bRet;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("ScenarioExist, %s", e.what());
  }
  return false;
}

bool CDatabase::ScenarioExist(const char* strFile) {
  try {
    soci::session sql(pool);
    std::string strName = strFile;
    sql << "select * from " + s_SCENARIO + " where name=:strName", soci::use(strName);

    bool bRet = sql.got_data();

    SYSTEM_LOGGER_DEBUG("Scenario Exist for %s  is %d", strFile, bRet);

    return bRet;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_DEBUG("Scenario Exist error: %s", e.what());
  }

  return true;
}

int64_t CDatabase::ScenarioCount() {
  int64_t lCount = 0;
  try {
    soci::session sql(pool);
    std::string strSQL = "select count(*) from " + s_SCENARIO;

    sql << strSQL, soci::into(lCount);
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_ERROR("Scenario Count error: %s", e.what());
  }

  SYSTEM_LOGGER_DEBUG("Scenario Count : %ld", lCount);

  return lCount;
}

int CDatabase::ScenarioIDs(std::string& strIDs) {
  strIDs.clear();

  try {
    soci::session sql(pool);
    std::string strSQL = "select id from " + s_SCENARIO;
    soci::rowset<soci::row> rs = (sql.prepare << strSQL);

    std::vector<std::string> ids;
    for (soci::rowset<soci::row>::iterator it = rs.begin(); it != rs.end(); ++it) {
      const soci::row& row = *it;
      int nID = row.get<int>(0);

      ids.push_back(std::to_string(nID));
    }

    if (ids.size() > 0) {
      std::string str = boost::algorithm::join(ids, ",");
      strIDs = ',';
      strIDs += str;
      strIDs += ',';
    }

    SYSTEM_LOGGER_DEBUG("Get Scenario ids. count : %d ", ids.size());

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("ScenarioIDs error. %s ", e.what());
  }

  return -1;
}

int CDatabase::ScenarioList(sTagGetScenarioListParam& param, CScenarioEntities& scenarios) {
  scenarios.Clear();
  try {
    soci::session sql(pool);
    std::string strSQL = "select * from " + s_SCENARIO;
    soci::rowset<soci::row> rs = (sql.prepare << strSQL);

    ScenarioEntities& ses = scenarios.Entities();

    for (soci::rowset<soci::row>::iterator it = rs.begin(); it != rs.end(); ++it) {
      const soci::row& row = *it;
      sTagEntityScenario e;
      e.m_nID = row.get<int>(0);
      e.m_strName = row.get<std::string>(1);
      e.m_strMap = row.get<std::string>(2);
      e.m_strPath = row.get<std::string>(3);
      e.m_strInfo = row.get<std::string>(4);
      e.m_strLabel = row.get<std::string>(5);
      e.m_strSets = row.get<std::string>(6);
      e.m_strContent = row.get<std::string>(7);
      ses.emplace_back(e);
    }

    SYSTEM_LOGGER_DEBUG("Get Scenario List. count : %d ", ses.size());

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("Get Scenario List error. %s ", e.what());
  }
  return -1;
}

int CDatabase::ScenarioInfoList(sTagGetScenarioInfoListParam& param, CScenarioEntities& scenarios) {
  std::string strIDS = boost::algorithm::join(param.m_ids, ",");

  scenarios.Clear();

  soci::session sql(pool);
  std::string strSQL = "select * from " + s_SCENARIO + " " + " where id in(" + strIDS + ")";
  soci::rowset<soci::row> rs = (sql.prepare << strSQL);

  ScenarioEntities& ses = scenarios.Entities();

  for (soci::rowset<soci::row>::iterator it = rs.begin(); it != rs.end(); ++it) {
    const soci::row& row = *it;
    sTagEntityScenario e;
    e.m_nID = row.get<int>(0);
    e.m_strName = row.get<std::string>(1);
    e.m_strMap = row.get<std::string>(2);
    e.m_strPath = row.get<std::string>(3);
    e.m_strInfo = row.get<std::string>(4);
    e.m_strLabel = row.get<std::string>(5);
    e.m_strSets = row.get<std::string>(6);
    e.m_strContent = row.get<std::string>(7);
    ses.emplace_back(e);
  }

  SYSTEM_LOGGER_DEBUG("Get Scenario List. count : %d ", ses.size());

  return 0;
}

int CDatabase::ScenarioSearch(sTagSearchScenarioParam& param, CScenarioEntities& scenarios) {
  scenarios.Clear();
  std::string strCondition = "'%" + param.m_strKeys + "%'";
  std::string strSQL = "select * from " + s_SCENARIO + " ";
  if (boost::algorithm::iequals("map", param.m_strType)) {
    strSQL += "where map like " + strCondition;
  } else if (boost::algorithm::iequals("info", param.m_strType)) {
    strSQL += "where info like " + strCondition;
  } else {
    strSQL += "where name like " + strCondition;
  }

  soci::session sql(pool);
  soci::rowset<soci::row> rs = (sql.prepare << strSQL);

  ScenarioEntities& ses = scenarios.Entities();

  for (soci::rowset<soci::row>::iterator it = rs.begin(); it != rs.end(); ++it) {
    const soci::row& row = *it;
    sTagEntityScenario e;
    e.m_nID = row.get<int>(0);
    e.m_strName = row.get<std::string>(1);
    e.m_strMap = row.get<std::string>(2);
    e.m_strPath = row.get<std::string>(3);
    e.m_strInfo = row.get<std::string>(4);
    e.m_strLabel = row.get<std::string>(5);
    e.m_strSets = row.get<std::string>(6);
    e.m_strContent = row.get<std::string>(7);
    ses.emplace_back(e);
  }

  SYSTEM_LOGGER_DEBUG("Get Scenario List. count : %d ", ses.size());

  return 0;
}

int CDatabase::Scenario(sTagEntityScenario& scene) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO + " where id=:nID", soci::use(scene.m_nID), soci::into(scene.m_nID),
      soci::into(scene.m_strName), soci::into(scene.m_strMap), soci::into(scene.m_strPath), soci::into(scene.m_strInfo),
      soci::into(scene.m_strLabel), soci::into(scene.m_strSets), soci::into(scene.m_strContent);

  SYSTEM_LOGGER_DEBUG("Scenario for %ld ", scene.m_nID);

  return 0;
}

int CDatabase::ScenarioByName(sTagEntityScenario& scene) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO + " where name =:strName", soci::use(scene.m_strName), soci::into(scene.m_nID),
      soci::into(scene.m_strName), soci::into(scene.m_strMap), soci::into(scene.m_strPath), soci::into(scene.m_strInfo),
      soci::into(scene.m_strLabel), soci::into(scene.m_strSets), soci::into(scene.m_strContent);

  SYSTEM_LOGGER_DEBUG("ScenarioByName for %s ", scene.m_strName.c_str());

  return 0;
}

int CDatabase::InsertScenario(sTagEntityScenario& scene) {
  std::string strName = scene.m_strName;
  std::string strMap = scene.m_strMap;
  std::string strPath = scene.m_strPath;
  std::string strInfo = scene.m_strInfo;
  std::string strLabel = scene.m_strLabel;
  std::string strSets = scene.m_strSets;
  std::string strContent = scene.m_strContent;

  SYSTEM_LOGGER_DEBUG("InsertScenario %s, %s, %s", strName.c_str(), strMap.c_str(), strPath.c_str());

  soci::session sql(pool);
  sql << "insert into scenes(name, map, path, info, label,sets, content)"
         " values(:name, :map, :path, :info, :label, :sets, :content)",
      soci::use(strName), soci::use(strMap), soci::use(strPath), soci::use(strInfo), soci::use(strLabel),
      soci::use(strSets), soci::use(strContent);

  int64_t llID = -1;
  bool bRet = sql.get_last_insert_id(s_SCENARIO, llID);
  if (bRet) {
    scene.m_nID = llID;
    return 0;
  } else {
    scene.m_nID = -1;
  }

  return -1;
}

int CDatabase::UpdateScenario(sTagEntityScenario& scene) {
  int64_t nID = scene.m_nID;
  std::string strName = scene.m_strName;
  std::string strMap = scene.m_strMap;
  std::string strPath = scene.m_strPath;
  std::string strInfo = scene.m_strInfo;
  std::string strLabel = scene.m_strLabel;
  std::string strSets = scene.m_strSets;
  std::string strContent = scene.m_strContent;
  SYSTEM_LOGGER_DEBUG("UpdateScenario %d, %s, %s, %s", nID, strName.c_str(), strLabel.c_str(), strContent.c_str());

  soci::session sql(pool);
  sql << "update " + s_SCENARIO +
             " set name=:strName, map=:strMap, path=:strPath, "
             " info=:strInfo, label=:strLabel, sets=:strSets, content=:strContent"
             " where id=:nID ",
      soci::use(strName), soci::use(strMap), soci::use(strPath), soci::use(strInfo), soci::use(strLabel),
      soci::use(strSets), soci::use(strContent), soci::use(nID);

  return 0;
}

int CDatabase::DeleteScenario(int64_t nID) {
  SYSTEM_LOGGER_DEBUG("DeleteScenario %d", nID);

  soci::session sql(pool);
  sql << "delete from " + s_SCENARIO + " where id=:nID", soci::use(nID);

  return 0;
}

bool CDatabase::ScenarioSetExist(int64_t nID) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO_SET + " where id = :nID", soci::use(nID);

  bool bRet = sql.got_data();

  SYSTEM_LOGGER_DEBUG("Scenario Set Exist for %ld  is %d", nID, bRet);

  return bRet;
}

int64_t CDatabase::ScenarioSetCount() {
  int64_t lCount = 0;

  soci::session sql(pool);
  sql << "select count(*) from " + s_SCENARIO_SET, soci::into(lCount);

  SYSTEM_LOGGER_DEBUG("Scenario Count : %ld", lCount);

  return lCount;
}

int CDatabase::ScenarioSet(sTagEntityScenarioSet& scenarioSet) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO_SET + " where id=:nID ", soci::use(scenarioSet.m_nID),
      soci::into(scenarioSet.m_nID), soci::into(scenarioSet.m_strName), soci::into(scenarioSet.m_strLabel),
      soci::into(scenarioSet.m_strScenes);

  SYSTEM_LOGGER_DEBUG("ScenarioSet for %ld ", scenarioSet.m_nID);

  return 0;
}

int CDatabase::ScenarioSetsByScenario(int64_t nID, CScenarioSets& sets) {
  sets.Clear();
  std::string strCondition = " '%," + std::to_string(nID) + ",%' ";
  std::string strSQL = "select * from " + s_SCENARIO_SET + " where content like " + strCondition;
  try {
    soci::session sql(pool);
    soci::rowset<soci::row> rs = (sql.prepare << strSQL);

    ScenarioSets& ss = sets.Sets();
    for (soci::rowset<soci::row>::iterator it = rs.begin(); it != rs.end(); ++it) {
      const soci::row& row = *it;
      sTagEntityScenarioSet s;
      s.m_nID = row.get<int>(0);
      s.m_strName = row.get<std::string>(1);
      s.m_strLabel = row.get<std::string>(2);
      s.m_strScenes = row.get<std::string>(3);
      ss.emplace_back(s);
    }
    SYSTEM_LOGGER_DEBUG("Get Scenario Set List. count : %d ", ss.size());
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("Get Scenario Set by Scenario error. reason: %s ", e.what());
  }

  return -1;
}

int CDatabase::ScenarioSetList(sTagGetScenarioSetListParam& param, CScenarioSets& sets) {
  sets.Clear();
  try {
    soci::session sql(pool);
    std::string strSQL = "select * from " + s_SCENARIO_SET + " order by id desc ";
    soci::rowset<soci::row> rs = (sql.prepare << strSQL);

    ScenarioSets& ss = sets.Sets();
    for (soci::rowset<soci::row>::iterator it = rs.begin(); it != rs.end(); ++it) {
      const soci::row& row = *it;
      sTagEntityScenarioSet s;
      s.m_nID = row.get<int>(0);
      s.m_strName = row.get<std::string>(1);
      s.m_strLabel = row.get<std::string>(2);
      s.m_strScenes = row.get<std::string>(3);
      ss.emplace_back(s);
    }

    SYSTEM_LOGGER_DEBUG("Get Scenario Set List. count : %d ", ss.size());

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("Get Scenario Set List. error! %s ", e.what());
  }
  return -1;
}

int CDatabase::InsertScenarioSet(sTagCreateScenarioSetParam& param) {
  std::string strName = param.m_strName;
  std::string strLabel = param.m_strLabel;
  std::string strContent = param.m_strScenes;
  SYSTEM_LOGGER_DEBUG("NewSceneSet  %s, %s, %s", strName.c_str(), strLabel.c_str(), strContent.c_str());

  soci::session sql(pool);
  sql << "insert into " + s_SCENARIO_SET +
             " (name, label, content) "
             " values(:name, :label, :content) ",
      soci::use(param.m_strName), soci::use(param.m_strLabel), soci::use(param.m_strScenes);

  int64_t llID = -1;
  bool bRet = sql.get_last_insert_id(s_SCENARIO_SET, llID);
  if (bRet) {
    param.m_nID = llID;
    return 0;
  } else {
    param.m_nID = -1;
  }

  return -1;
}

int CDatabase::UpdateScenarioSet(sTagEntityScenarioSet& scenarioSet) {
  std::string strName = scenarioSet.m_strName;
  std::string strLabel = scenarioSet.m_strLabel;
  std::string strContent = scenarioSet.m_strScenes;
  int nID = scenarioSet.m_nID;

  SYSTEM_LOGGER_DEBUG("UpdateSceneSet %d, %s, %s, %s", nID, strName.c_str(), strLabel.c_str(), strContent.c_str());

  soci::session sql(pool);
  sql << "update " + s_SCENARIO_SET +
             " set name=:strName, label=:strLabel, content=:strContent "
             " where id=:nID ",
      soci::use(strName), soci::use(strLabel), soci::use(strContent), soci::use(nID);

  return 0;
}

int CDatabase::UpdateScenarioSet(sTagUpdateScenarioSetParam& param) {
  std::string strName = param.m_strName;
  std::string strLabel = param.m_strLabel;
  std::string strContent = param.m_strScenes;
  int nID = param.m_nID;

  SYSTEM_LOGGER_DEBUG("UpdateSceneSet %d, %s, %s, %s", nID, strName.c_str(), strLabel.c_str(), strContent.c_str());

  soci::session sql(pool);
  sql << "update " + s_SCENARIO_SET +
             " set name=:strName, label=:strLabel, content=:strContent"
             " where id=:nID",
      soci::use(strName), soci::use(strLabel), soci::use(strContent), soci::use(nID);

  return 0;
}

int CDatabase::DeleteScenarioSet(sTagDeleteScenarioSetParam& param) {
  int nID = param.m_nID;

  SYSTEM_LOGGER_DEBUG("DeleteSceneSet %d", nID);

  soci::session sql(pool);
  sql << "delete from " + s_SCENARIO_SET + " where id=:nID", soci::use(nID);

  return 0;
}

int CDatabase::FixIDsStr(std::string strIn, std::string& strOut) {
  if (strIn.size() > 0) {
    if (strIn[0] != ',') {
      strOut = ",";
      strOut += strIn;
    } else {
      strOut = strIn;
    }

    if (strIn[strIn.length() - 1] != ',') {
      strOut += ",";
    }

  } else {
    strOut = "";
  }
  return 0;
}

int CDatabase::AddIDToIDs(std::string& strID, std::string& strIDs) {
  if (strIDs.size() > 0) {
    strIDs += strID;
    strIDs += ",";
  } else {
    strIDs = "," + strID + ",";
  }
  return 0;
}

int CDatabase::RemoveIDFromIDs(std::string& strSceneIDs, std::string& strID) {
  std::vector<std::string> ids;
  std::vector<std::string> ids_out;

  boost::algorithm::split(ids, strSceneIDs, boost::is_any_of(", "), boost::algorithm::token_compress_on);

  std::copy_if(std::begin(ids), std::end(ids), std::inserter(ids_out, std::begin(ids_out)),
               [strID](const std::string& id) {
                 if (id.size() == 0 || boost::algorithm::equals(id, strID)) {
                   return false;
                 }
                 return true;
               });

  std::string strSceneIDs_out = "";
  if (ids_out.size() > 0) {
    strSceneIDs_out = ",";
    strSceneIDs_out += boost::algorithm::join(ids_out, ",");
    strSceneIDs_out += ",";
  }

  SYSTEM_LOGGER_DEBUG("replace sceneids %s with sceneids %s", strSceneIDs.c_str(), strSceneIDs_out.c_str());
  strSceneIDs = strSceneIDs_out;

  return 0;
}

int CDatabase::ConvertToIDs(std::string& strSceneIDs, std::set<std::string>& ids_out) {
  std::set<std::string> ids;

  boost::algorithm::split(ids, strSceneIDs, boost::is_any_of(", "), boost::algorithm::token_compress_on);

  std::copy_if(std::begin(ids), std::end(ids), std::inserter(ids_out, std::begin(ids_out)),
               [](const std::string& id) { return id.size() > 0; });

  return 0;
}

int CDatabase::ConvertIDsToIDsStr(std::vector<std::string>& ids_in, std::string& strSceneIDs) {
  if (ids_in.size() > 0) {
    std::string strIDs = boost::algorithm::join(ids_in, ",");
    strSceneIDs = ",";
    strSceneIDs += strIDs;
    strSceneIDs += ",";
  } else {
    strSceneIDs = "";
  }

  return 0;
}

int CDatabase::ConvertIDsToIDsStr(std::set<std::string>& ids_in, std::string& strSceneIDs) {
  if (ids_in.size() > 0) {
    std::string strIDs = boost::algorithm::join(ids_in, ",");
    strSceneIDs = ",";
    strSceneIDs += strIDs;
    strSceneIDs += ",";
  } else {
    strSceneIDs = "";
  }

  return 0;
}
