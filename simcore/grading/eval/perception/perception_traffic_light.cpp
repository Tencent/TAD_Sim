// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "perception_traffic_light.h"
#include "osi_sensordata.pb.h"

#include "perception_helper.h"

namespace eval {
const char Perception_TrafficLight::_kpi_name[] = "Perception_Camera_TrafficLight";

Perception_TrafficLight::Perception_TrafficLight() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool Perception_TrafficLight::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // clean
  m_traffic_light_capture.clear();

  // set report info
  ReportHelper::SetCaseInfo(_case, m_Kpi);

  return true;
}
bool Perception_TrafficLight::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get perception msg
    eval::EvalMsg msg = _msg_mgr->Get(PERCEPTION_TOPIC);
    std::string payload = msg.GetPayload();

    if (payload.size() > 0) {
      // perception msg
      osi3::SensorData sensor_data;
      if (sensor_data.ParseFromString(payload)) {
        VLOG_2 << "camera tracked traffic light size:" << sensor_data.traffic_light_size();

        // capture traffic light
        for (auto i = 0; i < sensor_data.traffic_light_size(); ++i) {
          const osi3::DetectedTrafficLight &traffic_light = sensor_data.traffic_light().at(i);
          uint64_t traffic_light_id = traffic_light.header().tracking_id().value();
          std::shared_ptr<sim_msg::TestReport_XYPlot> tracked_plot;

          // create tracked obj plot if necessary
          {
            auto iter = m_traffic_light_capture.find(traffic_light_id);
            if (iter == m_traffic_light_capture.end()) {
              tracked_plot = std::make_shared<sim_msg::TestReport_XYPlot>();
              std::string disp_name = "tracked traffic light info with id " + std::to_string(traffic_light_id);
              ReportHelper::ConfigXYPlot(*tracked_plot.get(), disp_name.c_str(), "camera tracked traffic light", "time",
                                         "s", {"traffic light color"}, {"N/A"});
              m_traffic_light_capture[traffic_light_id] = tracked_plot;
            } else {
              tracked_plot = m_traffic_light_capture[traffic_light_id];
            }
          }

          /*
                  enum Color {
                          UNKNOWN = 0;
                          RED = 1;
                          YELLOW = 2;
                          GREEN = 3;
                  }
          */
          uint64_t traffic_light_color = 0;

          if (traffic_light.candidate().size() >= 1) {
            const osi3::TrafficLight_Classification_Color &color =
                traffic_light.candidate().at(0).classification().color();
            if (color == osi3::TrafficLight_Classification_Color_COLOR_RED) {
              traffic_light_color = 1;
            } else if (color == osi3::TrafficLight_Classification_Color_COLOR_YELLOW) {
              traffic_light_color = 2;
            } else if (color == osi3::TrafficLight_Classification_Color_COLOR_GREEN) {
              traffic_light_color = 3;
            }
          }
          if (isReportEnabled()) {
            tracked_plot->mutable_x_axis()->add_axis_data(helper.GetSimTime());
            tracked_plot->mutable_y_axis()->at(0).add_axis_data(traffic_light_color);
          }
          VLOG_2 << "traffic light color " << traffic_light_color << ", with id " << traffic_light_id;
        }
      } else {
        LOG_ERROR << "fail to parse payload on topic " << PERCEPTION_TOPIC;
        return false;
      }
    } else {
      VLOG_2 << "no payload on topic " << PERCEPTION_TOPIC;
      return true;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool Perception_TrafficLight::Stop(eval::EvalStop &helper) {
  // add report
  VLOG_2 << "camera capture traffic light size " << m_traffic_light_capture.size();
  if (isReportEnabled()) {
    for (auto iter = m_traffic_light_capture.begin(); iter != m_traffic_light_capture.end(); ++iter) {
      std::shared_ptr<sim_msg::TestReport_XYPlot> plot = iter->second;
      auto axis_data_size = plot->x_axis().axis_data_size();
      if (axis_data_size >= 2) {
        double duration = plot->x_axis().axis_data().at(axis_data_size - 1) - plot->x_axis().axis_data().at(0);
        VLOG_2 << "camera tracked traffic light id:" << iter->first << ", duraiton:" << duration;
        if (duration >= 1.0) {
          auto attach = _case.add_steps()->add_attach();

          std::string attach_name = "camera tracked traffic light info with id:" + std::to_string(iter->first);
          attach->set_name(attach_name.c_str());

          sim_msg::TestReport_PairData track_id_pair;
          track_id_pair.set_key("camera tracked traffic light id");
          track_id_pair.set_value(std::to_string(iter->first));

          ReportHelper::AddPair2Attach(*attach, track_id_pair);
          ReportHelper::AddXYPlot2Attach(*attach, *(plot.get()));
        } else {
          VLOG_2 << "object ignored, id:" << iter->first << ", duration less than 1 s";
        }
      } else {
        VLOG_2 << "object ignored, id:" << iter->first << ", axis data less than 2";
      }
    }
  }

  return true;
}

EvalResult Perception_TrafficLight::IsEvalPass() {
  return EvalResult(sim_msg::TestReport_TestState_PASS, "Perception Camera TrafficLight Capture Finished");
}
bool Perception_TrafficLight::ShouldStopScenario(std::string &reason) { return false; }
}  // namespace eval
