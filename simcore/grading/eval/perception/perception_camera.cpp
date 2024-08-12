// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "perception_camera.h"
#include "osi_sensordata.pb.h"

#include "perception_helper.h"

namespace eval {
const char EvalPerceptionCamera::_kpi_name[] = "Perception_Camera";
Define_Perception_Attribute(Perception_Camera, x);
Define_Perception_Attribute(Perception_Camera, y);
Define_Perception_Attribute(Perception_Camera, vx);
Define_Perception_Attribute(Perception_Camera, vy);
Define_Perception_Attribute(Perception_Camera, ax);
Define_Perception_Attribute(Perception_Camera, ay);
Define_Perception_Attribute(Perception_Camera, yaw);

EvalPerceptionCamera::EvalPerceptionCamera() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalPerceptionCamera::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    auto initPerceptionKPI = [this](eval::EvalInit &helper, const std::string &kpiID) {
      sim_msg::GradingKpiGroup_GradingKpi oneKPI;
      if (helper.getGradingKpiByName(kpiID, oneKPI)) {
        auto acompanyCase = std::make_shared<AccompanyCase>(kpiID, oneKPI, true);
        if (acompanyCase.get() == nullptr) return;

        m_accompanyCases.push_back(acompanyCase);
        if (isReportEnabled()) {
          ReportHelper::SetCaseInfo(acompanyCase->m_reportCase, oneKPI);
        }
      }
    };

    initPerceptionKPI(helper, g_constPerception_Camera_x);
    initPerceptionKPI(helper, g_constPerception_Camera_x_error);
    initPerceptionKPI(helper, g_constPerception_Camera_y);
    initPerceptionKPI(helper, g_constPerception_Camera_y_error);
    initPerceptionKPI(helper, g_constPerception_Camera_vx);
    initPerceptionKPI(helper, g_constPerception_Camera_vx_error);
    initPerceptionKPI(helper, g_constPerception_Camera_vy);
    initPerceptionKPI(helper, g_constPerception_Camera_vy_error);
    initPerceptionKPI(helper, g_constPerception_Camera_ax);
    initPerceptionKPI(helper, g_constPerception_Camera_ax_error);
    initPerceptionKPI(helper, g_constPerception_Camera_yaw);
    initPerceptionKPI(helper, g_constPerception_Camera_yaw_error);

    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // clean
  m_capture.clear();

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
  }

  return true;
}
bool EvalPerceptionCamera::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get perception msg
    eval::EvalMsg msg = _msg_mgr->Get(PERCEPTION_TOPIC);
    std::string payload = msg.GetPayload();

    if (payload.size() == 0) {
      VLOG_2 << "no payload on topic " << PERCEPTION_TOPIC;
      return true;
    }

    // perception msg
    osi3::SensorData sensor_data;
    if (sensor_data.ParseFromString(payload)) {
      LOG_ERROR << "fail to parse payload on topic " << PERCEPTION_TOPIC;
      return true;
    }

    // get ego
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();

    if (ego_front == nullptr) {
      LOG_WARNING << "ego actor missing.\n";
      return true;
    }

    // get dynamic fellows ground truth
    auto dynamic_fellows =
        _actor_mgr->GetFellowActorsByType<eval::CDynamicActorFLUPtr>(eval::ActorType::Actor_Dynamic_FLU);

    VLOG_2 << "camera tracked obj size:" << sensor_data.moving_object_size();

    // capture track object data
    for (auto i = 0; i < sensor_data.moving_object_size(); ++i) {
      // x dist
      double x = 0.0, x_truth = 0x0;
      double y = 0.0, y_truth = 0x0;
      double vx = 0.0, vx_truth = 0x0;
      double vy = 0.0, vy_truth = 0x0;
      double yaw = 0.0, yaw_truth = 0x0;
      double ax = 0.0, ax_truth = 0x0;
      double ay = 0.0, ay_truth = 0x0;

      // moving obj
      const osi3::DetectedMovingObject &obj = sensor_data.moving_object().at(i);

      // capture step
      uint64_t track_id = static_cast<uint64_t>(obj.header().tracking_id().value());
      TrackedObjPlotPtr tracked_plot;

      // get tracked obj plot
      {
        auto iter = m_capture.find(track_id);
        if (iter == m_capture.end()) {
          tracked_plot = std::make_shared<TrackedObjPlot>();
          m_capture[track_id] = tracked_plot;
        } else {
          tracked_plot = iter->second;
        }
      }

      tracked_plot->tracked_id = track_id;

      VLOG_2 << "camera tracked id:" << track_id;

      // get data
      x = obj.base().position().x();
      y = obj.base().position().y();
      vx = obj.base().velocity().x();
      vy = obj.base().velocity().y();
      yaw = obj.base().orientation().yaw();
      ax = obj.base().acceleration().x();
      ay = obj.base().acceleration().y();

      // get truth of tracked object
      CActorBase flu_truth;
      CActorBase *truth_ptr = eval::GetTruthOfTrackObj<eval::CDynamicActorFLUPtr>(dynamic_fellows, obj);

      if (truth_ptr != nullptr) {
        x_truth = truth_ptr->GetLocation().GetPosition().GetX();
        y_truth = truth_ptr->GetLocation().GetPosition().GetY();
        vx_truth = truth_ptr->GetSpeed().GetX();
        vy_truth = truth_ptr->GetSpeed().GetY();
        yaw_truth = truth_ptr->GetLocation().GetEuler().GetYaw();
        ax_truth = truth_ptr->GetAcc().GetX();
        ay_truth = truth_ptr->GetAcc().GetY();
        VLOG_2 << "ground truth id of tracked object " << track_id << " is " << truth_ptr->GetUUID();
      }

      // add to report
      if (isReportEnabled()) {
        tracked_plot->AddDataToPlot(helper.GetSimTime(), x, x_truth, y, y_truth, vx, vx_truth, vy, vy_truth, yaw,
                                    yaw_truth, ax, ax_truth, ay, ay_truth);
      }
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalPerceptionCamera::Stop(eval::EvalStop &helper) {
  // add report
  VLOG_2 << "camera capture object size " << m_capture.size();

  if (isReportEnabled()) {
    auto addAccompanyReport = [this](const std::string &kpiID, const sim_msg::TestReport_PairData &pairData,
                                     const sim_msg::TestReport_XYPlot &plot) {
      auto caseReport = m_accompanyCases.getReportByKpiID(kpiID);
      if (caseReport != nullptr) {
        auto attach = caseReport->add_steps()->add_attach();
        ReportHelper::AddPair2Attach(*attach, pairData);
        ReportHelper::AddXYPlot2Attach(*attach, plot);
      }
    };
    for (auto iter = m_capture.begin(); iter != m_capture.end(); ++iter) {
      TrackedObjPlotPtr tracked = iter->second;
      auto axis_data_size = tracked->x_plot.x_axis().axis_data_size();
      if (axis_data_size >= 2) {
        double duration =
            tracked->x_plot.x_axis().axis_data().at(axis_data_size - 1) - tracked->x_plot.x_axis().axis_data().at(0);
        VLOG_2 << "camera id:" << tracked->tracked_id << ", duraiton:" << duration;
        if (duration >= tracked->const_time_duration_thresh) {
          auto attach = _case.add_steps()->add_attach();

          std::string attach_name = "camera tracked object info, id:" + std::to_string(tracked->tracked_id);
          attach->set_name(attach_name.c_str());

          sim_msg::TestReport_PairData track_id_pair;
          track_id_pair.set_key("camera tracked object id");
          track_id_pair.set_value(std::to_string(tracked->tracked_id));

          ReportHelper::AddPair2Attach(*attach, track_id_pair);
          ReportHelper::AddXYPlot2Attach(*attach, tracked->x_plot);
          ReportHelper::AddXYPlot2Attach(*attach, tracked->y_plot);
          ReportHelper::AddXYPlot2Attach(*attach, tracked->vx_plot);
          ReportHelper::AddXYPlot2Attach(*attach, tracked->vy_plot);
          ReportHelper::AddXYPlot2Attach(*attach, tracked->yaw_plot);

          addAccompanyReport(g_constPerception_Camera_x, track_id_pair, tracked->x_plot);
          addAccompanyReport(g_constPerception_Camera_x_error, track_id_pair, tracked->x_error_plot);
          addAccompanyReport(g_constPerception_Camera_y, track_id_pair, tracked->y_plot);
          addAccompanyReport(g_constPerception_Camera_y_error, track_id_pair, tracked->y_error_plot);
          addAccompanyReport(g_constPerception_Camera_vx, track_id_pair, tracked->vx_plot);
          addAccompanyReport(g_constPerception_Camera_vx_error, track_id_pair, tracked->vx_error_plot);
          addAccompanyReport(g_constPerception_Camera_vy, track_id_pair, tracked->vy_plot);
          addAccompanyReport(g_constPerception_Camera_vy_error, track_id_pair, tracked->vy_error_plot);
          addAccompanyReport(g_constPerception_Camera_ax, track_id_pair, tracked->ax_plot);
          addAccompanyReport(g_constPerception_Camera_ax_error, track_id_pair, tracked->ax_error_plot);
          addAccompanyReport(g_constPerception_Camera_ay, track_id_pair, tracked->ay_plot);
          addAccompanyReport(g_constPerception_Camera_ay_error, track_id_pair, tracked->ay_error_plot);
          addAccompanyReport(g_constPerception_Camera_yaw, track_id_pair, tracked->yaw_plot);
          addAccompanyReport(g_constPerception_Camera_yaw_error, track_id_pair, tracked->yaw_error_plot);
        } else {
          VLOG_2 << "object ignored, id:" << tracked->tracked_id << ", duration less than 1 s";
        }
      } else {
        VLOG_2 << "object ignored, id:" << tracked->tracked_id << ", axis data less than 2";
      }
    }
  }
  return true;
}

EvalResult EvalPerceptionCamera::IsEvalPass() {
  return EvalResult(sim_msg::TestReport_TestState_PASS, "Perception Camera Capture Finished");
}
bool EvalPerceptionCamera::ShouldStopScenario(std::string &reason) { return false; }
}  // namespace eval
