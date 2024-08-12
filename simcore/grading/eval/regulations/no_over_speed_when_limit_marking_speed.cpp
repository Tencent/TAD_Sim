// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "no_over_speed_when_limit_marking_speed.h"

namespace eval {
const char NoOverSpeedWhenLimitMarkingSpeed::_kpi_name[] = "NoOverSpeedWhenLimitMarkingSpeed";
sim_msg::TestReport_XYPlot NoOverSpeedWhenLimitMarkingSpeed::s_no_over_speed_when_limit_marking_speed_plot;

NoOverSpeedWhenLimitMarkingSpeed::NoOverSpeedWhenLimitMarkingSpeed() {
  _speed = 0.0;
  _speed_limit_upper = 120.0 / 3.6;
  _speed_limit_lower = -1.0;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool NoOverSpeedWhenLimitMarkingSpeed::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_no_over_speed_when_limit_marking_speed_plot, "overspeed check(limit marking speed)",
                               "", "t", "s", {"result_speed"}, {"m/s"}, 1);

    ReportHelper::ConfigXYPlotThreshold(s_no_over_speed_when_limit_marking_speed_plot, "speed upper", 0, 0, INT32_MAX,
                                        "speed lower", 1, 0, INT32_MIN, 0);
    ReportHelper::ConfigXYPlotThresholdClearValue(s_no_over_speed_when_limit_marking_speed_plot, {0}, true, true);
  }

  return true;
}

bool NoOverSpeedWhenLimitMarkingSpeed::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      _speed = ego_front->GetSpeed().GetNormal();
      std::vector<eval::EvalMapObjectPtr> road_makings;
      ego_front->GetFrontRoadMarkings(road_makings);
      VLOG_1 << "road makings size : " << road_makings.size() << "\n";
      for (auto road_marking : road_makings) {
        if (road_marking->m_object_type == hadmap::OBJECT_TYPE_RoadSign && road_marking->m_map_objects.size() > 0) {
          for (auto indicate_sign : road_marking->m_map_objects) {
            VLOG_1 << "ENTERING CHECK ROAD SPEED LINES" << "\n";
            if (indicate_sign->getObjectType() == hadmap::OBJECT_TYPE_RoadSign) {
              if (indicate_sign->getObjectSubType() == hadmap::RoadSign_100_120) {
                _speed_limit_lower = 100 / 3.6;
                _speed_limit_upper = 120 / 3.6;
              } else if (indicate_sign->getObjectSubType() == hadmap::RoadSign_80_100) {
                _speed_limit_lower = 80 / 3.6;
                _speed_limit_upper = 100 / 3.6;
              }
            }
          }
        }
      }
      VLOG_1 << "CHECK" << "\n";

      //   EvalMapObjectPtr obj(new EvalMapObject());
      //   ego_front->GetFrontRoadMarkings(obj);
      //   for (auto indicate_sign : obj->m_map_objects) {
      //     if (indicate_sign->getObjectType() == hadmap::OBJECT_TYPE_RoadSign) {
      //       if (indicate_sign->getObjectSubType() == hadmap::RoadSign_100_120) {
      //         _speed_limit_lower = 100 / 3.6;
      //         _speed_limit_upper = 120 / 3.6;
      //       } else if (indicate_sign->getObjectSubType() == hadmap::RoadSign_80_100) {
      //         _speed_limit_lower = 80 / 3.6;
      //         _speed_limit_upper = 100 / 3.6;
      //       }
      //     }
      //   }
      if (map_info && !map_info->m_on_road) {
        _speed_limit_lower = -1.0;
        _speed_limit_upper = 120 / 3.6;
      }
      _detector_lower_speed_m_s.Detect(_speed, _speed_limit_lower);
      _detector_upper_speed_m_s.Detect(_speed, _speed_limit_upper);
      if (isReportEnabled()) {
        s_no_over_speed_when_limit_marking_speed_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_no_over_speed_when_limit_marking_speed_plot.mutable_y_axis()->at(0).add_axis_data(_speed);
        s_no_over_speed_when_limit_marking_speed_plot.mutable_y_axis()->at(0).mutable_threshold_upper()->add_value(
            _speed_limit_upper);
        s_no_over_speed_when_limit_marking_speed_plot.mutable_y_axis()->at(0).mutable_threshold_lower()->add_value(
            _speed_limit_lower);
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

bool NoOverSpeedWhenLimitMarkingSpeed::Stop(eval::EvalStop &helper) {
  if (isReportEnabled()) {
    // add report
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, s_no_over_speed_when_limit_marking_speed_plot);
  }

  return true;
}

void NoOverSpeedWhenLimitMarkingSpeed::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_speed()->set_speed(_speed);
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector_upper_speed_m_s, _kpi_name);
}

EvalResult NoOverSpeedWhenLimitMarkingSpeed::IsEvalPass() {
  if (m_KpiEnabled) {
    int _detect_count = _detector_lower_speed_m_s.GetCount() + _detector_upper_speed_m_s.GetCount();
    _case.mutable_info()->set_detected_count(_detect_count);
    if (_detect_count >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max speed");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max speed check skipped");
}

bool NoOverSpeedWhenLimitMarkingSpeed::ShouldStopScenario(std::string &reason) {
  int _detect_count = _detector_lower_speed_m_s.GetCount() + _detector_upper_speed_m_s.GetCount();
  auto ret = _detect_count >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max speed";
  _case.mutable_info()->set_request_stop(ret);

  return ret;
}
}  // namespace eval
