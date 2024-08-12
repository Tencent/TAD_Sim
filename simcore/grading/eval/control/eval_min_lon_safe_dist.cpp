// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_min_lon_safe_dist.h"

namespace eval {
const char EvalMinLonSafeDist::_kpi_name[] = "MinLonSafeDist";

sim_msg::TestReport_XYPlot EvalMinLonSafeDist::_s_long_dis_Part_plot;

EvalMinLonSafeDist::EvalMinLonSafeDist() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalMinLonSafeDist::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(_s_long_dis_Part_plot, "long dist", "long distance to part", "t", "s", {"long_dist"},
                               {"m"}, 1);
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
     * @param    y_axis_id       the id  of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(_s_long_dis_Part_plot, "", 0, 1, INT32_MAX, "dist lower", 1, 1,
                                        m_defaultThreshDouble, 0);
  }
  return true;
}
bool EvalMinLonSafeDist::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      DynamicActorFLUList &&DynamicActorFLUList =
          _actor_mgr->GetFellowActorsByType<CDynamicActorFLUPtr>(Actor_Dynamic_FLU);
      double min_long = 200;
      for (const auto &dynamic_actor : DynamicActorFLUList) {
        ActorType actor_type = dynamic_actor->GetType();
        if (actor_type == ActorType::Actor_Vehicle_FLU || actor_type == ActorType::Actor_Dynamic_FLU) {
          double ego_width = ego_front->GetShape().GetWidth();
          double fellow_width = dynamic_actor->GetShape().GetWidth();
          double ev_width = (ego_width + fellow_width) / 2.0;
          double ev_length = (ego_front->GetShape().GetLength() + dynamic_actor->GetShape().GetLength()) / 2;
          if (std::abs(dynamic_actor->GetLocation().GetPosition().GetY()) <= ev_width)
            min_long = std::min(min_long, std::abs(dynamic_actor->GetLocation().GetPosition().GetX()) - ev_length);
          VLOG_1 << "the min long distance is: " << min_long << "\n";
        }
      }
      VLOG_1 << "get the min Long Distance is : " << min_long << "\n";
      _fall_detector.Detect(min_long, m_defaultThreshDouble);

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_long_dis_Part_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_long_dis_Part_plot.mutable_y_axis()->at(0).add_axis_data(min_long);
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
bool EvalMinLonSafeDist::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_long_dis_Part_plot);
  }
  return true;
}

void EvalMinLonSafeDist::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalMinLonSafeDist::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "lower min distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "min long distance to Part check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "min long distance to Part check skipped");
}

bool EvalMinLonSafeDist::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "lower min long distance to part";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
