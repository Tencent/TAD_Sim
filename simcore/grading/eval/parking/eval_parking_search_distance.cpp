// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_search_distance.h"
#include <float.h>
#include <algorithm>
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSearchDistance::_kpi_name[] = "ParkingSearchDistance";
sim_msg::TestReport_PairData EvalParkingSearchDistance::s_parking_search_distance_pair;

EvalParkingSearchDistance::EvalParkingSearchDistance() {
  _parking_search_distance = DBL_MAX;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSearchDistance::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_search_distance_pair);
  }
  return true;
}

bool EvalParkingSearchDistance::Step(eval::EvalStep &helper) {
  double search_distance = DBL_MAX;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        Eigen::Vector3d l_middle_p;
        Eigen::Vector3d r_middle_p;
        // todo: 暂时不能确定上报角点方式（当前讨论结果是将识别的车位靠路边的两个角点上报）.
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          // 取左右两侧车身边缘线中点.
          RectCorners &&corners_enu = ego_front->TransCorners2BaseCoord();
          l_middle_p = (corners_enu[0] + corners_enu[3]) / 2;
          r_middle_p = (corners_enu[1] + corners_enu[2]) / 2;
        }

        for (const auto &parking : parkMsg.parking()) {
          // calculate distance
          double left_distance = GetPointToPakingLineDist2D(l_middle_p, parking);
          double right_distance = GetPointToPakingLineDist2D(r_middle_p, parking);
          VLOG_0 << "search dist: " << search_distance << " _max_parking_dist: " << _parking_search_distance << "\n";
          search_distance = std::min(search_distance, std::min(left_distance, right_distance));
          _parking_search_distance = std::min(_parking_search_distance, search_distance);
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

bool EvalParkingSearchDistance::Stop(eval::EvalStop &helper) {
  std::string result = _parking_search_distance == DBL_MAX ? "null" : std::to_string(_parking_search_distance);
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_search_distance_pair, R"(寻位距离)", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_distance_pair);
  }
  return true;
}

void EvalParkingSearchDistance::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSearchDistance::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "over maximum parking search distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking search distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking search distance check skipped");
}

bool EvalParkingSearchDistance::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "over maximum parking search distance";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
