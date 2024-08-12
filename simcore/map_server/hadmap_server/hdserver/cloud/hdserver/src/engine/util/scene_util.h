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
  static uint64_t FileSize(const char* strName);

  static std::string SceneFullPath(const char* strName);
  static std::string SceneFullName(const char* strName);
  static std::string ScenePath(const char* strDir, const char* strName);

  static std::string HadmapFullPath(const char* strName);
  static std::string HadmapFullName(const char* strName);
  static std::string HadmapPath(const char* strDir, const char* strName);

  static std::string GISImageFullPath(const char* strName);
  static std::string GISImageFullName(const char* strName);
  static std::string GISImagePath(const char* strDir, const char* strName);
  static bool ValidGISImageFormat(const char* strExt);
  static bool ValidFileFormat(const char* strExt, const char* strExts);
};
