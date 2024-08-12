// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_lane_change_duration.h"

namespace eval {
const char EvalLaneChangeDuration::_kpi_name[] = "EgoChangeLane_Duration";

sim_msg::TestReport_XlsxSheet s_ego_lc_duration;
sim_msg::TestReport_XYPlot EvalLaneChangeDuration::s_ego_lc_plot;

EvalLaneChangeDuration::EvalLaneChangeDuration() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalLaneChangeDuration::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    max_duration = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MAX_DURATION");
    min_duration = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MIN_DURATION");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_ego_lc_plot, "change lines", "change lines", "t", "s",
                               {"lateral Acc", "duration_time"}, {"m/s²", "s"}, 2);
    ReportHelper::ConfigXYPlotThreshold(s_ego_lc_plot, "lateral Acc upper", 0, 1, m_defaultThreshDouble,
                                        "lateral Acc lower", 1, 1, -m_defaultThreshDouble, 0);
    ReportHelper::ConfigXYPlotThreshold(s_ego_lc_plot, "duration_time upper", 0, 1, max_duration, "duration_time lower",
                                        1, 1, min_duration, 1);
    ReportHelper::ConfigXLSXSheet(s_ego_lc_duration, "ego lane change duration");
    m_sim_t.set_header("start time [s]");
    m_duration.set_header("lane change duration [s]");
  }

  return true;
}
bool EvalLaneChangeDuration::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      // get output of lane-change state-flow
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();
      double lateral_acc = 0;

      if ((output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
           output.m_veh_behav == VehicleBehavior::LaneChaning_Right)) {
        lateral_acc = ego_front->GetAcc().GetY();
        _detector.Detect(std::abs(lateral_acc), m_defaultThreshDouble);
      }

      // add data to xlsx sheet
      if (output.m_duration_valid) {
        m_sim_t.add_data()->assign(std::to_string(output.m_start_t).c_str());
        m_duration.add_data()->assign(std::to_string(output.m_duration).c_str());
        _fall_detector.Detect((output.m_duration < max_duration && output.m_duration > min_duration), 0.5);
      }

      if (isReportEnabled()) {
        s_ego_lc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_ego_lc_plot.mutable_y_axis()->at(0).add_axis_data(lateral_acc);
        s_ego_lc_plot.mutable_y_axis()->at(1).add_axis_data(output.m_duration);
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
bool EvalLaneChangeDuration::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    s_ego_lc_duration.add_sheet_data()->CopyFrom(m_sim_t);
    s_ego_lc_duration.add_sheet_data()->CopyFrom(m_duration);
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_ego_lc_plot);
    ReportHelper::AddXLSXSheet2Attach(*_case.add_steps()->add_attach(), s_ego_lc_duration);
  }
  return true;
}

void EvalLaneChangeDuration::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalLaneChangeDuration::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "lane change duration check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "lane change duration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "lane change indicator check skipped");
}
bool EvalLaneChangeDuration::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "lane change duration check fail";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
