// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <memory>
#include <string>
#include "traffic.pb.h"
#include "visualizer.pb.h"

class ChomperGAInterface {
 public:
  enum ErrCode : int { gaSucc = 0, gaErr = 1 };
  ChomperGAInterface() = default;
  virtual ~ChomperGAInterface() = default;
  virtual const char *module_name(void) const noexcept = 0;
  virtual const char *module_version(void) const noexcept = 0;

  virtual ErrCode init(std::string input_params) noexcept = 0;
  virtual ErrCode connect(const sim_msg::Visualizer2TrafficCommand &v2tcmd) noexcept = 0;
  virtual ErrCode step(sim_msg::Traffic2VisualizerCommand **resultInfo, bool &isFinish) noexcept = 0;
  virtual ErrCode release() noexcept = 0;
};

std::shared_ptr<ChomperGAInterface> createGATrainer(void);
