/**
 * @file TrafficFilter.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "traffic_filter.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "fov_filter_sensor.h"
#include "user_topic.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "traffic.pb.h"
#include "traffic_filtered.pb.h"

#ifndef DBL_MAX
#  define DBL_MAX 1.7976931348623158e+308  // max value
#endif                                     // !DBL_MAX

void TrafficFilter::Init(tx_sim::InitHelper &helper) {
  std::cout << "Traffic filter init.\n";
  // get user defined initiation parameters.
  // if we defined the parameters in TADSim UI, override the default values
  // here.

  std::string value = helper.GetParameter("FilterSensorIDs");
  if (!value.empty()) {
    std::stringstream ss(value);
    int id;
    while (ss >> id) {
      fovids.insert(id);
    }
  }
  if (fovids.empty()) {
    std::cout << "[trafficfilter]Use all fov sensor." << std::endl;
  } else {
    std::cout << "[trafficfilter]You are going to use sensor: ";
    for (auto id : fovids) {
      std::cout << id << " ";
    }
    std::cout << std::endl;
  }
  value = helper.GetParameter("FilterLimitZ");
  if (!value.empty()) {
    limitZ = atof(value.c_str());
  }
  std::cout << "FilterLimitZ = " << limitZ << std::endl;

  // by subscribe our interested topics, we expect that the two corresponding
  // messages which defined by traffic.proto and location.proto would received
  // in every Step callback.
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Publish(tx_sim::user_topic::kTrafficTilter);

  auto configDir = helper.GetParameter(tx_sim::constant::kInitKeyModuleSharedLibDirectory);
  // auto configDir = helper.GetParameter("_cwd");
}

void TrafficFilter::Reset(tx_sim::ResetHelper &helper) {
  // here we could get some information(e.g. the map which the scenario runs on,
  // suggested local origin coordinate and the destination of the ego car) of
  // the current scenario.
  map_ori = helper.map_local_origin();
  sim_msg::Location start_location;
  // get start location in reset or in first step. both works.
  start_location.ParseFromString(helper.ego_start_location());
  // std::cout << "[trafficfilter]The local origin coordinate of the map: (" <<
  // map_ori.x << ", " << map_ori.y << ", " << map_ori.z << ")" << "\n";
  for (const auto &fov : fovfilters) {
    ages[fov.first][0] = Age();
    ages[fov.first][1] = Age();
    ages[fov.first][2] = Age();
  }
  std::cout << "[trafficfilter]Finially, it`s going to use sensor: ";
  for (const auto &fov : fovfilters) {
    if (!fovids.empty() && fovids.find(fov.first) == fovids.end()) {
      continue;
    }
    std::cout << fov.first << " ";
  }
  std::cout << std::endl;
}

void TrafficFilter::Step(tx_sim::StepHelper &helper) {
  if (fovfilters.empty()) {
    std::cout << "[trafficfilter]no fov filter sensor." << std::endl;
    return;
  }

  double time_stamp = helper.timestamp();
  std::cout << "[trafficfilter]time stamp: " << time_stamp << "\n";

  // 2. get messages we subscribed.
  std::string payload_;
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
  sim_msg::Traffic traffic;
  traffic.ParseFromString(payload_);
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);
  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);

  egoLaneid = hadmap::txLaneId();
  hadmap::txLanePtr lanePtr;
  if (GetEgoLane(hadmap::txPoint(loc.position().x(), loc.position().y(), loc.position().z()), lanePtr)) {
    egoLaneid = lanePtr->getTxLaneId();
  }
  sim_msg::TrafficFiltered out_traffics;

  for (const auto &fov : fovfilters) {
    if (!fovids.empty() && fovids.find(fov.first) == fovids.end()) {
      continue;
    }

    sim_msg::Traffic out_traffic;

    fov.second->setCarPosition(locPosition);
    fov.second->setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());

    handle_traffic(*fov.second, traffic, locPosition, out_traffic);
    ages[fov.first][0].YearEnd();
    ages[fov.first][1].YearEnd();
    ages[fov.first][2].YearEnd();

    sim_msg::TrafficLight phase_light[4];

    bool suc = parse_phase(egoLaneid, traffic, phase_light);
    if (suc) {
      for (int i = 1; i < 4; i++) {
        *out_traffic.add_trafficlights() = phase_light[i];
      }
    }

    std::cout << "[trafficfilter]sensor " << fov.first << " filter out: " << out_traffic.cars().size() << " cars, "
              << out_traffic.dynamicobstacles().size() << " dynamics, " << out_traffic.staticobstacles().size()
              << " statics, "
              << "trafficlight(color)= ";
    if (out_traffic.trafficlights().empty()) {
      std::cout << "None\n";
    } else {
      std::cout << out_traffic.trafficlights()[0].color() << " " << out_traffic.trafficlights()[1].color() << " "
                << out_traffic.trafficlights()[2].color() << "\n";
    }

    (*out_traffics.mutable_traffic())[fov.first] = out_traffic;
  }

  out_traffics.SerializeToString(&payload_);
  helper.PublishMessage(tx_sim::user_topic::kTrafficTilter, payload_);
}

void TrafficFilter::Stop(tx_sim::StopHelper &helper) { std::cout << "[trafficfilter] module stopped." << std::endl; }

/// @brief
/// @param fov
/// @param traffic
/// @param locPosition
/// @param out_traffic
void TrafficFilter::handle_traffic(const FovFilterSensor &fov, const sim_msg::Traffic &traffic,
                                   const Eigen::Vector3d &locPosition, sim_msg::Traffic &out_traffic) {
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos(car.x(), car.y(), car.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
      continue;
    }
    if (!fov.inFov(pos)) {
      continue;
    }
    auto newdata = out_traffic.add_cars();
    *newdata = car;
    newdata->set_age(ages[fov.ID()][0](car.id()));
  }

  for (const auto &obs : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
      continue;
    }
    if (!fov.inFov(pos)) {
      continue;
    }
    auto newdata = out_traffic.add_dynamicobstacles();
    *newdata = obs;
    newdata->set_age(ages[fov.ID()][1](obs.id()));
  }

  for (const auto &sta : traffic.staticobstacles()) {
    Eigen::Vector3d pos(sta.x(), sta.y(), sta.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
      continue;
    }
    if (!fov.inFov(pos)) {
      continue;
    }
    auto newdata = out_traffic.add_staticobstacles();
    *newdata = sta;
    newdata->set_age(ages[fov.ID()][2](sta.id()));
  }
}

/// @brief
/// @param tganeid
/// @param traffic
/// @param phase_light
/// @return
bool TrafficFilter::parse_phase(const hadmap::txLaneId &tganeid, const sim_msg::Traffic &traffic,
                                sim_msg::TrafficLight phase_light[]) {
  if (tganeid.laneId == 0) {
    return false;
  }
  bool suc = false;
  for (const auto &mtl : traffic.trafficlights()) {
    bool islane = false;
    for (const auto &llane : mtl.control_lanes()) {
      if (llane.tx_road_id() == tganeid.roadId /* && llane.tx_section_id() == tganeid.sectionId*/) {
        islane = true;
        suc = true;
        break;
      }
    }
    if (islane) {
      for (const auto &p : mtl.control_phases()) {
        phase_light[p] = mtl;
      }
    }
  }
  return suc;
}
