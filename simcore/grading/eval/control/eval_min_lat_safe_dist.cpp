// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_min_lat_safe_dist.h"

namespace eval {
const char EvalMinLatSafeDist::_kpi_name[] = "MinLatSafeDist";

sim_msg::TestReport_XYPlot EvalMinLatSafeDist::_s_lateral_dis_Part_plot;

EvalMinLatSafeDist::EvalMinLatSafeDist() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalMinLatSafeDist::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(_s_lateral_dis_Part_plot, "lateral dist", "lateral dist to part", "t", "s", {"lat_dist"},
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
    ReportHelper::ConfigXYPlotThreshold(_s_lateral_dis_Part_plot, "", 0, 1, INT32_MAX, "lat_dist", 1, 1,
                                        m_defaultThreshDouble, 0);
  }

  return true;
}
bool EvalMinLatSafeDist::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      DynamicActorFLUList &&dynamic_actors_flu =
          _actor_mgr->GetFellowActorsByType<CDynamicActorFLUPtr>(Actor_Dynamic_FLU);
      _actor_mgr->GetFellowActorsByType<CDynamicActorFLUPtr>(Actor_Dynamic_FLU);
      double min_lateral = 10;
      VLOG_1 << "dynamic actor size is: " << dynamic_actors_flu.size() << "\n";
      for (const auto &dynamic_actor : dynamic_actors_flu) {
        ActorType actor_type = dynamic_actor->GetType();
        if (actor_type == ActorType::Actor_Vehicle_FLU || actor_type == ActorType::Actor_Dynamic_FLU) {
          double ego_length = ego_front->GetShape().GetLength();
          double fellow_length = dynamic_actor->GetShape().GetLength();
          double ev_length = (ego_length + fellow_length) / 2.0;
          double ev_width = (ego_front->GetShape().GetWidth() + dynamic_actor->GetShape().GetWidth()) / 2;
          VLOG_1 << "half cars length: " << ev_width << "\n";
          VLOG_1 << "long distance: " << dynamic_actor->GetLocation().GetPosition().GetX() << "\n";
          if (std::abs(dynamic_actor->GetLocation().GetPosition().GetX()) <= ev_length)
            min_lateral =
                std::min(min_lateral, std::fabs(dynamic_actor->GetLocation().GetPosition().GetY()) - ev_width);
          VLOG_1 << "the min lateral distance is: " << min_lateral << "\n";
        }
      }
      _fall_detector.Detect(min_lateral, m_defaultThreshDouble);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_lateral_dis_Part_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_lateral_dis_Part_plot.mutable_y_axis()->at(0).add_axis_data(min_lateral);
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
bool EvalMinLatSafeDist::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_lateral_dis_Part_plot);
  }
  return true;
}

void EvalMinLatSafeDist::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalMinLatSafeDist::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "lower min distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "min distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "min diatance check skipped");
}

bool EvalMinLatSafeDist::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above min distance";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
