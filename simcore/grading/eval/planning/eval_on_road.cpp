// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_on_road.h"
#include <cmath>
#include "common/coord_trans.h"

namespace eval {
const char EvalOnRoad::_kpi_name[] = "OnRoad";

sim_msg::TestReport_XYPlot EvalOnRoad::_s_on_lane_plot;

EvalOnRoad::EvalOnRoad() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _is_on_lane = true;
}

bool EvalOnRoad::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(_s_on_lane_plot, " on lane", "", "t", "s", {"on wrong lane"}, {"N/A"}, 1);
    ReportHelper::ConfigXYPlotThreshold(_s_on_lane_plot, "detect count upper", 0, 1, m_Kpi.passcondition().value(), "",
                                        1, 0, INT32_MIN, 0);
  }
  return true;
}

bool EvalOnRoad::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // calculate exp data
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      // get the map information pointer and check whether the pointer is null
      auto map_info = ego_front->GetMapInfo();
      if (map_info.get() == nullptr) {
        VLOG_1 << "map info missing\n";
        return false;
      }
      hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
      // step 1
      if (ego_lane) {
        hadmap::LANE_TYPE lane_type = ego_lane->getLaneType();
        hadmap::txLaneId lane_id = ego_lane->getTxLaneId();
        VLOG_1 << "[EvalOnRoad] lane type: " << lane_type << " lane id: " << lane_id << "\n";
        // step 2
        if (!(lane_type == hadmap::LANE_TYPE_Driving || lane_type == hadmap::LANE_TYPE_Parking ||
              lane_type == hadmap::LANE_TYPE_BIDIRECTIONAL || lane_type == hadmap::LANE_TYPE_CONNECTINGRAMP ||
              lane_type == hadmap::LANE_TYPE_ENTRY || lane_type == hadmap::LANE_TYPE_EXIT ||
              lane_type == hadmap::LANE_TYPE_MEDIAN || lane_type == hadmap::LANE_TYPE_OFFRAMP ||
              lane_type == hadmap::LANE_TYPE_ONRAMP)) {
          _is_on_lane = false;
        } else {
          _is_on_lane = true;
        }
      } else {
        _is_on_lane = map_info->m_in_junction;
      }

      // detector
      _detector.Detect(_is_on_lane, 0.5);
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }

    // add data to xy-pot
    if (isReportEnabled()) {
      _s_on_lane_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      _s_on_lane_plot.mutable_y_axis()->at(0).add_axis_data(!_is_on_lane);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalOnRoad::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), _s_on_lane_plot);
  }
  return true;
}

void EvalOnRoad::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalOnRoad::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "driving in the wrong lane");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "driving lane check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "driving lane check skipped");
}

bool EvalOnRoad::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "driving in the wrong lane";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
