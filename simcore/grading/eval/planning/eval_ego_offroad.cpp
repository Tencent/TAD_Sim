// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_ego_offroad.h"

namespace eval {
const char EvalEgoOffroad::_kpi_name[] = "Planning_EgoOffRoad";

sim_msg::TestReport_XYPlot EvalEgoOffroad::sEgoOffroad;

EvalEgoOffroad::EvalEgoOffroad() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  mEgoOffroad = false;
}
bool EvalEgoOffroad::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(sEgoOffroad, "ego off-road", "", "t", "s", {"ego offroad"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(sEgoOffroad, "thresh upper", 0, 1, 1);
  }

  return true;
}
bool EvalEgoOffroad::Step(eval::EvalStep &helper) {
  mEgoOffroad = false;

  if (!IsModuleValid()) {
    VLOG_0 << "eval algorithm module is not valid.\n";
    return false;
  }
  if (!m_KpiEnabled) {
    VLOG_1 << "eval algorithm is not enabled, " << _kpi_name << "\n";
    return true;
  }

  // get the ego pointer and check whether the pointer is null
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  if (ego_front == nullptr) {
    VLOG_1 << "ego missing.\n";
    return true;
  }

  // get map
  auto mapInfo = ego_front->GetMapInfo();

  if (mapInfo.get() == nullptr) {
    VLOG_0 << "map is nullptr.\n";
    return false;
  }

  // mEgoOffroad = (!mapInfo->m_on_road) && (!mapInfo->m_in_junction);
  mEgoOffroad = (!ego_front->CheckCornerWithinTheLaneBoundry()) && (!mapInfo->m_in_junction);
  _detector.Detect(mEgoOffroad, 0.5);

  // add data to xy-pot
  if (isReportEnabled()) {
    sEgoOffroad.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    sEgoOffroad.mutable_y_axis()->at(0).add_axis_data(mEgoOffroad);
  }

  return true;
}
bool EvalEgoOffroad::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, sEgoOffroad);
  }

  return true;
}

void EvalEgoOffroad::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalEgoOffroad::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max ego offroad");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max ego offroad check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max ego offroad check skipped");
}
bool EvalEgoOffroad::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max ego offroad";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
