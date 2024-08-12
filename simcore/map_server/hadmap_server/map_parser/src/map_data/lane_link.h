/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include "../engine/entity/aabb.h"
#include "../engine/entity/entity.h"
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"

class CLaneLink : public CEntity {
 public:
  CLaneLink();
  void FromTxLanelink(hadmap::txLaneLink& lanelink);
  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);

  PointsVec& Data() { return m_points; }
  LaneID& FromId() { return m_from_id; }
  LaneID& ToId() { return m_to_id; }
  junctionpkid JunctionId() { return m_junctionid; }

  lanelinkpkid Id() { return m_id; }
  void SetId(lanelinkpkid id) { m_id = id; }
  double GetLength() { return m_length; }

  /*CAABB&		AabbLl() { return m_aabb_ll; }
  CAABB&		AabbXyz() { return m_aabb_xyz; }*/

 protected:
  lanelinkpkid m_id;
  LaneID m_from_id;
  LaneID m_to_id;
  junctionpkid m_junctionid;
  PointsVec m_points;
  double m_length;
  /*CAABB			m_aabb_ll;
  CAABB			m_aabb_xyz;*/
};
