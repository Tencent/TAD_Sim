// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_lateral_acc.h"

#include <algorithm>

namespace eval {
const char EvalLateralAcc::_kpi_name[] = "MaxTurningAccelerate";

sim_msg::TestReport_XYPlot s_lateral_acc_plot;
sim_msg::TestReport_PairData g_avg_lateral_acc;

EvalLateralAcc::EvalLateralAcc() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _ay = 0.0;
  max_acc_straight = -1000;
  max_acc_curve = -1000;
}
bool EvalLateralAcc::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    thresh_lateral_acc_above_500r = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LATERAL_ACC_ABOVE_500R");
    thresh_lateral_acc_above_250r = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LATERAL_ACC_ABOVE_250R");
    thresh_lateral_acc_above_125r = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "LATERAL_ACC_ABOVE_125R");
    real_acc_thresh = m_defaultThreshDouble;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_lateral_acc_plot, "lateral acceleration", "", "t", "s", {"lateral acc"}, {"m/s2"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_lateral_acc_plot, "thresh upper", 0, 1, m_defaultThreshDouble, "thresh lower",
                                        1, 1, -m_defaultThreshDouble);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_lateral_acc_plot, {0}, true, true);
    ReportHelper::ConfigPairData(g_avg_lateral_acc);
  }

  return true;
}
bool EvalLateralAcc::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (nullptr == map_info.get()) {
        VLOG_0 << "map info missing.\n";
        return false;
      }

      _ay = ego_front->GetAcc().GetY();
      VLOG_1 << "_ay_abs is " << std::abs(_ay) << " m/s2.\n";

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
              // VLOG_2 << "s: " << s << ", curvatureV.m_starts: " << curvatureV.m_starts
              //        << ", curvatureV.m_length: " << curvatureV.m_length << "\n";
              // VLOG_2 << "cur_curvature is " << cur_curvature << ".\n";
              break;
            }
          }
        }
      }
      if (cur_curvature < 1.0 / 1000) {  // 曲率半径大于1000m
        real_acc_thresh = m_defaultThreshDouble;
        max_acc_straight = std::max(max_acc_straight, std::abs(_ay));
      } else if (cur_curvature < 1.0 / 500) {  // 大于500m但小于1000m
        real_acc_thresh = thresh_lateral_acc_above_500r;
        max_acc_curve = std::max(max_acc_curve, std::abs(_ay));
      } else if (cur_curvature < 1.0 / 250) {  // 大于250m但小于500m
        real_acc_thresh = thresh_lateral_acc_above_250r;
        max_acc_curve = std::max(max_acc_curve, std::abs(_ay));
      } else if (cur_curvature < 1.0 / 125) {  // 大于125m但小于250m
        real_acc_thresh = thresh_lateral_acc_above_125r;
        max_acc_curve = std::max(max_acc_curve, std::abs(_ay));
      } else {  // 小于125m，暂时未设计阈值，先用125m-250m的阈值
        real_acc_thresh = thresh_lateral_acc_above_125r;
        max_acc_curve = std::max(max_acc_curve, std::abs(_ay));
      }
      _detector.Detect(std::abs(_ay), real_acc_thresh);

      // add data to xy-pot
      if (isReportEnabled()) {
        s_lateral_acc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_lateral_acc_plot.mutable_y_axis()->at(0).add_axis_data(_ay);
        s_lateral_acc_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(real_acc_thresh);
        s_lateral_acc_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(-real_acc_thresh);
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
bool EvalLateralAcc::Stop(eval::EvalStop &helper) {
  // calculate average lateral acc
  // double average = 0.0;
  // if (s_lateral_acc_plot.y_axis_size() > 0) average = ReportHelper::CalAverage(s_lateral_acc_plot.y_axis(0));
  // ReportHelper::SetPairData(g_avg_lateral_acc, "average lateral acc", std::to_string(average));

  std::string straight_msg = max_acc_straight == -1000 ? "null" : std::to_string(max_acc_straight);
  std::string curve_msg = max_acc_curve == -1000 ? "null" : std::to_string(max_acc_curve);
  ReportHelper::SetPairData(g_avg_lateral_acc, "max lat acc ( straight | curve )", straight_msg + " | " + curve_msg);

  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_lateral_acc_plot);
    ReportHelper::AddPair2Attach(*attach, g_avg_lateral_acc);
  }

  return true;
}

EvalResult EvalLateralAcc::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max lateral acceleration");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral acceleration check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral acceleration check skipped");
}
bool EvalLateralAcc::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max lateral acceleration";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalLateralAcc::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_lateral_acc()->set_acceleration(_ay);
  msg.mutable_lateral_acc()->set_state(sim_msg::GRADING_ACCELERATION_NORMAL);
  if (_detector.GetLatestState()) {
    msg.mutable_lateral_acc()->set_state(sim_msg::GRADING_ACCELERATION_RAPIDACCELERATE);
  }
  msg.mutable_lateral_acc()->set_threshold(m_defaultThreshDouble);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalLateralAcc::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_overturningacceleration()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_overturningacceleration()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_overturningacceleration()->set_is_pass(IsEvalPass()._state ==
                                                                             sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
