/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "map_scene.h"

class CMapSceneV3 : public CMapScene {
 public:
  CMapSceneV3();
  ~CMapSceneV3();

  void SetName(std::string strName);
  std::string Name() { return m_strName; }

  void SetData(std::string strData);
  void SetGeometry(std::string strGeometry);

  void SetPreset(std::string strPreset);
  std::string GetPreset() { return m_strPreset; }

  std::string Data() { return m_strData; }
  std::string Geometry() { return m_strGeometry; }

  static int SaveToJson(const char* strSceneName, const char* strData, const char* strGeometry);
  int SaveOpenDrive(const char* strSceneName, const char* strPreset, const char* strData);
  static int SaveToOpenDrive(const char* strSceneName, const char* strData);

  static std::string ParseFromJson(const char* strSceneName);
  static std::string ParseFromOpenDrive(const char* strSceneName);

 protected:
  std::string m_strName;
  std::string m_strGeometry;
  std::string m_strData;
  std::string m_strPreset;
};
