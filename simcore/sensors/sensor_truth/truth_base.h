/**
 * @file TruthBase.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <cmath>
#include "location.pb.h"
#include "traffic.pb.h"
#include "txsim_module.h"
#include "union.pb.h"

class TruthBase : public tx_sim::SimModule {
 public:
  void Init(tx_sim::InitHelper &helper) {}
  void Reset(tx_sim::ResetHelper &helper) {}
  void Step(tx_sim::StepHelper &helper) {}
  void Stop(tx_sim::StopHelper &helper) {}

 protected:
  std::int64_t ego_id = 0;
  std::string device;
  std::string debugDir;
  tx_sim::Vector3d map_ori;

  friend class Truth;

  sim_msg::Union getUnion(tx_sim::StepHelper &helper);
  sim_msg::Traffic getTraffic(tx_sim::StepHelper &helper);
  sim_msg::Location getLocation(tx_sim::StepHelper &helper);
};

#define KPH2MPS 0.277777777777777777777
#define MPS2KPH 3.6
inline void rotate(double &x, double &y, double sita) {
  double s = x * std::cos(sita) - y * std::sin(sita);
  double t = x * std::sin(sita) + y * std::cos(sita);
  x = s;
  y = t;
}

bool SaveBMP(const std::string &fname, const std::vector<uint8_t> &buf, int w, int h);
