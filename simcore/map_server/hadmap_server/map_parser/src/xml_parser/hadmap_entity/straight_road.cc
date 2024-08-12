/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "straight_road.h"

CStraightRoad::CStraightRoad() {
  for (int i = 0; i < 3; ++i) {
    m_start[i] = 0;
    m_end[i] = 0;
  }

  m_type = HET_Straight;
  m_txRoad = nullptr;
}