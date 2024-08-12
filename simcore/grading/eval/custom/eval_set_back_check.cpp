// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_set_back_check.h"

namespace eval {
const char EvalSetBackCheck::_kpi_name[] = "SetBackCheck";

sim_msg::TestReport_XYPlot EvalSetBackCheck::_s_set_back_plot;
sim_msg::TestReport_PairData EvalSetBackCheck::_g_setback_pair;
EvalSetBackCheck::EvalSetBackCheck() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  event_setback = false;
}
bool EvalSetBackCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    event_setback = false;
    _setback_count = 0;
    scene_dis = 1.0;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(_g_setback_pair);
    ReportHelper::ConfigXYPlot(_s_set_back_plot, "setback", "", "t", "s", {"jerk"}, {"m/s³"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_set_back_plot, "thresh", 0, 0, 5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThresholdClearValue(_s_set_back_plot, {0}, true, false);
  }
  return true;
}
bool EvalSetBackCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get long acc and speed
      // step 1
      double _speed = ego_front->GetSpeed().GetNormal2D();
      double acc_change_long_limit = 0.0;
      if (_speed <= 5.0) {
        acc_change_long_limit = 5.0;
      } else if (_speed > 5.0 && _speed < 20.0) {
        acc_change_long_limit = 5.834 - 0.167 * _speed;
      } else if (_speed >= 20.0) {
        acc_change_long_limit = 2.5;
      }
      // step 2
      double jerk = ego_front->GetJerk().GetX();
      // add count for setback
      if (!event_setback && jerk >= acc_change_long_limit) {
        event_setback = true;
        _setback_count++;
      }
      if (event_setback && jerk < acc_change_long_limit) {
        event_setback = false;
      }
      // _detector.Detect((jerk < acc_change_long_limit), 0.5);
      VLOG_0 << " now x speed is:" << _speed << "m/s" << " x jerk limit is:" << acc_change_long_limit << " m/s^3.\n";
      scene_dis = helper.GetGradingMsg().mileage();
      VLOG_1 << "scene_dis: " << scene_dis << "km setback count: " << _setback_count << "\n";

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_set_back_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_set_back_plot.mutable_y_axis()->at(0).add_axis_data(jerk);
        _s_set_back_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(acc_change_long_limit);
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
bool EvalSetBackCheck::Stop(eval::EvalStop &helper) {
  // step 3
  ReportHelper::SetPairData(_g_setback_pair, "setback rate[]",
                            std::to_string(static_cast<int>(_setback_count * 100 / scene_dis)));
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_set_back_plot);
    ReportHelper::AddPair2Attach(*attach, _g_setback_pair);
  }
  return true;
}

void EvalSetBackCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalSetBackCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "setback happen");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "setback check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "setback check skipped");
}

bool EvalSetBackCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above setback count";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
