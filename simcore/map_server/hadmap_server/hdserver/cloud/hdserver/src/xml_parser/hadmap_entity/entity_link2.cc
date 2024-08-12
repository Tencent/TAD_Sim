/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "entity_link2.h"

CLinkEntityV2::CLinkEntityV2() { Reset(); }

void CLinkEntityV2::Reset() {
  m_from_road_id = 0;
  m_from_section_id = 0;
  m_from_lane_id = 0;
  m_to_road_id = 0;
  m_to_section_id = 0;
  m_to_lane_id = 0;
}