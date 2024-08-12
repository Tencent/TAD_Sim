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
  boost::filesystem::path simFilePath = strName;
  if (!boost::filesystem::exists(simFilePath)) {
    return false;
  }
  return true;
}
