// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_wheel_angle.h"
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingWheelAngle::_kpi_name[] = "ParkingWheelAngle";

sim_msg::TestReport_XYPlot EvalParkingWheelAngle::s_parking_wheel_angle_plot;

EvalParkingWheelAngle::EvalParkingWheelAngle() {
  _max_wheel_angle = 0.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingWheelAngle::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  if (isReportEnabled()) {
    // set report info
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_parking_wheel_angle_plot, "wheel angle", "", "t", "s", {"wheel angle"}, {"rad"}, 1);

    /**
     * set Threshold after ConfigXYPlot
     @param    xy_plot         TestReport_XYPlot
     @param    upper_desc      the desc of upper threshold.
     @param    upper_space     the IntervalSpace of upper threshold.
     @param    upper_type      the IntervalType of upper threshold.
     @param    upper_value     the value of upper threshold.
     @param    lower_desc      the desc of lower threshold.
     @param    lower_space     the IntervalSpace of lower threshold.
     @param    lower_type      the IntervalType of lower threshold.
     @param    lower_value     the value of lower threshold.
     @param    y_axis_id       the id  of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(s_parking_wheel_angle_plot, "wheel angle upper", 0, 0, m_defaultThreshDouble,
                                        "", 1, 0, INT32_MIN, 0);
    /**
     * [optional] clear threshold value if threshold is not constants
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(s_parking_wheel_angle_plot, {0}, false, true);
  }
  return true;
}

bool EvalParkingWheelAngle::Step(eval::EvalStep &helper) {
  double parking_wheel_angle = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_Completed == parkMsg.stage()) {
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        // step 1
        if (ego_front) parking_wheel_angle = std::abs(ego_front->GetVehicleState().chassis_state.SteeringWheelAngle);
        VLOG_0 << "parking wheel angel: " << parking_wheel_angle << "\n";
        _max_wheel_angle = std::max(_max_wheel_angle, parking_wheel_angle);
        _detector.Detect(parking_wheel_angle, m_defaultThreshDouble);
      }
    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
    if (isReportEnabled()) {
      s_parking_wheel_angle_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_parking_wheel_angle_plot.mutable_y_axis()->at(0).add_axis_data(_max_wheel_angle);
      /**
       * [optional] set threshold on step
       */
      // s_parking_wheel_angle_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(m_defaultThreshDouble);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}

bool EvalParkingWheelAngle::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_parking_wheel_angle_plot);
  }
  return true;
}

void EvalParkingWheelAngle::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingWheelAngle::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above limit of steering wheel angle");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "steering wheel angle check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "steering wheel angle check skipped");
}

bool EvalParkingWheelAngle::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above limit of steering wheel angle";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
