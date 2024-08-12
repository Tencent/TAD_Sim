// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_warn_sign.h"

namespace eval {
const char EvalBehavWarnSign::_kpi_name[] = "BehavWarnSign";

sim_msg::TestReport_XYPlot _s_behav_warn_sign_plot;

EvalBehavWarnSign::EvalBehavWarnSign() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalBehavWarnSign::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_behav_warn_sign_plot, "correct behavior", "", "t", "s",
                               {"has_warn_sign", "correct", "speed", "acc"}, {"N/A", "N/A", "m/s", "m/s²"}, 4);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_warn_sign_plot, "warn-sign", 0, 1, 0.5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_warn_sign_plot, "", 0, 1, INT32_MAX, "behav correct", 1, 0, 0.5, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_warn_sign_plot, "speed upper", 0, 1, 8.34, "", 1, 0, INT32_MIN, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_warn_sign_plot, "acc upper", 0, 1, 0.2, "", 1, 0, INT32_MIN, 3);
  }
  return true;
}

bool EvalBehavWarnSign::Step(eval::EvalStep &helper) {
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
      bool correct_behav = true;
      bool has_warn_sign = false;
      double _speed = ego_front->GetSpeed().GetNormal2D();
      double _acc = ego_front->GetAcc().GetX();
      eval::EvalMapObjectPtr obj = std::make_shared<EvalMapObject>();
      ego_front->GetWarningSign(obj);
      VLOG_1 << "warn_signs.size(): " << obj->m_map_objects.size() << "\n";
      for (auto &warn_sign : obj->m_map_objects) {
        auto it = warn_signs.find(warn_sign->getObjectSubType());
        has_warn_sign = (it != warn_signs.end());
        bool warn_behav = has_warn_sign ? (_speed < 8.34 && _acc < 0.2) : true;
        correct_behav = correct_behav && warn_behav;
        if (!warn_behav) set_msgs.insert(it->second);
      }
      _detector.Detect(correct_behav, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_warn_sign_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_warn_sign_plot.mutable_y_axis()->at(0).add_axis_data(has_warn_sign);
        _s_behav_warn_sign_plot.mutable_y_axis()->at(1).add_axis_data(correct_behav);
        _s_behav_warn_sign_plot.mutable_y_axis()->at(2).add_axis_data(_speed);
        _s_behav_warn_sign_plot.mutable_y_axis()->at(3).add_axis_data(_acc);
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
bool EvalBehavWarnSign::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_behav_warn_sign_plot);
  }
  return true;
}

void EvalBehavWarnSign::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalBehavWarnSign::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      std::string _msg_warn = "";
      for (std::string str : set_msgs) _msg_warn += (str + "\n");
      return EvalResult(sim_msg::TestReport_TestState_FAIL, _msg_warn);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before warn signs check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before warn signs check skipped");
}
bool EvalBehavWarnSign::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "behavior before warn signs check failed";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
