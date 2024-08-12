// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_search_speed.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSearchSpeed::_kpi_name[] = "ParkingSearchSpeed";

sim_msg::TestReport_PairData EvalParkingSearchSpeed::s_parking_search_speed_pair;

EvalParkingSearchSpeed::EvalParkingSearchSpeed() {
  _speed = 0.0;
  _max_parking_search_speed = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSearchSpeed::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_search_speed_pair);
  }
  return true;
}
bool EvalParkingSearchSpeed::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (!ego_front) {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
    _speed = ego_front->GetSpeed().GetNormal();
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        double search_speed = _speed;
        VLOG_0 << "total parking search speed: " << search_speed << " _max_search_speed: " << _max_parking_search_speed
               << "\n";
        _max_parking_search_speed = std::max(_max_parking_search_speed, search_speed);
        _detector.Detect(search_speed, m_defaultThreshDouble);
      }
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingSearchSpeed::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_search_speed_pair, R"(搜索速度)", std::to_string(_max_parking_search_speed));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_speed_pair);
  }
  return true;
}

void EvalParkingSearchSpeed::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSearchSpeed::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "parking search overspeed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking search overspeed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking search overspeed check skipped");
}
bool EvalParkingSearchSpeed::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "parking search overspeed";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
