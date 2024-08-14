// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/03/29
// @brief road info data structure

#pragma once

#include <stdint.h>
#include <vector>
#include "types/map_defs.h"

namespace planner {

// trajectory node
struct RoadInfo {
  // lane ids available in current node
  int64_t roadId;

  // if its junction node
  // record fromRoadId and toRoadId
  hadmap::roadpkid fromRId;
  hadmap::roadpkid toRId;

  bool reverseFlag;
  std::vector<hadmap::lanepkid> endEnableLanes;
  double begin;
  double end;
  double length;
  uint64_t attribute;
  RoadInfo()
      : roadId(0),
        fromRId(ROAD_PKID_INVALID),
        toRId(ROAD_PKID_INVALID),
        reverseFlag(false),
        begin(0.0),
        end(1.0),
        length(0.0),
        attribute(0) {}
};

typedef std::vector<RoadInfo> RoadInfoArray;

}  // namespace planner
