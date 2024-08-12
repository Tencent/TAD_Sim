// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_search_angle.h"
#include <algorithm>
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSearchAngle::_kpi_name[] = "ParkingSearchAngle";
sim_msg::TestReport_PairData EvalParkingSearchAngle::s_parking_search_angle_pair;

EvalParkingSearchAngle::EvalParkingSearchAngle() {
  _max_parking_search_angle = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSearchAngle::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_search_angle_pair);
  }
  return true;
}

bool EvalParkingSearchAngle::Step(eval::EvalStep &helper) {
  double search_angle = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        Eigen::Vector3d ego_vector;
        // todo: 暂时不能确定上报角点方式（当前讨论结果是将识别的车位靠路边的两个角点上报）.
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          RectCorners &&corners_enu = ego_front->TransCorners2BaseCoord();
          ego_vector = corners_enu[0] - corners_enu[3];
        }

        for (const auto &parking : parkMsg.parking()) {
          // calculate angle
          Eigen::Vector3d parking_vector((parking.lt().x() - parking.lb().x()), (parking.lt().y() - parking.lb().y()),
                                         (parking.lt().z() - parking.lb().z()));
          search_angle = GetAngleDiff(ego_vector, parking_vector, 1);
          VLOG_0 << "search angel: " << search_angle << " _max_parking_angel: " << _max_parking_search_angle << "\n";
          _max_parking_search_angle = std::max(_max_parking_search_angle, search_angle);
        }
        _detector.Detect(search_angle, m_defaultThreshDouble);
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

bool EvalParkingSearchAngle::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    std::string indicator_name = "搜索角s";
    indicator_name.pop_back();
    ReportHelper::SetPairData(s_parking_search_angle_pair, indicator_name, std::to_string(_max_parking_search_angle));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_angle_pair);
  }
  return true;
}

void EvalParkingSearchAngle::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSearchAngle::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "over maximum parking search angle");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking search angle check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking search angle check skipped");
}

bool EvalParkingSearchAngle::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "over maximum parking search angle";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
