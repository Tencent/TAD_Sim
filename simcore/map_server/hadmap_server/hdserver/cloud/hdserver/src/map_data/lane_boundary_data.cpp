/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "lane_boundary_data.h"
#include "common/log/system_logger.h"
#include "map_curve.h"

CLaneBoundaryData::CLaneBoundaryData() { SetType(ENTITY_TYPE_Lane_Boundary); }

void CLaneBoundaryData::FromTxLaneBoundary(const hadmap::txLaneBoundary& lane_boundary) {
  // lane boundary data
  m_id = lane_boundary.getId();
  m_srs = lane_boundary.getSrs();
  m_mark = static_cast<HADMAP_LANE_MARK>(lane_boundary.getLaneMark());

  // geom data
  const hadmap::txCurve* curv = lane_boundary.getGeometry();
  if (curv) {
    CMapCurve::FromTxCurve(m_points, (*curv));
    CMapCurve::FormAabb(m_points, m_aabb_ll);
  } else {
    SYSTEM_LOGGER_DEBUG("laneboundary %d curve is null", m_id);
  }
}

void CLaneBoundaryData::Lonlat2Local(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2Local(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
}

void CLaneBoundaryData::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2ENU(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
}
