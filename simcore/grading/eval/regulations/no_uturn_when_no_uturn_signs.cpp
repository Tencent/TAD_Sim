// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_uturn_when_no_uturn_signs.h"

namespace eval {
const char NoUTurnWhenNoUTurnSigns::_kpi_name[] = "NoUTurnWhenNoUTurnSigns";
sim_msg::TestReport_XYPlot NoUTurnWhenNoUTurnSigns::s_no_turn_when_no_turn_signs_plot;

NoUTurnWhenNoUTurnSigns::NoUTurnWhenNoUTurnSigns() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool NoUTurnWhenNoUTurnSigns::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _event_no_uturn = false;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_turn_when_no_turn_signs_plot, "NoRightTurnSignalWhenTurningToLeft check", "", "t",
                               "s", {"_result", "_cond_u_turn", "_cond_has_u_turn_sign"}, {"OnOff", "OnOff", "OnOff"},
                               3);

    ReportHelper::ConfigXYPlotThreshold(s_no_turn_when_no_turn_signs_plot, "NoUTurnWhenNoUTurnSigns Violation ", 0, 1,
                                        _threshold, "", 0, 0, INT32_MIN, 0);
  }
  return true;
}

bool NoUTurnWhenNoUTurnSigns::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (!IsModuleValid() || !m_KpiEnabled) {
    VLOG_0 << _kpi_name << " kpi not enabled.\n";
    return false;
  }

  // get the ego pointer and check whether the pointer is null
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  if (!ego_front) {
    VLOG_0 << _kpi_name << " ego actor missing.\n";
    return false;
  }

  // get the map information pointer and check whether the pointer is null
  auto map_info = ego_front->GetMapInfo();
  if (map_info.get() == nullptr) {
    VLOG_0 << _kpi_name << " ego actor missing with map.\n";
    return false;
  }

  // hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
  // if (!ego_lane) {
  //     VLOG_0 << _kpi_name << " ego actor missing with lane.\n";
  //     return false;
  // }

  // hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
  // if (!ego_road) {
  //     VLOG_0 << _kpi_name << " ego actor missing with road.\n";
  //     return false;
  // }

  // _cond_has_u_turn_sign = false;
  _result = 0.0;

  // todo: 1.get sign from map
  EvalMapObjectPtr obj(new EvalMapObject());
  ego_front->GetProhibitionSign(obj, 30);
  auto ban_signs = obj->m_map_objects;
  for (auto ban_sign : ban_signs) {
    hadmap::OBJECT_SUB_TYPE sign_sub_type = ban_sign->getObjectSubType();
    if (sign_sub_type == hadmap::SIGN_BAN_UTURN) {
      _cond_has_u_turn_sign = true;
    }
  }

  if (!_event_no_uturn && _cond_has_u_turn_sign && !map_info->m_on_road) {
    _event_no_uturn = true;
  }

  // 2.Get actual value
  // output of state-flow
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();

  bool _cond_uturn = false;
  _cond_uturn =
      (output.m_veh_behav == VehicleBehavior::U_TurnLeft || output.m_veh_behav == VehicleBehavior::U_TurnRight);
  if (_event_no_uturn) {
    if (map_info->m_on_road) {
      _result = _cond_uturn;
      _event_no_uturn = false;
      _cond_has_u_turn_sign = false;
    }
    VLOG_1 << "flag:" << output.m_veh_behav << "\n";
  }

  // 4.Check Detect and report
  // check if the car is u-turn under conditions
  // if (!_cond_has_u_turn_sign && _cond_u_turn) {
  //     _result = 1.0;
  // }

  // Detect and report
  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should no u-turn, current ego u-turn status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_turn_when_no_turn_signs_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_turn_when_no_turn_signs_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_no_turn_when_no_turn_signs_plot.mutable_y_axis()->at(1).add_axis_data(_cond_uturn);
    s_no_turn_when_no_turn_signs_plot.mutable_y_axis()->at(2).add_axis_data(_cond_has_u_turn_sign);
  }

  return true;
}

bool NoUTurnWhenNoUTurnSigns::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_no_turn_when_no_turn_signs_plot);
  }

  return true;
}

void NoUTurnWhenNoUTurnSigns::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult NoUTurnWhenNoUTurnSigns::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should no u-turn");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no u-turn check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "no u-turn check skipped");
}

bool NoUTurnWhenNoUTurnSigns::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should no u-turn";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
