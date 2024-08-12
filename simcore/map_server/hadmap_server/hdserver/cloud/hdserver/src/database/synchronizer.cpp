/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "synchronizer.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include "../engine/config.h"
#include "../engine/util/scene_util.h"
#include "../xml_parser/entity/scenario_set_params.h"
#include "../xml_parser/entity/simulation.h"
#include "./db.h"
#include "./entity/scenario.h"
#include "common/log/system_logger.h"

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

int CSynchronizer::GetFileList(FilePathes& files) {
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  p /= "scene";

  // 获取文件
  boost::filesystem::path scenePath = p;

  // 路径存在
  if (boost::filesystem::exists(scenePath)) {
    boost::filesystem::directory_iterator itr_end;
    boost::filesystem::directory_iterator itr(scenePath);

    for (; itr != itr_end; ++itr) {
      std::string strName = (*itr).path().string();
      std::string strFileName = (*itr).path().stem().string();
      std::string strExt = (*itr).path().extension().string();
      // if ((!boost::filesystem::is_directory(*itr)) && strExt == strInExt)
      if ((!boost::filesystem::is_directory(*itr)) && CSceneUtil::ValidFileFormat(strExt.c_str(), ".sim")) {
        files.push_back((*itr).path().string());
      }
    }
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
  CSimulation sim;
  sim.Parse(strPath.c_str(), false);
  sTagEntityScenario scenario;
  boost::filesystem::path p = strPath;
  scenario.m_strName = p.stem().string();
  scenario.m_strPath = strPath;

  boost::filesystem::path mapPath = sim.m_mapFile.m_strMapfile;
  scenario.m_strMap = mapPath.filename().string();
  scenario.m_strInfo = sim.m_strInfo;
  scenario.m_strLabel = "";
  scenario.m_strSets = ",1,";
  scenario.m_strContent = "";

  CDatabase::Instance().InsertScenario(scenario);

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
