// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_chassis_msg.h"

namespace eval {
const char EvalChassisMsg::_kpi_name[] = "ChassisMsgCycle";
sim_msg::TestReport_XYPlot EvalChassisMsg::s_cycle_plot;
sim_msg::TestReport_PairData EvalChassisMsg::s_counter;

EvalChassisMsg::EvalChassisMsg() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalChassisMsg::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_cycle_plot, "cycle diff[%]", "", "t", "s", {"cycle diff[%]"}, {"%"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_cycle_plot, "thresh upper", 0, 1, m_defaultThreshDouble);
    ReportHelper::ConfigPairData(s_counter);
  }

  m_pre_recv_t_ms = -1.0;
  m_cycle = 0.0;
  m_counter.ResetCounter(0);
  m_cycle = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, m_msg_cycle_name);
  m_topic = getThreshValueByID_String(m_Kpi, m_KpiEnabled, m_topic_name);
  m_pre_payload = "";

  VLOG_1 << "chassis msg cycle check, topic:" << m_topic << ", cycle time diff[%]:" << m_cycle << "\n";

  return true;
}
bool EvalChassisMsg::Step(eval::EvalStep &helper) {
  // defautl total loss of message
  double diff_percentage = 100.0;
  double sim_t_ms = helper.GetSimTime() * 1000.0;

  // check if new msg arrived
  auto NewMsgArrived = [](const EvalMsg &msg, const std::string &pre_payload, double &arrived_t_ms) {
    bool ret = false;
    arrived_t_ms = -1;
    if (pre_payload.empty()) {
      // if no pre message
      ret = msg.GetPayload().size() > 0;
    } else {
      // if pre message is valid
      ret = msg.GetPayload().size() > 0 && pre_payload != msg.GetPayload();
    }

    if (ret) arrived_t_ms = msg.GetSimTime().GetMilliseond();
    return ret;
  };

  if (IsModuleValid() && m_KpiEnabled && m_cycle > 0.0 && !m_topic.empty()) {
    EvalMsg msg = _msg_mgr->Get(m_topic);
    double diff = std::abs(sim_t_ms - m_pre_recv_t_ms);

    // check if new msg arrived
    bool new_msg_arrived = NewMsgArrived(msg, m_pre_payload, sim_t_ms);

    if (new_msg_arrived) {
      // frist message or calcualte diff percentage
      diff_percentage = m_pre_recv_t_ms > 0.0 ? std::abs(diff - m_cycle) / m_cycle * 100.0 : 0.0;

      // update pre recv time and payload
      m_pre_recv_t_ms = sim_t_ms;
      m_pre_payload = msg.GetPayload();

      VLOG_2 << "new chassis message arrived at " << sim_t_ms << " ms.\n";
    } else {
      // message lost or not yet
      diff_percentage = std::llround(diff) >= std::llround(m_cycle) ? 100.0 : 0.0;
      VLOG_2 << "no new chassis message arrived.\n";
    }

    if (diff_percentage > m_defaultThreshDouble) m_counter.UpdateCounterBy(1);
    VLOG_2 << "chassis msg not stable counter " << m_counter.GetCounter() << ", cycle_diff[%]:" << diff_percentage
           << "\n";
    if (isReportEnabled()) {
      s_cycle_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_cycle_plot.mutable_y_axis()->at(0).add_axis_data(diff_percentage < 1e-4 ? 0.0 : diff_percentage);
    }
  } else {
    LOG_ERROR << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalChassisMsg::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    s_counter.set_key(m_topic + " message unstable counter");
    s_counter.set_value(std::to_string(m_counter.GetCounter()).c_str());
    ReportHelper::AddXYPlot2Attach(*attach, s_cycle_plot);
    ReportHelper::AddPair2Attach(*attach, s_counter);
  }

  return true;
}

void EvalChassisMsg::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
EvalResult EvalChassisMsg::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max chassis msg cycle");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max chassis msg cycle check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max chassis msg cycle check skipped");
}
bool EvalChassisMsg::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max chassis msg cycle";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
