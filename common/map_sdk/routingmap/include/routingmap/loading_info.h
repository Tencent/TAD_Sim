// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <memory>

#include "structs/base_struct.h"
#include "types/map_defs.h"

namespace hadmap {
enum LoadingType { LOADING_COORD = 1, LOADING_ROADID = 2, LOADING_PAUSE = 3, LOADING_ROADLIST = 4, LOADING_EXIT = 5 };

struct LoadingInfo {
  LoadingType type;

  explicit LoadingInfo(LoadingType t) : type(t) {}

  virtual ~LoadingInfo() {}

  LoadingInfo& operator=(const LoadingInfo&) = delete;
};

struct PauseInfo : public LoadingInfo {
  size_t pauseMS;

  explicit PauseInfo(const size_t& ms) : LoadingInfo(LOADING_PAUSE), pauseMS(ms) {}

  PauseInfo(const PauseInfo& pi) : LoadingInfo(LOADING_PAUSE), pauseMS(pi.pauseMS) {}
};

struct ExitInfo : public LoadingInfo {
  ExitInfo() : LoadingInfo(LOADING_EXIT) {}

  ExitInfo(const ExitInfo&) : LoadingInfo(LOADING_EXIT) {}
};

struct CoordInfo : public LoadingInfo {
  txPoint coord;

  explicit CoordInfo(const txPoint& p) : LoadingInfo(LOADING_COORD), coord(p) {}

  CoordInfo(const CoordInfo& ci) : LoadingInfo(LOADING_COORD), coord(ci.coord) {}
};

struct RoadIdInfo : public LoadingInfo {
  roadpkid rid;

  explicit RoadIdInfo(const roadpkid& id) : LoadingInfo(LOADING_ROADID), rid(id) {}

  RoadIdInfo(const RoadIdInfo& ri) : LoadingInfo(LOADING_ROADID), rid(ri.rid) {}
};

struct RoadListInfo : public LoadingInfo {
  std::vector<roadpkid> rids;

  RoadListInfo() : LoadingInfo(LOADING_ROADLIST) {}

  explicit RoadListInfo(const std::vector<roadpkid>& ids) : LoadingInfo(LOADING_ROADLIST), rids(ids) {}

  RoadListInfo(const RoadListInfo& rl) : LoadingInfo(LOADING_ROADLIST), rids(rl.rids) {}
};

using LoadingInfoPtr = std::shared_ptr<LoadingInfo>;
using CoordInfoPtr = std::shared_ptr<CoordInfo>;
using RoadIdInfoPtr = std::shared_ptr<RoadIdInfo>;
using PauseInfoPtr = std::shared_ptr<PauseInfo>;
using ExitInfoPtr = std::shared_ptr<ExitInfo>;
using RoadListInfoPtr = std::shared_ptr<RoadListInfo>;

}  // namespace hadmap
