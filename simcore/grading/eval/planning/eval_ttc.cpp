// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_ttc.h"
#include <cmath>
#include "common/coord_trans.h"
#include "location.pb.h"
#include "traffic.pb.h"

namespace eval {
const char EvalTTC::_kpi_name[] = "MinAveragerTimeHeadway";

sim_msg::TestReport_XYPlot s_ttc_plot;

EvalTTC::EvalTTC() {
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
  _ttc = 0.0;
}
bool EvalTTC::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_ttc_plot, "time to collision", "", "t", "s", {"ttc"}, {"s"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_ttc_plot, "", 0, 1, INT32_MAX, "thresh lower", 1, 1, m_defaultThreshDouble);
  }

  m_pre_loc_payload = "";
  m_pre_traffic_payload = "";
  m_first_loc = true;
  m_first_traffic = true;

  return true;
}
bool EvalTTC::Step(eval::EvalStep &helper) {
  // grading msg
  sim_msg::Grading &grading = helper.GetGradingMsg();
  grading.mutable_event_detector()->set_timeheadaway_below_thresh(
      sim_msg::Grading_EventDetector_EventState_EventNotDetected);

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // calculate exp data
    CalLegacyGradingExpData(helper);

    _ttc = const_max_ttc * 1.5;
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front) {
      CPosition ego_milldle_front = ego_front->TransMiddleFront2BaseCoord();
      auto fellow = helper.GetLeadingVehicle();
      if (fellow) {
        CPosition fellow_milldle_rear = fellow->TransMiddleRear2BaseCoord();

        double speed_ego = ego_front->GetSpeed().GetNormal2D();
        double speed_fellow = fellow->GetSpeed().GetNormal2D();

        EVector3d ego_2_fellow = CEvalMath::Sub(fellow_milldle_rear, ego_milldle_front);

        double dist_relative = ego_2_fellow.GetNormal2D();
        double speed_relative = speed_fellow - speed_ego;
        if (speed_relative < 0.0) {
          _ttc = std::abs(dist_relative / speed_relative);
        }
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
    // detector
    if (_detector.Detect(_ttc, m_defaultThreshDouble)) {
      grading.mutable_event_detector()->set_timeheadaway_below_thresh(
          sim_msg::Grading_EventDetector_EventState_EventDetected);
    }
    VLOG_1 << "_ttc is " << _ttc << " s.\n";

    // add data to xy-pot
    _ttc = _ttc > const_max_ttc ? const_max_ttc : _ttc;
    if (isReportEnabled()) {
      s_ttc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_ttc_plot.mutable_y_axis()->at(0).add_axis_data(_ttc);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalTTC::Stop(eval::EvalStop &helper) {
  helper.SetFeedback("TTC", std::to_string(_detector.GetCount()));

  // add report
  if (isReportEnabled()) {
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_ttc_plot);
  }

  return true;
}

EvalResult EvalTTC::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "ttc too low");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "ttc check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "ttc check skipped");
}
bool EvalTTC::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "ttc too low";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalTTC::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_timeheadway()->set_ath(_ttc);
  msg.mutable_timeheadway()->set_state(sim_msg::GRADING_TIMEHEADWAY_NORMAL);
  if (_detector.GetLatestState()) {
    msg.mutable_timeheadway()->set_state(sim_msg::GRADING_TIMEHEADWAY_OVERLOW);
  }
  msg.mutable_timeheadway()->set_threshold(m_defaultThreshDouble);

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalTTC::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_tooclosettc()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_tooclosettc()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_tooclosettc()->set_is_pass(IsEvalPass()._state ==
                                                                 sim_msg::TestReport_TestState_PASS);
}
void EvalTTC::CalLegacyGradingExpData(eval::EvalStep &helper) {
  std::string loc_payload = _msg_mgr->Get(topic::LOCATION).GetPayload();
  std::string traffic_payload = _msg_mgr->Get(topic::TRAFFIC).GetPayload();

  sim_msg::Grading &grading_msg = helper.GetGradingMsg();
  double dt = 0.02;  // 0.02 ms

  if (loc_payload.size() > 0) {
    if (m_first_loc) {
      m_first_loc = false;
    } else if (loc_payload != m_pre_loc_payload) {
      sim_msg::Location pre_loc, cur_loc;

      if (pre_loc.ParseFromString(m_pre_loc_payload) && cur_loc.ParseFromString(loc_payload)) {
        double ori_longti = pre_loc.position().x();
        double ori_lati = pre_loc.position().y();
        double ori_alti = pre_loc.position().z();
        double x = cur_loc.position().x();
        double y = cur_loc.position().y();
        double z = cur_loc.position().z();

        coord_trans_api::lonlat2enu(x, y, z, ori_longti, ori_lati, ori_alti);

        double dot_x = x / dt;
        double dot_y = y / dt;
        double dot_s = std::hypot(dot_x, dot_y);

        grading_msg.mutable_expdata()->set_dot_s_ego(dot_s);
        grading_msg.mutable_expdata()->set_dot_x_ego(dot_x);
        grading_msg.mutable_expdata()->set_dot_y_ego(dot_y);
        VLOG_2 << "dot_s_ego:" << dot_s << ", dot_x_ego:" << dot_x << ", dot_y_ego:" << dot_y << "\n";
      }
    }

    // update payload
    m_pre_loc_payload = loc_payload;
  }

  if (traffic_payload.size() > 0) {
    if (m_first_traffic) {
      m_first_traffic = false;
    } else if (traffic_payload != m_pre_traffic_payload) {
      sim_msg::Traffic pre_traffic, cur_traffic;

      if (pre_traffic.ParseFromString(m_pre_traffic_payload) && cur_traffic.ParseFromString(traffic_payload)) {
        // clear
        grading_msg.mutable_expdata()->clear_dot_s_fellow();
        grading_msg.mutable_expdata()->clear_dot_x_fellow();
        grading_msg.mutable_expdata()->clear_dot_y_fellow();

        for (auto i = 0; i < cur_traffic.cars_size(); ++i) {
          const sim_msg::Car &car = cur_traffic.cars().at(i);

          // find car by id
          for (auto j = 0; j < pre_traffic.cars_size(); ++j) {
            const sim_msg::Car &car_pre = pre_traffic.cars().at(j);

            if (car.id() == car_pre.id()) {
              double ori_longti = car_pre.x();
              double ori_lati = car_pre.y();
              double ori_alti = car_pre.z();
              double x = car.x();
              double y = car.y();
              double z = car.z();

              coord_trans_api::lonlat2enu(x, y, z, ori_longti, ori_lati, ori_alti);

              double dot_x = x / dt;
              double dot_y = y / dt;
              double dot_s = std::hypot(dot_x, dot_y);

              auto dot_s_fellow = grading_msg.mutable_expdata()->mutable_dot_s_fellow()->Add();
              auto dot_x_fellow = grading_msg.mutable_expdata()->mutable_dot_x_fellow()->Add();
              auto dot_y_fellow = grading_msg.mutable_expdata()->mutable_dot_y_fellow()->Add();

              dot_s_fellow->set_id(car.id());
              dot_s_fellow->set_value(dot_s);

              dot_x_fellow->set_id(car.id());
              dot_x_fellow->set_value(dot_x);

              dot_y_fellow->set_id(car.id());
              dot_y_fellow->set_value(dot_y);

              VLOG_2 << "id:" << car.id() << ", dot_s:" << dot_s << ", dot_x:" << dot_x << ", dot_y:" << dot_y << "\n";
            }
          }
        }
      }
    }

    // update payload
    m_pre_traffic_payload = traffic_payload;
  }
}
}  // namespace eval
