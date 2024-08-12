/**
 * @file SensorTruth.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "sensor_truth.h"
#include <google/protobuf/util/json_util.h>
#include <stdio.h>
#include <Eigen/Core>
#include <fstream>
#include <functional>
#include <iostream>
#include "fov_filter_sensor.h"
#include "hmap.h"
#include "object_size.h"
#include "osi_util.h"
#include "user_topic.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "osi_datarecording.pb.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_predef.h"
#include "traffic.pb.h"
#include "union.pb.h"

/**
 * @brief sensortruth init
 *
 * @param helper helper of tx_sim
 */
void SensorTruth::Init(tx_sim::InitHelper &helper) {
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kLocation);

  helper.Publish(std::string(tx_sim::user_topic::kOsiSensorDatas) + (device.empty() ? "" : "_") + device);

  noise.initDrop(helper.GetParameter("NoiseDrop"));
  noise.initStay(helper.GetParameter("NoiseStay"));
  noise.initPosition(helper.GetParameter("NoisePosition"));
  noise.initSize(helper.GetParameter("NoiseSize"));
  noise.initVeolity(helper.GetParameter("NoiseVeolity"));
}

/**
 * @brief sensortruth reset
 *
 * @param helper helper of tx_sim
 */
void SensorTruth::Reset(tx_sim::ResetHelper &helper) { ages.clear(); }

/**
 * @brief sensortruth step
 *
 * @param helper helper of tx_sim
 */
void SensorTruth::Step(tx_sim::StepHelper &helper) {
  // If there are no FOVs defined, return immediately.
  if (fovfilters.empty()) {
    return;
  }
  // Get subscribed message from Traffic topic.
  sim_msg::Traffic traffic = getTraffic(helper);
  sim_msg::Union union_location = getUnion(helper);
  sim_msg::Location loc = getLocation(helper);
  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);

  double time_stamp = loc.t() * 1000;
  // Create an empty list of sensor data series.
  osi3::SensorDataSeriesList sensordatasl;
  // Print log statement.
  std::cout << "[SensorTruth(" << time_stamp << ")]: ";

  // Iterate through each FOV filter.
  for (const auto &fov : fovfilters) {
    // Add new sensor data series to the list.
    osi3::SensorData *sensordata = sensordatasl.add_sensor()->add_sensor_data();
    // Set timestamp in seconds and nanoseconds.
    sensordata->mutable_timestamp()->set_seconds(time_stamp * 0.001);
    sensordata->mutable_timestamp()->set_nanos(std::fmod(time_stamp, 1000) * 1000000);
    // Set sensor ID value.
    sensordata->mutable_sensor_id()->set_value(fov.second->ID());

    // Update FOV's position and rotation.
    fov.second->setCarPosition(locPosition);
    fov.second->setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());
    // handle other ego
    ages[fov.first][0].NewYear(helper.timestamp());
    ages[fov.first][1].NewYear(helper.timestamp());
    ages[fov.first][2].NewYear(helper.timestamp());
    ages[fov.first][3].NewYear(helper.timestamp());

    handle_ego(*fov.second, union_location, sensordata, time_stamp);
    handle_traffic(*fov.second, traffic, sensordata, time_stamp);

    // Increment year counter for aging purposes.
    ages[fov.first][0].YearEnd();
    ages[fov.first][1].YearEnd();
    ages[fov.first][2].YearEnd();
    ages[fov.first][3].YearEnd();

    // sort all objects by distance
    std::sort(sensordata->mutable_moving_object()->begin(), sensordata->mutable_moving_object()->end(),
              [](const osi3::DetectedMovingObject &s1, const osi3::DetectedMovingObject &s2) -> bool {
                auto pos1 = s1.base().position();
                auto pos2 = s2.base().position();
                return (pos1.x() * pos1.x() + pos1.y() * pos1.y() + pos1.z() * pos1.z()) <
                       (pos2.x() * pos2.x() + pos2.y() * pos2.y() + pos2.z() * pos2.z());
              });

    std::cout << "fov[" << fov.second->ID() << "] with " << sensordata->moving_object().size() << " move_objects, "
              << sensordata->stationary_object().size() << " stationary_objects, " << sensordata->traffic_light().size()
              << " trafficlight_objects, ";
  }
  std::cout << "\n";
  std::string buf;
  sensordatasl.SerializeToString(&buf);
  // publish message
  helper.PublishMessage(std::string(tx_sim::user_topic::kOsiSensorDatas) + (device.empty() ? "" : "_") + device, buf);
  if (!debugDir.empty()) {
    std::ofstream debug(debugDir + "/sensor_truth_" + std::to_string(helper.timestamp()) + ".json");
    if (debug.good()) {
      std::string json;
      google::protobuf::util::MessageToJsonString(sensordatasl, &json);
      debug << json;
    }
  }
}

/**
 * @brief sensortruth stop
 *
 * @param helper helper of tx sim
 */
void SensorTruth::Stop(tx_sim::StopHelper &helper) { std::cout << "SensorTruth stopped." << std::endl; }

/// @brief handle ego
/// @param fov
/// @param union_location
/// @param sensordata osi msg to add
/// @param tstamp time stamp
void SensorTruth::handle_ego(const FovFilterSensor &fov, const sim_msg::Union &union_location,
                             osi3::SensorData *sensordata, double tstamp) {
  for (const auto &ulocation : union_location.messages()) {
    std::int64_t thisid = std::atoi(ulocation.groupname().substr(ulocation.groupname().length() - 3).c_str());
    if (thisid == ego_id) continue;
    sim_msg::Location uuloction;
    if (uuloction.ParseFromString(ulocation.content())) {
      Eigen::Vector3d ego_position(uuloction.position().x(), uuloction.position().y(), uuloction.position().z());
      // Convert LonLat coordinates into ENU coordinates.
      coord_trans_api::lonlat2enu(ego_position.x(), ego_position.y(), ego_position.z(), map_ori.x, map_ori.y,
                                  map_ori.z);

      // Check if object is visible by current FOV.
      auto &objbb = ego_Bboxes[thisid];
      if (!fov.inFov(objbb.getBboxPts(ego_position, uuloction.rpy().z()))) {
        continue;
      }
      int age = ages[fov.ID()][3](thisid);

      // Generate detected moving objects.
      auto new_mov = sensordata->add_moving_object();
      new_mov->mutable_header()->set_age(age);

      Txsim2OsiDynamicObject(
          uuloction, thisid, new_mov, ages[fov.ID()][3].get_value(thisid),
          [&](double &x, double &y, double &z) {
            x = ego_position.x();
            y = ego_position.y();
            z = ego_position.z();
          },
          [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); });
      // Apply noise reduction logic.
      if (!MovingNoise(new_mov, tstamp, noise) && sensordata->moving_object_size() > 0)
        sensordata->mutable_moving_object()->RemoveLast();
    }
  }
}
/// @brief handle traffic truth
/// @param fov
/// @param traffic
/// @param sensordata
/// @param time_stamp
void SensorTruth::handle_traffic(const FovFilterSensor &fov, const sim_msg::Traffic &traffic,
                                 osi3::SensorData *sensordata, double time_stamp) {
  auto callback_rpy = [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); };
  auto callback_pos = [](const Eigen::Vector3d &pos) {
    return [&](double &x, double &y, double &z) {
      x = pos.x();
      y = pos.y();
      z = pos.z();
    };
  };
  // Process all cars within this FOV.
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos(car.x(), car.y(), car.z());
    // Convert LonLat coordinates into ENU coordinates.
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);

    // Check if object is visible by current FOV.
    auto &objbb = car_Bboxes[car.type()];
    objbb.len = car.length();
    objbb.wid = car.width();
    objbb.hei = car.height();
    if (!fov.inFov(objbb.getBboxPts(pos, car.heading()))) {
      continue;
    }
    int age = ages[fov.ID()][0](car.id());

    // Generate detected moving objects.
    auto mov_object = sensordata->add_moving_object();
    mov_object->mutable_header()->set_age(age);

    Txsim2OsiDynamicObject(car, mov_object, ages[fov.ID()][0].get_value(car.id()), callback_pos(pos), callback_rpy);
    // Apply noise reduction logic.
    if (!MovingNoise(mov_object, time_stamp, noise)) {
      if (sensordata->moving_object_size() > 0) {
        sensordata->mutable_moving_object()->RemoveLast();
      }
    }
  }

  // Process all dynamic obstacles within this FOV.
  for (const auto &obs : traffic.dynamicobstacles()) {
    // Convert LonLat coordinates into ENU coordinates.
    Eigen::Vector3d dyn_pos(obs.x(), obs.y(), obs.z());
    coord_trans_api::lonlat2enu(dyn_pos.x(), dyn_pos.y(), dyn_pos.z(), map_ori.x, map_ori.y, map_ori.z);

    // Find appropriate bounding box category or use default one.
    auto objbb = dyn_Bboxes[obs.type()];
    if (objbb.category < 0) {
      objbb = car_Bboxes[obs.type()];
    }
    if (objbb.category < 0) {
      std::cout << "Cannot find dynamic obstracle type " << obs.type() << std::endl;
      objbb.len = obs.length();
      objbb.wid = obs.width();
      objbb.hei = obs.height();
    }
    // Check if object is visible by current FOV.
    if (!fov.inFov(objbb.getBboxPts(dyn_pos, obs.heading()))) {
      continue;
    }

    int age = ages[fov.ID()][1](obs.id());
    // Generate detected moving objects.
    auto mov_object = sensordata->add_moving_object();
    mov_object->mutable_header()->set_age(age);

    Txsim2OsiDynamicObject(obs, mov_object, ages[fov.ID()][1].get_value(obs.id()), callback_pos(dyn_pos), callback_rpy);

    // Apply noise reduction logic.
    if (!MovingNoise(mov_object, time_stamp, noise)) {
      if (sensordata->moving_object_size() > 0) {
        sensordata->mutable_moving_object()->RemoveLast();
      }
    }
  }

  // Process all static obstacles within this FOV.
  for (const auto &sta : traffic.staticobstacles()) {
    Eigen::Vector3d sta_pos(sta.x(), sta.y(), sta.z());
    // Convert LonLat coordinates into ENU coordinates.
    coord_trans_api::lonlat2enu(sta_pos.x(), sta_pos.y(), sta_pos.z(), map_ori.x, map_ori.y, map_ori.z);
    // Find appropriate bounding box category or use default one.
    auto &objbb = sta_Bboxes[sta.type()];
    objbb.len = sta.length();
    objbb.wid = sta.width();
    objbb.hei = sta.height();
    // Check if object is visible by current FOV.
    if (!fov.inFov(objbb.getBboxPts(sta_pos, sta.heading()))) {
      continue;
    }
    int age = ages[fov.ID()][2](sta.id());

    // Generate detected moving objects.
    auto sta_object = sensordata->add_stationary_object();
    sta_object->mutable_header()->set_age(age);

    Txsim2OsiStationaryObject(sta, sta_object, callback_pos(sta_pos), callback_rpy);

    // Apply noise reduction logic.
    if (!StationaryNoise(sta_object, time_stamp, noise)) {
      if (sensordata->moving_object_size() > 0) {
        sensordata->mutable_moving_object()->RemoveLast();
      }
    }
  }

  // Process all traffic light within this FOV.
  for (const auto &light : traffic.trafficlights()) {
    Eigen::Vector3d lgt_pos(light.x(), light.y(), light.z());
    coord_trans_api::lonlat2enu(lgt_pos.x(), lgt_pos.y(), lgt_pos.z(), map_ori.x, map_ori.y, map_ori.z);
    // Check if object is visible by current FOV.
    if (!fov.inFov(lgt_pos)) {
      continue;
    }

    auto lig_object = sensordata->add_traffic_light();
    Txsim2OsiTrafficLightObject(light, lig_object, callback_pos(lgt_pos), callback_rpy);
  }
}
