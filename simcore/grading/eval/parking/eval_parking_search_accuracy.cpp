// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_search_accuracy.h"
#include "eval_parking_helper.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingSearchAccuracy::_kpi_name[] = "ParkingIdentifiedAccuracy";
sim_msg::TestReport_PairData EvalParkingSearchAccuracy::s_parking_search_parallel_accuracy_pair;
sim_msg::TestReport_PairData EvalParkingSearchAccuracy::s_parking_search_vertical_accuracy_pair;
sim_msg::TestReport_PairData EvalParkingSearchAccuracy::s_parking_search_slanted_accuracy_pair;
sim_msg::TestReport_PairData EvalParkingSearchAccuracy::s_parking_search_total_accuracy_pair;

double parallel_thresh_value = 0.0;
double vertical_thresh_value = 0.0;
double slanted_thresh_value = 0.0;

EvalParkingSearchAccuracy::EvalParkingSearchAccuracy() : _parking_spaces() {
  _parallel_identified_num = 0;
  _vertical_identified_num = 0;
  _slanted_identified_num = 0;
  _parallel_total_num = 0;
  _vertical_total_num = 0;
  _slanted_total_num = 0;
  _parallel_accuracy = 0.0;
  _vertical_accuracy = 0.0;
  _slanted_accuracy = 0.0;
  _total_accuracy = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSearchAccuracy::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigPairData(s_parking_search_parallel_accuracy_pair);
    ReportHelper::ConfigPairData(s_parking_search_vertical_accuracy_pair);
    ReportHelper::ConfigPairData(s_parking_search_slanted_accuracy_pair);
    ReportHelper::ConfigPairData(s_parking_search_total_accuracy_pair);
  }
  return true;
}

bool EvalParkingSearchAccuracy::isContainParkingSpace(const sim_msg::Parking *p) {
  bool is_contian = false;
  for (auto parking_ptr : _parking_spaces) {
    is_contian = isSamePoint(&parking_ptr->lt(), &p->lt()) && isSamePoint(&parking_ptr->lb(), &p->lb());
    break;
  }
  if (!is_contian) _parking_spaces.push_back(p);
  return is_contian;
}

// sim_msg::Parking EvalParkingSearchAccuracy::getClosestParkingSpace(Eigen::Vector3d ego_vec, double radius /* = 100
// */) {
//   sim_msg::Parking res;
//   if (IsModuleValid() && m_KpiEnabled) {
//
//     sim_msg::ParkingSpace park_space_msg;
//
//     EvalMsg &&s_msg = _msg_mgr->Get(topic::PARKING_SPACE);
//
//     if (s_msg.GetPayload().size() > 0 && park_space_msg.ParseFromString(s_msg.GetPayload())) {
//       double min_distance = radius + 1;
//       for (const auto &parking : park_space_msg.world().parking()) {
//         double dis = GetPointToPakingLineDist2D(ego_vec, parking);
//         if (dis < radius && dis < min_distance) {
//           min_distance = dis;
//           res = parking;
//         }
//       }
//       if (min_distance > radius)
//         VLOG_2 << "[getClosestParkingSpace]  not find parking space in " << radius << "\n";
//       else
//         VLOG_2 << "[getClosestParkingSpace] find parking space : " << res.DebugString() << " and the min_dis is: ["
//                << min_distance << "]\n";
//     }
//   } else {
//     VLOG_2 << "[getClosestParkingSpace] the Parse error\n";
//   }
//   return res;
// }

bool EvalParkingSearchAccuracy::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    sim_msg::ParkingSpace parkSpaceMsg;
    // sim_msg::Parking park;

    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    EvalMsg &&s_msg = _msg_mgr->Get(topic::PARKING_SPACE);

    // GetPointToPakingLineDist2D(Eigen::Vector3d(), parkSpaceMsg);
    // parkSpaceMsg.
    // EvalMsg &&parking_msg = _msg_mgr->Get(topic::PARKING);

    VLOG_0 << "message size: " << msg.GetPayload().size() << ", " << s_msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload()) && s_msg.GetPayload().size() > 0 &&
        parkSpaceMsg.ParseFromString(s_msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      VLOG_0 << "PakSpaceMsg Info: " << parkSpaceMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        // parking space type 也可能识别错误 所以这里只看真值的 parking space type
        for (auto &parking : parkMsg.parking()) {
          sim_msg::ParkingType pt = parking.type();
          bool is_identified = false;
          bool is_contain = isContainParkingSpace(&parking);
          if (!is_contain) {
            for (auto t_parking : parkSpaceMsg.world().parking()) {
              ParkingWGS84ToENU(_map_mgr, t_parking);
              is_identified = isSameParking(&(parking.lt()), &(parking.lb()), &t_parking);
              if (is_identified) {
                pt = t_parking.type();
                break;
              }
            }
          }
          VLOG_0 << "parking type : " << pt << "\n";

          switch (pt) {
            case sim_msg::ParkingType::TYPE_PARALLEL:
              _parallel_identified_num += is_identified ? 1 : 0;
              _parallel_total_num += is_contain ? 0 : 1;
              break;
            case sim_msg::ParkingType::TYPE_VERTICAL:
              _vertical_identified_num += is_identified ? 1 : 0;
              _vertical_total_num += is_contain ? 0 : 1;
              break;
            default:
              _slanted_identified_num += is_identified ? 1 : 0;
              _slanted_total_num += is_contain ? 0 : 1;
              break;
          }
        }
        _parallel_accuracy = _parallel_total_num == 0 ? 0.0 : _parallel_identified_num / _parallel_total_num;
        _vertical_accuracy = _vertical_total_num == 0 ? 0.0 : _vertical_identified_num / _vertical_total_num;
        _slanted_accuracy = _slanted_total_num == 0 ? 0.0 : _slanted_identified_num / _slanted_total_num;
        _total_accuracy =
            _parking_spaces.size() == 0
                ? 0.0
                : (_parallel_total_num + _vertical_total_num + _slanted_total_num) / _parking_spaces.size();
        // _detector.Detect(_parallel_total_num == 0 ? 0 : _parallel_identified_num / _parallel_total_num,
        // _eval_cfg.GetValueAsDouble("ParallelThreshValue", parallel_thresh_value));
        // _detector.Detect(_vertical_total_num == 0 ? 0 : _vertical_identified_num / _vertical_total_num,
        // _eval_cfg.GetValueAsDouble("VerticalThreshValue", vertical_thresh_value));
        // _detector.Detect(_slanted_total_num == 0 ? 0 : _slanted_identified_num / _slanted_total_num,
        // _eval_cfg.GetValueAsDouble("SlantedThreshValue", slanted_thresh_value));
        VLOG_0 << "para: " << _parallel_accuracy << "verrical: " << _vertical_accuracy
               << "_slant: " << _slanted_accuracy << "\n";
        VLOG_0 << "parking space size: " << _parking_spaces.size() << "\n";
        _detector.Detect(_total_accuracy, m_defaultThreshDouble);
      } else {
        _parallel_identified_num = 0;
        _vertical_identified_num = 0;
        _slanted_identified_num = 0;
        _parallel_total_num = 0;
        _vertical_total_num = 0;
        _slanted_total_num = 0;
        std::vector<const sim_msg::Parking *> empty_vector;
        _parking_spaces.swap(empty_vector);
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

bool EvalParkingSearchAccuracy::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    std::string _lateral_fl_name = "平行车位识别准确率s";
    _lateral_fl_name.pop_back();
    std::string _lateral_fr_name = "垂直车位识别准确率s";
    _lateral_fr_name.pop_back();
    std::string _lateral_rl_name = "斜列车位识别准确率s";
    _lateral_rl_name.pop_back();
    std::string _lateral_rr_name = "车位识别准确率s";
    _lateral_rr_name.pop_back();

    ReportHelper::SetPairData(s_parking_search_parallel_accuracy_pair, _lateral_fl_name,
                              std::to_string(_parallel_accuracy));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_parallel_accuracy_pair);

    ReportHelper::SetPairData(s_parking_search_vertical_accuracy_pair, _lateral_fr_name,
                              std::to_string(_vertical_accuracy));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_vertical_accuracy_pair);

    ReportHelper::SetPairData(s_parking_search_slanted_accuracy_pair, _lateral_rl_name,
                              std::to_string(_slanted_accuracy));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_slanted_accuracy_pair);

    // ReportHelper::SetPairData(s_parking_search_total_accuracy_pair, R"(Accuracy)",
    //                           std::to_string(_total_accuracy));
    ReportHelper::SetPairData(s_parking_search_total_accuracy_pair, _lateral_rr_name, std::to_string(_total_accuracy));
    ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_search_total_accuracy_pair);
  }
  return true;
}

void EvalParkingSearchAccuracy::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSearchAccuracy::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "low identified accuracy");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "identified accuracy check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "identified accuracy check skipped");
}
bool EvalParkingSearchAccuracy::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "low identified accuracy";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
