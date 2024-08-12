// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_ban_signs.h"

namespace eval {
const char EvalBehavBanSigns::_kpi_name[] = "BehavBanSigns";

sim_msg::TestReport_XYPlot EvalBehavBanSigns::_s_behav_no_sign_plot;

EvalBehavBanSigns::EvalBehavBanSigns() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  event_uturn = false;
  uturn_time = 0.0;
}
bool EvalBehavBanSigns::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_behav_no_sign_plot, "incorrect behavior", "", "t", "s", {"ban_sign", "speed"},
                               {"N/A", "m/s"}, 2);
  }
  return true;
}

bool EvalBehavBanSigns::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    auto CheckBanSigns = [this](hadmap::txObjectPtr ban_sign, CEgoActorPtr ego_front) {
      hadmap::OBJECT_SUB_TYPE sub_type = ban_sign->getObjectSubType();
      double dist_2_obj = ego_front->CheckDistanceWithObject(ego_front->GetObjectCLocation(ban_sign).GetPosition());
      bool _sign_violate = false;
      // ban car height 5m
      if (sub_type == hadmap::SIGN_BAN_HEIGHT_5) {
        _sign_violate = ego_front->GetShape().GetHeight() > 5.0;
        this->_detector.Detect(_sign_violate, 0.5);
        if (_sign_violate) this->set_msgs.insert("限高5m");
      } else if (sub_type == hadmap::SIGN_BAN_HEIGHT_3_5) {
        // ban car height 3.5m
        _sign_violate = ego_front->GetShape().GetHeight() > 3.5;
        this->_detector.Detect(_sign_violate, 0.5);
        if (_sign_violate) {
          this->set_msgs.insert("限高3.5m");
        }
      } else if (sub_type == hadmap::SIGN_BAN_HEIGHT_3) {
        // ban car width 3m
        _sign_violate = ego_front->GetShape().GetWidth() > 3.0;
        this->_detector.Detect(_sign_violate, 0.5);
        if (_sign_violate) {
          this->set_msgs.insert("限宽3m");
        }
      } else if (sub_type == hadmap::SIGN_BAN_DIVERINTO || sub_type == hadmap::SIGN_BAN_VEHICLE ||
                 sub_type == hadmap::SIGN_BAN_GO) {
        // ban cross about
        VLOG_1 << "the distance of obj : " << dist_2_obj << "\n";
        if (dist_2_obj <= 3.0) {
          _sign_violate = ego_front->GetSpeed().GetNormal() > 0.56;
          this->_detector.Detect(_sign_violate, 0.5);
          if (_sign_violate) {
            switch (sub_type) {
              case hadmap::SIGN_BAN_DIVERINTO:
                this->set_msgs.insert("禁止驶入");
                break;
              case hadmap::SIGN_BAN_VEHICLE:
                this->set_msgs.insert("禁止机动车通行");
                break;
              case hadmap::SIGN_BAN_GO:
                this->set_msgs.insert("禁止通行");
                break;
              default:
                break;
            }
          }
        }
      } else if (sub_type == hadmap::SIGN_BAN_TEMP_PARKING) {
        _sign_violate = ego_front->GetSpeed().GetNormal() < 0.56;
        this->_detector.Detect(_sign_violate, 0.5);
        if (_sign_violate) {
          this->set_msgs.insert("禁止停车");
        }
      } else if (sub_type == hadmap::SIGN_BAN_WEIGHT_10) {
        // todo: ban car weight
        ego_front->GetVehicleState();
        return true;
      } else if (sub_type == hadmap::SIGN_BAN_UTURN) {
        event_uturn = true;
      }
      return _sign_violate;
    };

    /// 禁止通行  SIGN_BAN_GO
    /// 禁止驶入  SIGN_BAN_DIVERINTO
    // 禁止掉头  SIGN_BAN_UTURN
    /// 禁止高度  SIGN_BAN_HEIGHT_5 SIGN_BAN_HEIGHT_3_5
    /// 禁止宽度  SIGN_BAN_HEIGHT_3
    // 禁止重量  SIGN_BAN_WEIGHT_50  SIGN_BAN_WEIGHT_20  SIGN_BAN_WEIGHT_30  SIGN_BAN_WEIGHT_10  SIGN_BAN_WEIGHT_40
    /// 禁止机动车通行  SIGN_BAN_VEHICLE
    // 禁止多种车辆通行
    /// 禁止停车  SIGN_BAN_TEMP_PARKING
    if (ego_front) {
      double _ego_speed = ego_front->GetSpeed().GetNormal();
      EvalMapObjectPtr obj = std::make_shared<EvalMapObject>();
      ego_front->GetProhibitionSign(obj);
      auto ban_signs = obj->m_map_objects;
      VLOG_1 << "ban_signs.size(): " << ban_signs.size() << "\n";
      bool error_behav = false;
      bool has_ban_sign = false;
      uint64_t ego_roadid = ego_front->GetLaneID().tx_road_id;
      for (auto ban_sign : ban_signs) {
        if (ego_roadid == ban_sign->getRoadId()) {
          has_ban_sign = true;
          error_behav = CheckBanSigns(ban_sign, ego_front) || error_behav;
        }
      }
      if (event_uturn) {
        uturn_time += getModuleStepTime();
        if (uturn_time < 30) {
          VehBehavType veh_type = helper.GetVehStateFlow().GetOutput().m_veh_behav;
          VLOG_1 << "veh_type: " << veh_type << "\n";
          bool uturn_check = (veh_type == VehicleBehavior::U_TurnLeft || veh_type == VehicleBehavior::U_TurnRight);
          error_behav = uturn_check || error_behav;
          if (uturn_check) set_msgs.insert("禁止掉头");
        } else {
          event_uturn = false;
          uturn_time = 0.0;
        }
      }
      _detector.Detect(error_behav, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_no_sign_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_no_sign_plot.mutable_y_axis()->at(0).add_axis_data(has_ban_sign);
        _s_behav_no_sign_plot.mutable_y_axis()->at(1).add_axis_data(_ego_speed);
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
bool EvalBehavBanSigns::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_behav_no_sign_plot);
  }
  return true;
}

void EvalBehavBanSigns::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalBehavBanSigns::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      std::string _msg_no_sign = "";
      for (std::string str : set_msgs) _msg_no_sign += (str + ".\n");
      return EvalResult(sim_msg::TestReport_TestState_FAIL, _msg_no_sign);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before ban sign check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before ban sign check skipped");
}
bool EvalBehavBanSigns::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "behavior before ban sign check fail.";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
