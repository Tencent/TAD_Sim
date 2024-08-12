// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routeplan/route_generation.h"
#include <float.h>
#include <iostream>

#include "common/log.h"
#include "routeplan/node_opt.h"
#include "routeplan/refine_traj.h"

using namespace hadmap;

namespace planner {

RouteGeneration::RouteGeneration(txMapHandle* pHandle) : astar(FUZZY_LOCATE) {
  NodeOpt::getInstance()->setMapHandle(pHandle);
}

bool RouteGeneration::setQueryParam(const hadmap::txPoint& begin, const hadmap::txPoint& end) {
  queryBeginPt = begin;
  queryEndPt = end;
  return astar.setQueryParam(begin, end);
}

bool RouteGeneration::pathPlan() {
  if (SUCC != astar.searchPath(path)) {
    txlog::error("search path failed!");
    return false;
  }
  return true;
}

// generate trajectory acccording to path
bool RouteGeneration::generateTrajectory() {
  PathList::iterator it = path.begin();
  traj.clear();
  for (; it != path.end(); it++) {
    NodePtr p_node = *it;
    RoadInfo p_road;
    p_road.roadId = p_node->nodeId.roadId;
    p_road.length = p_node->roadLen;

    traj.push_back(p_road);
  }
  return true;
}

bool RouteGeneration::refineTrajectory() {
  // judge road need to be reversed
  TrajRefiner::roadProc(traj);

  // set road progress
  TrajRefiner::cutRoad(queryBeginPt, queryEndPt, traj);

  // judge road turning type straight | left | right
  TrajRefiner::judgeAdjRoadLinkType(traj);

  // mock junction
  TrajRefiner::mockJunctionToRoad(traj);

  // set junction link type
  TrajRefiner::setJunctionLinkType(traj);

  return true;
}

void RouteGeneration::reset() {
  path.clear();
  traj.clear();
}

int32_t RouteGeneration::searchTrajectory(std::vector<txPoint>& dsts, RoadInfoArray& trajectory) {
  if (dsts.size() <= 1) {
    txlog::error("Dsts size need to be more than 2 points!");
    return -1;
  }

  std::vector<RoadInfoArray> trajs;
  std::set<int> vec_i;
  int startindex = 0;
  int nextindex = 1;
  for (size_t i = 0; i < dsts.size() - 1; ++i) {
    reset();

    txPoint startPt = dsts[startindex];
    txPoint endPt = dsts[nextindex];

    if (!setQueryParam(startPt, endPt)) {
      std::string info = "set query parameters failed ,remove point lon = " + std::to_string(endPt.x) +
                         " £¬lat = " + std::to_string(endPt.y) + "!";
      txlog::info(info.c_str());
      vec_i.insert(i + 1);
      nextindex++;
      continue;
    } else {
      txlog::info("set query parameters succ!");
    }

    if (!pathPlan()) {
      txlog::error("search path failed!");
      return -1;
    } else {
      txlog::info("path plan succ!");
    }

    if (!generateTrajectory()) {
      txlog::error("generate trajectory failed!");
      return -1;
    } else {
      txlog::info("generate trajectory succ!");
    }

    if (astar.getTagP(queryEndPt)) dsts[i + 1] = queryEndPt;

    refineTrajectory();

    // get refined trajectory
    trajs.push_back(traj);
    startindex = nextindex;
    nextindex++;
  }
  std::vector<txPoint> tmp;
  for (int i = 0; i < dsts.size(); i++) {
    if (vec_i.find(i) == vec_i.end()) {
      tmp.push_back(dsts.at(i));
    }
  }
  dsts = tmp;

  int64_t jnodeid = -1;
  for (auto& traj : trajs) {
    for (auto& tj : traj) {
      if (tj.roadId < 0) {
        tj.roadId = jnodeid--;
      }
    }
  }
  mergeTrajectory(trajs, trajectory);
  return 0;
}

void RouteGeneration::mergeTrajectory(const std::vector<RoadInfoArray>& trajs, RoadInfoArray& mergeTraj) {
  mergeTraj.clear();
  if (trajs.empty()) return;
  mergeTraj.assign(trajs[0].begin(), trajs[0].end());
  for (size_t i = 1; i < trajs.size(); ++i) {
    if (mergeTraj.back().roadId == trajs[i][0].roadId) {
      mergeTraj.back().end = trajs[i][0].end;
      mergeTraj.back().endEnableLanes = trajs[i][0].endEnableLanes;
      mergeTraj.insert(mergeTraj.end(), trajs[i].begin() + 1, trajs[i].end());
    } else {
      throw std::runtime_error("Merge Traj Error");
    }
  }
}
}  // namespace planner
