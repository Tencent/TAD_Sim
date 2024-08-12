/**
 * @file asn1msg_rsi2020.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "asn1msg_rsi2020.h"
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <fstream>
#include "../hmap.h"
#include "json/json.h"
#include "merge_proto.h"

/**
 * @brief rsi2020 message reset
 *
 * @param helper helper of txsim
 */
void CAsn1MsgRSI2020::Reset(tx_sim::ResetHelper &helper) {
  m_mapObjects.clear();
  const std::vector<hadmap::txLaneId> laneIds;
  std::vector<hadmap::OBJECT_TYPE> types;
  // get all objects from hdmap
  hadmap::txObjects objs;
  hadmap::getObjects(hMapHandle, laneIds, types, objs);
  for (const auto &item : objs) {
    hadmap::objectpkid objID = item->getId();
    m_mapObjects[objID] = item;
  }
}

/**
 * @brief convert rsi2020 message to protobuf message
 *
 * @param msg protobuf message
 * @return true on success
 * @return false
 */
bool CAsn1MsgRSI2020::Convert(google::protobuf::Message *msg) {
  v2x_asn1_2020::V2X_ASN1 *v2x_asn1_2020 = (v2x_asn1_2020::V2X_ASN1 *)msg;
  for (const auto &itemRsu : data.rsu) {
    v2x_asn1_2020::V2XRSIMsg *pRsiMsg = v2x_asn1_2020->mutable_rsi()->Add();
    // recover rsi message from privous label
    CoverRsi(itemRsu.PreRsu, pRsiMsg);
    // if has no prelabel
    if (pRsiMsg->rsilist_size() == 0) {
      pRsiMsg->set_rsuid(itemRsu.rsuid);
      for (const auto &itemObjectID : itemRsu.objs) {
        v2x_asn1_2020::V2XRSIData *pRTSData = pRsiMsg->mutable_rsilist()->Add();
        hadmap::txObjectPtr object = m_mapObjects[itemObjectID];
        if (!object) continue;
        pRTSData->set_rsiid(itemObjectID);
        hadmap::OBJECT_TYPE objType = object->getObjectType();
        uint32_t asnType = 0;
        if (hadmap::OBJECT_TYPE::OBJECT_TYPE_Text == objType) {
          asnType = 0;
        } else if (hadmap::OBJECT_TYPE::OBJECT_TYPE_TrafficLights == objType) {
          asnType = objType;
        }
        pRTSData->set_alerttype(asnType);
        hadmap::txPoint txPos = object->getPos();
      }
    }
  }
  return true;
}

/**
 * @brief write prelabel example json file
 *
 * @param fpath path to write
 */
void CAsn1MsgRSI2020::WritePreJsonExample(const std::string &fpath) {
  v2x_asn1_2020::V2XRSIMsg msg;
  // fillup protobuf message by random
  FillupProto(&msg);
  std::string buffer;
  google::protobuf::util::MessageToJsonString(msg, &buffer);
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(buffer.c_str(), buffer.c_str() + buffer.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre RSI faild: " << err;
    return;
  }
  Json::Value prejson;
  prejson["RSI"] = tree;
  Json::StreamWriterBuilder builder;
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::ofstream emp(fpath);
  writer->write(prejson, &emp);
  emp.close();
}

/**
 * @brief recover rsi from prelabel string
 *
 * @param preRSU prelabel string
 * @param pMsg to recover
 */
void CAsn1MsgRSI2020::CoverRsi(const std::string &preRSU, v2x_asn1_2020::V2XRSIMsg *pMsg) {
  if (preRSU.empty()) return;
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(preRSU.c_str(), preRSU.c_str() + preRSU.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre RSU faild: " << err;
    return;
  }
  Json::Value jsonv = tree["RSI"];
  if (!jsonv.isObject()) {
    return;
  }
  // json to message
  if (!google::protobuf::util::JsonStringToMessage(jsonv.toStyledString(), pMsg).ok()) {
    LOG(WARNING) << "CANNOT read RSI from preRSU";
    return;
  }
}
