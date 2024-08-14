// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "structs/base_struct.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_laneboundary.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_object.h"
#include "structs/hadmap_objgeom.h"
#include "structs/hadmap_road.h"
#include "structs/map_structs.h"

namespace mapdata_trans {
// points to txLineCurve
TXSIMMAP_API void Points3dToLineCurve(const hadmap::Points3d& geom, hadmap::txLineCurve* curvePtr);

TXSIMMAP_API void Points5dToLineCurve(const hadmap::Points5d& geom, hadmap::txLineCurve* curvePtr);

// txLineCurve to points
TXSIMMAP_API void LineCurveToPoints3d(const hadmap::txLineCurve* curvePtr, hadmap::Points3d& geom);

TXSIMMAP_API void LineCurveToPoints5d(const hadmap::txLineCurve* curvePtr, hadmap::Points5d& geom);

// convert lane data
TXSIMMAP_API void TxLaneToHadLane(const hadmap::tx_lane_t& lane, hadmap::txLanePtr& lanePtr);

TXSIMMAP_API void HadLaneToTxLane(const hadmap::txLanePtr lanePtr, hadmap::tx_lane_t& lane);

// convert lane boundary data
TXSIMMAP_API void TxBoundaryToHadBoundary(const hadmap::tx_laneboundary_t& boundary,
                                          hadmap::txLaneBoundaryPtr& boundaryPtr);

TXSIMMAP_API void HadBoundaryToTxBoundary(const hadmap::txLaneBoundaryPtr boundaryPtr,
                                          hadmap::tx_laneboundary_t& boundary);

// convert road data
TXSIMMAP_API void TxRoadToHadRoad(const hadmap::tx_road_t& road, hadmap::txRoadPtr& roadPtr);

TXSIMMAP_API void HadRoadToTxRoad(const hadmap::txRoadPtr roadPtr, hadmap::tx_road_t& road);

// convert lanelink data
TXSIMMAP_API void TxLaneLinkToHadLaneLink(const hadmap::tx_lanelink_t& lanelink, hadmap::txLaneLinkPtr& lanelinkPtr);

TXSIMMAP_API void HadLaneLinkToTxLaneLink(const hadmap::txLaneLinkPtr lanelinkPtr, hadmap::tx_lanelink_t& lanelink);

// convert object data
TXSIMMAP_API void TxObjectToHadObject(const hadmap::tx_object_t& object, hadmap::txObjectPtr& objectPtr);

// convert object geom data
TXSIMMAP_API void TxObjGeomToHadObjGeom(const hadmap::tx_object_geom_t& geom, hadmap::txObjGeomPtr& geomPtr);
}  // namespace mapdata_trans
