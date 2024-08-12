// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_search_duration.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSearchDuration::_kpi_name[] = "ParkingSearchDuration";

sim_msg::TestReport_PairData EvalParkingSearchDuration::s_parking_search_duration_pair;

EvalParkingSearchDuration::EvalParkingSearchDuration() {
  _max_parking_search_t = 0.0;
  _total_parking_search_t = 0.0;
  _start_parking_search_t = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSearchDuration::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_search_duration_pair);
  }
  return true;
}

bool EvalParkingSearchDuration::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {  // record start parking timestamp
        if (_start_parking_search_t < 0.5) _start_parking_search_t = helper.GetSimTime();

        // calculate parking search_duration
        _total_parking_search_t = helper.GetSimTime() - _start_parking_search_t;
        VLOG_0 << "total parking time: " << _total_parking_search_t
               << " _max_parking_search_t: " << _max_parking_search_t << "\n";
        _max_parking_search_t = std::max(_max_parking_search_t, _total_parking_search_t);
      } else {  // parking time reset when stage change.
        _start_parking_search_t = 0.0;
        _total_parking_search_t = 0.0;
      }
      _detector.Detect(_total_parking_search_t, m_defaultThreshDouble);
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingSearchDuration::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("ParkingTimeOut", std::to_string(_detector.GetCount()));

  // set report
  if (isReportEnabled()) {
    std::string result = _max_parking_search_t > 0 ? std::to_string(_max_parking_search_t) : "0";

    // ReportHelper::SetPairData(s_parking_search_duration_pair, R"(车位搜索时长)", result);
    ReportHelper::SetPairData(s_parking_search_duration_pair, "车位搜索时长", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_duration_pair);
  }
  return true;
}

void EvalParkingSearchDuration::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSearchDuration::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "search timeout");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "search timeout check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "search timeout check skipped");
}
bool EvalParkingSearchDuration::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "parking search timeout";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
