// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <list>
#include <queue>
#include <vector>
#include "eval/eval_base.h"
#include "trajectory.pb.h"

namespace eval {
using EvalTrajPoints = std::vector<std::shared_ptr<CDynamicActor>>;
struct FellowTrajInfo;
using FellowTrajInfoPtr = std::shared_ptr<FellowTrajInfo>;
using FellowTrajActualPtr = std::shared_ptr<CDynamicActor>;

// one fellow at timestamp "m_t" with its id, prediciton and actual trajectory in next 1 second
struct FellowTrajInfo {
 public:
  int64_t m_id;                      // id of fellow
  double m_t;                        // simulation timestamp, unit second
  sim_msg::Prediction m_prediction;  // prediction of fellow at timestamp "m_t"
  EvalTrajPoints m_actual_traj;      // actual fellow trajectory in next 1 second

  FellowTrajInfo() {
    m_id = INT64_MIN;
    m_t = 0.0;
    m_actual_traj.reserve(traj_max_size);
  }

 private:
  const int traj_max_size = 500;
};

struct FellowPredManager {
 public:
  // reset
  void Reset();

  // pop and return if has enough trajectory (simply popped out if too old)
  std::vector<FellowTrajInfoPtr> FilterFellows(double t_s, double t_thresh = 1.0);

  // push fellow prediction
  void Add(FellowTrajInfoPtr fellow_info_ptr);

  // update fellow trajectory
  void Add(int64_t fellow_id, FellowTrajActualPtr fellow);

  // size of fellow queue
  size_t Size();

 private:
  std::list<FellowTrajInfoPtr> m_fellow_info_list;
};
}  // namespace eval
