// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_stop_stop_line_check.h"

namespace eval {
const char EvalStopStopLineCheck::_kpi_name[] = "StopStopLineCheck";

sim_msg::TestReport_XYPlot EvalStopStopLineCheck::_s_stop_stop_line_plot;

EvalStopStopLineCheck::EvalStopStopLineCheck() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalStopStopLineCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_stop_stop_line_plot, "StopLineTime", "time line of stopping", "t", "s",
                               {"onStopLine"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_stop_stop_line_plot, "", 0, 1, 1, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}
bool EvalStopStopLineCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      eval::StopLine stop_line;
      VLOG_1 << "if cur ego car on stopline: " << ego_front->IsOnStopLine(stop_line) << "\n";
      VLOG_1 << "the distance of car and stopline : " << ego_front->GetStopLineRoadDistance() << "\n";
      VLOG_1 << "stopline status around junction " << ego_front->GetStateWithStopLine() << "\n";
      // step 1
      bool res = ego_front->IsOnStopLine(stop_line) && ego_front->GetSpeed().GetNormal2D() <= 0.02;
      _detector.Detect(res, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_stop_stop_line_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_stop_stop_line_plot.mutable_y_axis()->at(0).add_axis_data(res);
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
bool EvalStopStopLineCheck::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_stop_stop_line_plot);
  }
  return true;
}

void EvalStopStopLineCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalStopStopLineCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "stop on the stop line");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "donot stop on stop online");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "donot stop on stop online");
}

bool EvalStopStopLineCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "stop on the stop line";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
