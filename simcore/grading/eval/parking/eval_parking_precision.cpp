// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <cmath>
#include <iostream>
#include <string>

#include "eval_parking_helper.h"
#include "eval_parking_precision.h"
#include "parking_state.pb.h"

namespace eval {
using namespace std;
const char EvalParkingPrecision::_kpi_name[] = "ParkingPrecision";
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_lateral_fl_dist_pair;
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_lateral_fr_dist_pair;
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_lateral_rl_dist_pair;
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_lateral_rr_dist_pair;
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_vertical_f_pair;
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_vertical_r_pair;
sim_msg::TestReport_PairData EvalParkingPrecision::s_parking_body_angle_pair;

EvalParkingPrecision::EvalParkingPrecision() {
  _parking_id = 0;
  _lateral_fl_dist = 0.0;
  _lateral_fr_dist = 0.0;
  _lateral_rl_dist = 0.0;
  _lateral_rr_dist = 0.0;
  _vertical_f_dist = 0.0;
  _vertical_r_dist = 0.0;
  _lateral_thresh_value = 0.0;
  _vertical_thresh_value = 0.0;
  _angle_thresh_value = 0.0;
  _detect_count = 0;
  _lateral_angle = 0.0;

  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingPrecision::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _parking_id = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ParkingSpaceId");
    _lateral_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LateralThreshold");
    _vertical_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "VerticalThreshold");
    _angle_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "BodyAngleThreshold");

    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(s_parking_lateral_fl_dist_pair);
    ReportHelper::ConfigPairData(s_parking_lateral_fr_dist_pair);
    ReportHelper::ConfigPairData(s_parking_lateral_rl_dist_pair);
    ReportHelper::ConfigPairData(s_parking_lateral_rr_dist_pair);
    ReportHelper::ConfigPairData(s_parking_vertical_f_pair);
    ReportHelper::ConfigPairData(s_parking_vertical_r_pair);
    ReportHelper::ConfigPairData(s_parking_body_angle_pair);
  }
  return true;
}

bool EvalParkingPrecision::Step(eval::EvalStep &helper) {
  bool is_out_space = false;
  double lateral_fl_dist = 0.0;
  double lateral_fr_dist = 0.0;
  double lateral_rl_dist = 0.0;
  double lateral_rr_dist = 0.0;
  double vertical_f_dist = 0.0;
  double vertical_r_dist = 0.0;
  double lateral_angle = 0.0;
  sim_msg::Parking parking;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    sim_msg::ParkingSpace parkSpaceMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    EvalMsg &&s_msg = _msg_mgr->Get(topic::PARKING_SPACE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << ", " << s_msg.GetPayload().size() << '\n';
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload()) && s_msg.GetPayload().size() > 0 &&
        parkSpaceMsg.ParseFromString(s_msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      VLOG_0 << "PakSpaceMsg Info: " << parkSpaceMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_Completed == parkMsg.stage()) {
        for (auto t_parking : parkSpaceMsg.world().parking()) {
          if (_parking_id == t_parking.id()) {
            ParkingWGS84ToENU(_map_mgr, t_parking);
            parking.CopyFrom(t_parking);
            break;
          }
        }

        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          // 获取主车四轮pos
          const RectCorners &corners_enu = ego_front->GetWheelPosENU();
          // 判断停车是否压线.
          for (const auto &corner : corners_enu) {
            is_out_space = !InParkingSpace(corner, parking);
            VLOG_0 << "is_out_space: " << is_out_space << "\n";
            if (is_out_space) break;
          }
          // 判断车位横纵方向.
          double d1 = CEvalMath::Distance2D({parking.lt().x(), parking.lt().y(), 0.0},
                                            {parking.lb().x(), parking.lb().y(), 0.0});
          double d2 = CEvalMath::Distance2D({parking.lt().x(), parking.lt().y(), 0.0},
                                            {parking.rt().x(), parking.rt().y(), 0.0});

          // 计算纵横精度.
          if (d1 > d2) {
            // 前轮横向精度(左右前轮).
            lateral_fl_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.lb().x(), parking.lb().y(), 0.0}, corners_enu[0]),
                                       GetPointToSegDist({parking.rt().x(), parking.rt().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[0]));
            lateral_fr_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.lb().x(), parking.lb().y(), 0.0}, corners_enu[1]),
                                       GetPointToSegDist({parking.rt().x(), parking.rt().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[1]));
            // 后轮横向精度(左右后轮).
            lateral_rl_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.lb().x(), parking.lb().y(), 0.0}, corners_enu[3]),
                                       GetPointToSegDist({parking.rt().x(), parking.rt().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[3]));
            lateral_rr_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.lb().x(), parking.lb().y(), 0.0}, corners_enu[2]),
                                       GetPointToSegDist({parking.rt().x(), parking.rt().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[2]));
            // 前后纵向精度.
            vertical_f_dist = std::min(
                GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0}, {parking.rt().x(), parking.rt().y(), 0.0},
                                  (corners_enu[0] + corners_enu[1]) / 2),
                GetPointToSegDist({parking.lb().x(), parking.lb().y(), 0.0}, {parking.rb().x(), parking.rb().y(), 0.0},
                                  (corners_enu[0] + corners_enu[1]) / 2));
            vertical_r_dist = std::min(
                GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0}, {parking.rt().x(), parking.rt().y(), 0.0},
                                  (corners_enu[2] + corners_enu[3]) / 2),
                GetPointToSegDist({parking.lb().x(), parking.lb().y(), 0.0}, {parking.rb().x(), parking.rb().y(), 0.0},
                                  (corners_enu[2] + corners_enu[3]) / 2));

            // todo: 车身摆角.
            // lateral_angle = 0.0;
            lateral_angle = std::fabs((ego_front->GetLocation().GetEuler().GetYaw() - parking.yaw()) * 180 / 3.14);

          } else {
            // 前轮横向精度(左右前轮).
            lateral_fl_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.rt().x(), parking.rt().y(), 0.0}, corners_enu[0]),
                                       GetPointToSegDist({parking.lb().x(), parking.lb().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[0]));
            lateral_fr_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.rt().x(), parking.rt().y(), 0.0}, corners_enu[1]),
                                       GetPointToSegDist({parking.lb().x(), parking.lb().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[1]));
            // 后轮横向精度(左右后轮).
            lateral_rl_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.rt().x(), parking.rt().y(), 0.0}, corners_enu[3]),
                                       GetPointToSegDist({parking.lb().x(), parking.lb().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[3]));
            lateral_rr_dist = std::min(GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0},
                                                         {parking.rt().x(), parking.rt().y(), 0.0}, corners_enu[2]),
                                       GetPointToSegDist({parking.lb().x(), parking.lb().y(), 0.0},
                                                         {parking.rb().x(), parking.rb().y(), 0.0}, corners_enu[2]));
            // 前后纵向精度.
            vertical_f_dist = std::min(
                GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0}, {parking.lb().x(), parking.lb().y(), 0.0},
                                  (corners_enu[0] + corners_enu[1]) / 2),
                GetPointToSegDist({parking.rt().x(), parking.rt().y(), 0.0}, {parking.rb().x(), parking.rb().y(), 0.0},
                                  (corners_enu[0] + corners_enu[1]) / 2));
            vertical_r_dist = std::min(
                GetPointToSegDist({parking.lt().x(), parking.lt().y(), 0.0}, {parking.lb().x(), parking.lb().y(), 0.0},
                                  (corners_enu[2] + corners_enu[3]) / 2),
                GetPointToSegDist({parking.rt().x(), parking.rt().y(), 0.0}, {parking.rb().x(), parking.rb().y(), 0.0},
                                  (corners_enu[2] + corners_enu[3]) / 2));

            // todo: 车身摆角.
            lateral_angle = 0.0;
            lateral_angle = std::fabs(ego_front->GetLocation().GetEuler().GetYaw() - parking.yaw());
          }

          _detector_fl.Detect(lateral_fl_dist, _lateral_thresh_value);
          _detector_fr.Detect(lateral_fr_dist, _lateral_thresh_value);
          _detector_rl.Detect(lateral_rl_dist, _lateral_thresh_value);
          _detector_rr.Detect(lateral_rr_dist, _lateral_thresh_value);
          _detector_f.Detect(vertical_f_dist, _vertical_thresh_value);
          _detector_r.Detect(vertical_r_dist, _vertical_thresh_value);
          _detector_a.Detect(lateral_angle, _angle_thresh_value);

          _detect_count = _detector_fl.GetCount() + _detector_fr.GetCount() + _detector_rl.GetCount() +
                          _detector_rr.GetCount() + _detector_f.GetCount() + _detector_r.GetCount() +
                          _detector_a.GetCount();
          _lateral_fl_dist = std::max(_lateral_fl_dist, lateral_fl_dist);
          _lateral_fr_dist = std::max(_lateral_fr_dist, lateral_fr_dist);
          _lateral_rl_dist = std::max(_lateral_rl_dist, lateral_rl_dist);
          _lateral_rr_dist = std::max(_lateral_rr_dist, lateral_rr_dist);
          _vertical_f_dist = std::max(_vertical_f_dist, vertical_f_dist);
          _vertical_r_dist = std::max(_vertical_r_dist, vertical_r_dist);
          _lateral_angle = std::max(_lateral_angle, lateral_angle);
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

bool EvalParkingPrecision::Stop(eval::EvalStop &helper) {
  // set report

  if (isReportEnabled()) {
    std::string _lateral_fl_name = "左前轮横向精度s";
    _lateral_fl_name.pop_back();
    std::string _lateral_fr_name = "右前轮横向精度s";
    _lateral_fr_name.pop_back();
    std::string _lateral_rl_name = "左后轮横向精度s";
    _lateral_rl_name.pop_back();
    std::string _lateral_rr_name = "右后轮横向精度s";
    _lateral_rr_name.pop_back();
    std::string _vertical_f_name = "车头纵向精度s";
    _vertical_f_name.pop_back();
    std::string _vertical_r_name = "车尾纵向精度s";
    _vertical_r_name.pop_back();
    std::string _lateral_angle_name = "车身摆角s";
    _lateral_angle_name.pop_back();
    ReportHelper::SetPairData(s_parking_lateral_fl_dist_pair, _lateral_fl_name, std::to_string(_lateral_fl_dist) + "m");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_lateral_fl_dist_pair);
    ReportHelper::SetPairData(s_parking_lateral_fr_dist_pair, _lateral_fr_name, std::to_string(_lateral_fr_dist) + "m");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_lateral_fr_dist_pair);
    ReportHelper::SetPairData(s_parking_lateral_rl_dist_pair, _lateral_rl_name, std::to_string(_lateral_rl_dist) + "m");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_lateral_rl_dist_pair);
    ReportHelper::SetPairData(s_parking_lateral_rr_dist_pair, _lateral_rr_name, std::to_string(_lateral_rr_dist) + "m");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_lateral_rr_dist_pair);
    ReportHelper::SetPairData(s_parking_vertical_f_pair, _vertical_f_name, std::to_string(_vertical_f_dist) + "m");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_vertical_f_pair);
    ReportHelper::SetPairData(s_parking_vertical_r_pair, _vertical_r_name, std::to_string(_vertical_r_dist) + "m");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_vertical_r_pair);
    ReportHelper::SetPairData(s_parking_body_angle_pair, _lateral_angle_name, std::to_string(_lateral_angle) + "rad");
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_body_angle_pair);
  }
  return true;
}

void EvalParkingPrecision::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_fl, _kpi_name);
}

EvalResult EvalParkingPrecision::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detect_count);
    if (_detect_count >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "imprecise parking");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking precision check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking precision check skipped");
}
bool EvalParkingPrecision::ShouldStopScenario(std::string &reason) {
  auto ret = _detect_count >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "imprecise parking";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
