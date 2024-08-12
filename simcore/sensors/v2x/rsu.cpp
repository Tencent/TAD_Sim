/**
 * @file rsu.cpp
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "rsu.h"
#include <glog/logging.h>
#include <Eigen/Eigen>
#include <random>
#include "asn1msg/asn1msg_map.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "msg.h"
#include "sensors.h"
#include "sim_algorithm/sim_conflict_alg.h"
#include "traffic.pb.h"
#include "union.pb.h"
#include "utils/utils.h"

/**
 * @brief rsu init
 *
 * @param helper helper of txsim
 */
void Rsu::Init(tx_sim::InitHelper &helper) {
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
}

/**
 * @brief rsu reset
 *
 * @param helper helper of txsim
 */
void Rsu::Reset(tx_sim::ResetHelper &helper) {
  SimConfilctAlg::getInstance().PartialClear();
  rsu_timestamp_last.clear();
  rsu_frame_count.clear();
  groupname = helper.group_name();
}

/**
 * @brief rsu setep
 *
 * @param helper helper of txsim
 */
void Rsu::Step(tx_sim::StepHelper &helper) {
  // Call conflict_alg in every step
  SimRsuSensors sim_received_rsus;
  SimConfilctAlg::getInstance().PopReceivedRsuMessage(sim_received_rsus);

  for (auto &sim : sim_received_rsus.sim_sensor_objects) {
    if (!sim.second.SimCars.empty()) {
      // std::cout << "Rsu::Step:sim_received_rsus: rsuid= " << sim.first << "
      // object y() =" << sim.second.SimCars[0].y()
      //           << std::endl;
    }
  }

  PopMessage2Msg(sim_received_rsus);

  std::string payload_;
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
  sim_msg::Traffic traffic;
  traffic.ParseFromString(payload_);
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);

  sim_msg::Union union_location;
  union_location.ParseFromString(payload_);
  for (const auto &msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location loc2;
    if (loc2.ParseFromString(msg.content())) {
      traffic.add_cars()->CopyFrom(location2car(loc, eid));
    }
  }

  SimRsuSensors sim_rsus;
  double timestamp = helper.timestamp();
  Msg::getInstance().getMAP()->data.timestamp = timestamp;
  Msg::getInstance().getRSM()->data.timestamp = timestamp;
  Msg::getInstance().getSPAT()->data.timestamp = timestamp;
  Msg::getInstance().getRSI()->data.timestamp = timestamp;
  FilterMessage2Sim(traffic, loc, timestamp, sim_rsus);
  SimConfilctAlg::getInstance().PushRsuMessage(sim_rsus);
}

/**
 * @brief pop message to msg
 *
 * @param sim rsu sensors
 */
void Rsu::PopMessage2Msg(SimRsuSensors &sim) {
  auto map = Msg::getInstance().getMAP();
  auto rsm = Msg::getInstance().getRSM();
  auto spat = Msg::getInstance().getSPAT();
  auto rsi = Msg::getInstance().getRSI();
  // rsm
  for (auto &sim_rsm : sim.sim_sensor_objects) {
    auto &rsu = rsu_sensors.at(sim_rsm.first - 10000);
    V2XDataRSM::RsuBuff buffer;
    buffer.rsuid = sim_rsm.first - 10000;
    buffer.frameid = rsu_frame_count[std::make_pair(buffer.rsuid, MN_RSM)]++;
    buffer.PreRsu = rsu->config.PreRsu;
    buffer.pos = rsu->position;
    buffer.timestampe = sim_rsm.second.timestamp;

    for (const auto &car : sim_rsm.second.SimCars) {
      sim_msg::Car participant_car;
      participant_car = car;
      buffer.participant_cars.push_back(participant_car);
    }

    for (const auto &dyn : sim_rsm.second.SimDynamicObstacles) {
      sim_msg::DynamicObstacle participant_dynamicObstacle;
      participant_dynamicObstacle = dyn;
      buffer.participant_dynamicObstacles.push_back(participant_dynamicObstacle);
    }
    rsm->data.rsu.push_back(buffer);
  }

  // rsi
  for (auto &sim_rsi : sim.sim_traffic_events) {
    auto &rsu = rsu_sensors.at(sim_rsi.first - 10000);
    V2XDataRSI::RsuBuff buffer;
    buffer.rsuid = sim_rsi.first - 10000;
    buffer.frameid = rsu_frame_count[std::make_pair(buffer.rsuid, MN_RSI)]++;
    buffer.PreRsu = rsu->config.PreRsu;
    buffer.timestampe = sim_rsi.second.timestamp;

    for (const auto &event : sim_rsi.second.SimEventLists) {
      hadmap::objectpkid obj;
      buffer.objs.push_back(obj);
    }
    rsi->data.rsu.push_back(buffer);
  }

  // spat
  for (auto &sim_spat : sim.sim_traffic_lights) {
    auto &rsu = rsu_sensors.at(sim_spat.first - 10000);
    V2XDataSPAT::SpatBuff buffer;
    buffer.rsuid = sim_spat.first - 10000;
    buffer.frameid = rsu_frame_count[std::make_pair(buffer.rsuid, MN_SPAT)]++;
    buffer.PreRsu = rsu->config.PreRsu;
    buffer.timestampe = sim_spat.second.timestamp;
    buffer.juncs = sim_spat.second.SimJuncs;
    spat->data.spat.push_back(buffer);
  }

  // map
  for (auto &sim_map : sim.sim_traffic_maps) {
    auto &rsu = rsu_sensors.at(sim_map.first - 10000);
    V2XDataMAP::RsuBuffer buffer;
    buffer.rsuid = sim_map.first - 10000;
    buffer.frameid = rsu_frame_count[std::make_pair(buffer.rsuid, MN_MAP)]++;
    buffer.PreRsu = rsu->config.PreRsu;
    buffer.timestampe = sim_map.second.timestamp;
    buffer.junctions = sim_map.second.SimJunctions;
    map->data.rsuData.push_back(buffer);
  }
}

/**
 * @brief filter message, and push to sim
 *
 * @param traffic traffic message
 * @param loc location message
 * @param time timestamp
 * @param sim
 */
void Rsu::FilterMessage2Sim(const sim_msg::Traffic &traffic, const sim_msg::Location &loc, const double time,
                            SimRsuSensors &sim) {
  auto rsu_nearby = FilterRsu(loc);
  auto FP_TEST = [](double &last, double now, double fp) -> bool {
    if (fp < 1e-6) {
      return true;
    }
    if ((now - last) < (1000.0 / fp)) {
      return false;
    }
    last = now;
    return true;
  };

  for (auto rid : rsu_nearby) {
    const auto &rsu = rsu_sensors.at(rid);
    sim.sim_rsu_posotion[rid].lat = rsu->position.x();
    sim.sim_rsu_posotion[rid].lng = rsu->position.y();
    // rsm
    {
      auto &last_t = rsu_timestamp_last[std::make_pair(rid, MN_RSM)];
      int frequency = 10;
      frequency = rsu->config.FrequencyRSM;
      // if (SimConfilctAlg::getInstance().GetAvgDelay(rid + 10000) > 50) {
      // frequency /= 2; }
      if (FP_TEST(last_t, time, frequency)) {
        rsu_timestamp_last[std::make_pair(rid, MN_RSM)] = time;
        sim.sim_sensor_objects[rid].timestamp = time;
        sim.sim_sensor_objects[rid].RsuId = rid;
        sim.sim_sensor_objects[rid].SimCars = rsu->FilterRsuCars(traffic);
        sim.sim_sensor_objects[rid].SimDynamicObstacles = rsu->FilterRsuDynamicObjects(traffic);
      }
    }

    // rsi
    {
      auto &last_t = rsu_timestamp_last[std::make_pair(rid, MN_RSI)];
      int frequency = 10;
      frequency = rsu->config.FrequencyRSI;
      // if (SimConfilctAlg::getInstance().GetAvgDelay(rid + 10000) > 50) {
      // frequency /= 2; }
      if (FP_TEST(last_t, time, frequency)) {
        rsu_timestamp_last[std::make_pair(rid, MN_RSI)] = time;
        sim.sim_traffic_events[rid].timestamp = time;
        sim.sim_traffic_events[rid].RsuId = rid;
        const auto filter_events = rsu->FilterRsuEvents();
        for (const auto &event : filter_events) {
          sim.sim_traffic_events[rid].SimEventLists.push_back(event);
        }
      }
    }

    // spat
    {
      auto &last_t = rsu_timestamp_last[std::make_pair(rid, MN_SPAT)];
      int frequency = 10;
      frequency = rsu->config.FrequencySPAT;
      // if (SimConfilctAlg::getInstance().GetAvgDelay(rid + 10000) > 50) {
      // frequency /= 2; }
      if (FP_TEST(last_t, time, frequency)) {
        rsu_timestamp_last[std::make_pair(rid, MN_SPAT)] = time;
        sim.sim_traffic_lights[rid].timestamp = time;
        sim.sim_traffic_lights[rid].RsuId = rid;
        sim.sim_traffic_lights[rid].SimJuncs = rsu->FilterRsuJuncs();
      }
    }

    // map
    {
      auto &last_t = rsu_timestamp_last[std::make_pair(rid, MN_MAP)];
      int frequency = 10;
      frequency = rsu->config.FrequencyMAP;
      // if (SimConfilctAlg::getInstance().GetAvgDelay(rid + 10000) > 50) {
      // frequency /= 2; }
      if (FP_TEST(last_t, time, frequency)) {
        rsu_timestamp_last[std::make_pair(rid, MN_MAP)] = time;
        sim.sim_traffic_maps[rid].timestamp = time;
        sim.sim_traffic_maps[rid].RsuId = rid;
        sim.sim_traffic_maps[rid].SimJunctions = rsu->FilterRsuJuncs();
      }
    }
  }
}

std::vector<std::int64_t> Rsu::FilterRsu(const sim_msg::Location &loc) {
  std::vector<std::int64_t> ids;
  for (auto &rsu : rsu_sensors) {
    double rsu_lon = rsu.second->position.x();
    double rsu_lat = rsu.second->position.y();
    double distance = rsu.second->config.DistanceCommu;
    // if (Distance(rsu_lat, rsu_lon, loc.position().y(), loc.position().x()) >
    //     distance) {
    //   continue;
    // }
    ids.push_back(rsu.first);
  }
  return ids;
}
