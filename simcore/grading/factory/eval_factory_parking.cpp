// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/parking/eval_parking_access_times.h"
#include "eval/parking/eval_parking_comfort.h"
#include "eval/parking/eval_parking_curvature.h"
#include "eval/parking/eval_parking_duration.h"
#include "eval/parking/eval_parking_end_regulate.h"
#include "eval/parking/eval_parking_identified_distance.h"
#include "eval/parking/eval_parking_miss_rate.h"
#include "eval/parking/eval_parking_out_access_times.h"
#include "eval/parking/eval_parking_precision.h"
#include "eval/parking/eval_parking_road_width.h"
#include "eval/parking/eval_parking_safe_distance.h"
#include "eval/parking/eval_parking_search_accuracy.h"
#include "eval/parking/eval_parking_search_angle.h"
#include "eval/parking/eval_parking_search_distance.h"
#include "eval/parking/eval_parking_search_duration.h"
#include "eval/parking/eval_parking_search_memory_num.h"
#include "eval/parking/eval_parking_search_speed.h"
#include "eval/parking/eval_parking_slope.h"
#include "eval/parking/eval_parking_space_check.h"
#include "eval/parking/eval_parking_space_length.h"
#include "eval/parking/eval_parking_speed.h"
#include "eval/parking/eval_parking_speed_bump_height.h"
#include "eval/parking/eval_parking_success.h"
#include "eval/parking/eval_parking_type_search_accuracy.h"
#include "eval/parking/eval_parking_wheel_angle.h"

#include "eval_factory_interface.h"
#include "eval_factory_parking.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryParking);

EvalFactoryParking::EvalFactoryParking() {
  std::cout << "EvalFactoryParking constructed.\n";
  Register(EvalParkingAccessTimes::_kpi_name, EvalParkingAccessTimes::Build);
  Register(EvalParkingComfort::_kpi_name, EvalParkingComfort::Build);
  Register(EvalParkingCurvature::_kpi_name, EvalParkingCurvature::Build);
  Register(EvalParkingDuration::_kpi_name, EvalParkingDuration::Build);
  Register(EvalParkingEndRegulate::_kpi_name, EvalParkingEndRegulate::Build);
  Register(EvalParkingIdentifiedDistance::_kpi_name, EvalParkingIdentifiedDistance::Build);
  Register(EvalParkingMissRate::_kpi_name, EvalParkingMissRate::Build);
  Register(EvalParkingOutAccessTimes::_kpi_name, EvalParkingOutAccessTimes::Build);
  Register(EvalParkingPrecision::_kpi_name, EvalParkingPrecision::Build);
  Register(EvalParkingRoadWidth::_kpi_name, EvalParkingRoadWidth::Build);
  Register(EvalParkingSafeDistance::_kpi_name, EvalParkingSafeDistance::Build);
  Register(EvalParkingSearchAccuracy::_kpi_name, EvalParkingSearchAccuracy::Build);
  Register(EvalParkingSearchAngle::_kpi_name, EvalParkingSearchAngle::Build);
  Register(EvalParkingSearchDistance::_kpi_name, EvalParkingSearchDistance::Build);
  Register(EvalParkingSearchDuration::_kpi_name, EvalParkingSearchDuration::Build);
  Register(EvalParkingSearchMemoryNum::_kpi_name, EvalParkingSearchMemoryNum::Build);
  Register(EvalParkingSearchSpeed::_kpi_name, EvalParkingSearchSpeed::Build);
  Register(EvalParkingSlope::_kpi_name, EvalParkingSlope::Build);
  Register(EvalParkingSpaceCheck::_kpi_name, EvalParkingSpaceCheck::Build);
  Register(EvalParkingSpaceLength::_kpi_name, EvalParkingSpaceLength::Build);
  Register(EvalParkingSpeed::_kpi_name, EvalParkingSpeed::Build);
  Register(EvalParkingSpeedBumpHeight::_kpi_name, EvalParkingSpeedBumpHeight::Build);
  Register(EvalParkingSuccess::_kpi_name, EvalParkingSuccess::Build);
  Register(EvalParkingTypeSearchAccuracy::_kpi_name, EvalParkingTypeSearchAccuracy::Build);
  Register(EvalParkingWheelAngle::_kpi_name, EvalParkingWheelAngle::Build);
}
EvalFactoryParking::~EvalFactoryParking() {}
}  // namespace eval
