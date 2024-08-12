/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
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
  }

  std::vector<std::string> m_sceneNames;
  std::string m_strDestDir;
};

struct sTagScenesUploadParams {
  sTagScenesUploadParams() { Reset(); }

  void Reset() {
    m_strSourceDir = "";
    m_strType = "";
    m_sceneNames.clear();
  }

  std::string m_strType;
  std::vector<std::string> m_sceneNames;
  std::string m_strSourceDir;
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
