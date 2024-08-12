// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_success.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSuccess::_kpi_name[] = "ParkingSuccessRate";

sim_msg::TestReport_PairData EvalParkingSuccess::s_parking_success_pair;

EvalParkingSuccess::EvalParkingSuccess() {
  _total_parking_times = 0;
  _total_parking_success_times = 0;
  _parking_success_rate = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalParkingSuccess::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_success_pair);
  }
  return true;
}

bool EvalParkingSuccess::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_Completed == parkMsg.stage())
        _parking_success_rate = (++_total_parking_success_times) / (++_total_parking_times);
      else if (sim_msg::APA_test_flag::APA_Parking_failed == parkMsg.stage())
        _parking_success_rate = (_total_parking_success_times) / (++_total_parking_times);
      VLOG_0 << "total times: " << _total_parking_times << " success times: " << _total_parking_success_times << '\n';
      _detector.Detect(_parking_success_rate, m_defaultThreshDouble);
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingSuccess::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    std::string indicator_name = "泊车成功率s";
    indicator_name.pop_back();
    ReportHelper::SetPairData(s_parking_success_pair, indicator_name, std::to_string(_parking_success_rate));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_success_pair);
  }
  return true;
}

void EvalParkingSuccess::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSuccess::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "low success rate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "success rate check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "success rate check skipped");
}
bool EvalParkingSuccess::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "low parking success rate";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
