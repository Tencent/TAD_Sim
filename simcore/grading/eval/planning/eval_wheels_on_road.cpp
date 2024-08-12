// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_wheels_on_road.h"

namespace eval {
const char EvalWheelsOnRoad::_kpi_name[] = "WheelsOnRoad";

sim_msg::TestReport_XYPlot EvalWheelsOnRoad::_s_wheel_on_road_plot;

EvalWheelsOnRoad::EvalWheelsOnRoad() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalWheelsOnRoad::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_wheel_on_road_plot, "wheels on road", "if wheels on road", "t", "s", {"if_on_road"},
                               {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_wheel_on_road_plot, "", 0, 0, INT32_MAX, "", 1, 0, 1, 0);
  }

  return true;
}
bool EvalWheelsOnRoad::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    auto map = ego_front->GetMapInfo();
    if (map.get() == nullptr) {
      VLOG_0 << "map ptr is null.\n";
      return false;
    }
    if (ego_front) {
      // step 1
      bool res = ego_front->CheckWheelWithinTheLaneBoundry() || map->m_in_junction;
      _fall_detector.Detect(res, 0.5);

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_wheel_on_road_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_wheel_on_road_plot.mutable_y_axis()->at(0).add_axis_data(res);
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
bool EvalWheelsOnRoad::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_wheel_on_road_plot);
  }
  return true;
}

void EvalWheelsOnRoad::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalWheelsOnRoad::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "four wheels not on road");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "wheels on road check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "wheels on road check skipped");
}

bool EvalWheelsOnRoad::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "wheels not on road";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
