// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/control/eval_acc_time_of_acc_lane.h"
#include "eval/control/eval_brake_dist.h"
#include "eval/control/eval_brake_stop.h"
#include "eval/control/eval_control.h"
#include "eval/control/eval_dec_time_of_dec_lane.h"
#include "eval/control/eval_dist_to_front_stop_veh.h"
#include "eval/control/eval_dist_to_stop_line_red_light.h"
#include "eval/control/eval_drive_off_midline_dis.h"
#include "eval/control/eval_heading_error.h"
#include "eval/control/eval_jerk_of_foll_start.h"
#include "eval/control/eval_jerk_of_foll_stop.h"
#include "eval/control/eval_lane_keeping.h"
#include "eval/control/eval_max_dist_foll_acc.h"
#include "eval/control/eval_min_dist_foll_dec.h"
#include "eval/control/eval_min_lat_safe_dist.h"
#include "eval/control/eval_min_lon_safe_dist.h"
#include "eval/control/eval_react_time_of_foll_acc.h"
#include "eval/control/eval_react_time_of_foll_dec.h"
#include "eval/control/eval_react_time_of_foll_start.h"
#include "eval/control/eval_serpentine_check.h"
#include "eval/control/eval_sharp_brake_check.h"
#include "eval/control/eval_slow_to_avoid.h"
#include "eval/control/eval_smooth_start.h"
#include "eval/control/eval_speed_error.h"
#include "eval/control/eval_stable_speed.h"

#include "eval_factory_control.h"
#include "eval_factory_interface.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryControl);

EvalFactoryControl::EvalFactoryControl() {
  std::cout << "EvalFactoryControl constructed.\n";
  Register(EvalControl::_kpi_name, EvalControl::Build);
  Register(EvalHeadingError::_kpi_name, EvalHeadingError::Build);
  Register(EvalSpeedError::_kpi_name, EvalSpeedError::Build);
  Register(EvalAccTimeOfAccLane::_kpi_name, EvalAccTimeOfAccLane::Build);
  Register(EvalBrakeDist::_kpi_name, EvalBrakeDist::Build);
  Register(EvalBrakeStop::_kpi_name, EvalBrakeStop::Build);
  Register(EvalDecTimeOfDecLane::_kpi_name, EvalDecTimeOfDecLane::Build);
  Register(EvalDistToFrontStopVeh::_kpi_name, EvalDistToFrontStopVeh::Build);
  Register(EvalDistToStopLineRedLight::_kpi_name, EvalDistToStopLineRedLight::Build);
  Register(EvalDriveOffMidlineDis::_kpi_name, EvalDriveOffMidlineDis::Build);
  Register(EvalJerkOfFollStart::_kpi_name, EvalJerkOfFollStart::Build);
  Register(EvalJerkOfFollStop::_kpi_name, EvalJerkOfFollStop::Build);
  Register(EvalLaneKeeping::_kpi_name, EvalLaneKeeping::Build);
  Register(EvalMaxDistFollAcc::_kpi_name, EvalMaxDistFollAcc::Build);
  Register(EvalMinDistFollDec::_kpi_name, EvalMinDistFollDec::Build);
  Register(EvalMinLatSafeDist::_kpi_name, EvalMinLatSafeDist::Build);
  Register(EvalMinLonSafeDist::_kpi_name, EvalMinLonSafeDist::Build);
  Register(EvalReactTimeOfFollAcc::_kpi_name, EvalReactTimeOfFollAcc::Build);
  Register(EvalReactTimeOfFollDec::_kpi_name, EvalReactTimeOfFollDec::Build);
  Register(EvalReactTimeOfFollStart::_kpi_name, EvalReactTimeOfFollStart::Build);
  Register(EvalSerpentineCheck::_kpi_name, EvalSerpentineCheck::Build);
  Register(EvalSharpBrakeCheck::_kpi_name, EvalSharpBrakeCheck::Build);
  Register(EvalSlowToAvoid::_kpi_name, EvalSlowToAvoid::Build);
  Register(EvalSmoothStart::_kpi_name, EvalSmoothStart::Build);
  Register(EvalStableSpeed::_kpi_name, EvalStableSpeed::Build);
}
EvalFactoryControl::~EvalFactoryControl() {}
}  // namespace eval
