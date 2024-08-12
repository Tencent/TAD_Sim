// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_min_dist_foll_dec.h"

namespace eval {
const char EvalMinDistFollDec::_kpi_name[] = "MinDistFollDec";

sim_msg::TestReport_XYPlot EvalMinDistFollDec::_s_dis_follow_dec_plot;
sim_msg::TestReport_PairData EvalMinDistFollDec::_s_min_follow_dist_pair;

EvalMinDistFollDec::EvalMinDistFollDec() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalMinDistFollDec::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    min_follow_distance = 200;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(_s_min_follow_dist_pair);
    ReportHelper::ConfigXYPlot(_s_dis_follow_dec_plot, "min distance when dec", "", "t", "s",
                               {"whether ego-veh dec", "distance"}, {"N/A", "m"}, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_dis_follow_dec_plot, "whether dec", 0, 0, 0.5, "", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThreshold(_s_dis_follow_dec_plot, "", 0, 0, INT32_MAX, "distance lower", 1, 0,
                                        m_defaultThreshDouble, 1);
  }

  return true;
}
bool EvalMinDistFollDec::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      auto _fellow = helper.GetLeadingVehicle();
      double dist_relative = 200;
      bool whether_all_in_dec = false;
      double _ego_speed = ego_front->GetSpeed().GetNormal();
      double _ego_acc = ego_front->GetAcc().GetX();
      if (_fellow) {
        EVector3d ego_2_fellow =
            CEvalMath::Sub(_fellow->TransMiddleRear2BaseCoord(), ego_front->TransMiddleFront2BaseCoord());
        double _fell_acc = _fellow->GetAcc().GetX();
        whether_all_in_dec = (_fell_acc < 0 && _ego_acc < 0 && _ego_speed > 0.56);
        if (whether_all_in_dec) {
          dist_relative = ego_2_fellow.GetNormal2D();
          min_follow_distance = std::min(dist_relative, min_follow_distance);
        }
      }
      _fall_detector.Detect(dist_relative, m_defaultThreshDouble);
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_dis_follow_dec_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_dis_follow_dec_plot.mutable_y_axis()->at(0).add_axis_data(whether_all_in_dec);
        _s_dis_follow_dec_plot.mutable_y_axis()->at(1).add_axis_data(dist_relative);
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
bool EvalMinDistFollDec::Stop(eval::EvalStop &helper) {
  ReportHelper::SetPairData(_s_min_follow_dist_pair, "min distance[m]", std::to_string(min_follow_distance));
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddPair2Attach(*attach, _s_min_follow_dist_pair);
    ReportHelper::AddXYPlot2Attach(*attach, _s_dis_follow_dec_plot);
  }
  return true;
}

void EvalMinDistFollDec::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalMinDistFollDec::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount());

    if (_fall_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "lower min follow distance when decelarate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "min distance check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "min distance check skipped");
}

bool EvalMinDistFollDec::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "lower min distance";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
