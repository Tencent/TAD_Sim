// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_overtake_vio.h"

namespace eval {
const char EvalOvertakeVio::_kpi_name[] = "OvertakeVio";

sim_msg::TestReport_XYPlot EvalOvertakeVio::_s_takevio_plot;

EvalOvertakeVio::EvalOvertakeVio() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalOvertakeVio::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _event_overtake = false;
    _ctime = 0.0;
    from_left = false;
    veh_ptr = nullptr;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_takevio_plot, "overtake violations", " if overtakes violation", "t", "s",
                               {"if overtake"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_takevio_plot, "", 0, 1, 1, "", 1, 0, INT32_MIN, 0);
  }
  return true;
}
bool EvalOvertakeVio::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      auto fellow = helper.GetLeadingVehicle();
      const VehOutput &out = helper.GetVehStateFlow().GetOutput();
      bool overtake = false;
      if (!_event_overtake && fellow) {
        veh_ptr = fellow;
        EVector3d ego_2_fellow =
            CEvalMath::Sub(fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        double dist_relative = ego_2_fellow.GetNormal2D();
        if (!_event_overtake) {
          _event_overtake = dist_relative <= 20.0 && out.m_veh_behav == VehicleBehavior::LaneChaning_Right;
          if (!_event_overtake) {
            eval::LaneBoundry boundry_on;
            bool tmp_on_line = ego_front->IsOnSolidBoundry(boundry_on) || ego_front->IsOnSolidSolidBoundry(boundry_on);
            VLOG_1 << "cross on solid line.\n";
            _event_overtake = dist_relative <= 20.0 && tmp_on_line;
            from_left = _event_overtake;
          }
          if (_event_overtake) {
            veh_ptr = fellow;
          }
          VLOG_1 << "maybe overtake: " << _event_overtake << "\n";
        }
      }
      if (_event_overtake) {
        bool _behind_ego = ego_front->TheObstacleIsBehindVehicle(veh_ptr->GetLocation());
        bool _lanes_valid = ego_front->GetLaneID().IsLaneValid() && veh_ptr->GetLaneID().IsLaneValid();
        bool _equal_lane = _lanes_valid && ego_front->GetLaneID().Equal(veh_ptr->GetLaneID());
        _ctime += getModuleStepTime();
        if (_ctime > 15) {
          _ctime = 0.0;
          _event_overtake = false;
        } else if (_behind_ego && _equal_lane && !from_left) {
          _ctime = 0.0;
          _event_overtake = false;
          overtake = true;
        } else if (_behind_ego && _equal_lane && from_left) {
          _ctime = 0.0;
          _event_overtake = false;
          overtake = true;
          from_left = false;
        }
      }
      _detector.Detect(overtake, 0.5);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_takevio_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_takevio_plot.mutable_y_axis()->at(0).add_axis_data(overtake);
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
bool EvalOvertakeVio::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_takevio_plot);
  }
  return true;
}

void EvalOvertakeVio::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalOvertakeVio::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "overtaking violations happens");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "overtaking violations check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "overtaking violations check skipped");
}

bool EvalOvertakeVio::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "overtaking violations happens";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
