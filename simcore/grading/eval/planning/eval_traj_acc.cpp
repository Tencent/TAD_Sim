// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_traj_acc.h"
#include "trajectory.pb.h"

namespace eval {
const char EvalTrajAcc::_kpi_name[] = "Traj_MaxAcc";

sim_msg::TestReport_XYPlot EvalTrajAcc::m_traj_acc_plot;

EvalTrajAcc::EvalTrajAcc() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalTrajAcc::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(m_traj_acc_plot, "traj max acc", "", "t", "s", {"traj acc"}, {"m/s2"}, 1);
    ReportHelper::ConfigXYPlotThreshold(m_traj_acc_plot, "thresh upper", 0, 1, m_defaultThreshDouble);
  }

  return true;
}
bool EvalTrajAcc::Step(eval::EvalStep &helper) {
  double max_acc = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // find max acc from trajectory
    auto findMaxAcc = [](const sim_msg::Trajectory &traj, double &max_acc) {
      double cur_max_acc = 0.0;

      for (auto i = 0; i < traj.point_size(); ++i) {
        double &&acc = traj.point().at(i).a();
        cur_max_acc = acc >= cur_max_acc ? acc : cur_max_acc;
      }
      max_acc = std::max(cur_max_acc, 0.0);
    };

    sim_msg::Trajectory traj;
    EvalMsg &&msg = _msg_mgr->Get(topic::TRAJECTORY);

    if (msg.GetPayload().size() > 0 && traj.ParseFromString(msg.GetPayload())) {
      VLOG_2 << "trajectory points size:" << traj.point_size() << "\n";
      findMaxAcc(traj, max_acc);
      _detector.Detect(max_acc, m_defaultThreshDouble);

      // add data to plot
      if (isReportEnabled()) {
        m_traj_acc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        m_traj_acc_plot.mutable_y_axis()->at(0).add_axis_data(max_acc);
      }
    } else {
      VLOG_2 << "fail to parse trajectory msg, payload size:" << msg.GetPayload().size() << "\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalTrajAcc::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), m_traj_acc_plot);
  }

  return true;
}

void EvalTrajAcc::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalTrajAcc::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max trajectory acceleration");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory acceleration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory acceleration check skipped");
}
bool EvalTrajAcc::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max trajectory acceleration";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
