/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

typedef std::vector<std::string> FilePathes;

class CSynchronizer {
 public:
  int SyncDBToDisk();

  int SyncDiskToDB();

  static CSynchronizer& Instance();

  int OneSimFileToDB(std::string& strPath);

 protected:
  int GetFileList(FilePathes& files);

  int CreateDefaultScenarioSet();

  CSynchronizer();
};
