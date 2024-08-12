// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_react_opposite_veh.h"

namespace eval {

const char EvalReactOppositeVeh::_kpi_name[] = "ReactOppositeVeh";
sim_msg::TestReport_XYPlot EvalReactOppositeVeh::_s_occroad_plot;

EvalReactOppositeVeh::EvalReactOppositeVeh() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  isHasCar = false;
  isRightOp = false;
  event_happen = false;
}
bool EvalReactOppositeVeh::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_occroad_plot, "slow and avoid check", "", "t", "s", {"detected", "speed", "steer"},
                               {"N/A", "m/s", "N/A"}, 3);
    ReportHelper::ConfigXYPlotThreshold(_s_occroad_plot, "", 0, 1, INT32_MAX, "", 1, 1, INT32_MIN, 0);
  }

  return true;
}

bool EvalReactOppositeVeh::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // road occupy
      double _ego_speed = ego_front->GetSpeed().GetNormal2D();
      double yaw = std::abs(ego_front->GetRefLineLateralYaw());
      VLOG_1 << "lateral yaw :" << yaw << "\n";
      auto fellow = helper.GetLeadingVehicle();
      if (fellow) {
        double dist_relative =
            CEvalMath::Sub(ego_front->TransMiddleFront2BaseCoord(), fellow->TransMiddleFront2BaseCoord()).GetNormal2D();
        double angle =
            std::abs(CEvalMath::YawDiff(ego_front->GetLocation().GetEuler(), fellow->GetLocation().GetEuler()));
        VLOG_1 << "the angel yaw is : " << angle << "\n";
        if (!event_happen && angle > 2.879 && angle < 3.142 && dist_relative > 20 && dist_relative <= 50) {
          event_happen = true;
          VLOG_1 << "has checking the front car is occupy the lane.\n";
          isRightOp = false;
        }
        VLOG_1 << "dist of cars is: " << dist_relative << "\n";
        if (event_happen) {
          if (dist_relative < 20) {
            _detector.Detect(!isRightOp, 0.5);
            event_happen = false;
            isRightOp = false;
          } else if (!isRightOp && ego_front->GetAcc().GetX() < 0 && yaw >= 0.15) {
            isRightOp = true;
          }
        }
      } else if (event_happen) {
        VLOG_1 << "fellow car disappeared.\n";
        event_happen = false;
        isRightOp = false;
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_occroad_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_occroad_plot.mutable_y_axis()->at(0).add_axis_data(event_happen);
        _s_occroad_plot.mutable_y_axis()->at(1).add_axis_data(_ego_speed);
        _s_occroad_plot.mutable_y_axis()->at(2).add_axis_data(yaw >= 0.15);
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
bool EvalReactOppositeVeh::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_occroad_plot);
  }
  return true;
}

void EvalReactOppositeVeh::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalReactOppositeVeh::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above Slow and avoid count");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max Slow and avoid count check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max Slow and avoid count check skipped");
}

bool EvalReactOppositeVeh::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above Slow down and avoid count";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
