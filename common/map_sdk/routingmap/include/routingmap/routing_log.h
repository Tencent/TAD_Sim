// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include "structs/base_struct.h"
#include "types/map_defs.h"

namespace hadmap {
#define ROUTING_LOG(INFO) RoutingLog::getInstance()->Out(INFO);

class RoutingLog {
 private:
  RoutingLog();

  RoutingLog(const RoutingLog&) = delete;

  ~RoutingLog();

 public:
  static RoutingLog* getInstance();

  static void init(const std::string& logName);

 public:
  void Out(const std::string& info);

  void Out(const std::vector<roadpkid>& rids);

  void Out(const txPoint& pos);

 private:
  static RoutingLog* ins;

  std::string logPath;
};
}  // namespace hadmap
