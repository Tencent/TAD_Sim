// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/04/02
// @brief Refine trajectory path and record more info

#pragma once

#include <set>

#include "routeplan/traj_node.h"
#include "structs/base_struct.h"

namespace planner {

class TrajRefiner {
 public:
  static void roadProc(RoadInfoArray& traj);

  static void cutRoad(const hadmap::txPoint& start, const hadmap::txPoint& end, RoadInfoArray& traj);

  static void mockJunctionToRoad(RoadInfoArray& traj);

  static void judgeAdjRoadLinkType(RoadInfoArray& traj);

  static void setJunctionLinkType(RoadInfoArray& traj);
};

}  // namespace planner
