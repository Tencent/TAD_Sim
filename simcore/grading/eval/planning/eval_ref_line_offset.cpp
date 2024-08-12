// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_ref_line_offset.h"

namespace eval {
const char EvalRefLineOffset::_kpi_name[] = "LaneRefLineOffset";

sim_msg::TestReport_XYPlot EvalRefLineOffset::s_ref_line_offset_plot;
sim_msg::TestReport_PairData EvalRefLineOffset::s_ref_line_variance;

EvalRefLineOffset::EvalRefLineOffset() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _ref_line_offset = 0.0;
}
bool EvalRefLineOffset::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_ref_line_offset_plot, "reference line lateral offset", "", "t", "s",
                               {"lateral offset"}, {"m"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_ref_line_offset_plot, "thresh upper", 0, 1, m_defaultThreshDouble);
    ReportHelper::ConfigPairData(s_ref_line_variance);
  }

  return true;
}
bool EvalRefLineOffset::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      _ref_line_offset = ego_front->GetRefLineLateralOffset();
      _detector.Detect(std::abs(_ref_line_offset), m_defaultThreshDouble);
      VLOG_1 << "lateral offset to reference line is " << _ref_line_offset << " m.\n";

      // add data to xy-pot
      if (isReportEnabled()) {
        s_ref_line_offset_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_ref_line_offset_plot.mutable_y_axis()->at(0).add_axis_data(_ref_line_offset);
      }
    } else {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalRefLineOffset::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // calculate variance
    double variance = 0.0;
    if (s_ref_line_offset_plot.y_axis_size() > 0)
      variance = ReportHelper::CalVariance(s_ref_line_offset_plot.y_axis(0));
    ReportHelper::SetPairData(s_ref_line_variance, "reference line lateral offset variance", std::to_string(variance));

    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddPair2Attach(*attach, s_ref_line_variance);
    ReportHelper::AddXYPlot2Attach(*attach, s_ref_line_offset_plot);
  }

  return true;
}

void EvalRefLineOffset::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalRefLineOffset::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max lateral offset to reference line");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral offset to reference line check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral offset to reference line check skipped");
}
bool EvalRefLineOffset::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max lateral offset to reference line";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
