/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct sTagGISImageDeleteParams {
  sTagGISImageDeleteParams() { Reset(); }

  void Reset() { m_imageNames.clear(); }

  std::vector<std::string> m_imageNames;
};

struct sTagGISImageRenameParams {
  sTagGISImageRenameParams() { Reset(); }

  void Reset() {
    m_strOldName = "";
    m_strNewName = "";
  }

  std::string m_strOldName;
  std::string m_strNewName;
};

struct sTagGISImageUploadParams {
  sTagGISImageUploadParams() { Reset(); }

  void Reset() {
    m_strType = "";
    m_strSourceDir = "";
    m_imageNames.clear();
  }

  std::string m_strType;
  std::vector<std::string> m_imageNames;
  std::string m_strSourceDir;
};
