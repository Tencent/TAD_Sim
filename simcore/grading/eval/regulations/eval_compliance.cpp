// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_compliance.h"

namespace eval {
const char EvalCompliance::_kpi_name[] = "Compliance";

sim_msg::TestReport_XYPlot EvalCompliance::_s_compliance_plot;

EvalCompliance::EvalCompliance() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _violate_times = 0;
  _rush_score = 0;
  _speed_limit_score = 0;
  _error_lane_score = 0;
  _solid_line_score = 0;

  _event_overspeed = false;
  _event_solid_line = false;
  _event_err_lane = false;
  _event_red_light = false;
  over_speed_times = 0;
  _overspeed_score_flag = false;
  _errlane_score_flag = false;
  cur_light = GREEN;
  option_link = false;
}
bool EvalCompliance::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_speed_limit_from_scene = helper.GetScenarioInfo().m_ego_speed_limit;
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(
        _s_compliance_plot, "compliance info", "include times of violation of traffic rules and scores", "t", "s",
        {"non-compliance times", "I_compliance", "rushScore", "speedingScore", "errLaneScore", "crossSolidScore"},
        {"N/A", "N/A", "N/A", "N/A", "N/A", "N/A"}, 6);
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

    /**
     * [optional] clear threshold value if threshold is not constants
     * @param xy_plot         TestReport_XYPlot
     * @param y_axis_ids      y_axis id list to clear
     * @param upper_clear     whether clear upper threshold
     * @param lower_clear     whether clear lower threshold
     */
    // ReportHelper::ConfigXYPlotThresholdClearValue(_s_compliance_plot, {0}, true, false);
  }
  return true;
}

bool EvalCompliance::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    auto ego_trailer = _actor_mgr->GetEgoTrailerActorPtr();
    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info == nullptr) {
        VLOG_1 << "map info missing" << "\n";
        return false;
      }
      /* speed limit check */
      double _speed = ego_front->GetSpeed().GetNormal();
      // get lane speed limit from hadmap
      double speed_limit_from_map = -1.0;
      double _speed_limit = m_speed_limit_from_scene;
      if (map_info) {
        auto ego_lane = map_info->m_active_lane.m_lane;
        if (ego_lane) {
          speed_limit_from_map = ego_lane->getSpeedLimit() / 3.6;
          speed_limit_from_map = speed_limit_from_map > 1.0 ? speed_limit_from_map : -1.0;
        }
      }
      if (speed_limit_from_map < 0.5) {
        // 72km/h
        speed_limit_from_map = 20;
      }
      VLOG_1 << "cur speed limit is:" << speed_limit_from_map << "\n";
      if (!_event_overspeed && _speed > speed_limit_from_map) {
        _event_overspeed = true;
        over_speed_times = 1;
        _overspeed_time += getModuleStepTime();
        _cur_over = OVER0;
        VLOG_1 << "OVER SPEED........" << "\n";
      } else if (_event_overspeed) {
        if (_speed <= speed_limit_from_map) {
          // finish speeding status
          if (_overspeed_time >= 1.0) _violate_times++;
          if (_overspeed_time >= 1.0 &&
              (over_speed_times == 1 || (_overspeed_time >= 5.0 * over_speed_times && !_overspeed_score_flag))) {
            _speed_limit_score += _cur_over;
          }
          _event_overspeed = false;
          _overspeed_time = 0.0;
          over_speed_times = 0;
          _cur_over = NO_OVER;
          _overspeed_score_flag = false;
        } else {
          // continue to be speeding
          if (_overspeed_time >= 5.0 * over_speed_times && !_overspeed_score_flag) {
            over_speed_times++;
            _speed_limit_score += _cur_over;
            _overspeed_score_flag = true;
          } else if (_overspeed_time < 5.0 * over_speed_times) {
            _overspeed_score_flag = false;
          }
          VLOG_1 << "OVER SPEED........" << "\n";
          _overspeed_time += getModuleStepTime();
          if (_speed <= speed_limit_from_map * 1.3) {
            _cur_over = OVER0;
          } else if (_speed <= speed_limit_from_map * 1.5) {
            _cur_over = OVER30;
          } else {
            _cur_over = OVER50;
          }
        }
      }
      VLOG_1 << "speed limit has checked" << _speed << "\n";

      /* cross solid line check */
      {
        eval::LaneBoundry boundry_on;
        bool _cross_solid_line =
            ego_front->IsOnSolidBoundry(boundry_on) || ego_front->IsOnSolidSolidBoundry(boundry_on);
        if (!_cross_solid_line && ego_trailer)
          _cross_solid_line = ego_trailer->IsOnSolidBoundry(boundry_on) || ego_front->IsOnSolidSolidBoundry(boundry_on);
        if (!_event_solid_line && _cross_solid_line) {
          _event_solid_line = true;
          _violate_times++;
          _solid_line_score += 2;
          VLOG_1 << "a solid line is covered." << "\n";
        }
        if (!_cross_solid_line) {
          _event_solid_line = false;
          VLOG_1 << "the soild line is leave or not on soild line." << "\n";
        }
      }

      /* driving not as right lane; */
      {
        hadmap::txLaneLinks lanelinks = map_info->m_active_lane.m_next_lanelinks;
        for (int i : lane_phase) {
          VLOG_1 << "lane phase: " << i << "\n";
        }
        if (lanelinks.size() != 0) {
          std::vector<int> tmp;
          lane_phase.swap(tmp);
        }
        for (auto it : lanelinks) {
          double startYaw = 0;
          if (it->getGeometry() && !it->getGeometry()->empty()) {
            const hadmap::txLineCurve *pLinecurve = dynamic_cast<const hadmap::txLineCurve *>(it->getGeometry());
            startYaw = pLinecurve->yaw(0);
            int nSize = pLinecurve->size();
            double endYaw = pLinecurve->yaw(pLinecurve->size() - 2);
            double yawDiff = (endYaw - startYaw);
            if (yawDiff < -180.0) {
              yawDiff += 360.0;
            }
            if (yawDiff > 180) {
              yawDiff -= 360.0;
            }
            int phase = 0;
            if (std::abs(yawDiff) < 45.0) {
              phase = 0;  // "T"
            } else if (yawDiff >= 45 && yawDiff < 150) {
              phase = 1;  // "L";
            } else if (yawDiff >= 150 && yawDiff <= 180) {
              phase = 2;  // "L0";
            } else if (yawDiff > (-150) && yawDiff <= (-45)) {
              phase = 3;  // "R";
            } else {
              phase = 4;  // "R0";
            }
            lane_phase.emplace_back(phase);
            VLOG_1 << "allow dir include: " << phase << "\n";
          }
        }
        // _map_mgr->
        VehBehavType behav = helper.GetVehStateFlow().GetOutput().m_veh_behav;
        VLOG_1 << "behav: " << behav << "\n";
        auto lane_ptr = map_info->m_active_lane.m_lane;
        if (map_info) {
          if (!_event_err_lane && !map_info->m_on_road) {
            // before_lane_phase = lane_phase;
            std::vector<int> tmp;
            before_lane_phase.swap(tmp);
            before_lane_phase.assign(lane_phase.begin(), lane_phase.end());
            _event_err_lane = true;
          }
          bool _lane_res = true;
          if (_event_err_lane && map_info->m_on_road) {
            for (int i : before_lane_phase) {
              VLOG_1 << "before lane: " << i << "\n";
            }
            if (behav == VehicleBehavior::TurnLeft) {
              _lane_res = std::find(before_lane_phase.begin(), before_lane_phase.end(), 1) != before_lane_phase.end();
            } else if (behav == VehicleBehavior::TurnRight) {
              _lane_res = std::find(before_lane_phase.begin(), before_lane_phase.end(), 3) != before_lane_phase.end();
            } else if (behav == VehicleBehavior::U_TurnLeft) {
              _lane_res = std::find(before_lane_phase.begin(), before_lane_phase.end(), 2) != before_lane_phase.end();
            } else if (behav == VehicleBehavior::U_TurnRight) {
              _lane_res = std::find(before_lane_phase.begin(), before_lane_phase.end(), 4) != before_lane_phase.end();
            } else if (behav == VehicleBehavior::Driving) {
              _lane_res = std::find(before_lane_phase.begin(), before_lane_phase.end(), 0) != before_lane_phase.end();
            }
            VLOG_1 << _lane_res << "\n";
            if (!_lane_res) {
              _error_lane_score += 2;
              _violate_times++;
            }
            _event_err_lane = false;
          }
        }
      }
      /* rush trffic red light */
      {
        SignalLightActorList traffic_light_ptrs =
            _actor_mgr->GetFellowActorsByType<CTrafficLightActorPtr>(Actor_SignalLight);
        if (traffic_light_ptrs.size() > 0) {
          // get ego lane id
          const LaneID &ego_lane_id = ego_front->GetLaneID();
          // if signal light of ego lane is red
          for (auto signal_light_ptr : traffic_light_ptrs) {
            const ControlLanes &control_lanes = signal_light_ptr->GetControlLanes();
            for (auto i = 0; i < control_lanes.size(); ++i) {
              const LaneID &control_lane = control_lanes.at(i);
              // find control lane of ego lane
              if (ego_lane_id.IsLaneValid() && ego_lane_id.Equal(control_lane)) {
                cur_light = signal_light_ptr->GetSignalLightColor();
                break;
              }
            }
          }
        }
        if (map_info) {
          if (!_event_red_light && !map_info->m_on_road && cur_light == RED) {
            // _rush_score += 3;
            _violate_times++;
            _event_red_light = true;
            VLOG_1 << "ego car is not on road and is stopped." << "\n";
            // }else if(!_event_red_light && !map_info->m_on_road && _speed>1.0 && cur_light == RED){
            //   _rush_score += 6;
            //   _violate_times++;
            //   _event_red_light = true;
            //   VLOG_1<<"ego car rush the red light."<<"\n";
            // }else if(map_info->m_on_road){
            //   _event_red_light = false;
          }
          VLOG_1 << "is on road:" << map_info->m_on_road << "\n";
          if (_event_red_light && !map_info->m_on_road) {
            if (_speed <= 0.56) {
              _event_red_light = false;
              cur_light = GREEN;
              _rush_score += 2;
            }
          } else if (_event_red_light && map_info->m_on_road) {
            _event_red_light = false;
            cur_light = GREEN;
            _rush_score += 6;
          }
        }
      }
      _detector.Detect(_violate_times, 1);
      double i_compliance = _rush_score + _speed_limit_score + _error_lane_score + _solid_line_score;
      // add data to xy-pot
      if (isReportEnabled()) {
        _s_compliance_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_compliance_plot.mutable_y_axis()->at(0).add_axis_data(_violate_times);
        _s_compliance_plot.mutable_y_axis()->at(1).add_axis_data(i_compliance);
        _s_compliance_plot.mutable_y_axis()->at(2).add_axis_data(_rush_score);
        _s_compliance_plot.mutable_y_axis()->at(3).add_axis_data(_speed_limit_score);
        _s_compliance_plot.mutable_y_axis()->at(4).add_axis_data(_error_lane_score);
        _s_compliance_plot.mutable_y_axis()->at(5).add_axis_data(_solid_line_score);
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
bool EvalCompliance::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_compliance_plot);
  }
  return true;
}

void EvalCompliance::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalCompliance::IsEvalPass() {
  if (m_KpiEnabled) {
    /**
     * set detected count
     */
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "traffic compliance check fail");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "traffic compliance check pass");
    }
  }
  return EvalResult(sim_msg::TestReport_TestState_PASS, "traffic compliance check skipped");
}

bool EvalCompliance::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "traffic compliance check unpass";
  /**
   *  set request stop
   */
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
