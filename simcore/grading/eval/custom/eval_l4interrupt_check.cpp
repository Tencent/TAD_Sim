// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_l4interrupt_check.h"
#include "l4_signal.pb.h"

namespace eval {
const char EvalL4InterruptCheck::_kpi_name[] = "L4InterruptCheck";

sim_msg::TestReport_XYPlot EvalL4InterruptCheck::_s_l4_interrupt_plot;

EvalL4InterruptCheck::EvalL4InterruptCheck() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalL4InterruptCheck::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_l4_interrupt_plot, "algo get", "if get algorithm", "t", "s", {"interrupt_signal"},
                               {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_l4_interrupt_plot, "", 0, 1, 1.5, "", 0, 0, INT32_MIN, 0);
  }
  return true;
}
bool EvalL4InterruptCheck::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    int signal_interrupt = 0;
    EvalMsg interrupt_msg = _msg_mgr->Get(topic::L4_ALGORITHM_SIGNAL);
    sim_msg::L4AlgorithmSignal algori_signal;
    VLOG_0 << _kpi_name << " -message size: " << interrupt_msg.GetPayload().size() << "\n";
    if (interrupt_msg.GetPayload().size() > 0) {
      algori_signal.ParseFromString(interrupt_msg.GetPayload());
      VLOG_0 << algori_signal.ShortDebugString() << "\n";
      // local test
      // int tmp = rand() % 10000;
      // std::string parm = tmp > 10 ? "1" : "3";
      // map_signals->insert({"planning_status", parm});
      auto map_signals = algori_signal.mutable_signal();
      auto iterator = map_signals->find("planning_status");
      if (iterator != map_signals->end()) {
        signal_interrupt = std::stoi(iterator->second);
        _detector.Detect(signal_interrupt, 1.5);
      }
    }
    // add data to xy-pot
    if (isReportEnabled()) {
      _s_l4_interrupt_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      _s_l4_interrupt_plot.mutable_y_axis()->at(0).add_axis_data(signal_interrupt);
    }

  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalL4InterruptCheck::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_l4_interrupt_plot);
  }
  return true;
}

void EvalL4InterruptCheck::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalL4InterruptCheck::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "receive the failed signal of l4 algo");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "recv signal of l4 algo check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "receive the signal of l4 algo");
}

bool EvalL4InterruptCheck::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above the count of failed signal";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
