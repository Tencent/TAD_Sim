/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "../engine/entity/aabb.h"
#include "common/engine/math/vec3.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_data_predef.h"

#include <hadmap.h>

class CMapBlock {
 public:
 protected:
  CAABB m_aabb_ll;
  CAABB m_aabb_xyz;
  CVector3 m_center;

  RoadVec m_roads;
  SectionVec m_sections;
  LaneVec m_lanes;
  LaneBoundaryVec m_laneboundaries;
  LaneLinkVec m_links;
  MapObjectVec m_objects;

  RoadMap m_roadmap;
  SectionMap m_sectionmap;
  LaneMap m_lanemap;
  LaneBoundaryMap m_laneboundarymap;
  MapObjectMap m_objectmap;
  LaneLinkIDMap m_lanelinkmap;
  LaneLinkMap m_fromlanemap;
  LaneLinkMap m_tolanemap;
};
