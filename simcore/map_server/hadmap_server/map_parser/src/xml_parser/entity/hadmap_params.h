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
struct sTagHadmapsDeleteParams {
  sTagHadmapsDeleteParams() { Reset(); }

  void Reset() {}

  std::vector<std::string> m_hadmapNames;
};

struct sTagHadmapsDownloadParams {
  sTagHadmapsDownloadParams() { Reset(); }

  void Reset() {
    m_hadmapNames.clear();
    m_strDestName = "";
  }

  std::vector<std::string> m_hadmapNames;
  std::string m_strDestName;
};

struct sTagHadmapsUploadParams {
  sTagHadmapsUploadParams() { Reset(); }

  void Reset() {
    m_strType = "";
    m_strSourceDir = "";
    m_hadmapNames.clear();
    // m_strReplaceFlag = "true";
  }

  std::string m_strType;
  std::vector<std::tuple<std::string, std::string>> m_hadmapNames;  // 源文件名字  +  ReplaceFlag
  // std::vector<std::string>    m_hadmapNames; //源文件名字   目标文件名字
  std::string m_strSourceDir;
  // std::string m_strReplaceFlag; //
};
struct sTagGetForwardPointsParam {
  sTagGetForwardPointsParam() { Reset(); }

  void Reset() {
    x = 0.0;

    y = 0.0;

    z = 0.0;

    length = 0.0;

    interval = -1.0;

    hadmapName = "";
  }

  // x  lon - wgs84
  double x;
  // y lat - wgs84
  double y;
  // z  ele - wgs84
  double z;

  // distance from current point.
  double length;

  // sample interval
  double interval;

  std::string hadmapName;
};
