// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_acc.h"

namespace eval {
const char EvalAcc::_kpi_name[] = "MaxAcceleration_V";

sim_msg::TestReport_XYPlot s_ax_plot;
sim_msg::TestReport_PairData EvalAcc::s_max_long_acc_pair;

EvalAcc::EvalAcc() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _ax = 0.0;
  real_acc_thresh = 0.0;
}
bool EvalAcc::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    /* TBD
    _eval_cfg = helper.GetEvalCfg(_kpi_name);
    _eval_cfg.DebugShow();
    */
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    thresh_lower_speed = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LOWER_SPEED");
    thresh_acc_above_upper = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_ABOVE_UPPER");
    thresh_acc_below_lower = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_BELOW_LOWER");
    thresh_acc_lower2upper = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_LOWER2UPPER");
    thresh_acc_above_500r = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_ABOVE_500R");
    thresh_acc_above_250r = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_ABOVE_250R");
    thresh_acc_above_125r = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "ACC_ABOVE_125R");
    real_acc_thresh = thresh_acc_below_lower;
    max_acc_curve = -1000;
    max_acc_straight = -1000;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  /* TBD */
  if (isReportEnabled()) {
    /* TBD */
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_ax_plot, "acceleration", "", "t", "s", {"acc"}, {"m/s2"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_ax_plot, "thresh upper", 0, 1, real_acc_thresh);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_ax_plot, {0}, true, false);
  }

  return true;
}
bool EvalAcc::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_acceleration_above_thresh(
      sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  /* TBD
  if (IsModuleValid() && _eval_cfg._enabled) {
  */
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front == nullptr) {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }

    // get the map information pointer and check whether the pointer is null
    auto map_info = ego_front->GetMapInfo();
    if (map_info.get() == nullptr) {
      VLOG_1 << "map info missing.\n";
      return false;
    }

    _ax = ego_front->GetAcc().GetX();
    VLOG_1 << "ax is " << _ax << " m/s2.\n";

    double _ego_speed = ego_front->GetSpeed().GetNormal();
    double cur_curvature = 0.0;
    CPosition currentPoint = ego_front->GetLocation().GetPosition();
    _map_mgr->ENUToWGS84(currentPoint);
    hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
    if (ego_lane) {
      hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
      if (ego_road) {
        const hadmap::txLineCurve *pLine = dynamic_cast<const hadmap::txLineCurve *>(ego_road->getGeometry());
        double s = 0;
        double l = 0;
        double yaw = 0;
        pLine->xy2sl(currentPoint.GetX(), currentPoint.GetY(), s, l, yaw);
        for (const auto &curvatureV : ego_road->getCurvature()) {
          if (s >= curvatureV.m_starts && s < curvatureV.m_starts + curvatureV.m_length) {
            cur_curvature = std::abs(curvatureV.m_curvature);
            VLOG_2 << "cur_curvature is " << cur_curvature << ".\n";
            break;
          }
        }
      }
    }
    if (cur_curvature < 1.0 / 1000) {  // 曲率半径大于1000m
      if (_ego_speed > m_defaultThreshDouble) {
        real_acc_thresh = thresh_acc_above_upper;
      } else if (_ego_speed >= thresh_lower_speed) {
        real_acc_thresh = thresh_acc_lower2upper;
      } else {
        real_acc_thresh = thresh_acc_below_lower;
      }
      max_acc_straight = std::max(max_acc_straight, _ax);
    } else if (cur_curvature < 1.0 / 500) {  // 大于500m但小于1000m
      real_acc_thresh = thresh_acc_above_500r;
      max_acc_curve = std::max(max_acc_curve, _ax);
    } else if (cur_curvature < 1.0 / 250) {  // 大于250m但小于500m
      real_acc_thresh = thresh_acc_above_250r;
      max_acc_curve = std::max(max_acc_curve, _ax);
    } else if (cur_curvature < 1.0 / 125) {  // 大于125m但小于250m
      real_acc_thresh = thresh_acc_above_125r;
      max_acc_curve = std::max(max_acc_curve, _ax);
    } else {  // 小于125m，暂时未设计阈值，先用125m-250m的阈值
      real_acc_thresh = thresh_acc_above_125r;
      max_acc_curve = std::max(max_acc_curve, _ax);
    }

    /* TBD
    if (_detector.Detect(_ax, _eval_cfg._thresh_value)) {
    */
    if (_detector.Detect(_ax, real_acc_thresh)) {
      grading.mutable_event_detector()->set_acceleration_above_thresh(
          sim_msg::Grading_EventDetector_EventState_EventDetected);
    }

    // add data to xy-pot
    /* TBD */
    if (isReportEnabled()) {
      s_ax_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_ax_plot.mutable_y_axis()->at(0).add_axis_data(_ax);
      s_ax_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(real_acc_thresh);
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalAcc::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("OverAccelerate", std::to_string(_detector.GetCount()));
  helper.SetFeedback("OveraccelerationTime", std::to_string(0));
  std::string straight_msg = max_acc_straight == -1000 ? "null" : std::to_string(max_acc_straight);
  std::string curve_msg = max_acc_curve == -1000 ? "null" : std::to_string(max_acc_curve);
  ReportHelper::SetPairData(s_max_long_acc_pair, "max acc ( straight | curve )", straight_msg + " | " + curve_msg);

  // add report
  /* TBD */
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_ax_plot);
    ReportHelper::AddPair2Attach(*attach, s_max_long_acc_pair);
  }

  return true;
}
EvalResult EvalAcc::IsEvalPass() {
  /* TBD
  if (_eval_cfg._enabled) {
      if (_detector.GetCount() >= _eval_cfg._eval_value && _eval_cfg._eval_value >= 0.5) {
  */
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max acceleration");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max acceleration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max acceleration check skipped");
}
bool EvalAcc::ShouldStopScenario(std::string &reason) {
  /* TBD
  auto ret = _detector.GetCount() >= _eval_cfg._stop_value && _eval_cfg._stop_value >= 0.5;
  */
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max acceleration";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalAcc::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_acceleration()->set_acceleration(_ax);
  msg.mutable_acceleration()->set_state(sim_msg::GRADING_ACCELERATION_NORMAL);
  if (_detector.GetLatestState()) {
    msg.mutable_acceleration()->set_state(sim_msg::GRADING_ACCELERATION_RAPIDACCELERATE);
  }
  msg.mutable_acceleration()->set_threshold(real_acc_thresh);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalAcc::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_overacceleration()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_overacceleration()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_overacceleration()->set_is_pass(IsEvalPass()._state ==
                                                                      sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
