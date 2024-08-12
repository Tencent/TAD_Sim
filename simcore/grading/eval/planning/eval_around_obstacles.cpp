// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_around_obstacles.h"

namespace eval {
const char EvalAroundObstacles::_kpi_name[] = "AroundObstacles";

sim_msg::TestReport_XYPlot _s_aroundBarrier_plot;

EvalAroundObstacles::EvalAroundObstacles() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  aroundTime = 0.0;
  msg_around = "pass";
}
bool EvalAroundObstacles::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // default around barrier time
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_aroundBarrier_plot, "around barrier", "", "t", "s", {"around time"}, {"s"}, 1);
    /**
     * set Threshold after ConfigXYPlot
     * @param    xy_plot         TestReport_XYPlot
     * @param    upper_desc      the desc of upper threshold.
     * @param    upper_space     the IntervalSpace of upper threshold.
     * @param    upper_type      the IntervalType of upper threshold.
     * @param    upper_value     the value of upper threshold.
     * @param    lower_desc      the desc of lower threshold.
     * @param    lower_space     the IntervalSpace of lower threshold.
     * @param    lower_type      the IntervalType of lower threshold.
     * @param    lower_value     the value of lower threshold.
     * @param    y_axis_id       the id  of y axis. default 0
     */
    ReportHelper::ConfigXYPlotThreshold(_s_aroundBarrier_plot, "around time upper", 0, 1, m_defaultThreshDouble, "", 1,
                                        0, INT32_MIN, 0);
  }

  return true;
}

bool EvalAroundObstacles::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // get static actors
      eval::StaticActorList static_actors = helper.FindFrontStaticActor(ego_front, 20);
      // get dynamic actors
      std::vector<eval::DynamicActorList> dynamic_actors = helper.FindFrontDynamicActor(ego_front, dynamic_types, 20);
      VLOG_1 << "static_actors size: " << static_actors.size() << "\n";
      VLOG_1 << "the arounding barrires size: " << actors_checked.size() << "\n";
      if (!_event_around) {
        if (static_actors.size() > 0) {
          _event_around = true;
          actors_checked.emplace_back(static_actors.at(0));
        } else {
          for (auto actor_list : dynamic_actors) {
            if (actor_list.size() > 0) {
              _event_around = true;
              VLOG_1 << "dynamic actors checked: " << actor_list.size() << "\n";
              actors_checked.emplace_back(actor_list.at(0));
              break;
            }
          }
        }
      }
      if (_event_around) {
        aroundTime += getModuleStepTime();
        if (auto collision_actor = EvalStep::FindCollisionFellow(ego_front, actors_checked)) {
          VLOG_2 << "eval | collision with dynamic actor whose id is " << collision_actor->GetID() << "\n";
          _detector.Detect(m_defaultThreshDouble + 1, m_defaultThreshDouble);
          _event_around = false;
          aroundTime = 0.0;
          msg_around = "发生碰撞";
          std::vector<CStaticActorPtr> tmp;
          actors_checked.swap(tmp);
          VLOG_1 << "collision type: " << collision_actor->GetType() << "\n";
          VLOG_1 << "collision getTypeId: " << collision_actor->GetTypeID() << "\n";
        } else {
          _detector.Detect(aroundTime, m_defaultThreshDouble);
          if (aroundTime > m_defaultThreshDouble) msg_around = "绕障时间过长";
          bool _around_flag = true;
          for (auto &actor : actors_checked) {
            _around_flag = _around_flag && ego_front->TheObstacleIsBehindVehicle(actor->GetLocation());
            VLOG_1 << "if the barrer behind the car: " << _around_flag << "\n";
          }
          if (_around_flag) {
            _event_around = false;
            aroundTime = 0.0;
            std::vector<CStaticActorPtr> tmp;
            actors_checked.swap(tmp);
          }
        }
      }
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_aroundBarrier_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_aroundBarrier_plot.mutable_y_axis()->at(0).add_axis_data(aroundTime);
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
bool EvalAroundObstacles::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_aroundBarrier_plot);
  }
  return true;
}

void EvalAroundObstacles::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalAroundObstacles::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, msg_around);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max around barrier time check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max around barrier time check skipped");
}

bool EvalAroundObstacles::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max around barrier time";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
