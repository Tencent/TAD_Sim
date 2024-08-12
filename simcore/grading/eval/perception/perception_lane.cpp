// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "perception_lane.h"
#include "osi_sensordata.pb.h"

#include "perception_helper.h"

namespace eval {
const char EvalPerceptionLane::_kpi_name[] = "Perception_Lane";

EvalPerceptionLane::EvalPerceptionLane() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalPerceptionLane::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
  }

  return true;
}
bool EvalPerceptionLane::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get perception msg
    eval::EvalMsg msg = _msg_mgr->Get(PERCEPTION_TOPIC);
    std::string payload = msg.GetPayload();

    if (payload.size() > 0) {
      // perception msg
      osi3::SensorData sensor_data;
      if (sensor_data.ParseFromString(payload)) {
        // get ego
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

        if (ego_front == nullptr) {
          LOG_WARNING << "ego actor missing.\n";
          return true;
        }

        // capture data
        for (auto i = 0; i < sensor_data.lane_boundary_size(); ++i) {
          // moving obj
          const osi3::DetectedLaneBoundary &boundry = sensor_data.lane_boundary().at(i);

          // capture step
        }
      } else {
        LOG_ERROR << "fail to parse payload on topic " << PERCEPTION_TOPIC;
        return false;
      }
    } else {
      VLOG_2 << "no payload on topic " << PERCEPTION_TOPIC;
      return true;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalPerceptionLane::Stop(eval::EvalStop &helper) {
  // add report

  return true;
}

EvalResult EvalPerceptionLane::IsEvalPass() {
  return EvalResult(sim_msg::TestReport_TestState_PASS, "Perception Lane Capture Finished");
}
bool EvalPerceptionLane::ShouldStopScenario(std::string &reason) { return false; }
}  // namespace eval
