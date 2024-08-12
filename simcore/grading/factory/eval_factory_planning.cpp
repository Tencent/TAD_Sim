// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/planning/eval_around_obstacles.h"
#include "eval/planning/eval_behav_lane_add.h"
#include "eval/planning/eval_behav_lane_reduce.h"
#include "eval/planning/eval_behav_veh_cut_in.h"
#include "eval/planning/eval_behav_veh_cut_out.h"
#include "eval/planning/eval_cross_junction_time.h"
#include "eval/planning/eval_cross_solid_line.h"
#include "eval/planning/eval_ego_offroad.h"
#include "eval/planning/eval_lane_change_duration.h"
#include "eval/planning/eval_lane_change_indicator.h"
#include "eval/planning/eval_max_speed_on_ramp.h"
#include "eval/planning/eval_on_road.h"
#include "eval/planning/eval_plan_status.h"
#include "eval/planning/eval_react_opposite_veh.h"
#include "eval/planning/eval_ref_line_offset.h"
#include "eval/planning/eval_start_up_indicator.h"
#include "eval/planning/eval_success_rate_of_change_lane.h"
#include "eval/planning/eval_thw.h"
#include "eval/planning/eval_traffic_light.h"
#include "eval/planning/eval_traj_acc.h"
#include "eval/planning/eval_traj_dec.h"
#include "eval/planning/eval_traj_jerk.h"
#include "eval/planning/eval_traj_kappa.h"
#include "eval/planning/eval_traj_yaw_rate.h"
#include "eval/planning/eval_ttc.h"
#include "eval/planning/eval_turn_indicator.h"
#include "eval/planning/eval_undo_lane_change.h"
#include "eval/planning/eval_way_points.h"
#include "eval/planning/eval_wheels_on_road.h"

#include "eval_factory_interface.h"
#include "eval_factory_planning.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryPlanning);

EvalFactoryPlanning::EvalFactoryPlanning() {
  std::cout << "EvalFactoryPlanning constructed.\n";
  Register(EvalCrossSolidLine::_kpi_name, EvalCrossSolidLine::Build);
  Register(EvalTrafficLight::_kpi_name, EvalTrafficLight::Build);
  Register(EvalTTC::_kpi_name, EvalTTC::Build);
  Register(EvalTHW::_kpi_name, EvalTHW::Build);
  Register(EvalRefLineOffset::_kpi_name, EvalRefLineOffset::Build);
  Register(EvalLaneChangeIndicator::_kpi_name, EvalLaneChangeIndicator::Build);
  Register(EvalLaneChangeDuration::_kpi_name, EvalLaneChangeDuration::Build);
  Register(EvalTurnIndicator::_kpi_name, EvalTurnIndicator::Build);
  Register(EvalStartUpIndicator::_kpi_name, EvalStartUpIndicator::Build);
  Register(EvalTrajAcc::_kpi_name, EvalTrajAcc::Build);
  Register(EvalTrajDec::_kpi_name, EvalTrajDec::Build);
  Register(EvalTrajJerk::_kpi_name, EvalTrajJerk::Build);
  Register(EvalTrajKappa::_kpi_name, EvalTrajKappa::Build);
  Register(EvalTrajYawRate::_kpi_name, EvalTrajYawRate::Build);
  Register(EvalWayPoints::_kpi_name, EvalWayPoints::Build);
  Register(EvalPlanStatus::_kpi_name, EvalPlanStatus::Build);
  Register(EvalEgoOffroad::_kpi_name, EvalEgoOffroad::Build);
  Register(EvalAroundObstacles::_kpi_name, EvalAroundObstacles::Build);
  Register(EvalBehavLaneAdd::_kpi_name, EvalBehavLaneAdd::Build);
  Register(EvalBehavLaneReduce::_kpi_name, EvalBehavLaneReduce::Build);
  Register(EvalBehavVehCutIn::_kpi_name, EvalBehavVehCutIn::Build);
  Register(EvalBehavVehCutOut::_kpi_name, EvalBehavVehCutOut::Build);
  Register(EvalCrossJunctionTime::_kpi_name, EvalCrossJunctionTime::Build);
  Register(EvalMaxSpeedOnRamp::_kpi_name, EvalMaxSpeedOnRamp::Build);
  Register(EvalOnRoad::_kpi_name, EvalOnRoad::Build);
  Register(EvalSuccessRateOfChangeLane::_kpi_name, EvalSuccessRateOfChangeLane::Build);
  Register(EvalUndoLaneChange::_kpi_name, EvalUndoLaneChange::Build);
  Register(EvalWheelsOnRoad::_kpi_name, EvalWheelsOnRoad::Build);
  Register(EvalReactOppositeVeh::_kpi_name, EvalReactOppositeVeh::Build);
}
EvalFactoryPlanning::~EvalFactoryPlanning() {}
}  // namespace eval
