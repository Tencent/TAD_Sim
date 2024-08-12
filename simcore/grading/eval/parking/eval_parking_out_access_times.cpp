// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_out_access_times.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingOutAccessTimes::_kpi_name[] = "ParkingOutAccessTimes";

sim_msg::TestReport_PairData EvalParkingOutAccessTimes::s_parking_out_access_times_pair;

EvalParkingOutAccessTimes::EvalParkingOutAccessTimes() {
  access_times = 0;
  max_access_times = 0;
  last_gear_mode = VehicleState::GearMode::NO_CONTROL;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalParkingOutAccessTimes::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_out_access_times_pair);
  }
  return true;
}
bool EvalParkingOutAccessTimes::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_out == parkMsg.stage()) {
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          VehicleState::GearMode cur_gear_mode = ego_front->GetVehicleState().powertrain_state.gead_mode;
          // step 1
          if ((VehicleState::GearMode::REVERSE != last_gear_mode && VehicleState::GearMode::REVERSE == cur_gear_mode) ||
              (VehicleState::GearMode::REVERSE == last_gear_mode && VehicleState::GearMode::REVERSE != cur_gear_mode))
            access_times += 1;
          VLOG_0 << "cur gear mode: " << cur_gear_mode << "\n";
          max_access_times = std::max(max_access_times, access_times);
          last_gear_mode = cur_gear_mode;
          _detector.Detect(access_times, m_defaultThreshDouble);
        }
      } else {
        access_times = 0;
      }
      VLOG_0 << "max_access_time: " << max_access_times << " access_times: " << access_times << "\n";
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingOutAccessTimes::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_out_access_times_pair, R"(泊出揉库次数)", std::to_string(max_access_times));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_out_access_times_pair);
  }
  return true;
}

void EvalParkingOutAccessTimes::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingOutAccessTimes::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "exceeded the max out access times");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "out access times check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "out access times check skipped");
}
bool EvalParkingOutAccessTimes::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "exceeded the max out access times";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
