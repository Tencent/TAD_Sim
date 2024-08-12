// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_on_wrong_side.h"
#include <cmath>

namespace eval {
const char EvalOnWrongSide::_kpi_name[] = "OnWrongSide";

sim_msg::TestReport_XYPlot EvalOnWrongSide::_s_yaw_to_lane_plot;

EvalOnWrongSide::EvalOnWrongSide() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalOnWrongSide::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }
  _yaw = -1;
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_yaw_to_lane_plot, "yaw to lane", "", "t", "s", {"yaw"}, {"rad"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_yaw_to_lane_plot, "", 0, 0, 1.57, "", 1, 0, INT32_MIN, 0);
  }
  return true;
}

bool EvalOnWrongSide::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // calculate exp data
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      _yaw = std::abs(ego_front->GetRefLineLateralYaw());
      _detector.Detect(_yaw, 1.57);

    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      _s_yaw_to_lane_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      _s_yaw_to_lane_plot.mutable_y_axis()->at(0).add_axis_data(_yaw);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalOnWrongSide::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_yaw_to_lane_plot);
  }
  return true;
}

void EvalOnWrongSide::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalOnWrongSide::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "reverse driving");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "reverse driving check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "reverse driving check skipped");
}

bool EvalOnWrongSide::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "reverse driving";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
