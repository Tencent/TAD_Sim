// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_traj_dec.h"
#include "trajectory.pb.h"

namespace eval {
const char EvalTrajDec::_kpi_name[] = "Traj_MaxDec";

sim_msg::TestReport_XYPlot EvalTrajDec::s_traj_dec_plot;

EvalTrajDec::EvalTrajDec() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalTrajDec::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_traj_dec_plot, "trajectory max deceleration", "", "t", "s", {"traj dec"}, {"m/s2"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_traj_dec_plot, "", 0, 1, INT32_MAX, "thresh lower", 1, 1,
                                        m_defaultThreshDouble);
  }

  return true;
}
bool EvalTrajDec::Step(eval::EvalStep &helper) {
  double max_dec = 0.0;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // find max dec from trajectory
    auto findMaxDec = [](const sim_msg::Trajectory &traj, double &max_dec) {
      double cur_max_dec = 0.0;

      for (auto i = 0; i < traj.point_size(); ++i) {
        double acc = traj.point().at(i).a();
        cur_max_dec = acc <= cur_max_dec ? acc : cur_max_dec;
      }
      max_dec = std::min(cur_max_dec, 0.0);
    };

    sim_msg::Trajectory traj;
    EvalMsg &&msg = _msg_mgr->Get(topic::TRAJECTORY);

    if (msg.GetPayload().size() > 0 && traj.ParseFromString(msg.GetPayload())) {
      findMaxDec(traj, max_dec);
      _detector.Detect(max_dec, m_defaultThreshDouble);

      // add data to plot
      if (isReportEnabled()) {
        s_traj_dec_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_traj_dec_plot.mutable_y_axis()->at(0).add_axis_data(max_dec);
      }
    } else {
      LOG_ERROR << "fail to parse trajectory msg, payload size:" << msg.GetPayload().size() << "\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }

  return true;
}
bool EvalTrajDec::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_traj_dec_plot);
  }

  return true;
}

void EvalTrajDec::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalTrajDec::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max trajectory deceleration");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory deceleration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max trajectory deceleration check skipped");
}
bool EvalTrajDec::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max trajectory  deceleration";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
