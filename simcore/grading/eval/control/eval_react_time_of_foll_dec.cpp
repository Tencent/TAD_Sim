// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_react_time_of_foll_dec.h"

namespace eval {
const char EvalReactTimeOfFollDec::_kpi_name[] = "ReactTimeOfFollDec";

sim_msg::TestReport_XYPlot EvalReactTimeOfFollDec::_s_react_time_dec_plot;
sim_msg::TestReport_PairData EvalReactTimeOfFollDec::s_dec_react_time_pair;
// sim_msg::TestReport_XlsxSheet EvalReactTimeOfFollDec::_g_dec_react_sheet;
// sim_msg::TestReport_XlsxSheet_SheetData EvalReactTimeOfFollDec::sheet_col_id;
// sim_msg::TestReport_XlsxSheet_SheetData EvalReactTimeOfFollDec::sheet_col_start_time;
// sim_msg::TestReport_XlsxSheet_SheetData EvalReactTimeOfFollDec::sheet_col_dec_react_duration;

EvalReactTimeOfFollDec::EvalReactTimeOfFollDec() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalReactTimeOfFollDec::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _ACC_dis = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "DEC_DISTANCE");
    _event_happen = false;
    _react_time = 0.0;
    max_react_time = -1;
    // react_no = 0;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    // ReportHelper::ConfigXLSXSheet(_g_dec_react_sheet, "cross intersection msg", "each cross intersection
    // duration"); ReportHelper::ConfigSheetData(sheet_col_id, "No.");
    // ReportHelper::ConfigSheetData(sheet_col_start_time, "start_time [s]");
    // ReportHelper::ConfigSheetData(sheet_col_dec_react_duration, "duration [s]");
    // ReportHelper::AddSheetData2XLSX(_g_dec_react_sheet, sheet_col_id);
    // ReportHelper::AddSheetData2XLSX(_g_dec_react_sheet, sheet_col_start_time);
    // ReportHelper::AddSheetData2XLSX(_g_dec_react_sheet, sheet_col_dec_react_duration);
    ReportHelper::ConfigPairData(s_dec_react_time_pair);
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_react_time_dec_plot, "react time of dec", "", "t", "s", {"reactTime"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_react_time_dec_plot, "react upper", 0, 1, m_defaultThreshDouble, "", 1, 0,
                                        INT32_MIN, 0);
  }

  return true;
}
bool EvalReactTimeOfFollDec::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      double _res_time = _react_time;
      auto _fellow = helper.GetLeadingVehicle();
      if (_fellow) {
        EVector3d ego_2_fellow =
            CEvalMath::Sub(_fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        double dist_relative = ego_2_fellow.GetNormal2D();
        double _fell_speed = _fellow->GetSpeed().GetNormal2D();
        VLOG_1 << "the distance between the two cars is: " << dist_relative << "\n";
        VLOG_1 << "the front speed is: " << _fell_speed << "\n";
        // step 1. step 2.
        if (!_event_happen && _fell_speed > 0.56 && _fellow->GetAcc().GetX() < 0.0 && dist_relative < _ACC_dis) {
          VLOG_1 << "detect the follow dec event.\n";
          _event_happen = true;
        }
        if (_event_happen) {
          // step 3
          if (ego_front->GetAcc().GetX() < 0.0) {
            VLOG_1 << "react dec successful.\n";
            _event_happen = false;
            _detector.Detect(_react_time, m_defaultThreshDouble);
            // _g_dec_react_sheet.mutable_sheet_data()->at(0).add_data(std::to_string(++react_no));
            // _g_dec_react_sheet.mutable_sheet_data()->at(1).add_data(
            //     std::to_string(helper.GetSimTime() - _react_time));
            // _g_dec_react_sheet.mutable_sheet_data()->at(2).add_data(std::to_string(_react_time));
            max_react_time = std::max(max_react_time, _react_time);
            _react_time = 0.0;
          } else {
            VLOG_1 << "front cars decelarate,current car reacts time is:" << _react_time << "\n";
            _react_time += getModuleStepTime();
            _detector.Detect(_react_time, m_defaultThreshDouble);
          }
        }
      } else {
        _event_happen = false;
        _react_time = 0;
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_react_time_dec_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_react_time_dec_plot.mutable_y_axis()->at(0).add_axis_data(_res_time);
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
bool EvalReactTimeOfFollDec::Stop(eval::EvalStop &helper) {
  ReportHelper::SetPairData(s_dec_react_time_pair, "max react time[s]", std::to_string(max_react_time));
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_react_time_dec_plot);
    ReportHelper::AddPair2Attach(*attach, s_dec_react_time_pair);
    // ReportHelper::AddXLSXSheet2Attach(*attach, _g_dec_react_sheet);
  }
  return true;
}

void EvalReactTimeOfFollDec::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalReactTimeOfFollDec::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max react acc time when the front car decelerates");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max react time of dec check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max react time of dec check skipped");
}

bool EvalReactTimeOfFollDec::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max react time of dec";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
