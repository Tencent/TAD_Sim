// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/perception/perception_camera.h"
#include "eval/perception/perception_lane.h"
#include "eval/perception/perception_lidar.h"
#include "eval/perception/perception_obs.h"
#include "eval/perception/perception_traffic_light.h"

#include "eval_factory_interface.h"
#include "eval_factory_perception.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryPerception);

EvalFactoryPerception::EvalFactoryPerception() {
  std::cout << "EvalFactoryPerception constructed.\n";
  Register(EvalPerceptionCamera::_kpi_name, EvalPerceptionCamera::Build);
  Register(EvalPerceptionLane::_kpi_name, EvalPerceptionLane::Build);
  Register(EvalPerceptionLidar::_kpi_name, EvalPerceptionLidar::Build);
  Register(EvalPerceptionObs::_kpi_name, EvalPerceptionObs::Build);
  Register(Perception_TrafficLight::_kpi_name, Perception_TrafficLight::Build);
}
EvalFactoryPerception::~EvalFactoryPerception() {}
}  // namespace eval
