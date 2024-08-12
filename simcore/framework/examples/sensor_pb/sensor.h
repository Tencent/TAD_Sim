#pragma once

#include <map>
#include <string>
#include <vector>
#include "sensor_raw.pb.h"
#include "thread_pool.h"
#include "txsim_module.h"

class sim_sensor final : public tx_sim::SimModule {
 public:
  sim_sensor();
  virtual ~sim_sensor();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  std::string savePath = "/home/sim/sensor_data";
  int disNum = 30;

  ThreadPool threads;
};
