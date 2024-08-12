// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_access_times.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingAccessTimes::_kpi_name[] = "ParkingAccessTimes";

sim_msg::TestReport_PairData EvalParkingAccessTimes::s_parking_access_times_pair;

EvalParkingAccessTimes::EvalParkingAccessTimes() {
  _access_times = 0;
  _max_access_times = 0;
  _is_first_access = true;
  last_gear_mode = VehicleState::GearMode::NO_CONTROL;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalParkingAccessTimes::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _parking_type = getThreshValueByID_String(m_Kpi, m_KpiEnabled, "ParkingType");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_access_times_pair);
  }
  return true;
}
bool EvalParkingAccessTimes::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_in == parkMsg.stage()) {
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          VehicleState::GearMode cur_gear_mode = ego_front->GetVehicleState().powertrain_state.gead_mode;
          VLOG_0 << "cur gear mode: " << cur_gear_mode << "\n";
          // step 1
          if (_is_first_access) {
            // 对于进入APA_Parking_in后的第一次判断, 若当前挡位是否为R挡或D档，即算一次揉库（20240118）.
            if (VehicleState::GearMode::REVERSE == cur_gear_mode || VehicleState::GearMode::DRIVE == cur_gear_mode) {
              _access_times += 1;
              _is_first_access = false;
            }
          } else {
            // 后续从任何挡位与R档完成切换即算一次揉库.
            if ((VehicleState::GearMode::REVERSE != last_gear_mode &&
                 VehicleState::GearMode::REVERSE == cur_gear_mode) ||
                (VehicleState::GearMode::REVERSE == last_gear_mode && VehicleState::GearMode::REVERSE != cur_gear_mode))
              _access_times += 1;
          }
          last_gear_mode = cur_gear_mode;
          _max_access_times = std::max(_max_access_times, _access_times);
          _detector.Detect(_access_times, m_defaultThreshDouble);
        }
      } else {
        _access_times = 0;
        _is_first_access = true;
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

bool EvalParkingAccessTimes::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_access_times_pair, _parking_type + R"(泊车揉库次数)",
                              std::to_string(_max_access_times));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_access_times_pair);
  }
  return true;
}

void EvalParkingAccessTimes::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingAccessTimes::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "exceeded the max access times");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "access times check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "access times check skipped");
}
bool EvalParkingAccessTimes::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "exceeded the max access times";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
