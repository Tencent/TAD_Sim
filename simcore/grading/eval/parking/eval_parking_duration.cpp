// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_duration.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingDuration::_kpi_name[] = "ParkingDuration";

sim_msg::TestReport_PairData EvalParkingDuration::s_parking_duration_pair;

EvalParkingDuration::EvalParkingDuration() {
  _max_parking_t = 0.0;
  _total_parking_t = 0.0;
  _start_parking_t = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingDuration::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_duration_pair);
  }
  return true;
}

bool EvalParkingDuration::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_in == parkMsg.stage()) {  // record start parking timestamp
        if (_start_parking_t < 0.5) _start_parking_t = helper.GetSimTime();

        // calculate parking duration. step 1
        _total_parking_t = helper.GetSimTime() - _start_parking_t;
        _max_parking_t = std::max(_max_parking_t, _total_parking_t);
      } else {  // parking time reset when stage change.
        _start_parking_t = 0.0;
        _total_parking_t = 0.0;
      }
      VLOG_0 << "_total parking t: " << _total_parking_t << "\n";
      _detector.Detect(_total_parking_t, m_defaultThreshDouble);
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingDuration::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    std::string result = _max_parking_t > 0 ? std::to_string(_max_parking_t) : "0";
    ReportHelper::SetPairData(s_parking_duration_pair, R"(泊车时长)", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_duration_pair);
  }
  return true;
}

void EvalParkingDuration::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingDuration::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "timeout");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "timeout check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "timeout check skipped");
}
bool EvalParkingDuration::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "parking timeout";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
