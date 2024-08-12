// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_milleage.h"
#include <cmath>
#include "location.pb.h"

namespace eval {
const char EvalMileage::_kpi_name[] = "Milleage";

EvalMileage::EvalMileage() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _milleage = 0.0;
}
bool EvalMileage::Init(eval::EvalInit &helper) {
  _first_frame = true;
  return true;
}
bool EvalMileage::Step(eval::EvalStep &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      const CPosition &cur_position = ego_front->GetLocation().GetPosition();

      // nan protection
      if (std::isnan(cur_position.GetX()) || std::isnan(cur_position.GetY())) {
        auto locMsg = _msg_mgr->Get(topic::LOCATION);
        sim_msg::Location loc;
        loc.ParseFromString(locMsg.GetPayload());
        LOG_ERROR << "NaN position of ego detected.\n";
        LOG_ERROR << loc.DebugString() << "\n";
      } else {
        // mileage calculation
        if (!_first_frame) {
          _milleage += CEvalMath::Sub(cur_position, _last_postion).GetNormal2D();
          _last_postion = cur_position;
        } else {
          _last_postion = cur_position;
          _milleage = 0.0;
          _first_frame = false;
        }
      }
      VLOG_1 << "_milleage is " << _milleage << " m.\n";
    } else {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalMileage::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("TotalMilleage", std::to_string(_milleage));

  // set report
  ReportHelper::SetTotalMileage(*helper.GetReport(), _milleage / 1000.0);

  return true;
}

EvalResult EvalMileage::IsEvalPass() { return EvalResult(); }
bool EvalMileage::ShouldStopScenario(std::string &reason) { return false; }
void EvalMileage::SetGradingMsg(sim_msg::Grading &msg) { msg.set_mileage(_milleage / 1000.0); }
void EvalMileage::SetLegacyReport(sim_msg::Grading_Statistics &msg) { msg.set_total_milleage(_milleage / 1000.0); }
}  // namespace eval
