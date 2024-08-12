/**
 * @file asn1msg_spat2020.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "asn1msg_spat2020.h"
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <fstream>
#include <sstream>
#include "../hmap.h"
#include "../msg.h"
#include "asn1msg_map2020.h"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "json/json.h"
#include "merge_proto.h"
#include "read_xml.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_lane.h"

namespace fs = boost::filesystem;

bool CAsn1MsgSPAT2020::GetSignLights(std::string traffic_xml) {
  tinyxml2::XMLDocument doc;
  const tinyxml2::XMLElement *pRoot = GetXMLRoot(traffic_xml, doc);
  const tinyxml2::XMLElement *pSignlights = TryGetChildNode(pRoot, "signlights");
  if (!pSignlights) return false;
  std::string activatePlan = pSignlights->Attribute("activePlan");
  const tinyxml2::XMLElement *itemLight = pSignlights->FirstChildElement();
  while (itemLight) {
    std::string plan = itemLight->Attribute("plan");
    if (plan == activatePlan && std::string(itemLight->Attribute("status")) == "Activated") {
      hadmap::junctionpkid jid = std::stoull(itemLight->Attribute("junction"));
      SignLight light;
      light.phase = std::atoi(itemLight->Attribute("phaseNumber"));
      light.start_t = std::atoi(itemLight->Attribute("start_t"));
      light.time_green = std::atoi(itemLight->Attribute("time_green"));
      light.time_yellow = std::atoi(itemLight->Attribute("time_yellow"));
      light.time_red = std::atoi(itemLight->Attribute("time_red"));

      //
      // std::string sID = itemLight->Attribute("id");
      // std::string sRoutID = itemLight->Attribute("routeID");
      m_mapLights[jid].insert(light);
    }
    itemLight = itemLight->NextSiblingElement();
  }
  return true;
}

// 0 未知状态
// 1 信号灯未工作
// 2 红闪
// 3 红灯状态
// 4 绿灯待行状态
// 5 绿灯状态
// 6 受保护向位绿灯
// 7 黄灯状态
// 8 黄闪
void setPhaseState(v2x_asn1_2020::Phase *pPhase, double timestampe, int starttime, int green, int yellow, int red) {
  green *= 10;
  yellow *= 10;
  red *= 10;
  starttime *= 10;
  int ts = static_cast<int>((timestampe * 10));

  int total = red + yellow + green;
  int absTime = (ts + starttime) % total;

  auto timing_color = [](google::protobuf::RepeatedPtrField<v2x_asn1_2020::PhaseState> *pPhaseLists, int ts, int cur,
                         int start, int dura, int all, int state) {
    if (dura > 0) {
      v2x_asn1_2020::PhaseState *pPhaseState = pPhaseLists->Add();
      pPhaseState->set_timechangedetailsflag(1);
      v2x_asn1_2020::TimeChangeDetails *pTimeCountingDown = pPhaseState->mutable_timechangedetails();
      if (cur < start || cur > start + dura) {
        pTimeCountingDown->set_starttime((start - cur + all) % all);
      }
      pTimeCountingDown->set_minendtime((start + dura - cur + all) % all);
      pTimeCountingDown->set_maxendtime(pTimeCountingDown->minendtime());
      pTimeCountingDown->set_likelyendtime(pTimeCountingDown->minendtime());
      pTimeCountingDown->set_nextstarttime((start - cur + all) % all);
      pTimeCountingDown->set_nextduration(dura);
      pTimeCountingDown->set_confidence(20);

      pPhaseState->set_lightstate(state);
    }
  };
  if (absTime < green) {
    // green, yellow, red
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, 0, green, total, 5);
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, green, yellow, total, 7);
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, green + yellow, red, total, 3);
  } else if (absTime < green + yellow) {
    // yellow, green, red
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, green, yellow, total, 7);
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, green + yellow, red, total, 3);
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, 0, green, total, 5);
  } else {
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, green + yellow, red, total, 3);
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, 0, green, total, 5);
    timing_color(pPhase->mutable_phasestatelist(), ts, absTime, green, yellow, total, 7);
  }
}

void CAsn1MsgSPAT2020::Convert_byJunc(v2x_asn1_2020::V2XSPATMsg *pSpatMsg, const V2XDataSPAT::SpatBuff &buffer) {
  pSpatMsg->set_descriptivename("spat");
  pSpatMsg->set_msgcount(buffer.frameid);
  pSpatMsg->set_minuteoftheyear(std::round(buffer.timestampe / 60000));
  pSpatMsg->set_dsecond((std::size_t)buffer.timestampe % 60000);
  for (const auto inputJucntionItem : buffer.juncs) {
    if (m_mapLights.find(inputJucntionItem) == m_mapLights.end()) {
      continue;
    }
    const auto &mapLights = m_mapLights.at(inputJucntionItem);
    if (mapLights.empty()) continue;
    v2x_asn1_2020::IntersectionState *pIntersectionState = pSpatMsg->mutable_intersectionstatelist()->Add();
    pIntersectionState->mutable_nodereferenceid()->set_nodeid(mapHdJunctionToV2XNode[inputJucntionItem]);
    pIntersectionState->mutable_nodereferenceid()->set_roadregulatorid(0);
    pIntersectionState->set_minuteoftheyear(std::round(buffer.timestampe / 60000));
    pIntersectionState->set_dsecond((std::size_t)buffer.timestampe % 60000);
    for (const auto &light : mapLights) {
      v2x_asn1_2020::Phase *pPhase = pIntersectionState->mutable_phaselist()->Add();
      pPhase->set_phaseid(light.phase);
      setPhaseState(pPhase, buffer.timestampe / 1000.0, light.start_t - 1, light.time_green, light.time_yellow,
                    light.time_red);
    }
  }
}

/**
 * @brief callback by reset. load traffic liaght data from scenario
 *
 * @param helper helper of txsim
 */
void CAsn1MsgSPAT2020::Reset(tx_sim::ResetHelper &helper) {
  m_mapLights.clear();
  mapHdJunctionToV2XNode.clear();
  // get junction id map
  std::set<hadmap::junctionpkid> temp_juncs;
  for (const auto &juc : mapJunctions) {
    temp_juncs.insert(juc->getId());
  }
  mapHdJunctionToV2XNode.clear();
  for (auto jid : temp_juncs) {
    auto id = mapHdJunctionToV2XNode.size() + 1;
    mapHdJunctionToV2XNode[jid] = id;
    // std::cout << jid << "->" << id << ", ";
  }

  // get lights:
  // get file path
  std::string scene_file = helper.scenario_file_path();
  fs::path scenario_file(scene_file);
  fs::path parent_path = scenario_file.parent_path();
  fs::path file_name = scenario_file.filename();
  fs::path stem = file_name.stem();
  stem += "_traffic.xml";
  fs::path traffic_file = parent_path / stem;
  m_traffic_xml = traffic_file.lexically_normal().string();
  m_mapLights.clear();
  // load lights
  GetSignLights(m_traffic_xml);
}

/**
 * @brief cover spat message
 *
 * @param msg message to cover
 * @return true on success
 * @return false
 */
bool CAsn1MsgSPAT2020::Convert(google::protobuf::Message *msg) {
  v2x_asn1_2020::V2X_ASN1 *v2x_asn1_2020 = (v2x_asn1_2020::V2X_ASN1 *)msg;
  for (const auto &spat : data.spat) {
    v2x_asn1_2020::V2XSPATMsg *pSpatMsg = v2x_asn1_2020->mutable_spat()->Add();
    // recover from prelabelled json string
    CoverSPAT(spat.PreRsu, pSpatMsg);
    if (pSpatMsg->intersectionstatelist_size() == 0) {
      // if has no prelabel
      Convert_byJunc(pSpatMsg, spat);
    }
  }
  return true;
}

/**
 * @brief write prelabel example
 *
 * @param fpath path to save
 */
void CAsn1MsgSPAT2020::WritePreJsonExample(const std::string &fpath) {
  v2x_asn1_2020::V2XSPATMsg msg;
  // fill by random
  FillupProto(&msg);
  std::string buffer;
  // proto to json
  google::protobuf::util::MessageToJsonString(msg, &buffer);
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(buffer.c_str(), buffer.c_str() + buffer.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre SPAT faild: " << err;
    return;
  }
  // save to file
  Json::Value prejson;
  prejson["SPAT"] = tree;
  Json::StreamWriterBuilder builder;
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::ofstream emp(fpath);
  writer->write(prejson, &emp);
  emp.close();
}

/**
 * @brief recover from prelabel buffer
 *
 * @param preRSU prelabel rsu buffer
 * @param pMsg to be recover
 */
void CAsn1MsgSPAT2020::CoverSPAT(const std::string &preRSU, v2x_asn1_2020::V2XSPATMsg *pMsg) {
  if (preRSU.empty()) return;
  // json to protobuf
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(preRSU.c_str(), preRSU.c_str() + preRSU.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre RSU faild: " << err;
    return;
  }
  Json::Value jsonv = tree["SPAT"];
  if (!jsonv.isObject()) {
    return;
  }
  if (!google::protobuf::util::JsonStringToMessage(jsonv.toStyledString(), pMsg).ok()) {
    LOG(WARNING) << "CANNOT read SPAT from preRSU";
    return;
  }
}
