// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_stop_when_expressway_and_in_the_driveway.h"

namespace eval {
const char NoStopWhenExpresswayAndInTheDriveway::_kpi_name[] = "NoStopWhenExpresswayAndInTheDriveway";
sim_msg::TestReport_XYPlot NoStopWhenExpresswayAndInTheDriveway::s_no_stop_when_expressway_and_in_the_driveway_plot;

NoStopWhenExpresswayAndInTheDriveway::NoStopWhenExpresswayAndInTheDriveway() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoStopWhenExpresswayAndInTheDriveway::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_stop_when_expressway_and_in_the_driveway_plot,
                               "no_stop_when_expressway_and_in_the_driveway check", "", "t", "s",
                               {"result", "_cond_on_enter_expressway"}, {"OnOff", "OnOff"}, 2);

    ReportHelper::ConfigXYPlotThreshold(s_no_stop_when_expressway_and_in_the_driveway_plot, "stop Violation ", 0, 1,
                                        _threshold, "", 0, 0, INT32_MIN, 0);
  }

  return true;
}

bool NoStopWhenExpresswayAndInTheDriveway::Step(eval::EvalStep &helper) {
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

  // get the map information pointer and check whether the pointer is null
  auto map_info = ego_front->GetMapInfo();
  if (map_info.get() == nullptr) {
    VLOG_0 << _kpi_name << " ego actor missing with map.\n";
    return false;
  }

  hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
  if (!ego_lane) {
    VLOG_0 << _kpi_name << " ego actor missing with lane.\n";
    return false;
  }

  // param init
  _cond_on_enter_expressway = false;
  _result = 0.0;

  // 1.get the condition
  // whether ego is on enter expressway
  _cond_on_enter_expressway = map_info->m_active_road.m_on_motorway;

  // 4.Check Detect and report
  const VehOutput &output = helper.GetVehStateFlow().GetOutput();
  // if (output.m_duration_valid) {
  if (_cond_on_enter_expressway) {
    _result = (output.m_veh_behav == VehicleBehavior::Stopped);
  }
  // }

  _detector.Detect(_result, _threshold);
  VLOG_1 << "ego should no parking, current ego no parking status: " << _result << " \n";

  // add data to xy-pot
  if (isReportEnabled()) {
    s_no_stop_when_expressway_and_in_the_driveway_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
    s_no_stop_when_expressway_and_in_the_driveway_plot.mutable_y_axis()->at(0).add_axis_data(_result);
    s_no_stop_when_expressway_and_in_the_driveway_plot.mutable_y_axis()->at(1).add_axis_data(_cond_on_enter_expressway);
  }

  return true;
}

bool NoStopWhenExpresswayAndInTheDriveway::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(),
                                   s_no_stop_when_expressway_and_in_the_driveway_plot);
  }

  return true;
}

void NoStopWhenExpresswayAndInTheDriveway::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult NoStopWhenExpresswayAndInTheDriveway::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ego is parking");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS,
                        "ego is not parking, reversing or overtaking or driving on shoulder or cross solid line");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS,
                    "ego is not parking, reversing or overtaking or driving on shoulder or cross solid line");
}

bool NoStopWhenExpresswayAndInTheDriveway::ShouldStopScenario(std::string &reason) {
  auto ret_parking = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;

  if (ret_parking) reason = "ego is parking";

  return ret_parking;
}
}  // namespace eval
