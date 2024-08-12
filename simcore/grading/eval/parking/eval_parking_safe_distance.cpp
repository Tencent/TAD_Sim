// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_safe_distance.h"
#include <float.h>
#include <algorithm>
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSafeDistance::_kpi_name[] = "ParkingSafeDistance";
sim_msg::TestReport_PairData EvalParkingSafeDistance::s_parking_safe_distance_pair;

EvalParkingSafeDistance::EvalParkingSafeDistance() {
  _parking_safe_distance = DBL_MAX;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSafeDistance::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_safe_distance_pair);
  }
  return true;
}

bool EvalParkingSafeDistance::Step(eval::EvalStep &helper) {
  double safe_distance = DBL_MAX;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_in == parkMsg.stage()) {
        // get shape and pos of ego
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          RectCorners &&ego_corners = ego_front->TransCorners2BaseCoord();
          // check traffic vehicle safe distance
          // step 1 for traffic vehilce
          VehilceActorList &&vehicle_actors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);
          for (const auto &fellow : vehicle_actors) {
            RectCorners &&veh_corners = fellow->TransCorners2BaseCoord();
            for (const auto &v_corner : veh_corners) {
              // vehicle corner to ego distance
              safe_distance =
                  std::min(safe_distance, GetPointToPolygonDist2D({v_corner.x(), v_corner.y(), 0.0}, ego_corners));
            }

            for (const auto &e_corner : ego_corners) {
              // ego corner to vehicle distance
              safe_distance =
                  std::min(safe_distance, GetPointToPolygonDist2D({e_corner.x(), e_corner.y(), 0.0}, veh_corners));
            }
          }

          // check dynamic obstacle safe distance
          // step 2 for dynamic objs
          DynamicActorList &&dynamic_actors = _actor_mgr->GetFellowActorsByType<CDynamicActorPtr>(Actor_Dynamic);
          for (const auto &fellow : dynamic_actors) {
            const CLocation &loc = fellow->GetLocation();
            Eigen::Vector3d q{loc.GetPosition().GetX(), loc.GetPosition().GetY(), 0.0};
            safe_distance = std::min(safe_distance, GetPointToPolygonDist2D(q, ego_corners));
          }

          // check static obstacle safe distance
          StaticActorList &&static_actors = _actor_mgr->GetFellowActorsByType<CStaticActorPtr>(Actor_Static);
          for (const auto &fellow : static_actors) {
            const CLocation &loc = fellow->GetLocation();
            Eigen::Vector3d q{loc.GetPosition().GetX(), loc.GetPosition().GetY(), 0.0};
            safe_distance = std::min(safe_distance, GetPointToPolygonDist2D(q, ego_corners));
          }
          VLOG_0 << "veh size: " << vehicle_actors.size() << " dynamic size: " << dynamic_actors.size()
                 << " static size: " << static_actors.size() << "\n";
          VLOG_0 << "safe_distance: " << safe_distance << "\n";

          _detector.Detect(safe_distance, m_defaultThreshDouble);
          _parking_safe_distance = std::min(_parking_safe_distance, safe_distance);
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

bool EvalParkingSafeDistance::Stop(eval::EvalStop &helper) {
  std::string result = _parking_safe_distance == DBL_MAX ? "null" : std::to_string(_parking_safe_distance);

  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_safe_distance_pair, R"(安全距离)", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_safe_distance_pair);
  }
  return true;
}

void EvalParkingSafeDistance::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSafeDistance::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "over maximum parking safe distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking safe distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking safe distance check skipped");
}

bool EvalParkingSafeDistance::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "over maximum parking safe distance";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
