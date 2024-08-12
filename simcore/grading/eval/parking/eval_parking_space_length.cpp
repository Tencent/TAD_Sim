// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_space_length.h"
#include <float.h>
#include <algorithm>
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSpaceLength::_kpi_name[] = "ParkingSpaceLength";
sim_msg::TestReport_PairData EvalParkingSpaceLength::s_parking_space_length_pair;

double GetParkingLength(sim_msg::Parking *p) {
  double d1 = sqrt(pow(p->lt().x() - p->lb().x(), 2) + pow(p->lt().y() - p->lb().y(), 2));
  double d2 = sqrt(pow(p->lt().x() - p->rt().x(), 2) + pow(p->lt().y() - p->rt().y(), 2));
  return d1 > d2 ? d1 : d2;
}

EvalParkingSpaceLength::EvalParkingSpaceLength() {
  _parking_space_length = DBL_MAX;
  _parking_id = 0;
  _parking_type = "";
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSpaceLength::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _parking_id = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ParkingSpaceId");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_space_length_pair);
  }
  return true;
}

bool EvalParkingSpaceLength::Step(eval::EvalStep &helper) {
  double space_length = DBL_MAX;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    sim_msg::ParkingSpace parkSpaceMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    EvalMsg &&s_msg = _msg_mgr->Get(topic::PARKING_SPACE);  // parking space
    VLOG_0 << "message size: " << msg.GetPayload().size() << ", " << s_msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload()) && s_msg.GetPayload().size() > 0 &&
        parkSpaceMsg.ParseFromString(s_msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      VLOG_0 << "PakSpaceMsg Info: " << parkSpaceMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_in == parkMsg.stage()) {
        for (auto parking : parkSpaceMsg.world().parking()) {
          ParkingWGS84ToENU(_map_mgr, parking);
          if (_parking_id == parking.id()) {
            switch (parking.type()) {
              case sim_msg::TYPE_PARALLEL:
                _parking_type = "水平泊入泊出";
                break;
              case sim_msg::TYPE_VERTICAL:
                _parking_type = "垂直泊入泊出";
                break;
              case sim_msg::TYPE_SLANTED:
                _parking_type = "斜列泊入泊出";
                break;
              default:
                break;
            }
            space_length = GetParkingLength(&parking);
            break;
          }
        }
        VLOG_0 << "parking type: " << _parking_type << " space length: " << space_length << "\n";
        _parking_space_length = std::min(_parking_space_length, space_length);
        _detector.Detect(_parking_space_length, m_defaultThreshDouble);
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

bool EvalParkingSpaceLength::Stop(eval::EvalStop &helper) {
  std::string result = _parking_space_length == DBL_MAX ? "null" : std::to_string(_parking_space_length);

  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_space_length_pair, _parking_type + R"(泊车车位长度)", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_space_length_pair);
  }
  return true;
}

void EvalParkingSpaceLength::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSpaceLength::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "exceeded allowed parking space length");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking space length check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking space length check skipped");
}

bool EvalParkingSpaceLength::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "exceeded the maximum allowed length";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
