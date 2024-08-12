// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_thw.h"

namespace eval {
const char EvalTHW::_kpi_name[] = "MinDistanceFromCar";

sim_msg::TestReport_XYPlot s_thw_plot;

EvalTHW::EvalTHW() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _thw = 0.0;
}
bool EvalTHW::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_thw_plot, "thw", "", "t", "s", {"thw"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_thw_plot, "", 0, 1, INT32_MAX, "thresh lower", 1, 1, m_defaultThreshDouble);
  }

  return true;
}
bool EvalTHW::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    _thw = const_max_thw * 1.5;
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
      auto fellow = helper.GetLeadingVehicle();
      if (fellow) {
        CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();
        EVector3d &&ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);
        double egoSpeed = ego_front->GetSpeed().GetNormal2D();
        _thw = ego_2_fellow.GetNormal2D();

        // ego is moving
        if (egoSpeed > 0.1) {
          _thw = _thw / egoSpeed;
        } else {
          _thw = INT32_MAX;
        }
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }

    // detector
    _detector.Detect(_thw, m_defaultThreshDouble);
    VLOG_1 << "_thw is " << _thw << " s.\n";
    _thw = _thw > const_max_thw ? const_max_thw : _thw;

    // add data to xy-pot
    if (isReportEnabled()) {
      s_thw_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_thw_plot.mutable_y_axis()->at(0).add_axis_data(_thw);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalTHW::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("TimeHeadway", std::to_string(_detector.GetCount()));

  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_thw_plot);
  }

  return true;
}

EvalResult EvalTHW::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "thw too low");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "thw check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "thw check skipped");
}
bool EvalTHW::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "thw too low";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalTHW::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_distheadway()->set_dist_to_fellow(_thw);
  msg.mutable_distheadway()->set_dist_state(sim_msg::GRADING_DISTHEADWAY_NORMAL);
  if (_detector.GetLatestState()) {
    msg.mutable_distheadway()->set_dist_state(sim_msg::GRADING_DISTHEADWAY_OVERLOW);
  }
  msg.mutable_distheadway()->set_threshold(m_defaultThreshDouble);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalTHW::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_tooclosedistance()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_tooclosedistance()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_tooclosedistance()->set_is_pass(IsEvalPass()._state ==
                                                                      sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
