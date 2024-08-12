/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "section_data.h"

CSectionData::CSectionData() { SetType(ENTITY_TYPE_Section); }

void CSectionData::FromTXSection(const hadmap::txSection& sec) {
  // section info
  m_id = sec.getId();
  m_road_id = sec.getRoadId();
  m_length = sec.getLength();
  m_distance_to_junction = sec.getDistanceToJunction();

  // lanes
  const hadmap::txLanes& lanes = sec.getLanes();
  hadmap::txLanes::const_iterator itr = lanes.begin();
  for (; itr != lanes.end(); ++itr) {
    CLaneData* pLane = new CLaneData((*itr));

    m_lanes_data.push_back(pLane);
  }
}

void CSectionData::Lonlat2Local(double refLon, double refLat, double refAlt) {
  Lanes::iterator itr = m_lanes_data.begin();
  for (; itr != m_lanes_data.end(); ++itr) {
    (*itr)->Lonlat2Local(refLon, refLat, refAlt);
  }
}
void CSectionData::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  Lanes::iterator itr = m_lanes_data.begin();
  for (; itr != m_lanes_data.end(); ++itr) {
    (*itr)->Lonlat2ENU(refLon, refLat, refAlt);
  }
}
