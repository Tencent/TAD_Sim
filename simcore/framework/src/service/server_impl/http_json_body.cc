// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "http_json_body.h"

#include "glog/logging.h"

#include "utils/json_helper.h"


using namespace tx_sim::utils;


namespace tx_sim {
namespace service {

static const std::string kJsonKeyScenarioPath{"scenarioPath"};
static const std::string kJsonKeyPauseOnStart{"pauseOnStart"};


const std::string SimGeneralRespBody::Encode() {
  Json::Value root;
  EncodeFields(root);
  root["code"] = code;
  root["msg"] = msg;
  return Json::writeString(Json::StreamWriterBuilder(), root);
}


void SimPlayPollRespBody::EncodeFields(Json::Value& root) {
  root["scenario_running"] = scenario_running;
}


bool SimRunReqBody::Decode(const std::string& payload, std::string& err) {
  try {
    Json::Value root;
    ReadJsonDocFromString(root, payload);
    if (!root.isMember(kJsonKeyScenarioPath)) throw std::invalid_argument("no scenarioPath key provided.");
    scenario_path = root[kJsonKeyScenarioPath].asString();
    if (root.isMember(kJsonKeyPauseOnStart)) pause_on_start = root[kJsonKeyPauseOnStart].asBool();
  } catch (const std::exception& e) {
    LOG(ERROR) << "invalid Sim Run Request(" << payload << "): " << e.what();
    err = e.what();
    return false;
  }
  return true;
}


void SimL2wParamsGetRespBody::EncodeFields(Json::Value& root) {
  config.Encode(root["config"]);
  events.Encode(root["events"]);
}


bool SimL2wParamsPostReqBody::Decode(const std::string& payload, std::string& err) {
  try {
    Json::Value root;
    ReadJsonDocFromString(root, payload);
    if (!root.isMember(kJsonKeyScenarioPath)) throw std::invalid_argument("no scenarioPath key provided.");
    scene_path = root[kJsonKeyScenarioPath].asString();
    if (!root.isMember("params")) throw std::invalid_argument("no params key provided.");
    config.Decode(root["params"]);
  } catch (const std::exception& e) {
    LOG(ERROR) << "invalid Sim Run Request(" << payload << "): " << e.what();
    err = e.what();
    return false;
  }
  return true;
}

}  // namespace service
}  // namespace tx_sim
