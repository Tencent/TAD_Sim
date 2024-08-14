// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/04/02
// @brief responsible for path plan strategy

#pragma once

#include "mapengine/hadmap_engine.h"
#include "routeplan/a_star.h"
#include "routeplan/traj_node.h"

namespace planner {

class RouteGeneration {
 private:
  AStar astar;
  PathList path;
  RoadInfoArray traj;
  hadmap::txPoint queryBeginPt;
  hadmap::txPoint queryEndPt;

 public:
  RouteGeneration();
  explicit RouteGeneration(hadmap::txMapHandle* pHandle);

  ~RouteGeneration() {
    // no need to delete _hdmap_db
  }

  int32_t searchTrajectory(std::vector<hadmap::txPoint>& dsts, RoadInfoArray& trajectory);

 private:
  bool pathPlan();
  bool generateTrajectory();
  bool refineTrajectory();
  bool setQueryParam(const hadmap::txPoint& begin, const hadmap::txPoint& end);
  void reset();

  void mergeTrajectory(const std::vector<RoadInfoArray>& trajs, RoadInfoArray& mergeTraj);
};

}  // namespace planner
