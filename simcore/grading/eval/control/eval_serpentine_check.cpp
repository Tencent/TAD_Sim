// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_serpentine_check.h"

namespace eval {
const char EvalSerpentineCheck::_kpi_name[] = "SerpentineCheck";
sim_msg::TestReport_XYPlot _s_snake_plot;

EvalSerpentineCheck::EvalSerpentineCheck() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _record_lr = 2;
  _arr_ptr = nullptr;
  _cur_index = 0;
  _variation = 0;
}
bool EvalSerpentineCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // step 1
    size_ptr = static_cast<int>(m_defaultThreshDouble / getModuleStepTime());
    _arr_ptr = new bool[size_ptr]{0};
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_snake_plot, "snake change", "", "t", "s", {"count"}, {"times"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_snake_plot, "", 0, 1, 3, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}
bool EvalSerpentineCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      double _yaw_rate = ego_front->GetAngularV().GetZ();
      int _cur_lr = (std::fabs(_yaw_rate) < 1e-8) ? _record_lr : _yaw_rate > 0;
      _record_lr = (_record_lr == 2) ? _cur_lr : _record_lr;
      // sub left ptr value
      _variation -= _arr_ptr[_cur_index];
      VLOG_1 << "_arr_ptr " << _cur_index << " is:" << _arr_ptr[_cur_index] << "\n";
      // step 2
      // reset right ptr value
      _arr_ptr[_cur_index] = _record_lr ^ _cur_lr;
      VLOG_1 << " after _arr_ptr " << _cur_index << " is:" << _arr_ptr[_cur_index] << "\n";
      // get the newest snake count
      _variation += _arr_ptr[_cur_index];
      VLOG_1 << "_variation is: " << _variation << "\n";
      // the ptr add one to right
      _cur_index = (++_cur_index) % size_ptr;
      VLOG_1 << "next index: " << _cur_index << "\n";
      _record_lr = _cur_lr;
      _detector.Detect(_variation, 3);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_snake_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_snake_plot.mutable_y_axis()->at(0).add_axis_data(_variation);
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
bool EvalSerpentineCheck::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_snake_plot);
  }
  delete[] _arr_ptr;
  return true;
}

void EvalSerpentineCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalSerpentineCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "max yaw change");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max yaw change check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max yaw change check skipped");
}

bool EvalSerpentineCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max yaw change";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
