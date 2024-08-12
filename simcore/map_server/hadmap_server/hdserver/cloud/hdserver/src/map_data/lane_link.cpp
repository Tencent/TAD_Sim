/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "lane_link.h"
#include "common/log/system_logger.h"
#include "map_curve.h"

CLaneLink::CLaneLink() {
  SetType(ENTITY_TYPE_Lane_Link);
  m_length = 0;
}

void CLaneLink::FromTxLanelink(hadmap::txLaneLink& lanelink) {
  m_from_id.m_roadID = lanelink.fromRoadId();
  m_from_id.m_sectionID = lanelink.fromSectionId();
  m_from_id.m_laneID = lanelink.fromLaneId();
  m_to_id.m_roadID = lanelink.toRoadId();
  m_to_id.m_sectionID = lanelink.toSectionId();
  m_to_id.m_laneID = lanelink.toLaneId();
  m_id = lanelink.getId();

  m_junctionid = lanelink.getJunctionId();

  m_length = 0;
  const hadmap::txCurve* curve = lanelink.getGeometry();

  if (curve) {
    CMapCurve::FromTxLanelinkCurve(m_points, (*curve));
    CMapCurve::FormAabb(m_points, m_aabb_ll);
    m_length = curve->getLength();

    // SYSTEM_LOGGER_INFO("lanelink %d curve length is %f ", m_id, m_length);
  } else {
    SYSTEM_LOGGER_DEBUG("lanelink %d curve is null", m_id);
  }
}

void CLaneLink::Lonlat2Local(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2Local(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
}

void CLaneLink::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2ENU(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
}
