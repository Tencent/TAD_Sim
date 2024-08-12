// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_yield_pedestrian.h"

namespace eval {
const char EvalYieldPedestrian::_kpi_name[] = "YieldPedestrian";

sim_msg::TestReport_XYPlot _s_behav_pedestrian_plot;
sim_msg::TestReport_PairData EvalYieldPedestrian::_g_yield_pair;

EvalYieldPedestrian::EvalYieldPedestrian() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalYieldPedestrian::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    _event_yield = false;
    min_dist_to_person = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MIN_DIST");
    max_dist_to_person = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MAX_DIST");
    max_starting_time = getThreshValueByID_Double(m_Kpi, m_KpiEnabled, "MAX_START_TIME");
    stopped_distance = -1;
    restart_time = 0.0;
    _msg = "行为规范";
    // output indicator configuration details
    DebugShowKpi();
  }
  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigPairData(_g_yield_pair);
    ReportHelper::ConfigXYPlot(_s_behav_pedestrian_plot, "incorrect behavior", "", "t", "s",
                               {"checked_person", "if_stopped", "long_dist", "start_time"}, {"N/A", "N/A", "m", "s"},
                               4);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_pedestrian_plot, "", 0, 1, INT32_MAX, "if yield", 1, 0, 0.5, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_pedestrian_plot, "dist upper", 0, 1, max_dist_to_person, "dist lower",
                                        1, 0, min_dist_to_person, 2);
    ReportHelper::ConfigXYPlotThreshold(_s_behav_pedestrian_plot, "start time upper", 0, 1, max_starting_time, "", 1, 0,
                                        INT32_MIN, 3);
  }
  return true;
}

bool EvalYieldPedestrian::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      DynamicActorFLUList &&dynamic_actors_flu =
          _actor_mgr->GetFellowActorsByType<CDynamicActorFLUPtr>(Actor_Dynamic_FLU);
      CDynamicActorFLUPtr actor = nullptr;
      double ego_car_width = ego_front->GetShape().GetWidth();
      double ego_car_length = ego_front->GetShape().GetLength();
      double long_dist = -1;
      double lateral_dist = -1;
      bool un_proper_dist_2_people = false;
      bool not_avoid_persons = false;
      bool un_quik_start = false;
      const VehOutput &veh_out = helper.GetVehStateFlow().GetOutput();
      for (const auto dynamic_actor : dynamic_actors_flu) {
        // get the real distance between car and person which checked.
        long_dist = dynamic_actor->GetLocation().GetPosition().GetX() -
                    (ego_car_length + dynamic_actor->GetShape().GetLength()) / 2;
        lateral_dist = std::abs(dynamic_actor->GetLocation().GetPosition().GetY()) -
                       (ego_car_width + dynamic_actor->GetShape().GetWidth()) / 2;

        // get the persons who should be yielded.
        if (dynamic_actor->GetType() == Actor_Dynamic_FLU && (dynamic_actor->GetTypeID() <= 18) &&
            (dynamic_actor->GetTypeID() >= 0) && (dynamic_actor->GetLocation().GetPosition().GetX() > 0) &&
            (long_dist <= max_dist_to_person) && (lateral_dist <= 1.5)) {
          // get the most close person
          if (actor == nullptr)
            actor = dynamic_actor;
          else
            actor = actor->GetLocation().GetPosition().GetX() <= dynamic_actor->GetLocation().GetPosition().GetX()
                        ? actor
                        : dynamic_actor;
        }
      }
      VLOG_1 << "person lateral distance is : " << lateral_dist << "\n";
      VLOG_1 << "person long distance is : " << long_dist << "\n";
      if (actor) {
        // check the car if not avoid person. step 1
        not_avoid_persons = (long_dist <= 0.5 && veh_out.m_veh_behav != VehicleBehavior::Stopped);

        if (veh_out.m_veh_behav == VehicleBehavior::Stopped) {
          // open flag for detect of restarting time.
          _event_yield = true;

          // check the distance to the person when the car is stopped. step2
          stopped_distance = (stopped_distance == -1) ? long_dist : stopped_distance;
          un_proper_dist_2_people = (stopped_distance > max_dist_to_person || stopped_distance < min_dist_to_person);
        }
      }

      // check the starting time when persons walk away. step 3
      if (!actor && _event_yield) {
        if (veh_out.m_veh_behav == VehicleBehavior::Stopped) {
          restart_time += getModuleStepTime();
          un_quik_start = (restart_time > max_starting_time);
        } else {
          _event_yield = false;
          restart_time = 0.0;
          stopped_distance = -1;
        }
      }

      bool un_proper_actions = (un_proper_dist_2_people || not_avoid_persons || un_quik_start);
      if (un_proper_actions) {
        _msg = "未礼让行人: " + std::to_string(not_avoid_persons) +
               "\n礼让距离不合理: " + std::to_string(un_proper_dist_2_people) +
               "\n重新启动时间过长: " + std::to_string(un_quik_start);
      }
      VLOG_1 << "un proper distance to people : " << un_proper_dist_2_people << "\n";
      VLOG_1 << "not avoid persons : " << not_avoid_persons << "\n";
      VLOG_1 << "start time when persons walk away : " << restart_time << "\n";

      _detector.Detect(un_proper_actions, 0.5);

      // add data to xy-pot
      if (isReportEnabled()) {
        _s_behav_pedestrian_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_behav_pedestrian_plot.mutable_y_axis()->at(0).add_axis_data(actor != nullptr);
        _s_behav_pedestrian_plot.mutable_y_axis()->at(1).add_axis_data(!not_avoid_persons);
        _s_behav_pedestrian_plot.mutable_y_axis()->at(2).add_axis_data(stopped_distance);
        _s_behav_pedestrian_plot.mutable_y_axis()->at(3).add_axis_data(restart_time);
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
bool EvalYieldPedestrian::Stop(eval::EvalStop &helper) {
  ReportHelper::SetPairData(_g_yield_pair, "action msg", _msg);
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddPair2Attach(*attach, _g_yield_pair);
    ReportHelper::AddXYPlot2Attach(*attach, _s_behav_pedestrian_plot);
  }

  return true;
}

void EvalYieldPedestrian::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalYieldPedestrian::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, _msg);
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before pedestrian check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "behavior before pedestrian check skipped");
}
bool EvalYieldPedestrian::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "incorrect behavior before pedestrian";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
