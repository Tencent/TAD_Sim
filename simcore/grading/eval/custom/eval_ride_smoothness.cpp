// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_ride_smoothness.h"

namespace eval {
const char EvalRideSmoothness::_kpi_name[] = "RideSmoothness";

sim_msg::TestReport_XYPlot _s_ride_plot;

EvalRideSmoothness::EvalRideSmoothness() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  longitudinal_sum_squares = 0.0;
  lateral_sum_squares = 0.0;
  count = 0.0;
  longitudinal_rms_upper = 0.0;
  lateral_rms_upper = 0.0;
}
bool EvalRideSmoothness::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    longitudinal_rms_upper = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MXAcc");
    lateral_rms_upper = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MYAcc");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_ride_plot, "Ride", "", "t", "s", {"longitudinal_rms", "lateral_rms"}, {"N/A", "N/A"},
                               2);
    ReportHelper::ConfigXYPlotThreshold(_s_ride_plot, "longitudinal_rms_upper", 0, 1, longitudinal_rms_upper, "", 1, 1,
                                        INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_ride_plot, "lateral_rms_upper", 0, 1, lateral_rms_upper, "", 1, 0, INT32_MIN,
                                        1);
  }

  return true;
}
bool EvalRideSmoothness::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get ACC
      double long_acc = ego_front->GetAcc().GetX();
      double lateral_acc = ego_front->GetAcc().GetY();
      count++;

      longitudinal_sum_squares += long_acc * long_acc;
      double res_long = std::sqrt(longitudinal_sum_squares / count);

      lateral_sum_squares += lateral_acc * lateral_acc;
      double res_lateral = std::sqrt(lateral_sum_squares / count);

      _long_detector.Detect(res_long, longitudinal_rms_upper);
      _lateral_detector.Detect(res_lateral, lateral_rms_upper);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_ride_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_ride_plot.mutable_y_axis()->at(0).add_axis_data(res_long);
        _s_ride_plot.mutable_y_axis()->at(1).add_axis_data(res_lateral);
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
bool EvalRideSmoothness::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_ride_plot);
  }
  return true;
}

void EvalRideSmoothness::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _long_detector, _kpi_name);
}

EvalResult EvalRideSmoothness::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    int _detect_count = _lateral_detector.GetCount() + _long_detector.GetCount();
    _case.mutable_info()->set_detected_count(_detect_count);

    if (_detect_count >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above longitudinal or lateral rms upper");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "longitudinal and lateral rms check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "longitudinal and lateral rms check skipped");
}

bool EvalRideSmoothness::ShouldStopScenario(std::string &reason) {
  int _detect_count = _lateral_detector.GetCount() + _long_detector.GetCount();
  auto ret = _detect_count >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above longitudinal or lateral rms";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
