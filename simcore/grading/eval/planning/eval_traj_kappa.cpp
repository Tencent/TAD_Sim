// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_traj_kappa.h"
#include "trajectory.pb.h"

namespace eval {
const char EvalTrajKappa::_kpi_name[] = "Traj_MaxKappa";

sim_msg::TestReport_XYPlot EvalTrajKappa::s_traj_kappa_plot;

EvalTrajKappa::EvalTrajKappa() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalTrajKappa::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_traj_kappa_plot, "trajectory max kappa", "", "t", "s", {"traj kappa"}, {"rad/m"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_traj_kappa_plot, "thresh upper", 0, 1, m_defaultThreshDouble, "thresh lower",
                                        1, 1, -m_defaultThreshDouble);
  }

  return true;
}
bool EvalTrajKappa::Step(eval::EvalStep &helper) {
  double max_kappa = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // find max kappa from trajectory
    auto findMaxKappa = [](const sim_msg::Trajectory &traj, double &max_kappa) {
      double cur_max_kappa = 0.0;

      for (auto i = 0; i < traj.point_size(); ++i) {
        double kappa = std::abs(traj.point().at(i).kappa());
        cur_max_kappa = kappa >= cur_max_kappa ? kappa : cur_max_kappa;
      }
      max_kappa = std::max(cur_max_kappa, 0.0);
    };

    sim_msg::Trajectory traj;
    EvalMsg &&msg = _msg_mgr->Get(topic::TRAJECTORY);

    if (msg.GetPayload().size() > 0 && traj.ParseFromString(msg.GetPayload())) {
      findMaxKappa(traj, max_kappa);
      _detector.Detect(max_kappa, m_defaultThreshDouble);

      // add data to plot
      if (isReportEnabled()) {
        s_traj_kappa_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_traj_kappa_plot.mutable_y_axis()->at(0).add_axis_data(max_kappa);
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
bool EvalTrajKappa::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_traj_kappa_plot);
  }

  return true;
}

void EvalTrajKappa::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalTrajKappa::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max trajectory kappa");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory kappa check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory kappa check skipped");
}
bool EvalTrajKappa::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max trajectory kappa";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
