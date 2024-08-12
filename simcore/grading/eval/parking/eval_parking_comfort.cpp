// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_comfort.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingComfort::_kpi_name[] = "ParkingComfort";

sim_msg::TestReport_PairData EvalParkingComfort::s_parking_comfort_pair;

EvalParkingComfort::EvalParkingComfort() {
  lateral_acc_thresh_value = 0.0;
  vertical_acc_thresh_value = 0.0;
  acc_thresh_value = 0.0;
  _detect_count = 0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}
bool EvalParkingComfort::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    lateral_acc_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LateralAccThreshold");
    vertical_acc_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "VerticalAccThreshold");
    acc_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "AccThreshold");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_comfort_pair);
  }
  return true;
}
bool EvalParkingComfort::Step(eval::EvalStep &helper) {
  double lateral_acc = 0.0;
  double vertical_acc = 0.0;
  double acc = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_in == parkMsg.stage()) {
        // calculate acc
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          // X(纵向加速度),Y(横向加速度). step 1
          lateral_acc = std::abs(ego_front->GetAcc().GetY());
          vertical_acc = std::abs(ego_front->GetAcc().GetX());
          acc = std::abs(ego_front->GetAcc().GetNormal());
          VLOG_0 << "lateral_acc: " << lateral_acc << " vertival_acc: " << vertical_acc << " acc" << acc << "\n";
          _detector_l_acc.Detect(lateral_acc, lateral_acc_thresh_value);
          _detector_v_acc.Detect(vertical_acc, vertical_acc_thresh_value);
          _detector_acc.Detect(acc, acc_thresh_value);
          _detect_count = _detector_l_acc.GetCount() + _detector_v_acc.GetCount() + _detector_acc.GetCount();
        }
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

bool EvalParkingComfort::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_comfort_pair, R"(低于舒适度次数)", std::to_string(_detect_count));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_comfort_pair);
  }
  return true;
}

void EvalParkingComfort::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_acc, _kpi_name);
}

EvalResult EvalParkingComfort::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detect_count);
    if (_detect_count >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "low comfort");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "comfort check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "comfort check skipped");
}
bool EvalParkingComfort::ShouldStopScenario(std::string &reason) {
  auto ret = _detect_count >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "parking comfort is low";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
