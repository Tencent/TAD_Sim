// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_instruct_sign.h"

namespace eval {

const char EvalBehavInstructSign::_kpi_name[] = "BehavInstructSign";
sim_msg::TestReport_XYPlot _s_behav_instruct_sign_plot;

EvalBehavInstructSign::EvalBehavInstructSign() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  subtype = hadmap::OtherSubtype;
  _event_instruct = false;
}

bool EvalBehavInstructSign::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_behav_instruct_sign_plot, "correct behavior", "", "t", "s", {"correct"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_instruct_sign_plot, "", 0, 1, INT32_MAX, "", 1, 1, 0.5, 0);
  }
  return true;
}

bool EvalBehavInstructSign::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    // 靠右行驶  SIGN_INDOCATION_ALONG_RIGHT
    // 靠左行驶  SIGN_INDOCATION_ALONG_LEFT
    // 向右转弯  SIGN_INDOCATION_TURN_RIGHT
    // 向左转弯  SIGN_INDOCATION_TURN_LEFT
    // 直行标志牌  SIGN_INDOCATION_STRAIGHT
    // 直行向左转弯  SIGN_INDOCATION_STRAIGHT_LEFT
    // 直行向右转弯  SIGN_INDOCATION_STRAIGHT_RIGHT
    // 左转右转  SIGN_INDOCATION_LEFT_AND_RIGHT
    // 环岛  SIGN_INDOCATION_ROUNDABOUT
    // 自行车 SIGN_INDOCATION_NO_MOTOR
    // 隧道
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      double _speed = ego_front->GetSpeed().GetNormal();
      EvalMapObjectPtr obj = std::make_shared<EvalMapObject>();
      ego_front->GetIndicateSign(obj);
      bool correct_behav = true;
      uint64_t ego_road_id = ego_front->GetLaneID().tx_road_id;
      for (auto indicate_sign : obj->m_map_objects) {
        if (ego_road_id != indicate_sign->getRoadId()) continue;
        VLOG_1 << "type of sign: " << indicate_sign->getObjectSubType() << "\n";
        hadmap::OBJECT_SUB_TYPE sub_type = indicate_sign->getObjectSubType();
        bool indicate_behav = true;
        if (sub_type == hadmap::SIGN_INDOCATION_ALONG_RIGHT) {
          indicate_behav = ego_front->CurLaneIsRightmostLane();
          if (!indicate_behav) set_signs.insert("not keep right");
        } else if (sub_type == hadmap::SIGN_INDOCATION_ALONG_LEFT) {
          indicate_behav = ego_front->CurLaneIsLeftmostLane();
          if (!indicate_behav) set_signs.insert("not keep left");
        } else if (sub_type == hadmap::SIGN_INDOCATION_ROUNDABOUT) {
          indicate_behav = (_speed <= 0.2);
          if (!indicate_behav) set_signs.insert("roundabout speed above 0.2");
        } else if (sub_type == hadmap::SIGN_INDOCATION_WALK) {
          indicate_behav = (ego_front->GetAcc().GetX() < 0.2 && _speed < 8.34);
          if (!indicate_behav) set_signs.insert("acc or speed around walk passage too high");
        } else if (sub_type == hadmap::SIGN_INDOCATION_NO_MOTOR) {
          indicate_behav = (ego_front->GetAcc().GetX() < 0.2 && _speed < 8.34);
          if (!indicate_behav) set_signs.insert("acc or speed around no_mortor sign too high");
        } else if (sub_type == hadmap::SIGN_INDOCATION_TURN_RIGHT || sub_type == hadmap::SIGN_INDOCATION_TURN_LEFT ||
                   sub_type == hadmap::SIGN_INDOCATION_STRAIGHT || sub_type == hadmap::SIGN_INDOCATION_STRAIGHT_LEFT ||
                   sub_type == hadmap::SIGN_INDOCATION_STRAIGHT_RIGHT ||
                   sub_type == hadmap::SIGN_INDOCATION_LEFT_AND_RIGHT) {
          subtype = sub_type;
        }
        correct_behav = correct_behav && indicate_behav;
      }
      if (!_event_instruct && !map_info->m_on_road) _event_instruct = (subtype != hadmap::OtherSubtype);
      if (_event_instruct && map_info->m_on_road) {
        VehBehavType m_behav = helper.GetVehStateFlow().GetOutput().m_veh_behav;
        bool indicate_behav = true;
        if (m_behav == VehicleBehavior::TurnLeft) {
          indicate_behav =
              (subtype == hadmap::SIGN_INDOCATION_TURN_LEFT || subtype == hadmap::SIGN_INDOCATION_STRAIGHT_LEFT ||
               subtype == hadmap::SIGN_INDOCATION_LEFT_AND_RIGHT);
          if (!indicate_behav) set_signs.insert("can't turn left");
        } else if (m_behav == VehicleBehavior::TurnRight) {
          indicate_behav =
              (subtype == hadmap::SIGN_INDOCATION_TURN_RIGHT || subtype == hadmap::SIGN_INDOCATION_STRAIGHT_RIGHT ||
               subtype == hadmap::SIGN_INDOCATION_LEFT_AND_RIGHT);
          if (!indicate_behav) set_signs.insert("can't turn right");
        } else {
          indicate_behav =
              (subtype == hadmap::SIGN_INDOCATION_STRAIGHT || subtype == hadmap::SIGN_INDOCATION_STRAIGHT_RIGHT ||
               subtype == hadmap::SIGN_INDOCATION_STRAIGHT_LEFT);
          if (!indicate_behav) set_signs.insert("can't straight");
        }
        correct_behav = correct_behav && indicate_behav;
        _event_instruct = false;
        subtype = hadmap::OtherSubtype;
      }
      _fall_detector.Detect(correct_behav, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_instruct_sign_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_instruct_sign_plot.mutable_y_axis()->at(0).add_axis_data(correct_behav);
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalBehavInstructSign::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_behav_instruct_sign_plot);
  }
  return true;
}

void EvalBehavInstructSign::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalBehavInstructSign::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());
    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      std::string _msg_roadsign = "";
      for (auto str : set_signs) _msg_roadsign += (str + "\n");
      return EvalResult(sim_msg::TestReport_TestState_FAIL, _msg_roadsign);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before instruct sign check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before instruct sign check skipped");
}
bool EvalBehavInstructSign::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "error behavior before instruct signs";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
