// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_speed_bump_height.h"
#include <algorithm>
#include "common/coord_trans.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSpeedBumpHeight::_kpi_name[] = "ParkingSpeedBumpHeight";
sim_msg::TestReport_PairData EvalParkingSpeedBumpHeight::s_parking_speed_bump_height_pair;

EvalParkingSpeedBumpHeight::EvalParkingSpeedBumpHeight() {
  _parking_speed_bump_height = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSpeedBumpHeight::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_speed_bump_height_pair);
  }
  return true;
}

bool EvalParkingSpeedBumpHeight::Step(eval::EvalStep &helper) {
  double cur_speed_bump_height = 0.0;
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
          //  获取主车范围内减速带的高度.
          CPosition currentPoint = ego_front->GetLocation().GetPosition();
          // todo: query map SpeedBumps from hadmap. This function is not yet in effect because the hadmap api has not
          // been updated.
          EvalMapObjectPtr &&bump_objects = _map_mgr->GetMapObjects(hadmap::OBJECT_TYPE_SpeedBump, currentPoint, 3);
          VLOG_0 << "whether get bump_obj: " << (bump_objects.get() != nullptr) << "\n";
          if (bump_objects) {
            for (auto bump : bump_objects->m_map_objects) {
              if (bump->getName() == "J_002") {
                double length = 0.0;
                double width = 0.0;
                double height = 0.0;
                bump->getLWH(length, width, height);
                cur_speed_bump_height = std::max(cur_speed_bump_height, height);
              }
            }
            VLOG_0 << "cur bump height: " << cur_speed_bump_height << "\n";
            _detector.Detect(cur_speed_bump_height, m_defaultThreshDouble);
            _parking_speed_bump_height = std::max(cur_speed_bump_height, _parking_speed_bump_height);
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

bool EvalParkingSpeedBumpHeight::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::SetPairData(s_parking_speed_bump_height_pair, R"(泊车减速带高度)",
                              std::to_string(_parking_speed_bump_height));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_speed_bump_height_pair);
  }
  return true;
}

void EvalParkingSpeedBumpHeight::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSpeedBumpHeight::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "speed bump too high");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "speed bump check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "speed bump check skipped");
}

bool EvalParkingSpeedBumpHeight::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "speed bump too high";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
