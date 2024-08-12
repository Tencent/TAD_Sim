// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_road_width.h"
#include <float.h>
#include <algorithm>
#include "common/coord_trans.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingRoadWidth::_kpi_name[] = "ParkingRoadWidth";
sim_msg::TestReport_PairData EvalParkingRoadWidth::s_parking_road_width_pair;

EvalParkingRoadWidth::EvalParkingRoadWidth() {
  _parking_road_width = DBL_MAX;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingRoadWidth::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_road_width_pair);
  }
  return true;
}

bool EvalParkingRoadWidth::Step(eval::EvalStep &helper) {
  double cur_road_width = DBL_MAX;
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
              // 2.calculate sl
              const hadmap::txLineCurve *pLine = dynamic_cast<const hadmap::txLineCurve *>(ego_road->getGeometry());
              double s = 0.0;
              double l = 0.0;
              double yaw = 0.0;
              pLine->xy2sl(currentPoint.GetX(), currentPoint.GetY(), s, l, yaw);
              // 3.calculate leftBoundaryLane , rightBoundaryLane;
              double fStartS = 0.0;
              for (auto it : ego_road->getSections()) {
                if (s >= fStartS && s <= fStartS + it->getLength()) {
                  hadmap::txLanes lanes = it->getLanes();
                  if (lanes.size() > 0) {
                    // 求最右边的车道的s点.
                    hadmap::txLanePtr rightMostLane = lanes.back();
                    hadmap::txPoint ptRight(0, 0, 0);
                    if (rightMostLane != NULL) {
                      hadmap::txLaneBoundaryPtr bunPtr = rightMostLane->getRightBoundary();
                      if (bunPtr != NULL) {
                        hadmap::txPoint ptRight = bunPtr->getGeometry()->getPoint(s - fStartS);
                      }
                    }

                    // 求最左边的车道的s点.
                    hadmap::txPoint ptLeft(0, 0, 0);
                    hadmap::txLanePtr leftMostLane = lanes.front();
                    if (rightMostLane != NULL) {
                      hadmap::txLaneBoundaryPtr bunPtr = leftMostLane->getLeftBoundary();
                      if (bunPtr != NULL) {
                        ptLeft = bunPtr->getGeometry()->getPoint(s - fStartS);
                      }
                    }
                    double x = ptRight.x, y = ptRight.y, z = ptRight.z;
                    coord_trans_api::lonlat2enu(x, y, z, ptLeft.x, ptLeft.y, ptLeft.z);
                    cur_road_width = std::sqrt(x * x + y * y);
                    break;
                  }
                }
                fStartS += it->getLength();
              }
              VLOG_0 << "parking road width: " << _parking_road_width << " cur_road width: " << cur_road_width << "\n";
              _parking_road_width = std::min(_parking_road_width, cur_road_width);
              _detector.Detect(cur_road_width, m_defaultThreshDouble);
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

bool EvalParkingRoadWidth::Stop(eval::EvalStop &helper) {
  std::string result = _parking_road_width == DBL_MAX ? "null" : std::to_string(_parking_road_width);
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_road_width_pair, _parking_type + R"(道路宽度)", result);
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_road_width_pair);
  }
  return true;
}

void EvalParkingRoadWidth::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingRoadWidth::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, " road is too narrow ");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking road width check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking road width check skipped");
}

bool EvalParkingRoadWidth::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = " road is too narrow";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
