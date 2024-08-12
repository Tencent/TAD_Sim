// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <memory>

#include "lcm/lcm-cpp.hpp"

#include "txsim_module.h"

class LCMAdapter final : public tx_sim::SimModule {
 public:
  void Init(tx_sim::InitHelper& helper) override;
  void Reset(tx_sim::ResetHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;
  void Stop(tx_sim::StopHelper& helper) override;

 private:
  void HandleMessageFromLCM(const lcm::ReceiveBuffer* rbuf, const std::string& channel);
  void ParseTopicParams(const std::string& param, std::vector<std::string>& topics);
  std::vector<std::string> sub_topics_, pub_topics_;
  std::unique_ptr<lcm::LCM> lcm_;
  tx_sim::Vector3d destination_;
  std::string lcm_url_;
  std::string payload_, received_topic_;
};
