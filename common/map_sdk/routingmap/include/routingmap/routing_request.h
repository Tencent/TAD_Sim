// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <mutex>
#include <unordered_map>

#include "structs/hadmap_route.h"

#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"

#include "routingmap/id_generator.hpp"
#include "routingmap/routingmap_types.h"

namespace hadmap {
class RoutingRequest {
 public:
  explicit RoutingRequest(IdGeneratorPtr p);

  ~RoutingRequest();

  RoutingRequest(const RoutingRequest& rr) = delete;

 public:
  void init(txMapHandle* pH, const std::string& dbName);

  bool routingSync(const PointVec& poiLocs, const pkid_t& frameId, txRoute& route);

  RoutingStatus getRoute(txRoute& route);

 private:
  void resetRoadId(txRoute& route);

 private:
  IdGeneratorPtr idGenPtr;

  std::mutex routeMutex;

  RoutingStatus status;

  pkid_t frameId;

  txRoute routeResult;

  std::mutex handleMutex;

  txMapHandle* pHandle;

  std::string dbName;
};
}  // namespace hadmap
