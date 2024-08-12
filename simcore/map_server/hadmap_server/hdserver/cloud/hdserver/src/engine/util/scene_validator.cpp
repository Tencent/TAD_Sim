/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "scene_validator.h"
#include <boost/filesystem.hpp>
#include "../config.h"

bool CSceneValidator::SceneExist(const char* strName) {
  if (!strName) {
    return false;
  }

  boost::filesystem::path sceneDirPath = CEngineConfig::Instance().SceneDir();
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath.append(strName);
  std::string strExt = simFilePath.extension().string();

  std::string strStemName = simFilePath.stem().string();
  std::string strSimPath = "";
  if (strExt.size() == 0 || strExt != ".sim") {
    std::string strFileName = strName;
    strFileName.append(".sim");
    simFilePath = sceneDirPath;
    simFilePath.append(strFileName);
  }

  if (!boost::filesystem::exists(simFilePath)) {
    return false;
  }

  return true;
}
