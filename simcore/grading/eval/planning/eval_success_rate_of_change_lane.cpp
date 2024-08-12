// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_success_rate_of_change_lane.h"

namespace eval {
const char EvalSuccessRateOfChangeLane::_kpi_name[] = "SuccessRateOfChangeLane";

sim_msg::TestReport_XYPlot EvalSuccessRateOfChangeLane::_s_succ_cl_plot;
sim_msg::TestReport_PairData EvalSuccessRateOfChangeLane::_g_success_variance_pair;

EvalSuccessRateOfChangeLane::EvalSuccessRateOfChangeLane() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _change_count = 0;
  _success_count = 0;
  _change_state = false;
}
bool EvalSuccessRateOfChangeLane::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::ConfigPairData(_g_success_variance_pair);
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_succ_cl_plot, "change lane success", "", "t", "s", {"success"}, {"N/A"}, 1);
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
    ReportHelper::ConfigXYPlotThreshold(_s_succ_cl_plot, "", 0, 0, INT32_MAX, "success lower", 1, 1,
                                        m_defaultThreshDouble, 0);
  }

  return true;
}
bool EvalSuccessRateOfChangeLane::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();
      VLOG_1 << "the car's behav is : " << output.m_veh_behav << "\n";
      if (!_change_state) {
        if ((output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
             output.m_veh_behav == VehicleBehavior::LaneChaning_Right)) {
          _change_state = true;
        }
      }
      if (_change_state) {
        if (output.m_veh_behav == VehicleBehavior::Driving && !output.m_duration_valid) {
          _change_state = false;
          _change_count++;
        }
        if ((output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
             output.m_veh_behav == VehicleBehavior::LaneChaning_Right) &&
            output.m_duration_valid) {
          _success_count++;
          _change_count++;
          _change_state = false;
        }
      }
      double _res = _change_count == 0 ? 1.0 : (_success_count / static_cast<double>(_change_count));
      _fall_detector.Detect(_res, m_defaultThreshDouble);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_succ_cl_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_succ_cl_plot.mutable_y_axis()->at(0).add_axis_data(_res);
        /**
         * [optional] set threshold on step
         */
        // _s_succ_cl_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(_speed_deceleration_limit);
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
bool EvalSuccessRateOfChangeLane::Stop(eval::EvalStop &helper) {
  ReportHelper::SetPairData(_g_success_variance_pair, "change lane count", std::to_string(_change_count));
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_succ_cl_plot);
    ReportHelper::AddPair2Attach(*attach, _g_success_variance_pair);
  }
  return true;
}

void EvalSuccessRateOfChangeLane::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalSuccessRateOfChangeLane::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "lower the min success");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "min success check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "min success check skipped");
}

bool EvalSuccessRateOfChangeLane::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "lower min success";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
