/**
 * @file Truth.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <memory>
#include "truth_base.h"
#include "txsim_module.h"

class Truth final : public tx_sim::SimModule {
 public:
  Truth();
  virtual ~Truth();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  std::vector<std::shared_ptr<TruthBase>> sensors;

  std::string tadsimPath;
  std::string device;
  std::int64_t ego_id = 0;
  bool getSensorConfig(const std::string &sfname, const std::string &groupname);
};
