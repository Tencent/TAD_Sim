/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "db.h"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
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

int pool_size = 20;
soci::connection_pool pool(pool_size);

CDatabase::CDatabase() {
  // PresetDatabase();
  Connect();
  CreateTables();
  PresetDatabase();
}

CDatabase& CDatabase::Instance() {
  static CDatabase db;
  return db;
}

void CDatabase::Init() {
  CSynchronizer::Instance().loadDataHadmapConfig();
  if (ScenarioCount() == 0 || this->GetIsPreset()) {
    CSynchronizer::Instance().SyncDiskToDB2();
  }
}

int CDatabase::UpdateScenarioFrom1To1_1() {
  soci::session sql(pool);
  std::string strSql = "alter table " + s_SCENARIO + "add column type char(4) default 'sim'";

  try {
    sql << strSql;

  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("UpdateScenarioFrom1To1_1 error %s", e.what());
    return -1;
  }

  return 0;
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
    SYSTEM_LOGGER_ERROR("CreateVersionTable error: %s", e.what());
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
    SYSTEM_LOGGER_ERROR("%s", e.what());
    std::cerr << "Error: " << e.what() << "\n";
  }

  return 0;
}

int CDatabase::Connect() {
  std::string strResouceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path p = strResouceDir;
  p /= "simu_scene.sqlite";

  SYSTEM_LOGGER_INFO(" check simu_scene.sqlite existence. %d", boost::filesystem::exists(p), " at %s",
                     p.string().c_str(), " str resource dir: %s ", strResouceDir.c_str());

  // create new db here.
  try {
    std::string strConnectString = p.string();
    for (int i = 0; i < pool_size; ++i) {
      soci::session& sql = pool.at(i);
      sql.open(soci::sqlite3, strConnectString);
    }
    SYSTEM_LOGGER_INFO(" check simu_scene.sqlite existence. %d", boost::filesystem::exists(p), " at %s",
                       p.string().c_str(), " str resource dir: %s ", strResouceDir.c_str());
  } catch (std::exception& e) {
  }

  return 0;
}

int CDatabase::dbBack(CScenarioEntities& scenarios, CScenarioSets& ssets) {
  std::string strResouceDir = CEngineConfig::Instance().getBackDbDir();
  boost::filesystem::path p = strResouceDir;
  p /= "simu_scene.sqlite";
  if (boost::filesystem::exists(p)) {
    soci::connection_pool pl(1);
    try {
      std::string strConnectString = p.string();
      SYSTEM_LOGGER_INFO("connect to : %s", p.string().c_str());

      soci::session& sql = pl.at(0);
      sql.open(soci::sqlite3, strConnectString);

    } catch (std::exception& e) {
      SYSTEM_LOGGER_INFO("connect to backdb failed.");
    }
    sTagGetScenarioListParam param;

    // 1. select preset_scenesets.sqlite
    int ret = doScenarioList(param, scenarios, pl);

    if (ret != 0) {
      SYSTEM_LOGGER_INFO("Select back db error failed, ignore it");
      return -1;
    }

    SYSTEM_LOGGER_INFO("Total old %d scenarios. ", scenarios.Entities().size());

    sTagGetScenarioSetListParam param_sets;

    ret = doScenarioSetList(param_sets, ssets, pl);
    if (ret != 0) {
      SYSTEM_LOGGER_ERROR("Error select * preset scenesets db.");
      return -1;
    }
    pl.at(0).close();
    SYSTEM_LOGGER_INFO("close old connection pool for backdb");
  } else {
    SYSTEM_LOGGER_INFO("no dbback=%s", p.string().c_str());
  }
  return 0;
}

int CDatabase::dbPreset(CScenarioEntities& scenarios, CScenarioSets& ssets) {
  std::string strResouceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path p = strResouceDir;
  p /= "preset_scenesets.sqlite";

  boost::filesystem::path p_new = strResouceDir + "/preset_scenesets.sqlite.bak";

  if (boost::filesystem::exists(p)) {
    soci::connection_pool pl(1);

    try {
      std::string strConnectString = p.string();
      SYSTEM_LOGGER_INFO("connect to : %s", p.string().c_str());

      soci::session& sql = pl.at(0);
      sql.open(soci::sqlite3, strConnectString);

    } catch (std::exception& e) {
      SYSTEM_LOGGER_INFO("connect to backdb failed.");
    }

    sTagGetScenarioListParam param;

    // 1. select preset_scenesets.sqlite
    int ret = doScenarioList(param, scenarios, pl);

    if (ret != 0) {
      SYSTEM_LOGGER_INFO("Select back db error failed, ignore it");
      return -1;
    }

    SYSTEM_LOGGER_INFO("Total old %d scenarios. ", scenarios.Entities().size());

    sTagGetScenarioSetListParam param_sets;

    ret = doScenarioSetList(param_sets, ssets, pl);
    if (ret != 0) {
      SYSTEM_LOGGER_ERROR("Error select * preset scenesets db.");
      return -1;
    }

    pl.at(0).close();
    SYSTEM_LOGGER_INFO("close old connection pool for backdb");

    boost::filesystem::rename(p, p_new);
    SYSTEM_LOGGER_INFO("rename preset_scene.sqlite to bak file.");
  } else {
    SYSTEM_LOGGER_INFO("no presetdb=%s", p.string().c_str());
  }
  return 0;
}

int CDatabase::PresetDatabase() {
  std::string strResouceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path p = strResouceDir + "/preset_scenesets.sqlite";
  boost::filesystem::path p_new = strResouceDir + "/preset_scenesets.sqlite.bak";
  boost::filesystem::path p_old = strResouceDir + "/../../data/old_scenario/simu_scene.sqlite";
  // get old_scenario sceneset

  // p /= ;
  if (boost::filesystem::exists(p)) {
    SetIsPreset(true);
    SYSTEM_LOGGER_INFO("preset sceneario found: #%s#", p.string().c_str());
    soci::connection_pool pl(1);
    try {
      std::string strConnectString = p.string();
      SYSTEM_LOGGER_INFO("connect to : %s", p.string().c_str());
      soci::session& sql = pl.at(0);
      sql.open(soci::sqlite3, strConnectString);
    } catch (std::exception& e) {
      SYSTEM_LOGGER_INFO("connect to preset scenesets failed.");
    }
    CScenarioEntities scenarios;
    sTagGetScenarioListParam param;
    // 1. select preset_scenesets.sqlite
    int ret = doScenarioList(param, scenarios, pl);
    if (ret != 0) {
      SYSTEM_LOGGER_INFO("Select preset_scenesets.sqlite failed, ignore it");
      return -1;
    }
    SYSTEM_LOGGER_INFO("Total %d scenarios. ", scenarios.Entities().size());
    // 2. update into simu_scene.sqlite
    std::string strTemp;
    for (sTagEntityScenario& scene : scenarios.Entities()) {
      std::string oPath = scene.m_strPath;
      std::vector<std::string> strs;
      iter_split(strs, oPath, boost::algorithm::first_finder("scenario"));
      std::string newPath = strResouceDir;
      if (strs.size() == 2) {
        newPath += strs[1];

      } else {
        SYSTEM_LOGGER_INFO("Error parse oPath %s", oPath.c_str());
        continue;
      }
      std::replace(newPath.begin(), newPath.end(), '\\', '/');
      scene.m_strPath = newPath;
    }
    ClearDB();
    ret = BatchInsertScenario(scenarios.Entities());
    if (ret != 0) {
      SYSTEM_LOGGER_ERROR("Error batch insert scenes db.");
      return -1;
    }

    SYSTEM_LOGGER_INFO("insert  sceneario done for simu_scene.sqlite.");
    // 3. change local simu_database name
    // auto par_path = p.parent_path();
    // par_path /= "simu_scene.sqlite";
    // SYSTEM_LOGGER_INFO("after r")

    sTagGetScenarioSetListParam param_sets;
    CScenarioSets ssets;
    ret = doScenarioSetList(param_sets, ssets, pl);
    if (ret != 0) {
      SYSTEM_LOGGER_ERROR("Error select * preset scenesets db.");
      return -1;
    }

    for (auto s : ssets.Sets()) {
      sTagCreateScenarioSetParam param;
      param.m_nID = s.m_nID;
      param.m_strName = s.m_strName;
      param.m_strLabel = s.m_strLabel;
      param.m_strScenes = s.m_strScenes;
      int ret = InsertScenarioSet(param);
      if (ret != 0) {
        SYSTEM_LOGGER_ERROR("Error insert preset scenesets db.");
        return -1;
      }
    }

    pl.at(0).close();

    // do old scenrio
    soci::connection_pool pl2(1);
    try {
      std::string strConnectStringOld = p_old.string();
      SYSTEM_LOGGER_INFO("connect to : %s", p_old.string().c_str());
      soci::session& sql2 = pl2.at(0);
      sql2.open(soci::sqlite3, strConnectStringOld);
    } catch (std::exception& e) {
      SYSTEM_LOGGER_INFO("connect to old scenesets failed.");
    }
    sTagGetScenarioSetListParam param_sets2;
    CScenarioSets ssets2;
    ret = doScenarioSetList(param_sets2, ssets2, pl2);

    CScenarioEntities scenarios2;
    sTagGetScenarioListParam param2;
    // 1. select preset_scenesets.sqlite
    ret = doScenarioList(param2, scenarios2, pl);

    for (auto s : ssets2.Sets()) {
      sTagCreateScenarioSetParam param;
      param.m_nID = s.m_nID;
      param.m_strName = s.m_strName;
      param.m_strLabel = s.m_strLabel;
      param.m_strScenes = s.m_strScenes;
      int ret = InsertScenarioSet(param);
      if (ret != 0) {
        SYSTEM_LOGGER_ERROR("Error insert preset scenesets db.");
        return -1;
      }
    }
    SYSTEM_LOGGER_INFO("close old connection pool for preset_scene.sqlite");

    // move preset db to backup.
    boost::filesystem::rename(p, p_new);
    SYSTEM_LOGGER_INFO("rename preset_scene.sqlite to bak file.");

    return 0;
  } else {
    SYSTEM_LOGGER_INFO("preset scenesets db not found.");
    return -1;
  }
}

int CDatabase::CreateTables() {
  try {
    soci::session sql(pool);
    // table scene
    sql << "create table if not exists " + s_SCENARIO +
               "("
               "id integer primary key autoincrement, "
               // "name varchar(256) unique, "
               "name varchar(256) , "
               "map varchar(256) , "
               "path varchar(1024), "
               "info varchar(256), "
               "label varchar(1024), "
               "sets varchar(1024), "
               "content text, "
               "type char(4),"
               "traffictype char(6)); ";
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
    SYSTEM_LOGGER_ERROR("%s", e.what());
    std::cerr << "Error: " << e.what() << "\n";
  }

  return 0;
}

int CDatabase::ClearDB() {
  DeleteTables();
  CreateTables();

  return 0;
}

bool CDatabase::GetIsPreset() { return m_isPreset; }

void CDatabase::SetIsPreset(bool ispresent) { m_isPreset = ispresent; }

int CDatabase::DBVersion(sTagDBVersion& ver) {
  ver.nMajor = 1;
  ver.nMinor = 0;
  ver.nRevision = 0;

  sTagDBVersion curVer = ver;

  try {
    if (TableExists(s_VERSION)) {
      soci::session sql(pool);
      std::string strSql = "select * from " + s_VERSION;
      soci::rowset<soci::row> rs = sql.prepare << strSql;
      for (auto& r : rs) {
        int nMajor = r.get<int>("major");
        int nMinor = r.get<int>("minor");
        int nRevision = r.get<int>("revision");

        // major
        if (nMajor > curVer.nMajor) {
          curVer.nMajor = nMajor;
          curVer.nMinor = nMinor;
          curVer.nRevision = nRevision;

        } else if (nMajor == curVer.nMajor) {
          // minor
          if (nMinor > curVer.nMinor) {
            curVer.nMajor = nMajor;
            curVer.nMinor = nMinor;
            curVer.nRevision = nRevision;

          } else if (nMinor == curVer.nMinor) {
            // revision
            if (nRevision > curVer.nRevision) {
              curVer.nMajor = nMajor;
              curVer.nMinor = nMinor;
              curVer.nRevision = nRevision;
            }
          }
        }
      }

      ver = curVer;
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
    if (!TableExists(s_VERSION)) {
      int nRet = CreateVersionTable();
      if (nRet != 0) {
        return nRet;
      }
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
        // scenario.m_strSets = "," + strScenarioID + ",";
        scenario.m_strSets = ",1,";
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

bool CDatabase::ScenarioExist(const char* strFile, const char* strExt) {
  try {
    soci::session sql(pool);
    std::string strName = strFile;
    std::string strTempExt = strExt;
    sql << "select * from " + s_SCENARIO + " where name=:strName AND type=:strTempExt", soci::use(strName),
        soci::use(strTempExt);

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
    SYSTEM_LOGGER_ERROR("Scenario Count error : ", e.what());
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

int CDatabase::doScenarioList(sTagGetScenarioListParam& param, CScenarioEntities& scenarios,
                              soci::connection_pool& pl) {
  scenarios.Clear();
  try {
    soci::session sql(pl);
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
      e.m_strType = row.get<std::string>(8);
      e.m_strTrafficType = row.get<std::string>(9);
      ses.emplace_back(e);
    }

    SYSTEM_LOGGER_DEBUG("Get Scenario List. count : %d ", ses.size());

    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("Get Scenario List error. %s ", e.what());
  }
  return -1;
}

int CDatabase::ScenarioList(sTagGetScenarioListParam& param, CScenarioEntities& scenarios) {
  return doScenarioList(param, scenarios, pool);
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
    e.m_strType = row.get<std::string>(8);
    e.m_strTrafficType = row.get<std::string>(9);
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
    e.m_strType = row.get<std::string>(8);
    e.m_strTrafficType = row.get<std::string>(9);
    ses.emplace_back(e);
  }

  SYSTEM_LOGGER_DEBUG("Get Scenario List. count : %d ", ses.size());

  return 0;
}

int CDatabase::Scenario(sTagEntityScenario& scene) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO + " where id=:nID", soci::use(scene.m_nID), soci::into(scene.m_nID),
      soci::into(scene.m_strName), soci::into(scene.m_strMap), soci::into(scene.m_strPath), soci::into(scene.m_strInfo),
      soci::into(scene.m_strLabel), soci::into(scene.m_strSets), soci::into(scene.m_strContent),
      soci::into(scene.m_strType), soci::into(scene.m_strTrafficType);

  SYSTEM_LOGGER_DEBUG("Scenario for %ld ", scene.m_nID);

  return 0;
}

int CDatabase::ScenarioByName(sTagEntityScenario& scene) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO + " where name =:strName", soci::use(scene.m_strName), soci::into(scene.m_nID),
      soci::into(scene.m_strName), soci::into(scene.m_strMap), soci::into(scene.m_strPath), soci::into(scene.m_strInfo),
      soci::into(scene.m_strLabel), soci::into(scene.m_strSets), soci::into(scene.m_strContent),
      soci::into(scene.m_strType), soci::into(scene.m_strTrafficType);

  SYSTEM_LOGGER_DEBUG("ScenarioByName for %s ", scene.m_strName.c_str());

  return 0;
}

int CDatabase::BatchInsertScenario(std::vector<sTagEntityScenario>& sceneV) {
  std::vector<int64_t> int64_tId;
  std::vector<std::string> strName;         // scene.m_strName;
  std::vector<std::string> strMap;          // scene.m_strMap;
  std::vector<std::string> strPath;         // scene.m_strPath;
  std::vector<std::string> strInfo;         // scene.m_strInfo;
  std::vector<std::string> strLabel;        // scene.m_strLabel;
  std::vector<std::string> strSets;         // scene.m_strSets;
  std::vector<std::string> strContent;      // scene.m_strContent;
  std::vector<std::string> strType;         // scene.m_strType;
  std::vector<std::string> strTrafficType;  // scene.m_strTrafficType;
  int i = 1;
  for (sTagEntityScenario& scene : sceneV) {
    int64_tId.push_back(i++);
    strName.push_back(scene.m_strName);
    strMap.push_back(scene.m_strMap);
    std::replace(scene.m_strPath.begin(), scene.m_strPath.end(), '\\', '/');
    strPath.push_back(scene.m_strPath);
    strInfo.push_back(scene.m_strInfo);
    strLabel.push_back(scene.m_strLabel);
    strSets.push_back(scene.m_strSets);
    strContent.push_back(scene.m_strContent);
    strType.push_back(scene.m_strType);
    strTrafficType.push_back(scene.m_strTrafficType);
  }
  try {
    size_t pos;
    soci::session sql(pool);
    soci::transaction tr(sql);
    for (unsigned int i = 0; i < sceneV.size(); i++) {
      std::string values = "VALUES ";
      std::stringstream ss;
      ss << "(" << int64_tId[i] << ", "
         << "'" << strName[i] << "',"
         << "'" << strMap[i] << "',"
         << "'" << strPath[i] << "',"
         << "'" << strInfo[i] << "',"
         << "'" << strLabel[i] << "',"
         << "'" << strSets[i] << "',"
         << "'" << strContent[i] << "',"
         << "'" << strType[i] << "',"
         << "'" << strTrafficType[i] << "')";
      values += ss.str();
      sql << "insert into scenes ( id, name, map, path, info, label, sets, content, "
             "type , traffictype ) " +
                 values;
    }
    tr.commit();
    // sql << "insert into scenes(id, name, map, path, info, label,sets, content, "
  } catch (const std::exception& e) {
    SYSTEM_LOGGER_ERROR("Error: %s", e.what());
  }
  SYSTEM_LOGGER_DEBUG("BatchInsertScenario done");
  return 0;
}
int CDatabase::InsertScenario(sTagEntityScenario& scene) {
  std::string strName = scene.m_strName;
  std::string strMap = scene.m_strMap;
  std::replace(scene.m_strPath.begin(), scene.m_strPath.end(), '\\', '/');
  std::string strPath = scene.m_strPath;
  std::string strInfo = scene.m_strInfo;
  std::string strLabel = scene.m_strLabel;
  std::string strSets = scene.m_strSets;
  std::string strContent = scene.m_strContent;
  std::string strType = scene.m_strType;
  std::string strTrafficType = scene.m_strTrafficType;
  SYSTEM_LOGGER_DEBUG("InsertScenario %s, %s, %s", strName.c_str(), strMap.c_str(), strPath.c_str());

  soci::session sql(pool);

  if (scene.m_nID < 1) {
    sql << "insert into scenes(name, map, path, info, label,sets, content, "
           "type,traffictype)"
           " values(:name, :map, :path, :info, :label, :sets, :content, "
           ":type, :traffictype)",
        soci::use(strName), soci::use(strMap), soci::use(strPath), soci::use(strInfo), soci::use(strLabel),
        soci::use(strSets), soci::use(strContent), soci::use(strType), soci::use(strTrafficType);
  } else {
    sql << "insert into scenes(id, name, map, path, info, label,sets, "
           "content, "
           "type,traffictype)"
           " values(:id, :name, :map, :path, :info, :label, :sets, "
           ":content, "
           ":type,:traffictype)",
        soci::use(scene.m_nID), soci::use(strName), soci::use(strMap), soci::use(strPath), soci::use(strInfo),
        soci::use(strLabel), soci::use(strSets), soci::use(strContent), soci::use(strType), soci::use(strTrafficType);
  }

  long long llID = -1;  // NOLINT
  bool bRet = sql.get_last_insert_id(s_SCENARIO, llID);
  if (bRet) {
    scene.m_nID = llID;
    return 0;
  } else {
    scene.m_nID = -1;
  }

  return -1;
}

int CDatabase::InsertScenarios(std::vector<sTagEntityScenario>& scenes) {
  try {
    soci::session sql(pool);
    soci::transaction tr(sql);

    for (auto scene : scenes) {
      std::string strName = scene.m_strName;
      std::string strMap = scene.m_strMap;
      std::string strPath = scene.m_strPath;
      std::string strInfo = scene.m_strInfo;
      std::string strLabel = scene.m_strLabel;
      std::string strSets = scene.m_strSets;
      std::string strContent = scene.m_strContent;
      std::string strType = scene.m_strType;
      std::string strTrafficType = scene.m_strTrafficType;
      SYSTEM_LOGGER_DEBUG("InsertScenario %s, %s, %s", strName.c_str(), strMap.c_str(), strPath.c_str());

      sql << "insert into scenes(name, map, path, info, label,sets, "
             "content, type ,traffictype)"
             " values(:name, :map, :path, :info, :label, :sets, "
             ":content, :type, :traffictype)",
          soci::use(strName), soci::use(strMap), soci::use(strPath), soci::use(strInfo), soci::use(strLabel),
          soci::use(strSets), soci::use(strContent), soci::use(strType), soci::use(strTrafficType);
    }

    tr.commit();
    return 0;
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("InsertScenar");
    return -1;
  }
}

int CDatabase::doUpdateScenario(sTagEntityScenario& scene, soci::connection_pool& pl) {
  int64_t nID = scene.m_nID;
  std::string strName = scene.m_strName;
  std::string strMap = scene.m_strMap;
  std::string strPath = scene.m_strPath;
  std::string strInfo = scene.m_strInfo;
  std::string strLabel = scene.m_strLabel;
  std::string strSets = scene.m_strSets;
  std::string strContent = scene.m_strContent;
  std::string strType = scene.m_strType;
  std::string strTrafficType = scene.m_strTrafficType;
  // SYSTEM_LOGGER_DEBUG("UpdateScenario %d, %s, %s, %s", nID,
  // strName.c_str(), strLabel.c_str(), strContent.c_str());
  SYSTEM_LOGGER_DEBUG("UpdateScenario %d, %s, %s", nID, strName.c_str(), strLabel.c_str());

  soci::session sql(pl);
  sql << "update " + s_SCENARIO +
             " set name=:strName, map=:strMap, path=:strPath, "
             " info=:strInfo, label=:strLabel, sets=:strSets, "
             "content=:strContent, type=:strType ,traffictype=:strTrafficType"
             " where id=:nID ",
      soci::use(strName), soci::use(strMap), soci::use(strPath), soci::use(strInfo), soci::use(strLabel),
      soci::use(strSets), soci::use(strContent), soci::use(strType), soci::use(strTrafficType), soci::use(nID);

  return 0;
}

int CDatabase::UpdateScenario(sTagEntityScenario& scene) { return doUpdateScenario(scene, pool); }

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

bool CDatabase::ScenarioSetExist(std::string strSetName) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO_SET + " where name = :strSetName", soci::use(strSetName);

  bool bRet = sql.got_data();

  SYSTEM_LOGGER_DEBUG("Scenario Set Exist for %s  is %d", strSetName, bRet);

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

int CDatabase::ScenarioSetByName(sTagEntityScenarioSet& scenarioSet) {
  soci::session sql(pool);
  sql << "select * from " + s_SCENARIO_SET + " where name=:strName", soci::use(scenarioSet.m_strName),
      soci::into(scenarioSet.m_nID), soci::into(scenarioSet.m_strName), soci::into(scenarioSet.m_strLabel),
      soci::into(scenarioSet.m_strScenes);

  SYSTEM_LOGGER_DEBUG("ScenarioSet for %s", scenarioSet.m_strName);

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
  return doScenarioSetList(param, sets, pool);
}
int CDatabase::doScenarioSetList(sTagGetScenarioSetListParam& param, CScenarioSets& sets, soci::connection_pool& pl) {
  sets.Clear();
  try {
    soci::session sql(pl);
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
  SYSTEM_LOGGER_DEBUG("NewSceneSet %d %s, %s, %s", param.m_nID, strName.c_str(), strLabel.c_str(), strContent.c_str());
  soci::session sql1(pool);
  int nId;
  sql1 << "select id from scenesets where name=:name", soci::use(param.m_strName), soci::into(nId);
  if (sql1.got_data()) {
    param.m_nID = nId;
    return 0;
  }
  soci::session sql(pool);
  if (param.m_nID == 0) {
    sql << "insert into " + s_SCENARIO_SET +
               " (name, label, content) "
               " values(:name, :label, :content) ",
        soci::use(param.m_strName), soci::use(param.m_strLabel), soci::use(param.m_strScenes);
  } else {
    sql << "insert into " + s_SCENARIO_SET +
               " (id, name, label, content) "
               " values(:id, :name, :label, :content) ",
        soci::use(param.m_nID), soci::use(param.m_strName), soci::use(param.m_strLabel), soci::use(param.m_strScenes);
  }

  long long llID = -1;  // NOLINT
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
    std::vector<std::string> ids;
    std::vector<std::string> ids_out;

    boost::algorithm::split(ids, strIDs, boost::is_any_of(", "), boost::algorithm::token_compress_on);
    auto itr = std::find(ids.begin(), ids.end(), strID);
    if (itr == ids.end()) {
      strIDs += strID;
      strIDs += ",";
    }
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
