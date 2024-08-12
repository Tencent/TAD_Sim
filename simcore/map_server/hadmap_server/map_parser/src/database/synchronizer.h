/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct sTagEntityScenario;

typedef std::vector<std::string> FilePathes;

class CScenarioSets;
class CScenarioEntities;

class CSynchronizer {
 public:
  int SyncDBToDisk();

  int SyncDiskToDB();

  int SyncDiskToDB2();
  int CreatePresetScenarioSets();
  int loadDataHadmapConfig();
  int handleDbback(CScenarioEntities& scenarios, CScenarioSets& ssets);

  int handleDbPreset(CScenarioEntities& scenarios, CScenarioSets& ssets);

  static CSynchronizer& Instance();

  int OneSimFileToDB(std::string& strPath);

 protected:
  int GetFileList(FilePathes& files, bool isPreset = true);

  int CreateDefaultScenarioSet();

  int ExtractSceneInfo(std::string& strPath, sTagEntityScenario& scenario);

  CSynchronizer();
};
