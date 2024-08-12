// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_search_memory_num.h"
#include "eval_parking_helper.h"
#include "parking_state.pb.h"
namespace eval {
const char EvalParkingSearchMemoryNum::_kpi_name[] = "ParkingMemoryNum";
sim_msg::TestReport_XYPlot EvalParkingSearchMemoryNum::s_parking_memory_num_plot;

EvalParkingSearchMemoryNum::EvalParkingSearchMemoryNum() : _parking_spaces() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingSearchMemoryNum::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _parking_type = getThreshValueByID_String(m_Kpi, m_KpiEnabled, "ParkingType");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_parking_memory_num_plot, _parking_type + " memory num", "", "t", "s",
                               {_parking_type + "emory num"}, {"N/A"}, 1);
  }
  return true;
}

bool EvalParkingSearchMemoryNum::isContainParkingSpace(const sim_msg::Parking *p) {
  bool is_contian = false;
  for (auto parking_ptr : _parking_spaces) {
    is_contian = isSamePoint(&parking_ptr->lt(), &p->lt()) && isSamePoint(&parking_ptr->lb(), &p->lb());
    break;
  }
  if (!is_contian) _parking_spaces.push_back(p);
  return is_contian;
}

bool EvalParkingSearchMemoryNum::Step(eval::EvalStep &helper) {
  int memory_num = 0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_searching == parkMsg.stage()) {
        for (const auto &parking : parkMsg.parking()) {
          if (_parking_type == "平行车位" && parking.type() != sim_msg::ParkingType::TYPE_PARALLEL) continue;
          if (_parking_type == "垂直车位" && parking.type() != sim_msg::ParkingType::TYPE_VERTICAL) continue;
          isContainParkingSpace(&parking);
          VLOG_0 << "parking id: " << parking.id() << "\n";
        }

        memory_num = _parking_spaces.size();
        // add data to xy-pot

        // _detector.Detect(memory_num, m_defaultThreshDouble);
      } else {
        std::vector<const sim_msg::Parking *> empty_vector;
        _parking_spaces.swap(empty_vector);
      }
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
    if (isReportEnabled()) {
      s_parking_memory_num_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_parking_memory_num_plot.mutable_y_axis()->at(0).add_axis_data(memory_num);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingSearchMemoryNum::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_parking_memory_num_plot);
  }
  return true;
}

void EvalParkingSearchMemoryNum::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingSearchMemoryNum::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "parking memory failed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking memory check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking memory check skipped");
}

bool EvalParkingSearchMemoryNum::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "parking memory failed";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
