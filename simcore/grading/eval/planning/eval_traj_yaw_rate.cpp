// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_traj_yaw_rate.h"
#include "trajectory.pb.h"

namespace eval {
const char EvalTrajYawRate::_kpi_name[] = "Traj_MaxYawRate";

sim_msg::TestReport_XYPlot EvalTrajYawRate::s_traj_yawrate_plot;

EvalTrajYawRate::EvalTrajYawRate() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalTrajYawRate::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_traj_yawrate_plot, "traj max yaw_rate", "", "t", "s", {"traj yaw_rate"}, {"rad/s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_traj_yawrate_plot, "thresh upper", 0, 1, m_defaultThreshDouble,
                                        "thresh lower", 1, 1, -m_defaultThreshDouble);
  }

  return true;
}
bool EvalTrajYawRate::Step(eval::EvalStep &helper) {
  double max_yawrate = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // find max yaw rate from trajectory
    auto findMaxYawRate = [](const sim_msg::Trajectory &traj, double &max_yaw_rate) {
      double cur_max_yawrate = 0.0;

      for (auto i = 1; i < traj.point_size(); ++i) {
        double &&yaw_pre = traj.point().at(i - 1).theta();
        double &&t_pre = traj.point().at(i - 1).t();
        double &&yaw = traj.point().at(i).theta();
        double &&t = traj.point().at(i).t();

        double &&delta_t = t - t_pre;
        if (std::abs(delta_t) > 1e-6) {
          double yaw_rate = std::abs(yaw - yaw_pre) / delta_t;
          cur_max_yawrate = yaw_rate >= cur_max_yawrate ? yaw_rate : cur_max_yawrate;
        }
      }
      max_yaw_rate = std::max(cur_max_yawrate, 0.0);
    };

    sim_msg::Trajectory traj;
    EvalMsg &&msg = _msg_mgr->Get(topic::TRAJECTORY);

    if (msg.GetPayload().size() > 0 && traj.ParseFromString(msg.GetPayload())) {
      findMaxYawRate(traj, max_yawrate);
      _detector.Detect(std::abs(max_yawrate), m_defaultThreshDouble);

      // add data to plot
      if (isReportEnabled()) {
        s_traj_yawrate_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_traj_yawrate_plot.mutable_y_axis()->at(0).add_axis_data(max_yawrate);
      }
    } else {
      LOG_ERROR << "fail to parse trajectory msg, trajectory payload size:" << msg.GetPayload().size() << "\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalTrajYawRate::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_traj_yawrate_plot);
  }

  return true;
}

void EvalTrajYawRate::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalTrajYawRate::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max trajectory yaw rate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory yaw rate check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory yaw rate check skipped");
}
bool EvalTrajYawRate::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max trajectory yaw rate";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
