/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <hadmap.h>
#include "road.h"

class CStraightRoad : public CRoad {
 public:
  CStraightRoad();

  double m_start[3];
  double m_end[3];

  hadmap::txRoadPtr m_txRoad;
};
