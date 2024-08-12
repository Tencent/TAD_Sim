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
  m_color = "white";                // 白色
  m_type = HADMAP_LANE_MARK_Solid;  // 1
  // lane boundary data
  m_id = lane_boundary.getId();
  m_srs = lane_boundary.getSrs();
  // hadmap::LANE_MARK Temp = lane_boundary.getLaneMark();
  m_mark = static_cast<HADMAP_LANE_MARK>(lane_boundary.getLaneMark());

  // m_lane_width = lane_boundary.getLaneWidth();
  // m_double_interval = lane_boundary.getDoubleInterval();
  // m_dot_length = lane_boundary.getDotLen();
  // m_dot_space = lane_boundary.getDotSpace();
  // m_dot_offset = lane_boundary.getDotOffset();

  {
    if ((m_mark & hadmap::LANE_MARK_Solid) == hadmap::LANE_MARK_Solid) {
      m_type = HADMAP_LANE_MARK_Solid;  // 1
    }
    if ((m_mark & hadmap::LANE_MARK_Broken) == hadmap::LANE_MARK_Broken) {
      m_type = HADMAP_LANE_MARK_Broken;  // 2
    }
    if ((m_mark & hadmap::LANE_MARK_Solid2) == hadmap::LANE_MARK_Solid2) {
      m_type = HADMAP_LANE_MARK_Solid2;  // 16
    }
    if ((m_mark & hadmap::LANE_MARK_Broken2) == hadmap::LANE_MARK_Broken2) {
      m_type = HADMAP_LANE_MARK_Broken2;  // 32
    }
    if ((m_mark & hadmap::LANE_MARK_SolidSolid) == hadmap::LANE_MARK_SolidSolid) {
      m_type = HADMAP_LANE_MARK_SolidSolid;  // 17
    }
    if ((m_mark & hadmap::LANE_MARK_SolidBroken) == hadmap::LANE_MARK_SolidBroken) {
      m_type = HADMAP_LANE_MARK_SolidBroken;  // 33
    }
    if ((m_mark & hadmap::LANE_MARK_BrokenSolid) == hadmap::LANE_MARK_BrokenSolid) {
      m_type = HADMAP_LANE_MARK_BrokenSolid;  // 18
    }
    if ((m_mark & hadmap::LANE_MARK_BrokenBroken) == hadmap::LANE_MARK_BrokenBroken) {
      m_type = HADMAP_LANE_MARK_BrokenBroken;  // 34
    }
  }

  {
    if ((m_mark & hadmap::LANE_MARK_Blue) == hadmap::LANE_MARK_Blue) {
      // m_color = HADMAP_LANE_MARK_Blue;
      m_color = "blue";  // 458752
    } else if ((m_mark & hadmap::LANE_MARK_Red) == hadmap::LANE_MARK_Red) {
      m_color = "red";  // 327680
    } else if ((m_mark & hadmap::LANE_MARK_Green) == hadmap::LANE_MARK_Green) {
      m_color = "green";  // 393216
    } else if ((m_mark & hadmap::LANE_MARK_Yellow) == hadmap::LANE_MARK_Yellow) {
      m_color = "yellow";  // 65536
    }
  }

  // geom data
  const hadmap::txCurve* curv = lane_boundary.getGeometry();
  if (curv) {
    CMapCurve::FromTxCurve(m_points, (*curv));
    CMapCurve::FormAabb(m_points, m_aabb_ll);
  } else {
    SYSTEM_LOGGER_WARN("laneboundary %d curve is null", m_id);
  }
}

void CLaneBoundaryData::Lonlat2Local(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2Local(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
}
void CLaneBoundaryData::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  int id = m_id;
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2ENU(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
}
