// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "open_low_beam_when_driving_night.h"

namespace eval {
const char OpenLowBeamWhenDrivingNight::_kpi_name[] = "OpenLowBeamWhenDrivingNight";
sim_msg::TestReport_XYPlot OpenLowBeamWhenDrivingNight::s_open_low_beam_when_driving_night_plot;

OpenLowBeamWhenDrivingNight::OpenLowBeamWhenDrivingNight() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool OpenLowBeamWhenDrivingNight::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_open_low_beam_when_driving_night_plot, "open_low_beam_when_driving_night", "", "t",
                               "s", {"result_beam_proper_used", "_cond_night"}, {"OnOff", "OnOff"}, 2);

    ReportHelper::ConfigXYPlotThreshold(s_open_low_beam_when_driving_night_plot,
                                        "OpenLowBeamWhenDrivingNight Violation ", 0, 1, _threshold, "", 0, 0, INT32_MIN,
                                        0);
  }

  return true;
}

bool OpenLowBeamWhenDrivingNight::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (!IsModuleValid() || !m_KpiEnabled) {
    VLOG_0 << _kpi_name << " kpi not enabled.\n";
    return false;
  }

  // get the ego pointer and check whether the pointer is null
  auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
  if (!ego_front) {
    VLOG_0 << _kpi_name << " ego actor missing.\n";
    return false;
  }

  // param init
  _cond_night = false;
  _result = 0.0;

  // 1.get the condition
  // whether is night
  auto environment_ptr = _actor_mgr->GetEnvironmentActorPtr();
  if (environment_ptr) {
    auto time_of_day = environment_ptr->GetPartsOfDay();
    _cond_night = (time_of_day == eval::PARTS_OF_DAY_MIDNIGHT || time_of_day == eval::PARTS_OF_DAY_DAWN ||
                   time_of_day == eval::PARTS_OF_DAY_NIGHT);
  }

  // check if beam properly used
  const eval::VehicleBodyControl &veh_fb = ego_front->GetVehicleBodyCmd();
  if (_cond_night) {
    _result = veh_fb.m_low_beam != OnOff::ON;
  }
  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should use beam, current ego beam status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_open_low_beam_when_driving_night_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_open_low_beam_when_driving_night_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_open_low_beam_when_driving_night_plot.mutable_y_axis()->at(1).add_axis_data(_cond_night);
  }

  return true;
}

bool OpenLowBeamWhenDrivingNight::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_open_low_beam_when_driving_night_plot);
  }

  return true;
}

void OpenLowBeamWhenDrivingNight::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult OpenLowBeamWhenDrivingNight::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego should beam proper used");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "beam check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "beam check skipped");
}

bool OpenLowBeamWhenDrivingNight::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ego should beam proper used";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
