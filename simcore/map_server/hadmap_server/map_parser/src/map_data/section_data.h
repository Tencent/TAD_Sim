/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include "../engine/entity/entity.h"
#include "lane_data.h"

class CSectionData : public CEntity {
 public:
  CSectionData();
  void FromTXSection(const hadmap::txSection& sec);

  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);

  Lanes& Data() { return m_lanes_data; }
  sectionpkid Id() { return m_id; }
  roadpkid RoadId() { return m_road_id; }
  double Length() { return m_length; }
  double Distance2Junction() { return m_distance_to_junction; }

 protected:
  // section data
  sectionpkid m_id;
  roadpkid m_road_id;
  double m_length;
  double m_distance_to_junction;

  // lanes data
  Lanes m_lanes_data;
};
