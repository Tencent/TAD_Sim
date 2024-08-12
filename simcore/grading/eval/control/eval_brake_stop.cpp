// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_brake_stop.h"

namespace eval {
const char EvalBrakeStop::_kpi_name[] = "BrakeStop";

sim_msg::TestReport_XYPlot _s_brakeStop_plot;

EvalBrakeStop::EvalBrakeStop() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  event_brake_stop = false;
}
bool EvalBrakeStop::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_brakeStop_plot, "distance to obj", "", "t", "s", {"distance"}, {"m"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_brakeStop_plot, "", 0, 0, INT32_MAX, "min stop distance", 1, 0,
                                        m_defaultThreshDouble, 0);
  }

  return true;
}

bool EvalBrakeStop::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // step 1
      StaticActorList _static_actors = helper.FindFrontStaticActor(ego_front, 15);
      double _speed = ego_front->GetSpeed().GetNormal2D();
      double _relative_dis = 0;
      // if the event of braking stop do not happen,check it
      EVector3d ego_coord = ego_front->TransMiddleFront2BaseCoord();
      VLOG_1 << "static actors size: " << _static_actors.size() << "\n";
      if (_static_actors.size() > 0) {
        double min_distance = 200.0;
        for (auto _static_actor : _static_actors) {
          double _dis = CEvalMath::Sub(_static_actor->TransMiddleRear2BaseCoord(), ego_coord).GetNormal2D();
          min_distance = std::min(min_distance, _dis);
        }
        // step 2
        _relative_dis = min_distance;
        _fall_detector.Detect(_relative_dis, m_defaultThreshDouble);
      }
      // if (!event_brake_stop && _static_actors.size() >= 1 && _speed > 0.56) {
      //   double min_distance = 200.0;
      //   for (auto _static_actor : _static_actors) {
      //     double _dis = CEvalMath::Sub(_static_actor->TransMiddleRear2BaseCoord(), ego_coord).GetNormal2D();
      //     min_distance = std::min(min_distance, _dis);
      //   }
      //   _relative_dis = min_distance;
      //   if (_relative_dis > 1) event_brake_stop = true;
      // }
      // // if the event of braking stop have happened,check the distace
      // if (event_brake_stop) {
      //   _relative_dis =
      //       CEvalMath::Sub(ego_front->GetLocation().GetPosition(), _static_actors.at(0)->GetLocation().GetPosition())
      //           .GetNormal2D();
      //   if (_relative_dis < 1.0) {
      //     _fall_detector.Detect(0, 0.5);
      //     event_brake_stop = false;
      //   } else if (_speed <= 0.56) {
      //     event_brake_stop = false;
      //     _fall_detector.Detect(1, 0.5);
      //   }
      // }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_brakeStop_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_brakeStop_plot.mutable_y_axis()->at(0).add_axis_data(_relative_dis);
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
bool EvalBrakeStop::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_brakeStop_plot);
  }
  return true;
}

void EvalBrakeStop::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalBrakeStop::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "out min distance to obj,brake stop fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "brake stop successful");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "brake stop check skipped");
}

bool EvalBrakeStop::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above stop brake fail count";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
