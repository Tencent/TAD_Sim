// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_type_search_accuracy.h"
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {

const char EvalParkingTypeSearchAccuracy::_kpi_name[] = "ParkingTypeIdentifiedAccuracy";
sim_msg::TestReport_PairData EvalParkingTypeSearchAccuracy::s_parking_type_search_accuracy_pair;

EvalParkingTypeSearchAccuracy::EvalParkingTypeSearchAccuracy() : _parking_spaces() {
  _type_identified_num = 0;
  _type_total_num = 0;
  _type_identified_accuracy = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingTypeSearchAccuracy::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  if (isReportEnabled()) {
    // set report info
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_type_search_accuracy_pair);
  }
  return true;
}

bool EvalParkingTypeSearchAccuracy::isContainParkingSpace(const sim_msg::Parking *p) {
  bool is_contian = false;
  for (auto parking_ptr : _parking_spaces) {
    is_contian = isSamePoint(&parking_ptr->lt(), &p->lt()) && isSamePoint(&parking_ptr->lb(), &p->lb());
    break;
  }
  if (!is_contian) _parking_spaces.push_back(p);
  return is_contian;
}

bool EvalParkingTypeSearchAccuracy::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    sim_msg::ParkingSpace parkSpaceMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    EvalMsg &&s_msg = _msg_mgr->Get(topic::PARKING_SPACE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "," << s_msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload()) && s_msg.GetPayload().size() > 0 &&
        parkSpaceMsg.ParseFromString(s_msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      VLOG_0 << "PakSpaceMsg Info: " << parkSpaceMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        for (const auto &parking : parkMsg.parking()) {
          bool is_identified = false;
          bool is_type_identified = false;
          bool is_contain = isContainParkingSpace(&parking);

          if (!is_contain) {
            for (auto t_parking : parkSpaceMsg.world().parking()) {
              ParkingWGS84ToENU(_map_mgr, t_parking);
              is_identified = isSameParking(&(parking.lt()), &(parking.lb()), &t_parking);
              if (is_identified) {
                // todo: 车位类型目前定义不完善。后续确定车位上报方式后，完善多种车位类型的对比情况.
                is_type_identified = (parking.type() == t_parking.type());
                break;
              }
            }
          }
          _type_identified_num += is_type_identified ? 1 : 0;
          _type_total_num += is_contain ? 0 : 1;
        }
        _type_identified_accuracy = _type_total_num == 0 ? 0 : _type_identified_num / _type_total_num;
        VLOG_0 << "total type num: " << _type_total_num << " _type_identified_num: " << _type_identified_num << "\n";
        _detector.Detect(_type_identified_accuracy, m_defaultThreshDouble);
      } else {
        _type_identified_num = 0;
        _type_total_num = 0;
        std::vector<const sim_msg::Parking *> empty_vector;
        _parking_spaces.swap(empty_vector);
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

bool EvalParkingTypeSearchAccuracy::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // set report
    std::string indicator_name = "车位类型识别准确率s";
    indicator_name.pop_back();
    ReportHelper::SetPairData(s_parking_type_search_accuracy_pair, indicator_name,
                              std::to_string(_type_identified_accuracy));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_type_search_accuracy_pair);
  }
  return true;
}

void EvalParkingTypeSearchAccuracy::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingTypeSearchAccuracy::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "low  identified accuracy");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "identified accuracy check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "identified accuracy check skipped");
}
bool EvalParkingTypeSearchAccuracy::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "type low identified accuracy";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
