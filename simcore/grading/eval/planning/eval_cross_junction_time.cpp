// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_cross_junction_time.h"

namespace eval {
const char EvalCrossJunctionTime::_kpi_name[] = "CrossJunctionTime";

sim_msg::TestReport_XYPlot EvalCrossJunctionTime::_s_cro_ist_time_plot;

sim_msg::TestReport_XlsxSheet EvalCrossJunctionTime::_g_cross_intersection_sheet;
sim_msg::TestReport_XlsxSheet_SheetData EvalCrossJunctionTime::sheet_col_id;
sim_msg::TestReport_XlsxSheet_SheetData EvalCrossJunctionTime::sheet_col_start_time;
sim_msg::TestReport_XlsxSheet_SheetData EvalCrossJunctionTime::sheet_col_cross_intersection_duration;

EvalCrossJunctionTime::EvalCrossJunctionTime() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalCrossJunctionTime::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    cross_intersection_no = 0;
    _ctime = 0.0;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::ConfigXLSXSheet(_g_cross_intersection_sheet, "cross intersection msg",
                                  "each cross intersection duration");
    ReportHelper::ConfigSheetData(sheet_col_id, "No.");
    ReportHelper::ConfigSheetData(sheet_col_start_time, "start_time [s]");
    ReportHelper::ConfigSheetData(sheet_col_cross_intersection_duration, "duration [s]");
    ReportHelper::AddSheetData2XLSX(_g_cross_intersection_sheet, sheet_col_id);
    ReportHelper::AddSheetData2XLSX(_g_cross_intersection_sheet, sheet_col_start_time);
    ReportHelper::AddSheetData2XLSX(_g_cross_intersection_sheet, sheet_col_cross_intersection_duration);

    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_cro_ist_time_plot, "time of crossing intersection", "", "t", "s", {"time"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_cro_ist_time_plot, "", 0, 0, INT32_MAX, "", 1, 0, INT32_MIN, 0);
  }

  return true;
}
bool EvalCrossJunctionTime::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing.\n";
        return false;
      }
      double _res_time = _ctime;
      // step 1
      if (map_info->m_in_junction && !map_info->m_on_road) {
        _ctime += getModuleStepTime();
      } else {
        if (_ctime > 0.001) {
          _g_cross_intersection_sheet.mutable_sheet_data()->at(0).add_data(std::to_string(++cross_intersection_no));
          _g_cross_intersection_sheet.mutable_sheet_data()->at(1).add_data(
              std::to_string(helper.GetSimTime() - _ctime));
          _g_cross_intersection_sheet.mutable_sheet_data()->at(2).add_data(std::to_string(_ctime));
          VLOG_1 << "add intersection data success.\n";
        }
        _ctime = 0.0;
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_cro_ist_time_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_cro_ist_time_plot.mutable_y_axis()->at(0).add_axis_data(_res_time);
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
bool EvalCrossJunctionTime::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_cro_ist_time_plot);
    ReportHelper::AddXLSXSheet2Attach(*attach, _g_cross_intersection_sheet);
  }
  return true;
}

void EvalCrossJunctionTime::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalCrossJunctionTime::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "intersection time");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "intersection time check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "intersection time check skipped");
}

bool EvalCrossJunctionTime::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above intersection time";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
