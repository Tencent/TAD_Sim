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
class TXSIMMAP_API TransInterface {
 public:
  TransInterface();

  explicit TransInterface(const std::string& cfgPath);

  TransInterface(const TransInterface&) = delete;

  ~TransInterface();

 public:
  std::string location() const;

  bool get(const std::string& url, std::string& jsonData);

  bool post(const std::string& url, const std::string& postJson, std::string& jsonData);

 private:
  std::string baseUrl;
};

}  // namespace hadmap
