/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "common/map_data/map_data_predef.h"

struct sTagQueryNearbyInfoParams {
  sTagQueryNearbyInfoParams() { Reset(); }

  void Reset() {
    m_strHadmapName = "";
    m_dLon = -999;
    m_dLat = -999;
  }

  std::string m_strHadmapName;
  double m_dLon;
  double m_dLat;
};

struct sTagQueryNearbyInfoResult {
  sTagQueryNearbyInfoResult() { Reset(); }

  void Reset() {
    m_elemType = "";
    m_uRoadID = 0;
    m_uSecID = 0;
    m_laneID = 0;
    m_lanelinkID = 0;
  }

  std::string m_elemType;
  roadpkid m_uRoadID;
  sectionpkid m_uSecID;
  lanepkid m_laneID;
  lanelinkpkid m_lanelinkID;
};
