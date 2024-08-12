/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>

class CSceneUtil {
 public:
  enum SCENE_PATH_TYPE { SYS_SCENE = 0, DATA_SCENE = 1, ALL_PATH = 2 };
  static uint64_t FileSize(const char* strName);
  static std::string SceneFullPath(const char* strName);
  static std::string DataSceneFullPath(const char* strName);
  static std::string GetSceneFullPath(const char* strName, SCENE_PATH_TYPE mode);
  static std::string SceneFullName(const char* strName);
  static std::string SceneFullName(const char* strName, const char* strExt, bool bIsAllName = true);
  static std::string ScenePath(const char* strDir, const char* strName);

  static std::string HadmapFullPath(const char* strName);
  static std::string HadmapFullName(const char* strName);
  static std::string HadmapPath(const char* strDir, const char* strName);

  static std::string GISImageFullPath(const char* strName);
  static std::string GISImageFullName(const char* strName);
  static std::string GISImagePath(const char* strDir, const char* strName);
  static bool ValidGISImageFormat(const char* strExt);
  static bool ValidFileFormat(const char* strExt, const char* strExts);

  static std::string GISModelFullPath(const char* strName);
  static std::string GISModelFullName(const char* strName);
  static std::string GISModelPath(const char* strDir, const char* strName);
  static bool ValidGISModelFormat(const char* strExt);
};
