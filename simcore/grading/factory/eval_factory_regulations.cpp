// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

#include "eval/regulations/eval_behav_ban_signs.h"
#include "eval/regulations/eval_behav_instruct_sign.h"
#include "eval/regulations/eval_behav_red_light_check.h"
#include "eval/regulations/eval_behav_sign_line.h"
#include "eval/regulations/eval_behav_warn_sign.h"
#include "eval/regulations/eval_compliance.h"
#include "eval/regulations/eval_on_wrong_side.h"
#include "eval/regulations/eval_overtake_vio.h"
#include "eval/regulations/eval_rule38.h"
#include "eval/regulations/eval_rule40.h"
#include "eval/regulations/eval_rule45.h"
#include "eval/regulations/eval_rule46.h"
#include "eval/regulations/eval_rule47.h"
#include "eval/regulations/eval_rule48.h"
#include "eval/regulations/eval_rule49.h"
#include "eval/regulations/eval_rule50.h"
#include "eval/regulations/eval_rule51.h"
#include "eval/regulations/eval_rule53.h"
#include "eval/regulations/eval_rule57.h"
#include "eval/regulations/eval_rule58.h"
#include "eval/regulations/eval_rule59.h"
#include "eval/regulations/eval_rule63.h"
#include "eval/regulations/eval_rule67.h"
#include "eval/regulations/eval_rule78.h"
#include "eval/regulations/eval_rule79.h"
#include "eval/regulations/eval_rule80.h"
#include "eval/regulations/eval_rule81.h"
#include "eval/regulations/eval_rule82.h"
#include "eval/regulations/eval_rush_yellow_light.h"
#include "eval/regulations/eval_stop_stop_line_check.h"
#include "eval/regulations/eval_yield_pedestrian.h"
#include "eval/regulations/left_turn_signal_when_turning_to_left_at_intersection.h"
#include "eval/regulations/left_turn_signal_when_uturning_to_left_at_intersection.h"
#include "eval/regulations/no_left_turn_signal_when_turning_to_right.h"
#include "eval/regulations/no_over_speed_when_entering_or_exiting_non_motorized_lanes.h"
#include "eval/regulations/no_over_speed_when_limit_marking_speed.h"
#include "eval/regulations/no_over_speed_when_limit_sign_speed.h"
#include "eval/regulations/no_over_speed_when_limit_sign_speed_in_closed_area.h"
#include "eval/regulations/no_over_speed_when_limit_sign_speed_in_parking_lot.h"
#include "eval/regulations/no_over_speed_when_narrow_road.h"
#include "eval/regulations/no_over_speed_when_no_center_lane_on_city_road.h"
#include "eval/regulations/no_over_speed_when_poor_visibility.h"
#include "eval/regulations/no_over_speed_when_sharp_curve.h"
#include "eval/regulations/no_overtaking_when_expressway_acceleration_section.h"
#include "eval/regulations/no_overtaking_when_expressway_deceleration_section.h"
#include "eval/regulations/no_overtaking_when_expressway_ramps.h"
#include "eval/regulations/no_reversing_when_expressway.h"
#include "eval/regulations/no_reversing_when_intersections.h"
#include "eval/regulations/no_reversing_when_one_way_road.h"
#include "eval/regulations/no_reversing_when_tunnel.h"
#include "eval/regulations/no_right_turn_signal_when_turning_to_left.h"
#include "eval/regulations/no_stop_when_crosswalks.h"
#include "eval/regulations/no_stop_when_expressway_and_in_the_driveway.h"
#include "eval/regulations/no_stop_when_intersections.h"
#include "eval/regulations/no_stop_when_narrow_road.h"
#include "eval/regulations/no_stop_when_tunnel.h"
#include "eval/regulations/no_turn_left_when_no_left_turn_signs.h"
#include "eval/regulations/no_uturn_when_crosswalks.h"
#include "eval/regulations/no_uturn_when_no_uturn_signs.h"
#include "eval/regulations/no_uturn_when_tunnel.h"
#include "eval/regulations/open_low_beam_when_driving_night.h"
#include "eval/regulations/right_turn_signal_when_turning_to_right_at_intersection.h"
#include "eval/regulations/right_turn_signal_when_uturning_to_right_at_intersection.h"
#include "eval/regulations/run_but_wait_when_traffic_light_is_yellow.h"
#include "eval/regulations/run_when_traffic_light_is_green.h"
#include "eval/regulations/stop_outside_the_intersection_when_stop_signal.h"
#include "eval/regulations/stop_outside_the_stop_line_when_stop_signal_and_stop_lane.h"
#include "eval/regulations/stop_when_traffic_light_is_red.h"

#include "eval_factory_interface.h"
#include "eval_factory_regulations.h"

namespace eval {
// global factory variable
DefineEvalFactory(EvalFactoryRegulations);

EvalFactoryRegulations::EvalFactoryRegulations() {
  std::cout << "EvalFactoryRegulations constructed.\n";
  Register(EvalRule38::_kpi_name, EvalRule38::Build);
  Register(EvalRule40::_kpi_name, EvalRule40::Build);
  Register(EvalRule45::_kpi_name, EvalRule45::Build);
  Register(EvalRule46::_kpi_name, EvalRule46::Build);
  Register(EvalRule47::_kpi_name, EvalRule47::Build);
  Register(EvalRule48::_kpi_name, EvalRule48::Build);
  Register(EvalRule49::_kpi_name, EvalRule49::Build);
  Register(EvalRule50::_kpi_name, EvalRule50::Build);
  Register(EvalRule51::_kpi_name, EvalRule51::Build);
  Register(EvalRule53::_kpi_name, EvalRule53::Build);
  Register(EvalRule57::_kpi_name, EvalRule57::Build);
  Register(EvalRule58::_kpi_name, EvalRule58::Build);
  Register(EvalRule59::_kpi_name, EvalRule59::Build);
  Register(EvalRule63::_kpi_name, EvalRule63::Build);
  Register(EvalRule67::_kpi_name, EvalRule67::Build);
  Register(EvalRule78::_kpi_name, EvalRule78::Build);
  Register(EvalRule79::_kpi_name, EvalRule79::Build);
  Register(EvalRule80::_kpi_name, EvalRule80::Build);
  Register(EvalRule81::_kpi_name, EvalRule81::Build);
  Register(EvalRule82::_kpi_name, EvalRule82::Build);
  Register(LeftTurnSignalWhenTurningToLeftAtIntersection::_kpi_name,
           LeftTurnSignalWhenTurningToLeftAtIntersection::Build);
  Register(LeftTurnSignalWhenUTurningToLeftAtIntersection::_kpi_name,
           LeftTurnSignalWhenUTurningToLeftAtIntersection::Build);
  Register(NoLeftTurnSignalWhenTurningToRight::_kpi_name, NoLeftTurnSignalWhenTurningToRight::Build);
  Register(NoOverSpeedWhenLimitMarkingSpeed::_kpi_name, NoOverSpeedWhenLimitMarkingSpeed::Build);
  Register(NoOverSpeedWhenLimitSignSpeed::_kpi_name, NoOverSpeedWhenLimitSignSpeed::Build);
  Register(NoOverSpeedWhenLimitSignSpeedInClosedArea::_kpi_name, NoOverSpeedWhenLimitSignSpeedInClosedArea::Build);
  Register(NoOverSpeedWhenLimitSignSpeedInParkingLot::_kpi_name, NoOverSpeedWhenLimitSignSpeedInParkingLot::Build);
  Register(NoOvertakingWhenExpresswayAccelerationSection::_kpi_name,
           NoOvertakingWhenExpresswayAccelerationSection::Build);
  Register(NoOvertakingWhenExpresswayDecelerationSection::_kpi_name,
           NoOvertakingWhenExpresswayDecelerationSection::Build);
  Register(NoOvertakingWhenExpresswayRamps::_kpi_name, NoOvertakingWhenExpresswayRamps::Build);
  Register(NoReversingWhenExpressway::_kpi_name, NoReversingWhenExpressway::Build);
  Register(NoReversingWhenIntersections::_kpi_name, NoReversingWhenIntersections::Build);
  Register(NoStopWhenNarrowRoad::_kpi_name, NoStopWhenNarrowRoad::Build);
  Register(NoReversingWhenOneWayRoad::_kpi_name, NoReversingWhenOneWayRoad::Build);
  Register(NoReversingWhenTunnel::_kpi_name, NoReversingWhenTunnel::Build);
  Register(NoStopWhenTunnel::_kpi_name, NoStopWhenTunnel::Build);
  Register(NoRightTurnSignalWhenTurningToLeft::_kpi_name, NoRightTurnSignalWhenTurningToLeft::Build);
  Register(NoStopWhenCrosswalks::_kpi_name, NoStopWhenCrosswalks::Build);
  Register(NoStopWhenIntersections::_kpi_name, NoStopWhenIntersections::Build);
  Register(NoStopWhenExpresswayAndInTheDriveway::_kpi_name, NoStopWhenExpresswayAndInTheDriveway::Build);
  Register(NoTurnLeftWhenNoLeftTurnSigns::_kpi_name, NoTurnLeftWhenNoLeftTurnSigns::Build);
  Register(NoUTurnWhenCrosswalks::_kpi_name, NoUTurnWhenCrosswalks::Build);
  Register(NoUTurnWhenNoUTurnSigns::_kpi_name, NoUTurnWhenNoUTurnSigns::Build);
  Register(NoUTurnWhenTunnel::_kpi_name, NoUTurnWhenTunnel::Build);
  Register(OpenLowBeamWhenDrivingNight::_kpi_name, OpenLowBeamWhenDrivingNight::Build);
  Register(RightTurnSignalWhenTurningToRightAtIntersection::_kpi_name,
           RightTurnSignalWhenTurningToRightAtIntersection::Build);
  Register(RightTurnSignalWhenUTurningToRightAtIntersection::_kpi_name,
           RightTurnSignalWhenUTurningToRightAtIntersection::Build);
  Register(RunButWaitWhenTrafficLightIsYellow::_kpi_name, RunButWaitWhenTrafficLightIsYellow::Build);
  Register(RunWhenTrafficLightIsGreen::_kpi_name, RunWhenTrafficLightIsGreen::Build);
  Register(StopOutsideTheIntersectionWhenStopSignal::_kpi_name, StopOutsideTheIntersectionWhenStopSignal::Build);
  Register(StopOutsideTheStopLineWhenStopSignalAndStopLane::_kpi_name,
           StopOutsideTheStopLineWhenStopSignalAndStopLane::Build);
  Register(StopWhenTrafficLightIsRed::_kpi_name, StopWhenTrafficLightIsRed::Build);
  Register(NoOverSpeedWhenEnteringOrExitingNonMotorizedLanes::_kpi_name,
           NoOverSpeedWhenEnteringOrExitingNonMotorizedLanes::Build);
  Register(NoOverSpeedWhenNarrowRoad::_kpi_name, NoOverSpeedWhenNarrowRoad::Build);
  Register(NoOverSpeedWhenNoCenterLaneOnCityRoad::_kpi_name, NoOverSpeedWhenNoCenterLaneOnCityRoad::Build);
  Register(NoOverSpeedWhenPoorVisibility::_kpi_name, NoOverSpeedWhenPoorVisibility::Build);
  Register(NoOverSpeedWhenSharpCurve::_kpi_name, NoOverSpeedWhenSharpCurve::Build);
  Register(EvalBehavInstructSign::_kpi_name, EvalBehavInstructSign::Build);
  Register(EvalBehavSignLine::_kpi_name, EvalBehavSignLine::Build);
  Register(EvalBehavWarnSign::_kpi_name, EvalBehavWarnSign::Build);
  Register(EvalCompliance::_kpi_name, EvalCompliance::Build);
  Register(EvalOnWrongSide::_kpi_name, EvalOnWrongSide::Build);
  Register(EvalOvertakeVio::_kpi_name, EvalOvertakeVio::Build);
  Register(EvalRushYellowLight::_kpi_name, EvalRushYellowLight::Build);
  Register(EvalStopStopLineCheck::_kpi_name, EvalStopStopLineCheck::Build);
  Register(EvalYieldPedestrian::_kpi_name, EvalYieldPedestrian::Build);
  Register(EvalBehavBanSigns::_kpi_name, EvalBehavBanSigns::Build);
  Register(EvalBehavRedLightCheck::_kpi_name, EvalBehavRedLightCheck::Build);
}
EvalFactoryRegulations::~EvalFactoryRegulations() {}
}  // namespace eval
