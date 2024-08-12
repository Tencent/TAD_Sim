/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "synchronizer.h"
#include <json/reader.h>
#include <json/value.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>
#include <iostream>
#include "../engine/config.h"
#include "../engine/util/scene_util.h"
#include "../xml_parser/entity/parser.h"
#include "../xml_parser/entity/scenario_set_params.h"
#include "../xml_parser/entity/simulation.h"
#include "./db.h"
#include "./entity/scenario.h"
#include "./entity/scenario_set.h"
#include "boost/filesystem/fstream.hpp"
#include "common/log/system_logger.h"
#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"

CSynchronizer::CSynchronizer() {}

CSynchronizer& CSynchronizer::Instance() {
  static CSynchronizer sync;

  return sync;
}

int CSynchronizer::SyncDBToDisk() {
  SYSTEM_LOGGER_INFO("SyncDBToDisk start!");

  SYSTEM_LOGGER_INFO("SyncDBToDisk end!");

  return 0;
}

int CSynchronizer::SyncDiskToDB() {
  SYSTEM_LOGGER_INFO("SyncDiskToDB start!");

  CDatabase::Instance().ClearDB();

  FilePathes files;
  GetFileList(files);

  FilePathes::iterator itr = files.begin();
  for (; itr != files.end(); ++itr) {
    OneSimFileToDB((*itr));
  }

  CreateDefaultScenarioSet();

  SYSTEM_LOGGER_INFO("SyncDiskToDB end!");

  return 0;
}

int CSynchronizer::SyncDiskToDB2() {
  SYSTEM_LOGGER_INFO("SyncDiskToDB2 start!");

  FilePathes files;
  if (CDatabase::Instance().GetIsPreset()) {
    GetFileList(files, false);
  } else {
    CDatabase::Instance().ClearDB();
    GetFileList(files);
  }
  std::vector<sTagEntityScenario> scenarios;
  sTagEntityScenario scenario;
  FilePathes::iterator itr = files.begin();
  for (; itr != files.end(); ++itr) {
    scenario.Reset();
    ExtractSceneInfo((*itr), scenario);
    scenarios.push_back(scenario);
  }
  //
  CScenarioEntities scenarios_db, scenarios_preset;
  CScenarioSets ssets_db, ssets_preset;
  CDatabase::Instance().dbBack(scenarios_db, ssets_db);
  // from old scence to update other info
  for (auto& itScene : scenarios) {
    for (auto itOldScene : scenarios_db.Entities())
      if (itScene.m_strName == itOldScene.m_strName) {
        itScene.m_strTrafficType = itOldScene.m_strTrafficType;
        break;
      }
  }

  int nRet = CDatabase::Instance().InsertScenarios(scenarios);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("SyncDiskToDB2 end! failed!");
    return -1;
  }

  if (!CDatabase::Instance().GetIsPreset()) {
    CreateDefaultScenarioSet();
  } else {
    // set default
    sTagGetScenarioListParam param1;
    CScenarioEntities scenarios1;
    CDatabase::Instance().ScenarioList(param1, scenarios1);
    std::string strcontent = ",";
    std::map<int, bool> m_mapvalue;
    for (auto it : scenarios1.Entities()) {
      m_mapvalue[it.m_nID] = true;
    }
    //
    sTagGetScenarioSetListParam param2;
    CScenarioSets sets2;
    CDatabase::Instance().ScenarioSetList(param2, sets2);
    for (auto& it : m_mapvalue) {
      int id = it.first;
      for (auto ita : sets2.Sets()) {
        if (ita.m_strScenes.find(std::string(",").append(std::to_string(id).append(","))) != std::string::npos) {
          it.second = false;
          break;
        }
      }
    }
    for (auto it : m_mapvalue) {
      if (it.second == true) {
        strcontent.append(std::to_string(it.first)).append(",");
      }
    }

    SYSTEM_LOGGER_INFO("default scene = %s!", strcontent.c_str());
    // default create done.
    sTagUpdateScenarioSetParam param;
    param.m_strName = "未分组场景";
    param.m_strLabel = "默认场景";
    param.m_strScenes = strcontent;
    param.m_nID = 1;
    CDatabase::Instance().UpdateScenarioSet(param);
  }
  handleDbback(scenarios_db, ssets_db);
  // CDatabase::Instance().dbPreset(scenarios_preset,ssets_preset);
  // handleDbPreset(scenarios_preset,ssets_preset);
  SYSTEM_LOGGER_INFO("SyncDiskToDB2 end!");
  return 0;
}

int CSynchronizer::handleDbback(CScenarioEntities& scenarios, CScenarioSets& ssets) {
  if (scenarios.Entities().size() > 0 && ssets.Sets().size() > 0) {
    sTagGetScenarioListParam paramNew;
    CScenarioEntities scenariosNew;

    CDatabase::Instance().ScenarioList(paramNew, scenariosNew);
    std::map<std::string, sTagEntityScenario> mapNameOldScene, mapNameNewScene;
    std::map<int, sTagEntityScenario> mapIDOldScene;

    for (sTagEntityScenario& sceneOld : scenarios.Entities()) {
      mapNameOldScene[sceneOld.m_strName] = sceneOld;
      mapIDOldScene[sceneOld.m_nID] = sceneOld;
    }
    for (sTagEntityScenario& sceneNew : scenariosNew.Entities()) {
      mapNameNewScene[sceneNew.m_strName] = sceneNew;
      // mapIDNewScene[sceneNew.m_nID]=sceneNew;
    }
    for (auto oldSet : ssets.Sets()) {
      if (oldSet.m_nID == 1) /*默认场景前面已经处理过了*/
        continue;
      std::string strSetID = std::to_string(oldSet.m_nID);
      sTagCreateScenarioSetParam param;
      param.m_nID = 0;
      param.m_strName = oldSet.m_strName;
      param.m_strLabel = oldSet.m_strLabel;

      std::set<std::string> oldSceneIDs, newSceneIDs;
      CDatabase::Instance().ConvertToIDs(oldSet.m_strScenes, oldSceneIDs);

      for (auto strSceneID : oldSceneIDs) {
        int nOldSceneID = atoi(strSceneID.c_str());
        if (mapIDOldScene.find(nOldSceneID) != mapIDOldScene.end()) {
          sTagEntityScenario& oldScenario = mapIDOldScene[nOldSceneID];
          if (mapNameNewScene.find(oldScenario.m_strName) != mapNameNewScene.end()) {
            sTagEntityScenario& newScenario = mapNameNewScene[oldScenario.m_strName];
            SYSTEM_LOGGER_INFO("find dbback scene set ..setid=%s,scenename=%s,newsceneid=%d", strSetID.c_str(),
                               oldScenario.m_strName.c_str(), newScenario.m_nID);
            CDatabase::Instance().AddIDToIDs(strSetID, newScenario.m_strSets);
            std::string strNewScendid = std::to_string(newScenario.m_nID);
            CDatabase::Instance().AddIDToIDs(strNewScendid, param.m_strScenes);
          }
        }
      }
      if (param.m_strScenes.size() > 1) {
        sTagEntityScenarioSet scenarioSet;
        scenarioSet.m_nID = param.m_nID;
        scenarioSet.m_strName = param.m_strName;
        scenarioSet.m_strLabel = param.m_strLabel;
        scenarioSet.m_strScenes = param.m_strScenes;
        int nRet = CDatabase::Instance().CreateOneScenarioSet(scenarioSet);
        if (nRet != 0) {
          SYSTEM_LOGGER_ERROR("1111Error insert  scenesets db.");
          continue;
        }
      }
    }
  }
  return 0;
}
int CSynchronizer::handleDbPreset(CScenarioEntities& scenarios, CScenarioSets& ssets) {
  if (scenarios.Entities().size() > 0 && ssets.Sets().size() > 0) {
    sTagGetScenarioListParam paramNew;
    CScenarioEntities scenariosNew;

    CDatabase::Instance().ScenarioList(paramNew, scenariosNew);
    std::map<std::string, sTagEntityScenario> mapNameOldScene, mapNameNewScene;
    std::map<int, sTagEntityScenario> mapIDOldScene;

    for (sTagEntityScenario& sceneOld : scenarios.Entities()) {
      mapNameOldScene[sceneOld.m_strName] = sceneOld;
      mapIDOldScene[sceneOld.m_nID] = sceneOld;
    }
    for (sTagEntityScenario& sceneNew : scenariosNew.Entities()) {
      mapNameNewScene[sceneNew.m_strName] = sceneNew;
      // mapIDNewScene[sceneNew.m_nID]=sceneNew;
    }
    for (auto oldSet : ssets.Sets()) {
      if (oldSet.m_nID == 1) /*默认场景前面已经处理过了*/
        continue;
      std::string strSetID = std::to_string(oldSet.m_nID);
      sTagCreateScenarioSetParam param;
      param.m_nID = 0;
      param.m_strName = oldSet.m_strName;
      param.m_strLabel = oldSet.m_strLabel;

      std::set<std::string> oldSceneIDs, newSceneIDs;
      CDatabase::Instance().ConvertToIDs(oldSet.m_strScenes, oldSceneIDs);

      for (auto strSceneID : oldSceneIDs) {
        int nOldSceneID = atoi(strSceneID.c_str());
        if (mapIDOldScene.find(nOldSceneID) != mapIDOldScene.end()) {
          sTagEntityScenario& oldScenario = mapIDOldScene[nOldSceneID];
          if (mapNameNewScene.find(oldScenario.m_strName) != mapNameNewScene.end()) {
            sTagEntityScenario& newScenario = mapNameNewScene[oldScenario.m_strName];
            SYSTEM_LOGGER_INFO("find dbback scene set ..setid=%s,scenename=%s,newsceneid=%d", strSetID.c_str(),
                               oldScenario.m_strName.c_str(), newScenario.m_nID);
            CDatabase::Instance().AddIDToIDs(strSetID, newScenario.m_strSets);
            std::string strNewScendid = std::to_string(newScenario.m_nID);
            CDatabase::Instance().AddIDToIDs(strNewScendid, param.m_strScenes);
          }
        }
      }
      if (param.m_strScenes.size() > 1) {
        if (!CDatabase::Instance().ScenarioSetExist(param.m_strName)) {
          sTagEntityScenarioSet scenarioSet;
          scenarioSet.m_nID = param.m_nID;
          scenarioSet.m_strName = param.m_strName;
          scenarioSet.m_strLabel = param.m_strLabel;
          scenarioSet.m_strScenes = param.m_strScenes;
          int nRet = CDatabase::Instance().CreateOneScenarioSet(scenarioSet);
          if (nRet != 0) {
            SYSTEM_LOGGER_ERROR("1111Error insert  scenesets db.");
            continue;
          }
        } else {
          SYSTEM_LOGGER_INFO("old set name=%s exists", param.m_strName.c_str());
        }
      }
    }
  }
  return 0;
}

int CSynchronizer::loadDataHadmapConfig() {
  std::set<string> setHadmapName;
  boost::filesystem::path pHadmapData = CEngineConfig::Instance().getHadmapDataDir();
  if (boost::filesystem::exists(pHadmapData)) {
    boost::filesystem::directory_iterator itr_end;
    boost::filesystem::directory_iterator itr(pHadmapData);

    for (; itr != itr_end; ++itr) {
      std::string strName = (*itr).path().string();
      std::string strExt = (*itr).path().extension().string();
      if ((!boost::filesystem::is_directory(*itr)) &&
          (boost::algorithm::iequals(strExt, ".sqlite") || boost::algorithm::iequals(strExt, ".xodr") ||
           boost::algorithm::iequals(strExt, ".xml"))) {
        setHadmapName.insert((*itr).path().filename().string());
      }
    }
  }
  SYSTEM_LOGGER_INFO("loadDataHadmapConfig size=%d", setHadmapName.size());

  for (auto strNewName : setHadmapName) {
    tagHadmapAttr* pHA = CEngineConfig::Instance().HadmapAttr(strNewName);
    if (!pHA) {
      SYSTEM_LOGGER_INFO("load datahadmap=%s start", strNewName.c_str());

      CMapFile mapfile;
      boost::filesystem::path hadmapDestPath = CEngineConfig::Instance().getHadmapDataDir();
      std::string strDestFullName = CSceneUtil::HadmapFullName(strNewName.c_str());
      hadmapDestPath /= strDestFullName;
      int nRet = 0;
      try {
        nRet = mapfile.ParseMapV2(hadmapDestPath.string().c_str(), false);
      } catch (const std::exception& e) {
        SYSTEM_LOGGER_ERROR("open hadmap %s error : %s!", hadmapDestPath.string().c_str(), e.what());
        continue;
      }
      if (nRet != 0) {
        SYSTEM_LOGGER_ERROR("open hadmap %s error", hadmapDestPath.string().c_str());
      } else {
        int nIndex = CEngineConfig::Instance().HadmapIndex(strNewName.c_str());
        // chanage attr 1
        tagHadmapAttr attr;
        attr.m_name = strNewName;
        attr.m_unRealLevelIndex = nIndex;
        attr.m_strUnrealLevelIndex = std::to_string(nIndex);
        attr.m_dRefLon = mapfile.m_center[0];
        attr.m_dRefLat = mapfile.m_center[1];
        attr.m_dRefAlt = mapfile.m_center[2];
        attr.m_nPreset = 0;
        attr.m_strPreset = "0";
        std::string str;
        std::stringstream ss1, ss2, ss3;
        ss1 << std::fixed << std::setprecision(8) << attr.m_dRefAlt;
        attr.m_strRefAlt = ss1.str();
        attr.m_dRefAlt = stod(attr.m_strRefAlt);

        ss2 << std::fixed << std::setprecision(8) << attr.m_dRefLon;
        attr.m_strRefLon = ss2.str();
        attr.m_dRefLon = stod(attr.m_strRefLon);

        ss3 << std::fixed << std::setprecision(8) << attr.m_dRefLat;
        attr.m_strRefLat = ss3.str();
        attr.m_dRefLat = stod(attr.m_strRefLat);

        CEngineConfig::Instance().UpdateHadmapAttr(strNewName, attr);
      }
      SYSTEM_LOGGER_INFO("load datahadmap=%s end", strNewName.c_str());
    } else {
      SYSTEM_LOGGER_INFO("load datahadmap=%s cache exists", strNewName.c_str());
    }
  }

  return 0;
}

int CSynchronizer::GetFileList(FilePathes& files, bool isPreset) {
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  p /= "scene";

  // 获取文件
  boost::filesystem::path scenePath = p;
  int num = 0;
  if (isPreset) {
    num = 2;
  } else {
    num = 1;
    scenePath = CEngineConfig::Instance().getSceneDataDir();
  }
  // 路径存在
  for (int i = 0; i < num; i++) {
    SYSTEM_LOGGER_INFO("GetFileList path=%s", scenePath.string().c_str());
    if (boost::filesystem::exists(scenePath)) {
      boost::filesystem::directory_iterator itr_end;
      boost::filesystem::directory_iterator itr(scenePath);
      for (; itr != itr_end; ++itr) {
        std::string strName = (*itr).path().string();
        if (strName.find("_convert.sim") != std::string::npos) continue;
        std::string strFileName = (*itr).path().stem().string();
        std::string strExt = (*itr).path().extension().string();
        if ((!boost::filesystem::is_directory(*itr)) && CSceneUtil::ValidFileFormat(strExt.c_str(), ".sim,.xosc")) {
          files.push_back((*itr).path().string());
        }
      }
    }
    scenePath = CEngineConfig::Instance().getSceneDataDir();
  }

  // 排序
  std::sort(files.begin(), files.end(), [](std::string aName, std::string bName) {
    boost::to_lower(aName);
    boost::to_lower(bName);
    if (aName < bName) {
      return true;
    }
    return false;
  });

  return 0;
}

int CSynchronizer::OneSimFileToDB(std::string& strPath) {
  boost::filesystem::path p = strPath;

  sTagEntityScenario scenario;
  scenario.m_strName = p.stem().string();
  if (CDatabase::Instance().ScenarioExist(scenario.m_strName.c_str())) {
    return 0;
  }

  std::string strExt = p.extension().string();
  if (boost::iequals(strExt, ".sim")) {
    CSimulation sim;
    sim.Parse(strPath.c_str(), false);
    scenario.m_strType = "sim";
    boost::filesystem::path mapPath = sim.m_mapFile.m_strMapfile;
    scenario.m_strMap = mapPath.filename().string();
    scenario.m_strInfo = sim.m_strInfo;

  } else {
    XOSCReader_1_0_v4 reader;
    sTagSimuTraffic infos;
    reader.ParseXOSC(strPath.c_str(), infos);
    scenario.m_strType = "xosc";
    scenario.m_strMap = infos.m_simulation.m_mapFile.m_strMapfile;
    scenario.m_strInfo = "";
  }
  scenario.m_strPath = strPath;

  scenario.m_strLabel = "";
  scenario.m_strSets = ",1,";
  scenario.m_strContent = "";

  CDatabase::Instance().InsertScenario(scenario);

  return 0;
}

int CSynchronizer::ExtractSceneInfo(std::string& strPath, sTagEntityScenario& scenario) {
  boost::filesystem::path p = strPath;

  scenario.m_strName = p.stem().string();
  scenario.m_strType = p.extension().string();
  if (CDatabase::Instance().ScenarioExist(scenario.m_strName.c_str(), scenario.m_strType.c_str())) {
    return 0;
  }

  std::string strExt = p.extension().string();
  if (boost::iequals(strExt, ".sim")) {
    CSimulation sim;
    sim.Parse(strPath.c_str(), false);
    scenario.m_strType = "sim";
    boost::filesystem::path mapPath = sim.m_mapFile.m_strMapfile;
    scenario.m_strMap = mapPath.filename().string();
    scenario.m_strInfo = sim.m_strInfo;

  } else {
    XOSCReader_1_0_v4 reader;
    sTagSimuTraffic infos;
    reader.ParseXOSC(strPath.c_str(), infos);
    scenario.m_strType = "xosc";
    boost::filesystem::path mapPath = infos.m_simulation.m_mapFile.m_strMapfile;
    scenario.m_strMap = mapPath.filename().string();
    scenario.m_strInfo = infos.m_simulation.m_strInfo;
    SYSTEM_LOGGER_INFO("scenario m_strInfo %s", scenario.m_strInfo.c_str());
  }
  scenario.m_strPath = strPath;

  scenario.m_strLabel = "";
  scenario.m_strSets = ",1,";
  if (strPath.find(CEngineConfig::Instance().ResourceDir()) == std::string::npos)
    scenario.m_strContent = "0";
  else
    scenario.m_strContent = "";

  return 0;
}

int CSynchronizer::CreateDefaultScenarioSet() {
  sTagCreateScenarioSetParam param;

  param.m_strName = "未分组场景";
  param.m_strLabel = "默认场景";

  std::string strIDs = "";
  CDatabase::Instance().ScenarioIDs(strIDs);
  param.m_strScenes = strIDs;
  return CDatabase::Instance().InsertScenarioSet(param);
}
