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

  void SetId(std::string strId);
  std::string GetId() { return m_strId; }

  void SetDesc(std::string strDesc);
  std::string GetDesc() { return m_strDesc; }

  void SetData(std::string strData);
  void SetGeometry(std::string strGeometry);

  void SetPreset(std::string strPreset) { m_strPreset = strPreset; }

  void SetIsThird(bool flag) { m_isThird = flag; }

  bool getIsThird() { return m_isThird; }

  void SetBucket(std::string b) { m_bucket = b; }
  std::string GetBucket() { return m_bucket; }

  void SetBaseVersion(const std::string& v) { m_strBaseVersion = v; }
  std::string GetBaseVersion() { return m_strBaseVersion; }

  void SetJsonKey(std::string j) { m_jsonKey = j; }
  std::string GetJsonKey() { return m_jsonKey; }

  std::string GetPreset() { return m_strPreset; }

  std::string Data() { return m_strData; }
  std::string Geometry() { return m_strGeometry; }

  static int SaveToJson(const char* strSceneName, const char* strData, const char* strGeometry);
  // int SaveOpenDrive(const char* strSceneName, const char* strData);

  int SaveOpenDrive(const char* strSceneName, const char* strPreset, const char* strData);
  // static int SaveToOpenDrive(const char* strSceneName, const char* strData);
  static int SaveToOpenDrive(const char* strSceneName, const char* strData);

  static std::string ParseFromJson(const char* strSceneName);
  static std::string ParseFromOpenDrive(const char* strSceneName);

 protected:
  std::string m_strName;
  std::string m_strGeometry;
  std::string m_strData;
  std::string m_strPreset;
  std::string m_strId;
  std::string m_strDesc;
  std::string m_bucket;
  std::string m_jsonKey;
  bool m_isThird = false;
  std::string m_strBaseVersion;
};
