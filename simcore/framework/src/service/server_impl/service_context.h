// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <mutex>

#include "zmq.h"

#include "coordinator/config.h"
#include "core_bundles.h"

namespace tx_sim {
namespace service {

class LocalServiceContext final {
 public:
  std::shared_ptr<zmq::context_t> zmq_ctx;

  LocalServiceContext(const std::string& app_root_path, uint16_t instance_id);
  ~LocalServiceContext() {}

  CoreComponents& core() { return core_; }

  void UpdateScenarioLog2WorldConfig(const std::string& s_path, const tx_sim::coordinator::Log2WorldPlayConfig& cfg) {
    std::lock_guard<std::mutex> lk(scenario_l2w_configs_mtx_);
    scenario_l2w_configs_[s_path] = cfg;
  }

  tx_sim::coordinator::Log2WorldPlayConfig GetScenarioLog2WorldConfig(const std::string& s_path) {
    std::lock_guard<std::mutex> lk(scenario_l2w_configs_mtx_);
    return scenario_l2w_configs_[s_path];
  }

 private:
  CoreComponents core_;
  std::unordered_map<std::string, tx_sim::coordinator::Log2WorldPlayConfig> scenario_l2w_configs_;
  std::mutex scenario_l2w_configs_mtx_;
};

}  // namespace service
}  // namespace tx_sim
