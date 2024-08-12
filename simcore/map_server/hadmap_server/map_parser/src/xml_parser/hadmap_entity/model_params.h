/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct sTagGISModelDeleteParams {
  sTagGISModelDeleteParams() { Reset(); }

  void Reset() { m_modelNames.clear(); }

  std::vector<std::string> m_modelNames;
};

struct sTagGISModelRenameParams {
  sTagGISModelRenameParams() { Reset(); }

  void Reset() {
    m_strOldName = "";
    m_strNewName = "";
  }

  std::string m_strOldName;
  std::string m_strNewName;
};

struct sTagGISModelUploadParams {
  sTagGISModelUploadParams() { Reset(); }

  void Reset() {
    m_strType = "";
    m_strSourceDir = "";
    m_modelNames.clear();
  }

  std::string m_strType;
  std::vector<std::string> m_modelNames;
  std::string m_strSourceDir;
};
