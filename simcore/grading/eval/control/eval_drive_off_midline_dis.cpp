// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_drive_off_midline_dis.h"

namespace eval {
const char EvalDriveOffMidlineDis::_kpi_name[] = "DriveOffMidlineDis";

sim_msg::TestReport_XYPlot _s_driveoffline_plot;

EvalDriveOffMidlineDis::EvalDriveOffMidlineDis() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalDriveOffMidlineDis::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    offsetDis = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "OffsetCenterDis");
    _event_away_line = false;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_driveoffline_plot, "run_dist of deviation", "distance of running and lateral", "t",
                               "s", {"run_distance", "later_distance"}, {"m", "m"}, 2);
    /**
     * set Threshold after ConfigXYPlot
     * @param    xy_plot         TestReport_XYPlot
     * @param    upper_desc      the desc of upper threshold.
     * @param    upper_space     the IntervalSpace of upper threshold.
     * @param    upper_type      the IntervalType of upper threshold.
     * @param    upper_value     the value of upper threshold.
     * @param    lower_desc      the desc of lower threshold.
     * @param    lower_space     the IntervalSpace of lower threshold.
     * @param    lower_type      the IntervalType of lower threshold.
     * @param    lower_value     the value of lower threshold.
     * @param    y_axis_id       the id  of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(_s_driveoffline_plot, "run_dist upper", 0, 0, m_defaultThreshDouble, "", 1, 0,
                                        INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_driveoffline_plot, "lateral_dist upper", 0, 1, offsetDis, "", 1, 0,
                                        INT32_MIN, 1);
    /**
     * [optional] clear threshold value if threshold is not constants
     * @param xy_plot         TestReport_XYPlot
     * @param y_axis_ids      y_axis id list to clear
     * @param upper_clear     whether clear upper threshold
     * @param lower_clear     whether clear lower threshold
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(_s_driveoffline_plot, {0}, true, false);
  }

  return true;
}
bool EvalDriveOffMidlineDis::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get lane center offset
      double _off_distance = std::abs(ego_front->GetRefLineLateralOffset());
      double moved_dis = 0.0;
      VLOG_1 << "the run distance limit is " << m_defaultThreshDouble << " m." << " the away center line limit is "
             << offsetDis << " m.\n";
      if (!_event_away_line && _off_distance > offsetDis) {
        _event_away_line = true;
        start_dis = helper.GetGradingMsg().mileage() * 1000;
      } else if (_event_away_line) {
        moved_dis = helper.GetGradingMsg().mileage() * 1000 - start_dis;
        VLOG_1 << "car has move when away the center line:" << moved_dis
               << " the cur away center line is : " << _off_distance << "\n";
        if (_off_distance <= offsetDis) {
          start_dis = 0.0;
          _event_away_line = false;
          moved_dis = 0.0;
          VLOG_1 << "away the center line finish" << "\n";
        }
        _detector.Detect(moved_dis, m_defaultThreshDouble);
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_driveoffline_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_driveoffline_plot.mutable_y_axis()->at(0).add_axis_data(moved_dis);
        _s_driveoffline_plot.mutable_y_axis()->at(1).add_axis_data(_off_distance);
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
bool EvalDriveOffMidlineDis::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_driveoffline_plot);
  }
  return true;
}

void EvalDriveOffMidlineDis::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalDriveOffMidlineDis::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max runing distance out offset");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max runing distance out offset check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "max runing distance out offset check skipped");
}

bool EvalDriveOffMidlineDis::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max runing distance out offset";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
