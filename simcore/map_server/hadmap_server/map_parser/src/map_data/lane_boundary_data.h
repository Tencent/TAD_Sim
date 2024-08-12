/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include <string>
#include "../engine/entity/aabb.h"
#include "../engine/entity/entity.h"
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"

class CLaneBoundaryData : public CEntity {
 public:
  CLaneBoundaryData();
  void FromTxLaneBoundary(const hadmap::txLaneBoundary& lane_boundary);
  PointsVec& Data() { return m_points; }

  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);

  laneboundarypkid Id() { return m_id; }
  uint32_t Srs() { return m_srs; }
  HADMAP_LANE_MARK LaneMark() { return m_mark; }
  std::string Color() { return m_color; }
  HADMAP_LANE_MARK Type() { return m_type; }

  double LaneWidth() { return m_lane_width; }
  double DubleInterval() { return m_double_interval; }
  double DotLength() { return m_dot_length; }
  double DotSpace() { return m_dot_space; }
  double DotOffset() { return m_dot_offset; }

  /*CAABB&		AabbLl() { return m_aabb_ll; }
  CAABB&		AabbXyz() { return m_aabb_xyz; }*/

 protected:
  laneboundarypkid m_id;
  uint32_t m_srs;
  HADMAP_LANE_MARK m_mark;
  std::string m_color;
  HADMAP_LANE_MARK m_type;
  PointsVec m_points;

  double m_lane_width;
  double m_double_interval;
  double m_dot_length;
  double m_dot_space;
  double m_dot_offset;

  /*CAABB		m_aabb_ll;
  CAABB		m_aabb_xyz;*/
};
