// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_dec_time_of_dec_lane.h"

namespace eval {
const char EvalDecTimeOfDecLane::_kpi_name[] = "DecTimeOfDecLane";

sim_msg::TestReport_XYPlot EvalDecTimeOfDecLane::_s_behav_dec_lane_plot;

EvalDecTimeOfDecLane::EvalDecTimeOfDecLane() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _on_dec_lane = false;
  ctime = 0.0;
  all_time = 0.0;
  _md_step_time = getModuleStepTime();
}
bool EvalDecTimeOfDecLane::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_behav_dec_lane_plot, "Percentage", "", "t", "s", {"onDecLane", "percentage"},
                               {"N/A", "N/A"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_dec_lane_plot, "", 0, 1, 0.5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_dec_lane_plot, "", 0, 1, 0.34, "", 1, 0, INT32_MIN, 1);
  }
  return true;
}

bool EvalDecTimeOfDecLane::Step(eval::EvalStep &helper) {
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
      double percentage = 0;
      bool if_on_dec_lane = map_info->m_active_lane.m_on_lane_deceleration;
      double ego_acc = ego_front->GetAcc().GetX();
      if (if_on_dec_lane) {
        all_time += _md_step_time;
        if (ego_acc > -0.01) ctime += _md_step_time;
      }

      if (_on_dec_lane && !if_on_dec_lane) {
        percentage = ctime / all_time;
        ctime = 0.0;
        all_time = 0.0;
      }
      _on_dec_lane = if_on_dec_lane;
      _detector.Detect(percentage, 0.34);
      VLOG_1 << "if on dec lane :" << if_on_dec_lane << "\n";

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_dec_lane_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_dec_lane_plot.mutable_y_axis()->at(0).add_axis_data(_on_dec_lane);
        _s_behav_dec_lane_plot.mutable_y_axis()->at(1).add_axis_data(percentage);
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
bool EvalDecTimeOfDecLane::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_behav_dec_lane_plot);
  }
  return true;
}

void EvalDecTimeOfDecLane::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalDecTimeOfDecLane::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max percentage");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max percentage check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max percentage check skipped");
}

bool EvalDecTimeOfDecLane::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max percentage";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
