/**
 * @file v2x.cpp
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "v2x.h"
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <stdio.h>
#include <Eigen/Core>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include "catalog.h"
#include "Eigen/Geometry"
#include "hmap.h"
#include "location.pb.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "msg.h"
#include "obu.h"
#include "rsu.h"
#include "sensors.h"
#include "traffic.pb.h"

using namespace std::chrono;

/**
 * @brief Construct a new Sim V 2 X:: Sim V 2 X object
 *
 */
SimV2X::SimV2X() {}

/**
 * @brief Destroy the Sim V 2 X:: Sim V 2 X object
 *
 */
SimV2X::~SimV2X() {}

/**
 * @brief v2x init
 *
 * @param helper helper txsim
 */
void SimV2X::Init(tx_sim::InitHelper &helper) {
  std::string value = helper.GetParameter("tadsim_dir");
  if (!value.empty()) {
    tadsimPath = value;
  }
  auto config_dir = helper.GetParameter(tx_sim::constant::kInitKeyModuleSharedLibDirectory);

  modules.push_back(std::make_shared<Obu>());
  modules.push_back(std::make_shared<Rsu>());
  // msg 在最后一个，方便发消息

  for (auto &sens : modules) {
    sens->Init(helper);
  }
  Msg::getInstance().Init(helper);
}

/**
 * @brief v2x reset
 *
 * @param helper helper txsim
 */
void SimV2X::Reset(tx_sim::ResetHelper &helper) {
  std::string hdfile = helper.map_file_path();
  if (!tadsimPath.empty()) {
    // auto hdname = boost::filesystem::path(hdfile).filename();
    auto hdname = boost::filesystem::path(hdfile.substr(
        std::max(static_cast<int>(hdfile.find_last_of('/')), static_cast<int>(hdfile.find_last_of('\\'))) + 1));
    hdfile = (boost::filesystem::path(tadsimPath) / "data/scenario/hadmap" / hdname).string();
  }
  if (!LoadMap(hdfile)) {
    LOG(ERROR) << "load map  error";
  }
  sim_msg::Scene scene;
  if (!scene.ParseFromString(helper.scene_pb())) {
    LOG(ERROR) << "parse scene faild.";
    return;
  }
  if (!LoadSensor(scene, helper.map_local_origin(), helper.group_name())) {
    LOG(ERROR) << "load sensor  error";
  }
  Catalog::getInstance().init(scene);

  LOG(INFO) << "load " << ego_obu_sensors.size() << " ego obu";
  LOG(INFO) << "load " << traffic_obu_sensors.size() << " traffic obu";
  LOG(INFO) << "load " << rsu_sensors.size() << " rsu";

  // 主车没有obu
  if (ego_obu_sensors.find(ego_id) == ego_obu_sensors.end()) {
    LOG(INFO) << "the ego obu donot work.";
    modules.clear();
  } else {
    for (auto &sens : modules) {
      sens->map_ori = helper.map_local_origin();
      sens->Reset(helper);
    }
    Msg::getInstance().Reset(helper);
  }
}

/**
 * @brief get current time
 *
 * @return time_t
 */
time_t gettime() {
  system_clock::time_point time_point_now = system_clock::now();  // get current time
  system_clock::duration duration_since_epoch =
      time_point_now.time_since_epoch();  // from 1970-01-01 00:00:00 to current
  return duration_cast<microseconds>(duration_since_epoch).count();
}

/**
 * @brief v2x step
 *
 * @param helper helper txsim
 */
void SimV2X::Step(tx_sim::StepHelper &helper) {
  if (ego_obu_sensors.empty()) {
    return;
  }

  Msg::getInstance().Clear();
  // time_t time1 = gettime();
  for (auto &sens : modules) {
    sens->Step(helper);
  }
  // time_t time2 = gettime();
  // std::cout << "spend time1:" << time2 - time1 << std::endl;
  Msg::getInstance().Step(helper);
  // time_t time3 = gettime();
  // std::cout << "spend time2:" << time3 - time2 << std::endl;
}

/**
 * @brief v2x stop
 *
 * @param helper helper txsim
 */
void SimV2X::Stop(tx_sim::StopHelper &helper) {
  for (auto &sens : modules) {
    sens->Stop(helper);
  }
  Msg::getInstance().Stop(helper);
}

TXSIM_MODULE(SimV2X)
