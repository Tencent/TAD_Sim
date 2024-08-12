// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

#include "structs/base_struct.h"
#include "structs/hadmap_predef.h"
#include "types/map_types.h"

namespace hadmap {
class RoutingUtils {
 public:
  TXSIMMAP_API static std::string reverseRoadIndex(const roadpkid& rid);

  TXSIMMAP_API static roadpkid reverseRoadIndex2RId(const std::string& rIndex);

  TXSIMMAP_API static std::string linkIndex(const roadpkid& fromRId, const roadpkid& toRId);

  TXSIMMAP_API static std::string linkIndex(const txLaneId& fromLaneId, const txLaneId& toLaneId);

  // distance in Cartesian
  TXSIMMAP_API static double pointsDistance(const txPoint& p0, const txPoint& p1);

  TXSIMMAP_API static double pointsDisWGS84(const txPoint& p0, const txPoint& p1);

  TXSIMMAP_API static bool createOffsetGeom(const PointVec& originGeom, double startOffset, 
                                            double endOffset, PointVec& geom);

  TXSIMMAP_API static bool geomConnectDirectly(const txCurve* fromCurvePtr, const txCurve* toCurvePtr);

  TXSIMMAP_API static DIR_TYPE turnDir(const txCurve* curvePtr);
};
}  // namespace hadmap
