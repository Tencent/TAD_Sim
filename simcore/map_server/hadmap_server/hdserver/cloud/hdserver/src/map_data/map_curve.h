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

#include <hadmap.h>

class CMapCurve {
 public:
  static void FromTxCurve(PointsVec& points, const hadmap::txCurve& curve);
  static void FromTxPlane(PointsVec& points, const hadmap::txLineCurve& curve);
  static void FromTxLanelinkCurve(PointsVec& points, const hadmap::txCurve& curve);
  static void Lonlat2Local(PointsVec& points, double refLon, double refLat, double refAlt);

  static void Lonlat2ENU(PointsVec& points, double refLon, double refLat, double refAlt);
  static void Global2Local(PointsVec& points, double refLon, double refLat, double refAlt);
  static void FormAabb(PointsVec& points, CAABB& aabb);

 protected:
};
