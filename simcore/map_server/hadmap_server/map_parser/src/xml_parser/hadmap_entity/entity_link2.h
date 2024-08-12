/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/engine/math/vec3.h"

class CLinkEntityV2 {
 public:
  CLinkEntityV2();

  void Reset();

  uint64_t m_from_road_id;
  uint64_t m_from_section_id;
  int64_t m_from_lane_id;
  uint64_t m_to_road_id;
  uint64_t m_to_section_id;
  int64_t m_to_lane_id;
  std::vector<CPoint3D> m_points;
};
