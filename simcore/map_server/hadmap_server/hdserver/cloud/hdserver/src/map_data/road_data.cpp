/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "road_data.h"
#include "common/log/system_logger.h"
#include "map_curve.h"
#include "section_data.h"

CRoadData::CRoadData() { Reset(); }

void CRoadData::Reset() {
  SetType(ENTITY_TYPE_Road);
  m_points.clear();
}

void CRoadData::FromTXRoad(hadmap::txRoad& road) {
  m_id = road.getId();

  // 顶点数据
  const hadmap::txCurve* curv = road.getGeometry();
  if (curv) {
    CMapCurve::FromTxCurve(m_points, (*curv));
    CMapCurve::FormAabb(m_points, m_aabb_ll);
  } else {
    SYSTEM_LOGGER_DEBUG("road %d curve is null", m_id);
  }

  m_taskid = road.getTaskId();
  m_name = road.getName();
  m_dLength = road.getLength();
  m_dGround = road.getGround();
  m_type = static_cast<HADMAP_ROAD_TYPE>(road.getRoadType());
  m_direction = road.getDirection();
  m_toll = road.getToll();
  m_funcClass = road.getFuncClass();
  m_urban = road.isUrban();
  m_paved = road.isPaved();
  m_bidirection = road.isBidirectional();
  m_transType = road.getTransType();
  m_material = static_cast<HADMAP_ROAD_MATERIAL>(road.getMaterial());
  // m_attribute = road.getAttribute();

  // 其他数据
  const hadmap::txSections& secs = road.getSections();
  hadmap::txSections::const_iterator itr = secs.begin();
  for (; itr != secs.end(); ++itr) {
    CSectionData* sec_data = new CSectionData();
    sec_data->FromTXSection(*(*itr));
    m_sections.push_back(sec_data);
  }
}

void CRoadData::Lonlat2Local(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2Local(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);

  Sections::iterator itr = m_sections.begin();
  for (; itr != m_sections.end(); ++itr) {
    (*itr)->Lonlat2Local(refLon, refLat, refAlt);
  }
}

void CRoadData::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2ENU(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);

  Sections::iterator itr = m_sections.begin();
  for (; itr != m_sections.end(); ++itr) {
    (*itr)->Lonlat2ENU(refLon, refLat, refAlt);
  }
}
