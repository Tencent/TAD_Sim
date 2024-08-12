// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include "../sensor_objects_def.h"

struct SimAlgorithmRet {
  double frequency = 1;
};

class SimBase {
 public:
  virtual void RunSim() = 0;
};

class SimObuBase : public SimBase {
 public:
  SimObuBase() {}
  ~SimObuBase() {}

  void RunSim() override;
  virtual void ObuSimExcute(const SimObus& sim, SimAlgorithmRet& sim_ret) = 0;
};

class SimRsuBase : public SimBase {
 public:
  SimRsuBase() {}
  ~SimRsuBase() {}

  void RunSim() override;
  virtual void RsuSimExcute(const SimSensorObjects& sim, SimAlgorithmRet& sim_ret) = 0;
};

class SimConfictBase : public SimBase {
 public:
  SimConfictBase() {}
  ~SimConfictBase() {}

  void RunSim() override;
};
