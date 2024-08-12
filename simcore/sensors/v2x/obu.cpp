/**
 * @file obu.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "obu.h"
#include <Eigen/Eigen>
#include <random>
#include "./sim_algorithm/sim_conflict_alg.h"
#include "catalog.h"
#include "asn1msg/asn1msg_bsm.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "msg.h"
#include "sensors.h"
#include "traffic.pb.h"
#include "union.pb.h"
#include "utils/utils.h"

/**
 * @brief obu initialize
 *
 * @param helper helper of txsim
 */
void Obu::Init(tx_sim::InitHelper& helper) {
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);

  m_simObu_.push_back(std::make_shared<SimObuCongestion>());
  // m_step_ms = std::stod(helper.GetParameter("stepTime"));
}

/**
 * @brief obu reset
 *
 * @param helper helper of txsim
 */
void Obu::Reset(tx_sim::ResetHelper& helper) {
  SimConfilctAlg::getInstance().PartialClear();
  m_traffic_obu_data_.clear();
  m_simObu_.clear();
  m_obu_timestamp_last_.clear();
  last_timestamp = 0;
  m_step_ms = 20.0;
  m_sub_step_ms = m_step_ms / SimConfilctAlg::getInstance().GetSubStep();
  SimConfilctAlg::getInstance().SetFrequency(m_step_ms);
}

/**
 * @brief obu step
 *
 * @param helper helper of txsim
 */
void Obu::Step(tx_sim::StepHelper& helper) {
  // Call conflict_alg in every step
  if (helper.timestamp() - last_timestamp != m_step_ms) {
    m_step_ms = helper.timestamp() - last_timestamp;
    if (m_step_ms > 0) {
      m_sub_step_ms = m_step_ms / SimConfilctAlg::getInstance().GetSubStep();
      SimConfilctAlg::getInstance().SetFrequency(m_step_ms);
    }
  }
  last_timestamp = helper.timestamp();

  SimConfilctAlg::getInstance().RunSim();
  SimObuSensors sim_received_obus;
  SimConfilctAlg::getInstance().PopReceivedObuMessage(sim_received_obus);

  // for (auto& sim : sim_received_obus.sim_obus) {
  //   std::cout << "Obu::Step:sim_received_obus: obuid= " << sim.first << " car y() =" << sim.second.CarObu.y() <<
  //   std::endl;
  // }
  auto bsm = Msg::getInstance().getBSM();
  bsm->data.timestamp = helper.timestamp();

  for (auto it = sim_received_obus.sim_obus.begin(); it != sim_received_obus.sim_obus.end(); ++it) {
    int id = it->first;
    V2XDataBSM::CarBuf carbuf;
    carbuf.obuid = sim_received_obus.sim_obus[id].ObuId;
    carbuf.timestampe = sim_received_obus.sim_obus[id].timestamp;
    // if (carbuf.obuid == 14) {
    //   std: :cout << helper.timestamp() << " After received: timestamp = " << carbuf.timestampe << \
      //      " delay " << helper.timestamp() - carbuf.timestampe << " ms" << std::endl;
    // }
    // std::cout << "AvgDelay = " << SimConfilctAlg::getInstance().GetAvgDelay(14) << std::endl;

    carbuf.frameid = 0;

    const auto car = sim_received_obus.sim_obus[id].CarObu;
    carbuf.car = car;
    carbuf.preBSM = traffic_obu_sensors[car.id()]->config.PreBSM;

    bsm->data.cars.push_back(carbuf);
  }
  // Send bsm messages to the conflict_alg module for transmission according to the transmission cycle.
  std::string payload_;
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
  sim_msg::Traffic traffic;
  traffic.ParseFromString(payload_);
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);
  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);
  helper.GetSubscribedMessage(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation, payload_);

  SimConfilctAlg::getInstance().UpdateEgoCarPosition(loc);

  sim_msg::Union union_location;
  union_location.ParseFromString(payload_);
  for (const auto& msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    if (loc.ParseFromString(msg.content())) {
      traffic.add_cars()->CopyFrom(location2car(loc, eid));
    }
  }

  // fequency control
  auto FP_TEST = [](double& last, double now, double fp) -> bool {
    if (fp < 1e-6) {
      return true;
    }
    if ((now - last) < (1000.0 / fp)) {
      return false;
    }
    last = now;
    return true;
  };

  SimObuSensors sim_obus;
  double timestamp = helper.timestamp();
  int frequency = 10;
  for (const auto& car : traffic.cars()) {
    // LOG(INFO) << "Obu::Step: traffic.cars: obuid= " << car.id() << " car y() =" << car.y() << std::endl;
    if (traffic_obu_sensors.find(car.id()) == traffic_obu_sensors.end()) {
      continue;
    }
    const auto& obu = traffic_obu_sensors.at(car.id());
    auto& last_t = m_obu_timestamp_last_[car.id()];

    frequency = obu->config.FrequencyBSM;
    // if (SimConfilctAlg::getInstance().GetAvgDelay(car.id()) > 50) { frequency /= 2; }
    if (!FP_TEST(last_t, timestamp, frequency)) {
      continue;
    }
    m_obu_timestamp_last_[car.id()] = timestamp;
    sim_obus.sim_obus[car.id()].ObuId = car.id();
    sim_obus.sim_obus[car.id()].timestamp = timestamp;
    sim_obus.sim_obus[car.id()].CarObu = car;
  }

  // SimObuDataInput(traffic, helper.timestamp(), sim_obus);
  SimConfilctAlg::getInstance().PushObuMessage(sim_obus);
}

/**
 * @brief simulation of receiving data
 *
 * @param traffic traffic message
 * @param time timestamp
 * @param sim output sensors
 */
void Obu::SimObuDataInput(const sim_msg::Traffic& traffic, const double time, SimObuSensors& sim) {
  for (const auto& car : traffic.cars()) {
    sim.sim_obus[car.id()].ObuId = car.id();
    sim.sim_obus[car.id()].timestamp = time;
    sim.sim_obus[car.id()].CarObu = car;
  }
}

/**
 * @brief filter cars in the traffic
 *
 * @param traffic traffic message
 * @param egoCar location message
 */
void Obu::FilterCars(const sim_msg::Traffic& traffic, const sim_msg::Location& egoCar) {
  for (const auto& car : traffic.cars()) {
    if (CheckObuId(car.id())) {
      double distance = Distance(egoCar.position().y(), egoCar.position().x(), car.y(), car.x());

      if (distance <= EGO_CAR_COVERAGE_DISTANCE) {
        m_traffic_obu_data_[car.id()].CarObu = car;
      }
    }
  }
}

/**
 * @brief check obu id
 *
 * @param id
 * @return true on success
 * @return false
 */
bool Obu::CheckObuId(int id) {
  for (auto obu : obu_mount_config_list) {
    obu.id == id;
    return true;
  }
  return false;
}
