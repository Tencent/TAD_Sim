// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_road.h"

namespace hadmap {
using AreaPointVec = std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>>;
// parse opendrive map data
TXSIMMAP_API bool parseOpenDrive(const std::string& data_path, txRoads& roads, txLaneLinks& links, txObjects& objs,
                                 txJunctions& junc, txOdHeaderPtr& header, hadmap::txPoint& refPoint,
                                 bool doubleRoad = true);
TXSIMMAP_API bool parseOpenDriveArea(const std::string& data_path, txRoads& roads, txLaneLinks& links, txObjects& objs,
                                     txJunctions& juncs, AreaPointVec ptr, txOdHeaderPtr& header, txPoint& refPoint);
}  // namespace hadmap
