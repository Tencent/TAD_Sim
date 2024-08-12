// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_speed_error.h"
#include "utils/proto_helper.h"

#include "trajectory.pb.h"
#include "trajectoryFollowing.pb.h"

#include <algorithm>
#include <numeric>

namespace eval {
const char EvalSpeedError::_kpi_name[] = "Control_MaxSpeedTrackError";

sim_msg::TestReport_XYPlot EvalSpeedError::s_speed_error_plot;
sim_msg::TestReport_PairData EvalSpeedError::s_average_speed_error;

EvalSpeedError::EvalSpeedError() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalSpeedError::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _speed = 0.0;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_speed_error_plot, "max speed track error", "", "t", "s", {"speed error"}, {"m/s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_speed_error_plot, "thresh upper", 0, 1, m_defaultThreshDouble, "thresh lower",
                                        1, 1, -m_defaultThreshDouble);
    ReportHelper::ConfigPairData(s_average_speed_error);
  }

  return true;
}
bool EvalSpeedError::Step(eval::EvalStep &helper) {
  double speed_error = 0.0;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      _speed = ego_front->GetSpeed().GetNormal();
      sim_msg::Trajectory traj;
      EvalMsg eval_msg = _msg_mgr->Get(topic::TRAJECTORY);

      if (eval_msg.GetPayload().size() > 0 && traj.ParseFromString(eval_msg.GetPayload())) {
        eval::Traj2ENU(traj);
        int index = eval::GetNearestByTime(traj, ego_front->GetSimTime().GetSecond());
        if (0 <= index && index <= traj.point_size()) {
          speed_error = _speed - traj.point().at(index).v();
          _detector.Detect(std::abs(speed_error), m_defaultThreshDouble);

          // add data to xy-pot
          if (isReportEnabled()) {
            s_speed_error_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
            s_speed_error_plot.mutable_y_axis()->at(0).add_axis_data(speed_error);
          }
        }
      }

      VLOG_1 << "control track speed error " << speed_error << " m/s.\n";
    } else {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalSpeedError::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();

    double average_heading_error = 0.0;
    if (s_speed_error_plot.y_axis_size() > 0)
      average_heading_error = ReportHelper::CalAverage(s_speed_error_plot.y_axis().at(0));
    ReportHelper::SetPairData(s_average_speed_error, "average track speed error",
                              std::to_string(average_heading_error));

    ReportHelper::AddXYPlot2Attach(*attach, s_speed_error_plot);
    ReportHelper::AddPair2Attach(*attach, s_average_speed_error);
  }

  return true;
}

void EvalSpeedError::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalSpeedError::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max track speed error");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max track speed error check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max track speed error check skipped");
}
bool EvalSpeedError::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max track speed error";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
