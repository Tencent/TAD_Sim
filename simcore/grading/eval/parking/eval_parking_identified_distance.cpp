// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_identified_distance.h"
#include <algorithm>
#include "eval_parking_helper.h"
#include "float.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingIdentifiedDistance::_kpi_name[] = "ParkingIdentifiedDistance";
sim_msg::TestReport_PairData EvalParkingIdentifiedDistance::s_parking_identified_distance_pair;

EvalParkingIdentifiedDistance::EvalParkingIdentifiedDistance() {
  _parking_identified_distance = DBL_MAX;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingIdentifiedDistance::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _sensor_type = getThreshValueByID_String(m_Kpi, m_KpiEnabled, "SensorType");
    // output indicator configuration details
    DebugShowKpi();
  }
  VLOG_0 << "[EvalParkingAccessTimes] m_Kpi = " << m_Kpi.DebugString() << "\n";

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_identified_distance_pair);
  }
  return true;
}

bool EvalParkingIdentifiedDistance::Step(eval::EvalStep &helper) {
  double search_distance = DBL_MAX;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        Eigen::Vector3d l_front_p;
        Eigen::Vector3d r_front_p;
        // todo: 暂时不能确定上报角点方式（当前讨论结果是将识别的车位靠路边的两个角点上报）.
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          // 取车前轮左右两点
          const RectCorners &corners_enu = ego_front->GetWheelPosENU();
          l_front_p = corners_enu[0];
          r_front_p = corners_enu[1];
        }

        for (const auto &parking : parkMsg.parking()) {
          // calculate distance
          double left_distance = GetPointToPakingLineDist2D(l_front_p, parking);
          double right_distance = GetPointToPakingLineDist2D(r_front_p, parking);
          search_distance = std::min(search_distance, std::min(left_distance, right_distance));
          VLOG_0 << "left_dist: " << left_distance << " right_dist: " << right_distance << "\n";
          _parking_identified_distance = std::min(_parking_identified_distance, search_distance);
        }
        _detector.Detect(search_distance, m_defaultThreshDouble);
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

bool EvalParkingIdentifiedDistance::Stop(eval::EvalStop &helper) {
  std::string result = _parking_identified_distance == DBL_MAX ? "null" : std::to_string(_parking_identified_distance);
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_identified_distance_pair, _sensor_type + R"(可识别停车位横向距离)", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_identified_distance_pair);
  }
  return true;
}

void EvalParkingIdentifiedDistance::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingIdentifiedDistance::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "over maximum parking identified distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking identified distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking identified distance check skipped");
}

bool EvalParkingIdentifiedDistance::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "over maximum parking identified distance";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
