// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_driving_safety.h"

namespace eval {
const char EvalDrivingSafety::_kpi_name[] = "DrivingSafety";

sim_msg::TestReport_XYPlot _s_drive_safety_plot;

EvalDrivingSafety::EvalDrivingSafety() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _count_collision = 0;
  _collosion_happend = false;
}
bool EvalDrivingSafety::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_drive_safety_plot, "collision accident rate", "", "t", "s", {"I_collision"},
                               {"times/million km"}, 1);
    /**
     * set Threshold after ConfigXYPlot
     * @param    xy_plot         TestReport_XYPlot
     * @param    upper_desc      the desc of upper threshold.
     * @param    upper_space     the IntervalSpace of upper threshold.
     * @param    upper_type      the IntervalType of upper threshold.
     * @param    upper_value     the value of upper threshold.
     * @param    lower_desc      the desc of lower threshold.
     * @param    lower_space     the IntervalSpace of lower threshold.
     * @param    lower_type      the IntervalType of lower threshold.
     * @param    lower_value     the value of lower threshold.
     * @param    y_axis_id       the id of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(_s_drive_safety_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}
bool EvalDrivingSafety::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    DynamicActorList &&dynamic_actors = _actor_mgr->GetFellowActorsByType<CDynamicActorPtr>(Actor_Dynamic);
    VehilceActorList &&vehicle_actors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);
    if (ego_front) {
      // find collision with vehicle and dynamic
      auto collision_actor = EvalStep::FindCollisionFellow(ego_front, vehicle_actors);
      auto collision_dynamic = EvalStep::FindCollisionFellow(ego_front, dynamic_actors);
      if (!_collosion_happend && (collision_actor || collision_dynamic)) {
        _collosion_happend = true;
        _count_collision++;
      }
      if (_collosion_happend && !(collision_actor || collision_dynamic)) {
        _collosion_happend = false;
      }
      VLOG_1 << "if collision happened: " << _collosion_happend << "\n";
      double dis = helper.GetGradingMsg().mileage() == 0 ? 1 : helper.GetGradingMsg().mileage();
      VLOG_1 << "dis is:" << dis << "\n";
      double _res = _count_collision * 1e6 / dis;
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_drive_safety_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_drive_safety_plot.mutable_y_axis()->at(0).add_axis_data(_res);
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalDrivingSafety::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_drive_safety_plot);
  }
  return true;
}

void EvalDrivingSafety::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalDrivingSafety::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "no check");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "no check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "no check skipped");
}

bool EvalDrivingSafety::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "no check";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
