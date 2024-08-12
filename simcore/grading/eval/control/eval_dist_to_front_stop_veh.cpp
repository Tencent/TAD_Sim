// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_dist_to_front_stop_veh.h"

namespace eval {
const char EvalDistToFrontStopVeh::_kpi_name[] = "DistToFrontStopVeh";

sim_msg::TestReport_XYPlot EvalDistToFrontStopVeh::_s_dis_to_front_plot;

EvalDistToFrontStopVeh::EvalDistToFrontStopVeh() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalDistToFrontStopVeh::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _max_stop_dis = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MAX_STOP_DISTANCE");
    _min_stop_dis = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MIN_STOP_DISTANCE");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_dis_to_front_plot, "distanceToCar", "when stopping,the distance to the front car",
                               "t", "s", {"distance"}, {"m"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_dis_to_front_plot, "stop distance upper", 0, 1, _max_stop_dis,
                                        "stop distance lower", 1, 1, _min_stop_dis, 0);
  }

  return true;
}
bool EvalDistToFrontStopVeh::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      auto _fellow = helper.GetLeadingVehicle();
      double _res_distance = 0.0;
      if (_fellow) {
        if (ego_front->GetSpeed().GetNormal2D() <= 0.1 && _fellow->GetSpeed().GetNormal2D() <= 0.1) {
          VLOG_1 << "the current car and the front car are stopped,starting check the distance between cars" << "\n";
          EVector3d ego_2_fellow =
              CEvalMath::Sub(_fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
          _res_distance = ego_2_fellow.GetNormal();
          VLOG_1 << "the distance between cars when stopped is: " << _res_distance << "\n";
          VLOG_1 << "the front car speed: " << _fellow->GetSpeed().GetNormal2D() << "\n";
          VLOG_1 << "the ego car speed: " << ego_front->GetSpeed().GetNormal2D() << "\n";
          _detector.Detect(_res_distance, _max_stop_dis);
          _fall_detector.Detect(_res_distance, _min_stop_dis);
        }
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_dis_to_front_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_dis_to_front_plot.mutable_y_axis()->at(0).add_axis_data(_res_distance);
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

bool EvalDistToFrontStopVeh::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_dis_to_front_plot);
  }
  return true;
}

void EvalDistToFrontStopVeh::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalDistToFrontStopVeh::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount() + _fall_detector.GetCount());

    if (_detector.GetCount() + _fall_detector.GetCount() >= m_Kpi.passcondition().value() &&
        m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max distance");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max distance check skipped");
}

bool EvalDistToFrontStopVeh::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() + _fall_detector.GetCount() >= m_Kpi.finishcondition().value() &&
             m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max distance";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
