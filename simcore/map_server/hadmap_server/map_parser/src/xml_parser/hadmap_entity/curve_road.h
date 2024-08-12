/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <hadmap.h>
#include <vector>
#include "common/engine/math/vec3.h"
#include "road.h"

class CCurveRoad : public CRoad {
 public:
  CCurveRoad();

  std::vector<CVector3> m_controlPoints;

  hadmap::txRoadPtr m_txRoad;
};
