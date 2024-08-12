// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_cross_solid_line.h"

namespace eval {
const char EvalCrossSolidLine::_kpi_name[] = "CrossSolidLine";

sim_msg::TestReport_XYPlot s_on_solidline_plot;

EvalCrossSolidLine::EvalCrossSolidLine() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _cross_solid_line = false;
}
bool EvalCrossSolidLine::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _lane_id_on_solid = 0;
    _event_solid = false;
    _c_time = 0.0;
    tip_msg = "cross solid line";
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_on_solidline_plot, "cross solid line", "", "t", "s", {"on solid line"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_on_solidline_plot, "thresh upper", 0, 1, 1);
  }
  return true;
}
bool EvalCrossSolidLine::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_on_solidline(sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  _cross_solid_line = false;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    auto ego_trailer = _actor_mgr->GetEgoTrailerActorPtr();

    if (ego_front) {
      auto lane_id = ego_front->GetLaneID();
      VLOG_1 << "ego lane roadpkid:" << lane_id.tx_road_id << ", sectionpkid:" << lane_id.tx_section_id
             << ", lane_pkid:" << lane_id.tx_lane_id << ", ego lanelink pkid:" << lane_id.tx_lanelink_id << "\n";

      eval::LaneBoundry boundry_on;
      EvalLane lane = ego_front->GetMapInfo()->m_active_lane;
      bool is_on_lane = (nullptr != lane.m_lane.get());
      EvalLaneLink lanelink = ego_front->GetMapInfo()->m_active_lanelink;
      bool is_on_lanelink = (nullptr != lanelink.m_lanelink.get());
      VLOG_1 << "is on lane: " << is_on_lane << ". is on lanelink: " << is_on_lanelink << "\n";
      if (is_on_lane && !is_on_lanelink) {  // lane and lanelink constraint
        auto ref_line_lateral_yaw = ego_front->GetRefLineLateralYaw() / const_deg_2_rad;
        VLOG_1 << "ref_line_lateral_yaw is " << ref_line_lateral_yaw << "\n";
        // The following constraints are to avoid road overlaps
        if (std::abs(ref_line_lateral_yaw) <= 30) {  // yaw constraint
          _cross_solid_line = ego_front->IsOnSolidBoundry(boundry_on);
          if (!_cross_solid_line && ego_trailer) _cross_solid_line = ego_trailer->IsOnSolidBoundry(boundry_on);
        } else {
          VLOG_1 << "ref_line_lateral_yaw is larger than 30, will not eval whether it is on solid boundry." << "\n";
        }
      } else {
        VLOG_1 << "ego not on lane or ego on lanelink, will not eval whether it is on solid boundry." << "\n";
      }

      int cur_lane_id = lane_id.tx_lane_id;
      if (!_event_solid) {
        _event_solid = _cross_solid_line;
        _lane_id_on_solid = _event_solid ? cur_lane_id : _lane_id_on_solid;
      }

      if (_event_solid) {
        _c_time += getModuleStepTime();
        if (_c_time < 10.0) {
          if (_lane_id_on_solid != 0 && cur_lane_id != 0 && _lane_id_on_solid != cur_lane_id) {
            tip_msg = "Illegal lane change on solid line";
            _event_solid = false;
            _c_time = 0.0;
          }
        } else {
          _event_solid = false;
          _c_time = 0.0;
        }
      }

      if (_detector.Detect(_cross_solid_line, 1)) {
        grading.mutable_event_detector()->set_on_solidline(sim_msg::Grading_EventDetector_EventState_EventDetected);
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        s_on_solidline_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_on_solidline_plot.mutable_y_axis()->at(0).add_axis_data(_cross_solid_line);
      }

      // log
      if (_cross_solid_line)
        VLOG_1 << "ego on solid line, boundry pkid:" << boundry_on.boundry_pkid
               << ", lane mark type:" << boundry_on.lane_mark_type << "\n";
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalCrossSolidLine::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("OnSolidLine", std::to_string(_detector.GetCount()));

  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_on_solidline_plot);
  }

  return true;
}

EvalResult EvalCrossSolidLine::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, tip_msg);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "cross solid line check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "cross solid line check skipped");
}
bool EvalCrossSolidLine::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "cross solid line";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalCrossSolidLine::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_lane()->set_isonsolidline(_cross_solid_line);
  msg.mutable_lane()->set_isondottedline(false);
  // msg.mutable_lane()->set_isonstopline(false);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalCrossSolidLine::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_crosssolidline()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_crosssolidline()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_crosssolidline()->set_is_pass(IsEvalPass()._state ==
                                                                    sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
