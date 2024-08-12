// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_miss_rate.h"
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingMissRate::_kpi_name[] = "ParkingMissRate";
sim_msg::TestReport_PairData EvalParkingMissRate::s_parking_miss_pair;

EvalParkingMissRate::EvalParkingMissRate() : _parking_spaces(), _t_parking_spaces() {
  _parking_miss_rate = 0.0;
  _search_range = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingMissRate::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // _eval_cfg.GetValueAsDouble("SearchRange", _search_range);
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_miss_pair);
  }
  return true;
}

bool EvalParkingMissRate::isContainParkingSpace(const sim_msg::Parking *p) {
  bool is_contian = false;
  for (auto parking_ptr : _parking_spaces) {
    is_contian = isSameParking(&(p->lt()), &(p->lb()), parking_ptr);
    break;
  }
  if (!is_contian) _parking_spaces.push_back(p);
  return is_contian;
}

bool EvalParkingMissRate::isContainParkingSpaceById(const sim_msg::Parking *p) {
  bool is_contian = false;
  for (auto parking_ptr : _t_parking_spaces) {
    is_contian = (parking_ptr->id() == p->id());
    break;
  }
  if (!is_contian) _t_parking_spaces.push_back(p);
  return is_contian;
}

bool EvalParkingMissRate::Step(eval::EvalStep &helper) {
  double miss_rate = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    sim_msg::ParkingSpace parkSpaceMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    EvalMsg &&s_msg = _msg_mgr->Get(topic::PARKING_SPACE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << s_msg.GetPayload().size() << '\n';
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload()) && s_msg.GetPayload().size() > 0 &&
        parkSpaceMsg.ParseFromString(s_msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      VLOG_0 << "PakSpaceMsg Info: " << parkSpaceMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        for (const auto &parking : parkMsg.parking()) {
          isContainParkingSpace(&parking);
        }
        // parking space support fov search (user defined)
        for (const auto &t_parking : parkSpaceMsg.world().parking()) {
          isContainParkingSpaceById(&t_parking);
        }
        VLOG_0 << "t_parking_space size: " << _t_parking_spaces.size() << "\n";
        miss_rate = _t_parking_spaces.size() == 0 ? 0 : (1 - _parking_spaces.size() / _t_parking_spaces.size());
        _parking_miss_rate = std::max(_parking_miss_rate, miss_rate);
        _detector.Detect(miss_rate, m_defaultThreshDouble);
      } else {
        std::vector<const sim_msg::Parking *> empty_vector;
        _parking_spaces.swap(empty_vector);
        _t_parking_spaces.swap(empty_vector);
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

bool EvalParkingMissRate::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    std::string indicator_name = "漏识率s";
    indicator_name.pop_back();
    ReportHelper::SetPairData(s_parking_miss_pair, indicator_name, std::to_string(_parking_miss_rate));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_miss_pair);
  }
  return true;
}

void EvalParkingMissRate::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingMissRate::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "high miss rate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "miss rate check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "miss rate check skipped");
}
bool EvalParkingMissRate::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "high miss rate";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
