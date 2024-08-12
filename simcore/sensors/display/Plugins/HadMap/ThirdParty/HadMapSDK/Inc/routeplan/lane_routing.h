// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <vector>
#include "routeplan/route_plan.h"

namespace LaneRoute {

struct LaneInfo {
  enum class Type { Lane = 0, Link };
  Type type;

  hadmap::txLaneId lane_id;
  hadmap::lanelinkpkid link_id;

  hadmap::PointVec points;
  double length;
};

bool findLaneRoute(hadmap::txMapHandle* map_handle, const hadmap::txPoint& start, const hadmap::txPoint& goal,
                   std::vector<LaneInfo>& lane_route);
}  // namespace LaneRoute
