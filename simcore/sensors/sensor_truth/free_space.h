// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <map>
#include <set>
#include <vector>
#include "fov_filter_sensor.h"
#include "hmap.h"
#include "truth_base.h"
#include "kdtree_flann.h"

class FreeSpace : public TruthBase {
 public:
  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  double maxDis = 50;
  double limitZ = 0;
  // int utm_zone = 50;
  double grid_resolution = 0.5;
  std::string debugbmp;

  struct BoundLine {
    std::vector<Eigen::Vector3d> lines;
    int type = 0;
  };
  std::vector<BoundLine> boundPoints;
  kdtree_flann<double, std::pair<size_t, size_t>> bpFlann;

  std::set<int> fovids;

  const int N = 720;

  void bound_free_ring(FovFilterSensor &fov, std::vector<double> &fs, const std::vector<Eigen::Vector3d> &pts,
                       const std::vector<bool> &tag);

  std::vector<Eigen::Vector3d> sampling_line(const std::vector<Eigen::Vector3d> &pts);

  std::vector<Eigen::Vector3d> sampling_ply(const std::vector<Eigen::Vector3d> &pts);

  void traffic_free(FovFilterSensor &fov, std::vector<double> &fs,
                    std::vector<std::pair<int, std::vector<Eigen::Vector3d>>> &grid_pts, const Eigen::Vector3d pos,
                    double len, double wid, double heading, int label);

  void soline(std::vector<Eigen::Vector3d> &pts);

  auto addline(std::vector<Eigen::Vector3d> &pts, int type);

  bool InitMap();
};
