// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_drving_efficiency.h"

namespace eval {
const char EvalDrvingEfficiency::_kpi_name[] = "DrvingEfficiency";

sim_msg::TestReport_XYPlot _s_travel_efficiency_plot;

EvalDrvingEfficiency::EvalDrvingEfficiency() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  i_traffic_sum = 0.0;
  frames_sum = 0;
}
bool EvalDrvingEfficiency::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_travel_efficiency_plot, "travel efficiency", "", "t", "s", {"I_traffic"}, {"N/A"}, 1);
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
  }

  return true;
}

bool EvalDrvingEfficiency::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing" << "\n";
        return false;
      }
      double speed_limit_from_map = 1;
      if (map_info) {
        auto ego_lane = map_info->m_active_lane.m_lane;
        map_info->m_active_lane.m_right_lane;
        if (ego_lane) {
          speed_limit_from_map = ego_lane->getSpeedLimit() / 3.6;
          speed_limit_from_map = speed_limit_from_map > 1.0 ? speed_limit_from_map : 1.0;
        }
      }
      double ego_speed = ego_front->GetSpeed().GetNormal2D();
      eval::VehilceActorList veh_actors = helper.FindSurroundingVehicleActor(ego_front, 200);
      double avg_traffic = speed_limit_from_map;
      if (veh_actors.size() == 0) {
        avg_traffic = speed_limit_from_map;
      } else {
        double _speed_veh_sum = 0.0;
        for (CVehicleActorPtr actor : veh_actors) {
          _speed_veh_sum += actor->GetSpeed().GetNormal2D();
        }
        avg_traffic = _speed_veh_sum / veh_actors.size();
      }
      avg_traffic = avg_traffic > 0.5 ? avg_traffic : 0.5;
      frames_sum++;
      i_traffic_sum += ego_speed / avg_traffic;
      double res = i_traffic_sum / frames_sum;
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_travel_efficiency_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_travel_efficiency_plot.mutable_y_axis()->at(0).add_axis_data(res);
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
bool EvalDrvingEfficiency::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_travel_efficiency_plot);
  }
  return true;
}

void EvalDrvingEfficiency::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalDrvingEfficiency::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above travel efficiency");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "travel efficiency check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "travel efficiency check skipped");
}

bool EvalDrvingEfficiency::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above travel efficiency";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
