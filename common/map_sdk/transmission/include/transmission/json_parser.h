// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

#include "common/macros.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_laneboundary.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_object.h"
#include "structs/hadmap_road.h"
#include "structs/hadmap_route.h"

namespace hadmap {
class TXSIMMAP_API JsonParser {
 public:
  static bool parseRoad(const std::string& jsonStr, txRoads& roads);

  static bool parseSection(const std::string& jsonStr, txSections& sections);

  static bool parseLane(const std::string& jsonStr, txLanes& lanes);

  static bool parseBoundary(const std::string& jsonStr, txLaneBoundaries& boundaries);

  static bool parseLink(const std::string& jsonStr, txLaneLinks& links);

  static bool parseObj(const std::string& jsonStr, txObjects& objects);

  static bool parseRoute(const std::string& jsonStr, txRoute& route);
};
}  // namespace hadmap
