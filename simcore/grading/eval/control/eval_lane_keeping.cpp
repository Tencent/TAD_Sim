// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_lane_keeping.h"
#include <cmath>

namespace eval {
const double PI = 3.141592653;
const char EvalLaneKeeping::_kpi_name[] = "LaneKeeping";

sim_msg::TestReport_XYPlot EvalLaneKeeping::_s_lane_keeping_plot;

EvalLaneKeeping::EvalLaneKeeping() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalLaneKeeping::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _dist_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "DistanceThreshold");
    _dir_thresh_value = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "AngleThreshold");
    // output indicator configuration details
    DebugShowKpi();
  }

  _record_over_dist = false;
  _record_over_dir = false;
  _record_duration = 0.0;
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_lane_keeping_plot, "lane keeping", "", "t", "s", {"dist offset", "dir offset"},
                               {"m", "rad"}, 2);
    //  {"dist offset", "dir offset", "lane change"}, {"m", "rad", "N/A"}, 3);
    ReportHelper::ConfigXYPlotThreshold(_s_lane_keeping_plot, "distance upper", 0, 0, _dist_thresh_value,
                                        "distance lower", 1, 0, -_dist_thresh_value, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_lane_keeping_plot, "angle upper", 0, 0, _dir_thresh_value, "angle lower", 1,
                                        0, -_dir_thresh_value, 1);
  }
  return true;
}

bool EvalLaneKeeping::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // calculate exp data
    double cur_dist = 0.0;
    double cur_dir = 0.0;
    bool cur_lane_change = false;
    // step 1. 2.
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // check whether record offset result
      // output of state-flow
      const VehOutput &output = helper.GetVehStateFlow().GetOutput();
      cur_lane_change = (output.m_veh_behav == VehicleBehavior::LaneChaning_Left ||
                         output.m_veh_behav == VehicleBehavior::LaneChaning_Right) &&
                        output.m_duration_valid;
      // step 3. 4. get lateral distance and angel
      cur_dist = ego_front->GetRefLineLateralOffset();
      cur_dir = ego_front->GetRefLineLateralYaw();
      if (cur_lane_change || (!_record_over_dist && !_record_over_dir)) {
        _record_over_dir = false;
        _record_over_dist = false;
        _record_duration = 0.0;
      }
      // step 5. 6.
      if (_record_duration > 10) {
        _record_over_dist = (std::fabs(cur_dist) > _dist_thresh_value);
        _record_over_dir = (std::fabs(cur_dir) > _dir_thresh_value);
      } else {
        // step 7
        _record_over_dist = (_record_over_dist || (std::fabs(cur_dist) > _dist_thresh_value));
        _record_over_dir = (_record_over_dir || (std::fabs(cur_dir) > _dir_thresh_value));
      }
      if (_record_over_dist || _record_over_dir) {
        _record_duration += getModuleStepTime();
      }
      // step 8
      _detector_dist.Detect(_record_over_dist && _record_duration > 10, 0.5);
      _detector_dir.Detect(_record_over_dir && _record_duration > 10, 0.5);

    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      _s_lane_keeping_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      _s_lane_keeping_plot.mutable_y_axis()->at(0).add_axis_data(cur_dist);
      _s_lane_keeping_plot.mutable_y_axis()->at(1).add_axis_data(cur_dir);
      // _s_lane_keeping_plot.mutable_y_axis()->at(2).add_axis_data(cur_lane_change);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalLaneKeeping::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_lane_keeping_plot);
  }
  return true;
}

void EvalLaneKeeping::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_dist, _kpi_name);
}

EvalResult EvalLaneKeeping::IsEvalPass() {
  if (m_KpiEnabled) {
    int detect_count = _detector_dist.GetCount() + _detector_dir.GetCount();
    _case.mutable_info()->set_detected_count(detect_count);
    if (detect_count >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "driving out of the lane");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "lane keeping check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "lane keeping check skipped");
}

bool EvalLaneKeeping::ShouldStopScenario(std::string &reason) {
  int detect_count = _detector_dist.GetCount() + _detector_dir.GetCount();
  auto ret = detect_count >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "driving out of the lane";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
