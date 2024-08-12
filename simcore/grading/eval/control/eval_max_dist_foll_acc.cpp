// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_max_dist_foll_acc.h"

namespace eval {
const char EvalMaxDistFollAcc::_kpi_name[] = "MaxDistFollAcc";

sim_msg::TestReport_XYPlot EvalMaxDistFollAcc::_s_dis_follow_acc_plot;
sim_msg::TestReport_PairData EvalMaxDistFollAcc::_s_max_follow_dist_pair;

EvalMaxDistFollAcc::EvalMaxDistFollAcc() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalMaxDistFollAcc::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    max_follow_dist = -1;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(_s_max_follow_dist_pair);
    ReportHelper::ConfigXYPlot(_s_dis_follow_acc_plot, "distance when follow acc", "", "t", "s",
                               {"whether ego-veh acc", "distance"}, {"N/A", "m"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_dis_follow_acc_plot, "whether acc", 0, 0, 0.5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_dis_follow_acc_plot, "distance upper", 0, 0, 0, "", 1, 0, INT32_MIN, 1);
    ReportHelper::ConfigXYPlotThresholdClearValue(_s_dis_follow_acc_plot, {1}, true, false);
  }

  return true;
}
bool EvalMaxDistFollAcc::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      auto _fellow = helper.GetLeadingVehicle();
      bool whether_all_in_acc = false;
      double _ego_speed = ego_front->GetSpeed().GetNormal();
      double _ego_acc = ego_front->GetAcc().GetX();
      double dist_relative = 0.0;
      double cur_thresh_dis = _ego_speed < 0.56 ? 30.0 : 1.2 * m_defaultThreshDouble * _ego_speed;
      if (_fellow) {
        EVector3d ego_2_fellow =
            CEvalMath::Sub(_fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        double _fell_speed = _fellow->GetSpeed().GetNormal2D();
        double _fell_acc = _fellow->GetAcc().GetX();
        whether_all_in_acc = (_fell_acc > 0 && _fell_speed > 0.56 && _ego_acc > 0.0);
        if (whether_all_in_acc) {
          dist_relative = ego_2_fellow.GetNormal2D();
          max_follow_dist = std::max(max_follow_dist, dist_relative);
        }
      }
      _detector.Detect(dist_relative, cur_thresh_dis);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_dis_follow_acc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_dis_follow_acc_plot.mutable_y_axis()->at(0).add_axis_data(whether_all_in_acc);
        _s_dis_follow_acc_plot.mutable_y_axis()->at(1).add_axis_data(dist_relative);
        _s_dis_follow_acc_plot.mutable_y_axis()->at(1).mutable_threshold_upper()->add_value(cur_thresh_dis);
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
bool EvalMaxDistFollAcc::Stop(eval::EvalStop &helper) {
  ReportHelper::SetPairData(_s_max_follow_dist_pair, "max distance[m]", std::to_string(max_follow_dist));
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddPair2Attach(*attach, _s_max_follow_dist_pair);
    ReportHelper::AddXYPlot2Attach(*attach, _s_dis_follow_acc_plot);
  }
  return true;
}

void EvalMaxDistFollAcc::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalMaxDistFollAcc::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max distance between cars when accelerate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max distance between cars check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max distance between cars check skipped");
}

bool EvalMaxDistFollAcc::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max distance between cars count";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
