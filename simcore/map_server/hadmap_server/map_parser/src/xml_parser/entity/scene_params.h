/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <tuple>
#include <vector>
struct sTagScenarioSaveParams {
  sTagScenarioSaveParams() { Reset(); }

  void Reset() {}
};

struct sTagScenesDeleteParams {
  sTagScenesDeleteParams() { Reset(); }

  void Reset() {
    m_sceneNames.clear();
    m_ids.clear();
  }

  std::vector<std::string> m_sceneNames;
  std::vector<std::string> m_ids;
};

struct sTagScenesDownloadParams {
  sTagScenesDownloadParams() { Reset(); }
  void Reset() {
    m_sceneNames.clear();
    m_strDestDir = "";
    m_strExportType = "sim";
  }

  std::vector<std::string> m_sceneNames;
  std::string m_strDestDir;
  std::string m_strExportType;
};

struct sTagScenesUploadParams {
  sTagScenesUploadParams() { Reset(); }

  void Reset() {
    m_strSourceDir = "";
    m_strType = "";
    m_strScenarioSet = "";
    m_sceneNames.clear();
    m_strReplaceFlag = "true";
  }

  std::string m_strType;
  // std::vector<std::string>    m_sceneNames;
  std::vector<std::tuple<std::string, std::string>> m_sceneNames;  // 源文件名字  +  ReplaceFlag
  std::string m_strSourceDir;
  std::string m_category;
  std::string m_strScenarioSet;
  std::string m_strReplaceFlag;  //
};

struct sTagScenesCopyParams {
  sTagScenesCopyParams() { Reset(); }

  void Reset() { m_sceneNames.clear(); }

  std::vector<std::string> m_sceneNames;
};

struct sTagSceneRenameParams {
  sTagSceneRenameParams() { Reset(); }

  void Reset() {
    m_oldName = "";
    m_newName = "";
  }

  std::string m_oldName;
  std::string m_newName;
};
