// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_yaw_rate.h"

namespace eval {
const char EvalYawRate::_kpi_name[] = "MaxYawRate";

sim_msg::TestReport_XYPlot s_yaw_rate_plot;

EvalYawRate::EvalYawRate() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _yaw_rate = 0.0;
  default_curved_threshold = 0.15;
}
bool EvalYawRate::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    default_curved_threshold = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "CURVED_THRESHOLD");
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_yaw_rate_plot, "yaw rate", "", "t", "s", {"yaw_rate"}, {"rad/s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_yaw_rate_plot, "thresh upper", 0, 1, m_defaultThreshDouble, "thresh lower", 1,
                                        1, -m_defaultThreshDouble);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_yaw_rate_plot, {0}, true, true);
  }

  return true;
}
bool EvalYawRate::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      double cur_yaw_rate_thresh = default_curved_threshold;
      if (map_info.get() != nullptr) {
        CPosition currentPoint = ego_front->GetLocation().GetPosition();
        // 1.trans current pos to 84
        _map_mgr->ENUToWGS84(currentPoint);
        hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
        if (ego_lane) {
          hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
          if (ego_road) {
            // calculate s l
            double cur_curvature = 0.0;
            const hadmap::txLineCurve *pLine = dynamic_cast<const hadmap::txLineCurve *>(ego_road->getGeometry());
            double s = 0;
            double l = 0;
            double yaw = 0;
            pLine->xy2sl(currentPoint.GetX(), currentPoint.GetY(), s, l, yaw);
            for (const auto &curvatureV : ego_road->getCurvature()) {
              if (s >= curvatureV.m_starts && s < curvatureV.m_starts + curvatureV.m_length) {
                cur_curvature = std::fabs(curvatureV.m_curvature);
                break;
              }
            }
            if (cur_curvature < 1e-3) cur_yaw_rate_thresh = m_defaultThreshDouble;
          }
        }
      }
      _yaw_rate = ego_front->GetAngularV().GetZ();
      VLOG_1 << "yaw rate is " << _yaw_rate << " rad/s.\n";
      _detector.Detect(std::abs(_yaw_rate), cur_yaw_rate_thresh);

      // add data to xy-pot
      if (isReportEnabled()) {
        s_yaw_rate_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_yaw_rate_plot.mutable_y_axis()->at(0).add_axis_data(_yaw_rate);
        s_yaw_rate_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(cur_yaw_rate_thresh);
        s_yaw_rate_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(-cur_yaw_rate_thresh);
      }
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
bool EvalYawRate::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_yaw_rate_plot);
  }

  return true;
}

void EvalYawRate::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalYawRate::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max yaw rate");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max yaw rate check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max yaw rate check skipped");
}
bool EvalYawRate::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max yaw rate";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
