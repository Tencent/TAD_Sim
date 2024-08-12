// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_pred_first_effective.h"
#include "trajectory.pb.h"

namespace eval {
const char EvalPredFirstEffective::_kpi_name[] = "Prediction_FirstEffective";

sim_msg::TestReport_XlsxSheet EvalPredFirstEffective::s_1st_effective;

EvalPredFirstEffective::EvalPredFirstEffective() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalPredFirstEffective::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXLSXSheet(s_1st_effective, "prediction first effective message");
    ReportHelper::ConfigSheetData(m_index, "index");
    ReportHelper::ConfigSheetData(m_fellow_id, "fellow id");
    ReportHelper::ConfigSheetData(m_t_present, "present time [s]");
    ReportHelper::ConfigSheetData(m_t_effective, "duration before effective [s]");
  }

  m_veh_record.clear();
  m_present.clear();
  m_effective.clear();

  return true;
}
bool EvalPredFirstEffective::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    VehilceActorList &&veh_fellows = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);

    // mark present id and time
    for (size_t i = 0; i < veh_fellows.size(); ++i) {
      CVehicleActorPtr veh_ptr = veh_fellows.at(i);
      if (veh_ptr != nullptr) {
        int64_t veh_id = veh_ptr->GetID();
        auto iter = m_present.find(veh_id);
        if (iter == m_present.end()) m_present[veh_id] = helper.GetSimTime();
      }
    }

    // mark effective id and time
    EvalMsg &&msg = _msg_mgr->Get(topic::PREDICTIONS);
    sim_msg::Predictions pred_msg;

    if (msg.GetPayload().size() > 0 && pred_msg.ParseFromString(msg.GetPayload())) {
      for (auto i = 0; i < pred_msg.obs_size(); ++i) {
        const sim_msg::Prediction &pred = pred_msg.obs().at(i);
        int64_t id = pred.id();
        auto iter = m_effective.find(id);
        if (iter == m_effective.end() && pred.point_size() > 0) m_effective[id] = helper.GetSimTime();
      }
    }

    // check if effective prediction message is too slow
    for (auto iter = m_present.begin(); iter != m_present.end(); ++iter) {
      int64_t id = iter->first;
      double t_present = iter->second;
      double t_effective = helper.GetSimTime();

      auto iter_effective = m_effective.find(id);
      if (iter_effective != m_effective.end()) t_effective = m_effective[id];
      m_veh_record[id] = {t_present, t_effective};
      _detector.Detect(t_effective - t_present, m_defaultThreshDouble);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalPredFirstEffective::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    int index = 0;
    for (auto iter = m_veh_record.begin(); iter != m_veh_record.end(); ++iter) {
      double t_present = iter->second.first;
      double t_effective = iter->second.second;
      if (t_effective - t_present - m_defaultThreshDouble > 1e-3) {
        m_index.mutable_data()->Add(std::to_string(index));
        m_fellow_id.mutable_data()->Add(std::to_string(iter->first));
        m_t_present.mutable_data()->Add(std::to_string(t_present));
        m_t_effective.mutable_data()->Add(std::to_string(t_effective - t_present));
        index++;
      }
    }

    ReportHelper::AddSheetData2XLSX(s_1st_effective, m_index);
    ReportHelper::AddSheetData2XLSX(s_1st_effective, m_fellow_id);
    ReportHelper::AddSheetData2XLSX(s_1st_effective, m_t_present);
    ReportHelper::AddSheetData2XLSX(s_1st_effective, m_t_effective);
    ReportHelper::AddXLSXSheet2Attach(*_case.add_steps()->add_attach(), s_1st_effective);
  }

  return true;
}

void EvalPredFirstEffective::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalPredFirstEffective::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above prediction first effective message");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "prediction first effective message check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "prediction first effective message check skipped");
}
bool EvalPredFirstEffective::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above prediction first effective message";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
