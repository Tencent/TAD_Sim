/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "map_scene.h"

class CMapSceneV2 : public CMapScene {
 public:
  CMapSceneV2();
  ~CMapSceneV2();

  void SetName(std::string strName);
  std::string Name() { return m_strName; }

  void SetData(std::string strData);
  std::string Data() { return m_strData; }

  static int SaveToJson(const char* strSceneName, const char* strData);

  static std::string ParseFromJson(const char* strSceneName);

 protected:
  std::string m_strName;
  std::string m_strData;
};
