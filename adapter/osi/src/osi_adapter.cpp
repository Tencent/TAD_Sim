// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "osi_adapter.h"
#include <cmath>
#include "common/constants.h"
#include "common/log.h"

#include "detectedlane.pb.h"
#include "environment.pb.h"
#include "traffic.pb.h"

#include "osi_environment.pb.h"
#include "osi_object.pb.h"
#include "osi_sensordata.pb.h"
#include "osi_trafficlight.pb.h"

#define SPLIT_LINE "========================================\n"

namespace adapter {
OSIAdapter::OSIAdapter() {
  _t_ms = 0;
  VLOG_0 << "OSIAdapter constructed.\n";
}
OSIAdapter::~OSIAdapter() {}

void OSIAdapter::Init(tx_sim::InitHelper &helper) {
  // set vlog level
  std::string glog_level = helper.GetParameter("_log_level");
  if (!glog_level.empty()) {
    FLAGS_v = std::atoi(glog_level.c_str());
  }

  // pub topics
  helper.Subscribe(std::string(topic::DETECTED_LINES));
  helper.Subscribe(std::string(topic::ENVIRONMENTAL));
  helper.Subscribe(std::string(topic::TRAFFIC));
  helper.Subscribe(std::string(topic::LOCATION));
  helper.Publish(std::string(topic::OSI_SENSORDATA));
  helper.Publish(std::string(topic::OSI_ENVIRONMENT));
  helper.Publish(std::string(topic::OSI_GROUNDTRUTH));
  helper.Publish(std::string(topic::OSI_HOSTVEHICLEDATA));
}

void OSIAdapter::Reset(tx_sim::ResetHelper &helper) { VLOG_0 << "osi adapter reset." << std::endl; }

void OSIAdapter::Step(tx_sim::StepHelper &helper) { Transform(helper); }

void OSIAdapter::Stop(tx_sim::StopHelper &helper) {
  VLOG_1 << SPLIT_LINE;
  VLOG_0 << "...Finished\n";
}

void OSIAdapter::Transform(tx_sim::StepHelper &helper) {
  VLOG_1 << SPLIT_LINE << std::endl;
  VLOG_0 << "start txsim_detected_lines to  osi_sensordata ..." << std::endl;
  // TXSIM_DETECTED_LINES <---> osi_sensordata
  // TadSimMsg sensor_msg = _msg_mgr->Get("TXSIM_DETECTED_LINES");
  // prase pb msg
  std::string txsim_detected_lines_payload;
  helper.GetSubscribedMessage(std::string(topic::DETECTED_LINES), txsim_detected_lines_payload);

  if (txsim_detected_lines_payload.size() > 0) {
    sim_msg::DetectedLaneBoundarySeries detected_lane_boundary_series;
    detected_lane_boundary_series.ParseFromString(txsim_detected_lines_payload);
    // transform
    osi3::SensorData sensor_data;

    for (auto detected_lane_boundary : detected_lane_boundary_series.left()) {
      auto lane_boundary_ptr = sensor_data.add_lane_boundary();
      for (auto candidate_lane_boundary : detected_lane_boundary.candidate()) {
        auto candidate_ptr = lane_boundary_ptr->add_candidate();
        candidate_ptr->set_probability(candidate_lane_boundary.probability());

        osi3::LaneBoundary::Classification classification;
        // color int32  [0,2] 黄=2/白=1
        switch (candidate_lane_boundary.color()) {
          case 1:
            classification.set_color(
                osi3::LaneBoundary::Classification::Color::LaneBoundary_Classification_Color_COLOR_WHITE);
            break;
          case 2:
            classification.set_color(
                osi3::LaneBoundary::Classification::Color::LaneBoundary_Classification_Color_COLOR_UNKNOWN);
            break;
          default:
            classification.set_color(
                osi3::LaneBoundary::Classification::Color::LaneBoundary_Classification_Color_COLOR_UNKNOWN);
            break;
        }

        // double_line_type int32  [0,4] 内虚外实=1，内实外虚=2，双实=3，双虚=4
        /*
         * OSI V3.5.0
         * There is no special representation for double lines,e.g. solid /solid or dashed / solid.
         * In such cases, each lane will define its own side of the lane boundary.
         */

        candidate_ptr->mutable_classification()->CopyFrom(classification);
      }
    }
    for (auto detected_lane_boundary : detected_lane_boundary_series.right()) {
      auto lane_boundary_ptr = sensor_data.add_lane_boundary();
      for (auto candidate_lane_boundary : detected_lane_boundary.candidate()) {
        auto candidate_ptr = lane_boundary_ptr->add_candidate();
        candidate_ptr->set_probability(candidate_lane_boundary.probability());

        osi3::LaneBoundary::Classification classification;
        // color int32  [0,2] 黄=2/白=1
        switch (candidate_lane_boundary.color()) {
          case 1:
            classification.set_color(
                osi3::LaneBoundary::Classification::Color::LaneBoundary_Classification_Color_COLOR_WHITE);
            break;
          case 2:
            classification.set_color(
                osi3::LaneBoundary::Classification::Color::LaneBoundary_Classification_Color_COLOR_UNKNOWN);
            break;
          default:
            classification.set_color(
                osi3::LaneBoundary::Classification::Color::LaneBoundary_Classification_Color_COLOR_UNKNOWN);
            break;
        }
        candidate_ptr->mutable_classification()->CopyFrom(classification);
      }
    }
    // publish msg
    std::string osi_sensordata_payload;
    sensor_data.SerializeToString(&osi_sensordata_payload);
    helper.PublishMessage(std::string(topic::OSI_SENSORDATA), osi_sensordata_payload);
    VLOG_0 << "txsim_detected_lines to osi_sensordata transform finished" << std::endl;
    VLOG_1 << "txsim_detected_lines_payload = " << detected_lane_boundary_series.DebugString() << std::endl;
    VLOG_1 << "osi_sensordata_payload = " << sensor_data.DebugString() << std::endl;
  }

  VLOG_1 << SPLIT_LINE << std::endl;
  VLOG_0 << "start environment to osi_environment ..." << std::endl;
  // TXSIM_ENVIRONMENT <---> OSI_ENVIRONMENT
  // prase pb msg
  std::string txsim_environment_payload;
  helper.GetSubscribedMessage(std::string(topic::ENVIRONMENTAL), txsim_environment_payload);

  // transform
  if (txsim_environment_payload.size() > 0) {
    sim_msg::EnvironmentalConditions txsim_environment;
    txsim_environment.ParseFromString(txsim_environment_payload);

    osi3::EnvironmentalConditions environment_data;
    environment_data.set_ambient_illumination(
        osi3::EnvironmentalConditions::AmbientIllumination(static_cast<int>(txsim_environment.ambient_illumination())));

    osi3::EnvironmentalConditions::TimeOfDay tod;
    tod.set_seconds_since_midnight(txsim_environment.time_of_day().seconds_since_midnight());
    environment_data.mutable_time_of_day()->CopyFrom(tod);
    environment_data.set_atmospheric_pressure(txsim_environment.atmospheric_pressure());
    environment_data.set_temperature(txsim_environment.temperature());
    environment_data.set_relative_humidity(txsim_environment.relative_humidity());
    // can not transform Precipitation
    // environment_data.set_precipitation(osi3::EnvironmentalConditions
    // // ::Precipitation((int)txsim_environment.precipitation()));
    double vis = txsim_environment.fog().visibility();
    if (vis >= 0.0 && vis < 50.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_DENSE);
    } else if (vis >= 50.0 && vis < 200.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_THICK);
    } else if (vis >= 200.0 && vis < 1000.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_LIGHT);
    } else if (vis >= 1000.0 && vis < 2000.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_MIST);
    } else if (vis >= 2000.0 && vis < 4000.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_POOR_VISIBILITY);
    } else if (vis >= 4000.0 && vis < 10000.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_MODERATE_VISIBILITY);
    } else if (vis >= 10000.0 && vis < 40000.0) {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_GOOD_VISIBILITY);
    } else if (vis >= 40000.0) {
      environment_data.set_fog(
          osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_EXCELLENT_VISIBILITY);
    } else {
      environment_data.set_fog(osi3::EnvironmentalConditions::Fog::EnvironmentalConditions_Fog_FOG_UNKNOWN);
    }
    // publish msg
    std ::string osi_environment_payload;
    environment_data.SerializeToString(&osi_environment_payload);
    helper.PublishMessage(std::string(topic::OSI_ENVIRONMENT), osi_environment_payload);
    VLOG_0 << "environment to osi_environment transform finished" << std::endl;
    VLOG_1 << "txsim_environment_payload = " << txsim_environment.DebugString() << std::endl;
    VLOG_1 << "osi_environment_payload = " << environment_data.DebugString() << std::endl;
  }

  VLOG_1 << SPLIT_LINE << std::endl;
  VLOG_0 << "start traffic to osi_groundtruth ..." << std::endl;
  // TRAFFIC <--->OSI_GROUNDTRUTH (OSI_OBJECT | OSI_TRAFFICLIGHT)
  // prase pb msg
  std::string txsim_traffic_payload;
  helper.GetSubscribedMessage(std::string(topic::TRAFFIC), txsim_traffic_payload);

  // transform
  // OSI_OBJECT
  if (txsim_traffic_payload.size() > 0) {
    sim_msg::Traffic traffic;
    traffic.ParseFromString(txsim_traffic_payload);
    osi3::GroundTruth ground_truth;
    for (auto static_obstacle : traffic.staticobstacles()) {
      auto so_ptr = ground_truth.add_stationary_object();
      osi3::Identifier id;
      id.set_value(static_obstacle.id());
      so_ptr->mutable_id()->CopyFrom(id);

      osi3::Dimension3d dim;
      dim.set_length(static_obstacle.length());
      dim.set_width(static_obstacle.width());
      dim.set_height(static_obstacle.height());

      osi3::Vector3d pos;
      pos.set_x(static_obstacle.x());
      pos.set_y(static_obstacle.y());
      pos.set_z(static_obstacle.z());

      // heading
      osi3::Orientation3d head;
      head.set_yaw(static_obstacle.heading());

      osi3::BaseStationary base;
      base.mutable_dimension()->CopyFrom(dim);
      base.mutable_position()->CopyFrom(pos);
      base.mutable_orientation()->CopyFrom(head);
      so_ptr->mutable_base()->CopyFrom(base);
    }

    for (auto dynamic_obstacle : traffic.dynamicobstacles()) {
      auto mov_obj_ptr = ground_truth.add_moving_object();
      osi3::Identifier id;
      id.set_value(dynamic_obstacle.id());
      mov_obj_ptr->mutable_id()->CopyFrom(id);

      osi3::Dimension3d dim;
      dim.set_length(dynamic_obstacle.length());
      dim.set_width(dynamic_obstacle.width());
      dim.set_height(dynamic_obstacle.height());

      osi3::Vector3d pos;
      pos.set_x(dynamic_obstacle.x());
      pos.set_y(dynamic_obstacle.y());
      pos.set_z(dynamic_obstacle.z());

      osi3::Vector3d vel;
      vel.set_x(dynamic_obstacle.v());
      vel.set_y(dynamic_obstacle.vl());

      osi3::Vector3d acc;
      acc.set_x(dynamic_obstacle.acc());

      // heading
      osi3::Orientation3d head;
      head.set_yaw(dynamic_obstacle.heading());

      osi3::BaseMoving base;
      base.mutable_dimension()->CopyFrom(dim);
      base.mutable_position()->CopyFrom(pos);
      base.mutable_velocity()->CopyFrom(vel);
      base.mutable_acceleration()->CopyFrom(acc);
      base.mutable_orientation()->CopyFrom(head);

      mov_obj_ptr->mutable_base()->CopyFrom(base);
    }

    for (auto car : traffic.cars()) {
      auto c_ptr = ground_truth.add_moving_object();
      osi3::Identifier id;
      id.set_value(car.id());
      c_ptr->mutable_id()->CopyFrom(id);

      osi3::Dimension3d dim;
      dim.set_length(car.length());
      dim.set_width(car.width());
      dim.set_height(car.height());

      osi3::Vector3d pos;
      pos.set_x(car.x());
      pos.set_y(car.y());
      pos.set_z(car.z());

      osi3::Vector3d vel;
      double v = std::sqrt(car.v() * car.v() + car.vl() * car.vl());
      vel.set_x(v * std::cos(car.heading()));
      vel.set_y(v * std::sin(car.heading()));
      vel.set_z(0);

      osi3::Vector3d acc;
      acc.set_x(car.acc() * std::cos(car.heading()));
      acc.set_y(car.acc() * std::sin(car.heading()));
      acc.set_z(0);

      // heading
      osi3::Orientation3d head;
      head.set_roll(0);
      head.set_pitch(0);
      head.set_yaw(car.heading());

      osi3::BaseMoving veh;
      veh.mutable_dimension()->CopyFrom(dim);
      veh.mutable_position()->CopyFrom(pos);
      veh.mutable_velocity()->CopyFrom(vel);
      veh.mutable_acceleration()->CopyFrom(acc);
      veh.mutable_orientation()->CopyFrom(head);
      c_ptr->mutable_base()->CopyFrom(veh);
    }

    // OSI_TRAFFICLIGHT
    for (auto light : traffic.trafficlights()) {
      auto light_ptr = ground_truth.add_traffic_light();
      osi3::Identifier id;
      id.set_value(light.id());
      light_ptr->mutable_id()->CopyFrom(id);

      osi3::Vector3d pos;
      pos.set_x(light.x());
      pos.set_y(light.y());
      pos.set_z(light.z());

      osi3::BaseStationary base;
      base.mutable_position()->CopyFrom(pos);
      light_ptr->mutable_base()->CopyFrom(base);

      osi3::TrafficLight::Classification classification;
      // eGreen = 0, eYellow = 1, eRed = 2, eGrey = 3, eBlack = 4
      switch (light.color()) {
        case 0:
          classification.set_color(
              osi3::TrafficLight::Classification::Color::TrafficLight_Classification_Color_COLOR_GREEN);
          break;
        case 1:
          classification.set_color(
              osi3::TrafficLight::Classification::Color::TrafficLight_Classification_Color_COLOR_YELLOW);
          break;
        case 2:
          classification.set_color(
              osi3::TrafficLight::Classification::Color::TrafficLight_Classification_Color_COLOR_RED);
          break;
        case 3:  // grey color can not find
          classification.set_color(
              osi3::TrafficLight::Classification::Color::TrafficLight_Classification_Color_COLOR_OTHER);
          break;
        case 4:  // black color can not find
          classification.set_color(
              osi3::TrafficLight::Classification::Color::TrafficLight_Classification_Color_COLOR_OTHER);
          break;
        default:
          break;
      }

      for (auto lane : light.control_lanes()) {
        auto lane_id_ptr = classification.add_assigned_lane_id();
        lane_id_ptr->set_value(lane.tx_lane_id());
      }

      light_ptr->mutable_classification()->CopyFrom(classification);
    }

    // publish msg
    std::string osi_ground_truth_payload;
    ground_truth.SerializeToString(&osi_ground_truth_payload);
    helper.PublishMessage(std::string(topic::OSI_GROUNDTRUTH), osi_ground_truth_payload);
    VLOG_0 << "traffic to osi_groundtruth transform finished" << std::endl;
    VLOG_0 << "txsim_traffic_payload size = " << txsim_traffic_payload.size() << std::endl;
    VLOG_0 << "osi_ground_truth_payload size = " << osi_ground_truth_payload.size() << std::endl;
  }

  // LOCATION <--->OSI_HOSTVEHICLEDATA
  // prase pb msg
  std::string txsim_location_payload;
  helper.GetSubscribedMessage(std::string(topic::LOCATION), txsim_location_payload);

  // transform
  if (txsim_location_payload.size() > 0) {
    sim_msg::Location location;
    location.ParseFromString(txsim_location_payload);
    osi3::HostVehicleData hostVehicle;

    // position
    osi3::Vector3d position;
    position.set_x(location.position().x());
    position.set_y(location.position().y());
    position.set_z(location.position().z());

    // velocity
    osi3::Vector3d vel;
    vel.set_x(location.velocity().x());
    vel.set_y(location.velocity().y());
    vel.set_z(location.velocity().z());

    // angular
    osi3::Orientation3d angular;
    angular.set_roll(location.angular().x());
    angular.set_pitch(location.angular().y());
    angular.set_yaw(location.angular().z());

    // rpy
    osi3::Orientation3d rpy;
    rpy.set_roll(location.rpy().x());
    rpy.set_pitch(location.rpy().y());
    rpy.set_yaw(location.rpy().z());

    // acceleration
    osi3::Vector3d acc;
    acc.set_x(location.acceleration().x());
    acc.set_y(location.acceleration().y());
    acc.set_z(location.acceleration().z());

    osi3::BaseMoving baseMovingLocation;
    baseMovingLocation.mutable_position()->CopyFrom(position);
    baseMovingLocation.mutable_velocity()->CopyFrom(vel);
    baseMovingLocation.mutable_orientation_rate()->CopyFrom(angular);
    baseMovingLocation.mutable_orientation()->CopyFrom(rpy);
    baseMovingLocation.mutable_acceleration()->CopyFrom(acc);

    hostVehicle.mutable_location()->CopyFrom(baseMovingLocation);
    // publish msg
    std::string osi_hostvehicledata_payload;
    hostVehicle.SerializeToString(&osi_hostvehicledata_payload);
    helper.PublishMessage(std::string(topic::OSI_HOSTVEHICLEDATA), osi_hostvehicledata_payload);
    VLOG_0 << "location to osi_hostvehicledata transform finished" << std::endl;
    VLOG_1 << "txsim_location_payload = " << location.DebugString() << std::endl;
    VLOG_1 << "osi_hostvehicledata_payload = " << hostVehicle.DebugString() << std::endl;
    VLOG_0 << "txsim_location_payload size = " << txsim_location_payload.size() << std::endl;
    VLOG_0 << "osi_hostvehicledata_payload size = " << osi_hostvehicledata_payload.size() << std::endl;
  }
}
}  // namespace adapter
