#pragma once
#include <limits.h>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <string>
#include "memory_io.h"
#include "txsim_module.h"

class CameraFun : public tx_sim::SimModule {
 public:
  CameraFun() = default;

  void Init(tx_sim::InitHelper& helper) override;
  void Reset(tx_sim::ResetHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;
  void Stop(tx_sim::StopHelper& helper) override;

 protected:
 private:
  SharedMemoryReader src_sim;
  int w = 0, h = 0;

  cv::Mat cvImg;
};
