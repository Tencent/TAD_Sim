// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_behav_sign_line.h"

namespace eval {
const char EvalBehavSignLine::_kpi_name[] = "BehavSignLine";

sim_msg::TestReport_XYPlot _s_behav_road_markline_plot;

EvalBehavSignLine::EvalBehavSignLine() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  subtype = hadmap::OtherSubtype;
  _event_markline = false;
  _msg_roadmark = "";
}
bool EvalBehavSignLine::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_behav_road_markline_plot, "incorrect behavior", "", "t", "s", {"speed"}, {"m/s"}, 1);
  }
  return true;
}

bool EvalBehavSignLine::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing" << "\n";
        return false;
      }
      double _speed = ego_front->GetSpeed().GetNormal();
      std::vector<eval::EvalMapObjectPtr> road_makings;
      ego_front->GetFrontRoadMarkings(road_makings);
      VLOG_1 << "road_makings.size(): " << road_makings.size() << "\n";
      for (auto road_marking : road_makings) {
        if (road_marking->m_object_type == hadmap::OBJECT_TYPE_CrossWalk && road_marking->m_map_objects.size() > 0) {
          VLOG_1 << "cur roadmark is : crosswalk\n";
          if (ego_front->GetAcc().GetX() > 0.2) {
            _cross_walk_detector.Detect(_speed * std::sqrt(2 * 6 * 5) * 3.6, 30);
            if (_speed * std::sqrt(2 * 6 * 5) * 3.6 >= 30) _msg_roadmark = "斑马线附近加速度和速度过大";
            set_msgs.insert(_msg_roadmark);
          }
        }
        if (road_marking->m_object_type == hadmap::OBJECT_TYPE_Stop && road_marking->m_map_objects.size() > 0) {
          VLOG_1 << "cur roadmark is : stopline\n";
          _stop_line_detector.Detect(_speed, std::sqrt(2 * 6 * 5));
          if (_speed > std::sqrt(2 * 6 * 5)) _msg_roadmark = "停止线附近速度过快";
          set_msgs.insert(_msg_roadmark);
        }
        if (road_marking->m_object_type == hadmap::OBJECT_TYPE_Arrow && road_marking->m_map_objects.size() > 0) {
          for (auto real_arrow : road_marking->m_map_objects) {
            if (map_info->m_on_road && (real_arrow->getObjectSubType() == hadmap::Arrow_Straight ||
                                        real_arrow->getObjectSubType() == hadmap::Arrow_Left ||
                                        real_arrow->getObjectSubType() == hadmap::Arrow_Right)) {
              VLOG_1 << "cur roadmark is : " << real_arrow->getObjectSubType() << "\n";
              subtype = real_arrow->getObjectSubType();
            }
          }
        }
      }
      if (subtype != hadmap::OtherSubtype && !map_info->m_on_road) {
        _event_markline = true;
        VLOG_1 << "event happen" << "\n";
      }
      if (_event_markline && map_info->m_on_road) {
        auto ego_behav = helper.GetVehStateFlow().GetOutput().m_veh_behav;
        VLOG_1 << "behav: " << ego_behav << "\n";
        VLOG_1 << "subtype: " << subtype << "\n";
        if (ego_behav == VehicleBehavior::TurnLeft) {
          _fall_detector.Detect(subtype == hadmap::Arrow_Left, 0.5);
          if (subtype != hadmap::Arrow_Left) _msg_roadmark = "不能左转";
        } else if (ego_behav == VehicleBehavior::TurnRight) {
          _fall_detector.Detect(subtype == hadmap::Arrow_Right, 0.5);
          if (subtype != hadmap::Arrow_Right) _msg_roadmark = "不能右转";
        } else {
          _fall_detector.Detect(subtype == hadmap::Arrow_Straight, 0.5);
          if (subtype != hadmap::Arrow_Straight) _msg_roadmark = "不能直行";
        }
        set_msgs.insert(_msg_roadmark);
        subtype = hadmap::OtherSubtype;
        _event_markline = false;
      }

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_road_markline_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_road_markline_plot.mutable_y_axis()->at(0).add_axis_data(_speed);
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
bool EvalBehavSignLine::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_behav_road_markline_plot);
  }
  return true;
}

void EvalBehavSignLine::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _fall_detector, _kpi_name);
}

EvalResult EvalBehavSignLine::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_fall_detector.GetCount() + _stop_line_detector.GetCount() +
                                             _cross_walk_detector.GetCount());
    if (_fall_detector.GetCount() + _stop_line_detector.GetCount() + _cross_walk_detector.GetCount() >=
            m_Kpi.passcondition().value() &&
        m_Kpi.passcondition().value() >= 0.5) {
      _msg_roadmark = "";
      for (std::string str : set_msgs) {
        _msg_roadmark += (str + "\n");
      }
      return EvalResult(sim_msg::TestReport_TestState_FAIL, _msg_roadmark);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before markLine check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before markLine check skipped");
}
bool EvalBehavSignLine::ShouldStopScenario(std::string &reason) {
  auto ret = _fall_detector.GetCount() + _stop_line_detector.GetCount() + _cross_walk_detector.GetCount() >=
                 m_Kpi.finishcondition().value() &&
             m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = _msg_roadmark;
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
