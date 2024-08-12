// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_curvature.h"
#include <float.h>
#include <algorithm>
#include "common/coord_trans.h" /*coord_trans_api*/
#include "parking_state.pb.h"

namespace eval {
const char EvalParkingCurvature::_kpi_name[] = "ParkingCurvature";
sim_msg::TestReport_XYPlot EvalParkingCurvature::s_parking_curvature_plot;

EvalParkingCurvature::EvalParkingCurvature() {
  _parking_curvature = DBL_MAX;
  VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n";
}

bool EvalParkingCurvature::Init(eval::EvalInit &helper) {
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
    ReportHelper::ConfigXYPlot(s_parking_curvature_plot, "max curvature", "", "t", "s", {"curvature"}, {"rad"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_parking_curvature_plot, "", 0, 0, INT32_MAX, "curvature lower", 1, 0,
                                        m_defaultThreshDouble, 0);
  }
  return true;
}

bool EvalParkingCurvature::Step(eval::EvalStep &helper) {
  double cur_curvature = DBL_MAX;
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    sim_msg::ParkingState parkMsg;
    EvalMsg &&msg = _msg_mgr->Get(topic::PARKING_STATE);
    VLOG_0 << "message size: " << msg.GetPayload().size() << "\n";
    if (msg.GetPayload().size() > 0 && parkMsg.ParseFromString(msg.GetPayload())) {
      VLOG_0 << "PakMsg Info: " << parkMsg.ShortDebugString() << "\n";
      if (sim_msg::APA_test_flag::APA_Parking_in == parkMsg.stage()) {
        // get the ego pointer and check whether the pointer is null
        auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
        if (ego_front) {
          // get the map information pointer and check whether the pointer is null
          auto map_info = ego_front->GetMapInfo();
          if (map_info.get() == nullptr) {
            VLOG_0 << "map info missing\n";
            return false;
          }
          CPosition currentPoint = ego_front->GetLocation().GetPosition();
          // // 1.trans current pos to 84
          _map_mgr->ENUToWGS84(currentPoint);
          hadmap::txLanePtr ego_lane = map_info->m_active_lane.m_lane;
          if (ego_lane) {
            hadmap::txRoadPtr ego_road = map_info->m_active_road.m_road;
            if (ego_road) {
              // 2.calculate s l
              const hadmap::txLineCurve *pLine = dynamic_cast<const hadmap::txLineCurve *>(ego_road->getGeometry());
              double s = 0;
              double l = 0;
              double yaw = 0;
              pLine->xy2sl(currentPoint.GetX(), currentPoint.GetY(), s, l, yaw);
              for (const auto &curvatureV : ego_road->getCurvature()) {
                if (s >= curvatureV.m_starts && s < curvatureV.m_starts + curvatureV.m_length) {
                  cur_curvature = curvatureV.m_curvature;
                  break;
                }
              }
              VLOG_0 << "curvature: " << cur_curvature << " park_curvature: " << _parking_curvature << "\n";
              _parking_curvature = std::min(_parking_curvature, cur_curvature);
              _detector.Detect(cur_curvature, m_defaultThreshDouble);
            }
          }
        }
      }

    } else {
      VLOG_1 << "fail to parse msg, payload size:" << msg.GetPayload().size() << "\n";
    }
    // add data to xy-pot
    if (isReportEnabled()) {
      double result = _parking_curvature == DBL_MAX ? -1 : _parking_curvature;
      s_parking_curvature_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
      s_parking_curvature_plot.mutable_y_axis()->at(0).add_axis_data(result);
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}

bool EvalParkingCurvature::Stop(eval::EvalStop &helper) {
  // set report
  if (isReportEnabled()) {
    // ReportHelper::SetPairData(s_parking_curvature_plot, "ParkingCurvature", result);
    // ReportHelper::AddPair2Attach(*_case.add_steps()->add_attach(), s_parking_curvature_plot);
    ReportHelper::AddXYPlot2Attach(*_case.add_steps()->add_attach(), s_parking_curvature_plot);
  }
  return true;
}

void EvalParkingCurvature::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalParkingCurvature::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "excessive curvature");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "parking curvature check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "parking curvature check skipped");
}

bool EvalParkingCurvature::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "excessive curvature";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
