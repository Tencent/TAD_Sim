// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_stop_time_statistics.h"

namespace eval {
const char EvalStopTimeStatistics::_kpi_name[] = "StopTimeStatistics";

sim_msg::TestReport_XYPlot EvalStopTimeStatistics::_s_stop_time_plot;
sim_msg::TestReport_XlsxSheet EvalStopTimeStatistics::_g_stop_time_sheet;
sim_msg::TestReport_XlsxSheet_SheetData EvalStopTimeStatistics::sheet_col_id;
sim_msg::TestReport_XlsxSheet_SheetData EvalStopTimeStatistics::sheet_col_start_time;
sim_msg::TestReport_XlsxSheet_SheetData EvalStopTimeStatistics::sheet_col_stop_duration;

EvalStopTimeStatistics::EvalStopTimeStatistics() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _event_happen = false;
  _stop_time = 0.0;
  stop_time_no = 0;
}
bool EvalStopTimeStatistics::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::ConfigXLSXSheet(_g_stop_time_sheet, "stopped time", "each stopped time");
    ReportHelper::ConfigSheetData(sheet_col_id, "No.");
    ReportHelper::ConfigSheetData(sheet_col_start_time, "start_time [s]");
    ReportHelper::ConfigSheetData(sheet_col_stop_duration, "duration [s]");
    ReportHelper::AddSheetData2XLSX(_g_stop_time_sheet, sheet_col_id);
    ReportHelper::AddSheetData2XLSX(_g_stop_time_sheet, sheet_col_start_time);
    ReportHelper::AddSheetData2XLSX(_g_stop_time_sheet, sheet_col_stop_duration);
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_stop_time_plot, "stopTime", "time of stopped", "t", "s", {"time"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_stop_time_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}
bool EvalStopTimeStatistics::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      double _res_time = _stop_time;
      double _speed = std::abs(ego_front->GetSpeed().GetNormal2D());
      // step 1
      if (!_event_happen && _speed <= 0.56) {
        _event_happen = true;
      } else if (_event_happen) {
        _stop_time += getModuleStepTime();
        // step 2
        if (_speed > 0.56) {
          _g_stop_time_sheet.mutable_sheet_data()->at(0).add_data(std::to_string(++stop_time_no));
          _g_stop_time_sheet.mutable_sheet_data()->at(1).add_data(std::to_string(helper.GetSimTime() - _stop_time));
          _g_stop_time_sheet.mutable_sheet_data()->at(2).add_data(std::to_string(_stop_time));
          _event_happen = false;
          _stop_time = 0.0;
        }
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_stop_time_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_stop_time_plot.mutable_y_axis()->at(0).add_axis_data(_res_time);
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
bool EvalStopTimeStatistics::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_stop_time_plot);
    ReportHelper::AddXLSXSheet2Attach(*attach, _g_stop_time_sheet);
  }
  return true;
}

void EvalStopTimeStatistics::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalStopTimeStatistics::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max stopped time");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max stopped time check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max stopped time check skipped");
}

bool EvalStopTimeStatistics::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max stopped time";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
