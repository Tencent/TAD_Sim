// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_lane_reduce.h"

namespace eval {
const char EvalBehavLaneReduce::_kpi_name[] = "BehavLaneReduce";

sim_msg::TestReport_XYPlot _s_lanereduce_plot;

EvalBehavLaneReduce::EvalBehavLaneReduce() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  event_lane_reduce = false;
  rightOp = false;
}
bool EvalBehavLaneReduce::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_lanereduce_plot, "lane reduce", "", "t", "s", {"change lane", "lane reduce"},
                               {"N/A", "N/A"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_lanereduce_plot, "", 0, 1, INT32_MAX, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}

bool EvalBehavLaneReduce::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      bool whether_decrease = ego_front->IsLaneNumDecreasing(20.0);
      VLOG_1 << "ego ref line max distance is larger than: " << ego_front->GetRefLineMaxDistance() << "\n";
      VLOG_1 << "if lane reduce: " << whether_decrease << "\n";
      // step 1
      if (!event_lane_reduce && whether_decrease) {
        event_lane_reduce = true;
      }
      if (event_lane_reduce) {
        // check event is finishing
        // step 2
        if (!whether_decrease) {
          _fall_detector.Detect(rightOp, 0.5);
          event_lane_reduce = false;
          rightOp = false;
        } else if (!rightOp) {
          const VehOutput &output = helper.GetVehStateFlow().GetOutput();
          rightOp = (output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
                     output.m_veh_behav == VehicleBehavior::LaneChaning_Right) &&
                    output.m_duration_valid;
        }
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_lanereduce_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_lanereduce_plot.mutable_y_axis()->at(0).add_axis_data(rightOp);
        _s_lanereduce_plot.mutable_y_axis()->at(1).add_axis_data(whether_decrease);
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
bool EvalBehavLaneReduce::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_lanereduce_plot);
  }
  return true;
}

void EvalBehavLaneReduce::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalBehavLaneReduce::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "current traffic lane reduce,but not change lane");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "current traffic lane reduce,change lane successfully");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "lanes reduce, check of changing lane skipped");
}

bool EvalBehavLaneReduce::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "current traffic lane reduce,but not change lane";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
