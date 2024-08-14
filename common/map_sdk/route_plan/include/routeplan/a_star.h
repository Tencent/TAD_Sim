// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/03/29
// @brief A Star algorithm declaration

#pragma once

#include "mapengine/hadmap_engine.h"
#include "routeplan/route_struct.h"

namespace planner {
enum SearchState { INVALID = -2, UN_MATCHED = -1, SUCC = 0 };

enum AStarStrategy { ACCURATE_LOCATE = 1, FUZZY_LOCATE = 2 };

class AStar {
 private:
  QueryParam queryParam;
  AStarStrategy strategy;
  bool chTag;
  hadmap::txPoint tagP;

 public:
  explicit AStar(AStarStrategy s = ACCURATE_LOCATE) : strategy(s) {}

  ~AStar() {}

 public:
  void resetQueryParam();

  bool setQueryParam(const hadmap::txPoint& srcPt, const hadmap::txPoint& dstPt);

  SearchState searchPath(PathList& path);

  // if FUZZY_LOCATE and tagP is in opposite lane
  // then return true and tagP
  // else return false and tagP not change
  bool getTagP(hadmap::txPoint& tagP) const;

 private:
  bool genPathList(const NodePtr& beginPtr, const NodePtr& endPtr, PathList& path);

  void printPathList(const PathList& path) const;
};
}  // namespace planner
