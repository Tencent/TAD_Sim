// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_react_time_of_foll_acc.h"

namespace eval {
const char EvalReactTimeOfFollAcc::_kpi_name[] = "ReactTimeOfFollAcc";

sim_msg::TestReport_XYPlot EvalReactTimeOfFollAcc::_s_react_time_acc_plot;
sim_msg::TestReport_PairData EvalReactTimeOfFollAcc::s_acc_react_time_pair;

EvalReactTimeOfFollAcc::EvalReactTimeOfFollAcc() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalReactTimeOfFollAcc::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _ACC_dis = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_DISTANCE");
    _event_happen = false;
    _react_time = 0.0;
    max_react_time = -1;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_react_time_acc_plot, "react time of acc", "", "t", "s", {"reactTime"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_react_time_acc_plot, "react upper", 0, 1, m_defaultThreshDouble, "", 1, 0,
                                        INT32_MIN, 0);
    ReportHelper::ConfigPairData(s_acc_react_time_pair);
  }

  return true;
}
bool EvalReactTimeOfFollAcc::Step(eval::EvalStep &helper) {
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
        // step 1. step 2.
        if (!_event_happen && _fell_speed > 0.56 && _fellow->GetAcc().GetX() > 0.0 && dist_relative > _ACC_dis) {
          _event_happen = true;
        }
        if (_event_happen) {
          // step 3
          if (ego_front->GetAcc().GetX() > 0.1) {
            _event_happen = false;
            _detector.Detect(_react_time, m_defaultThreshDouble);
            max_react_time = std::max(max_react_time, _react_time);
            _react_time = 0.0;
          } else {
            VLOG_1 << "front cars accelearte,current car reacts time is:" << _react_time << "\n";
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
        _s_react_time_acc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_react_time_acc_plot.mutable_y_axis()->at(0).add_axis_data(_res_time);
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
bool EvalReactTimeOfFollAcc::Stop(eval::EvalStop &helper) {
  ReportHelper::SetPairData(s_acc_react_time_pair, "max react time[s]", std::to_string(max_react_time));
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddPair2Attach(*attach, s_acc_react_time_pair);
    ReportHelper::AddXYPlot2Attach(*attach, _s_react_time_acc_plot);
  }
  return true;
}

void EvalReactTimeOfFollAcc::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalReactTimeOfFollAcc::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max react acc time when the front car accelerates");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max react time of acc check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max react time of acc check skipped");
}

bool EvalReactTimeOfFollAcc::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max react time of acc";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
