/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <string>
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
  }

  std::string m_strType;
  std::vector<std::string> m_hadmapNames;
  std::string m_strSourceDir;
};

struct sTagCloudHadmapUploadCallbackParam {
  sTagCloudHadmapUploadCallbackParam() { Reset(); }

  void Reset() { m_hadmapNames.clear(); }
  std::vector<std::string> m_hadmapNames;
  std::map<std::string, std::pair<std::string, std::string>> m_hadmapCosInfos;
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

struct sTagGetLaneRelParam {
  sTagGetLaneRelParam() { Reset(); }

  void Reset() { m_hadmapNames.clear(); }
  std::vector<std::string> m_hadmapNames;
};

struct sTagGetRoadRelParam {
  sTagGetRoadRelParam() { Reset(); }

  void Reset() { m_hadmapNames.clear(); }
  std::vector<std::string> m_hadmapNames;
};
