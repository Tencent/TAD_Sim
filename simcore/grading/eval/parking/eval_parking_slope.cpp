// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_slope.h"
#include <algorithm>
#include "common/coord_trans.h" /*coord_trans_api*/
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSlope::_kpi_name[] = "ParkingSlope";
sim_msg::TestReport_PairData EvalParkingSlope::s_parking_slope_pair;

EvalParkingSlope::EvalParkingSlope() {
  _parking_slope = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSlope::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_slope_pair);
  }
  return true;
}

bool EvalParkingSlope::Step(eval::EvalStep &helper) {
  double cur_slope = 0.0;
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
          // get the map information pointer and check whether the pointer is null
          auto map_info = ego_front->GetMapInfo();
          if (map_info.get() == nullptr) {
            VLOG_0 << "map info missing\n";
            return false;
          }
          CPosition currentPoint = ego_front->GetLocation().GetPosition();
          // // 1.trans current pos to 84
          _map_mgr->ENUToWGS84(currentPoint);
          hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
          if (ego_lane) {
            hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
            if (ego_road) {
              // // 2.calculate sl
              const hadmap::txLineCurve *pLine = dynamic_cast<const hadmap::txLineCurve *>(ego_road->getGeometry());
              double s = 0;
              double l = 0;
              double yaw = 0;
              pLine->xy2sl(currentPoint.GetX(), currentPoint.GetY(), s, l, yaw);
              for (const auto &slopeV : ego_road->getSlope()) {
                if (s >= slopeV.m_starts && s < slopeV.m_starts + slopeV.m_length) {
                  cur_slope = slopeV.m_slope;
                  break;
                }
              }
              _parking_slope = std::max(_parking_slope, cur_slope);
              _detector.Detect(cur_slope, m_defaultThreshDouble);
            }
          }
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

bool EvalParkingSlope::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_slope_pair, "泊车坡度", std::to_string(_parking_slope));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_slope_pair);
  }
  return true;
}

void EvalParkingSlope::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSlope::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "excessive slope");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking slope check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking slope check skipped");
}

bool EvalParkingSlope::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "excessive slope";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
