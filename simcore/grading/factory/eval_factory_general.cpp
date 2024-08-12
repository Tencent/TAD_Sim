// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/general/eval_acc.h"
#include "eval/general/eval_car_speed_statistics.h"
#include "eval/general/eval_chassis_msg.h"
#include "eval/general/eval_collision.h"
#include "eval/general/eval_collision_active.h"
#include "eval/general/eval_collision_passive.h"
#include "eval/general/eval_control_msg.h"
#include "eval/general/eval_dec.h"
#include "eval/general/eval_duration.h"
#include "eval/general/eval_endpoint.h"
#include "eval/general/eval_lat_acc_jerk.h"
#include "eval/general/eval_lat_dec_jerk.h"
#include "eval/general/eval_lateral_acc.h"
#include "eval/general/eval_loc_msg.h"
#include "eval/general/eval_long_acc_jerk.h"
#include "eval/general/eval_long_dec_jerk.h"
#include "eval/general/eval_prediction_msg.h"
#include "eval/general/eval_speed.h"
#include "eval/general/eval_stop_time_statistics.h"
#include "eval/general/eval_traj_msg.h"
#include "eval/general/eval_yaw_rate.h"

#include "eval_factory_general.h"
#include "eval_factory_interface.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryGeneral);

EvalFactoryGeneral::EvalFactoryGeneral() {
  std::cout << "EvalFactoryGeneral constructed.\n";
  Register(EvalAcc::_kpi_name, EvalAcc::Build);
  Register(EvalSpeed::_kpi_name, EvalSpeed::Build);
  Register(EvalDec::_kpi_name, EvalDec::Build);
  Register(EvalLateralAcc::_kpi_name, EvalLateralAcc::Build);
  Register(EvalDuration::_kpi_name, EvalDuration::Build);
  Register(EvalEndPoint::_kpi_name, EvalEndPoint::Build);
  Register(EvalCollision::_kpi_name, EvalCollision::Build);
  Register(EvalYawRate::_kpi_name, EvalYawRate::Build);
  Register(EvalLocMsg::_kpi_name, EvalLocMsg::Build);
  Register(EvalTrajMsg::_kpi_name, EvalTrajMsg::Build);
  Register(EvalPredictionMsg::_kpi_name, EvalPredictionMsg::Build);
  Register(EvalControlMsg::_kpi_name, EvalControlMsg::Build);
  Register(EvalChassisMsg::_kpi_name, EvalChassisMsg::Build);
  Register(EvalLongDecJerk::_kpi_name, EvalLongDecJerk::Build);
  Register(EvalLongAccJerk::_kpi_name, EvalLongAccJerk::Build);
  Register(EvalCarSpeedStatistics::_kpi_name, EvalCarSpeedStatistics::Build);
  Register(EvalCollisionActive::_kpi_name, EvalCollisionActive::Build);
  Register(EvalCollisionPassive::_kpi_name, EvalCollisionPassive::Build);
  Register(EvalStopTimeStatistics::_kpi_name, EvalStopTimeStatistics::Build);
  Register(EvalLatAccJerk::_kpi_name, EvalLatAccJerk::Build);
  Register(EvalLatDecJerk::_kpi_name, EvalLatDecJerk::Build);
}
EvalFactoryGeneral::~EvalFactoryGeneral() {}
}  // namespace eval
