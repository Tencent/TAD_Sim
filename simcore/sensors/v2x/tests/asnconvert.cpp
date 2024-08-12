/**
 * @file asnconvert.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */

#include "asnconvert.h"
#include <google/protobuf/util/json_util.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "../asn1msg/asn1msg_bsm2020.h"
#include "../asn1msg/asn1msg_map2020.h"
#include "../asn1msg/asn1msg_rsi2020.h"
#include "../asn1msg/asn1msg_rsm2020.h"
#include "../asn1msg/asn1msg_spat2020.h"
#include "mapengine/hadmap_engine.h"

/**
 * @brief Construct a new CAsn1Convert::CAsn1Convert object
 *
 */
CAsn1Convert::CAsn1Convert() {}

/**
 * @brief Destroy the CAsn1Convert::CAsn1Convert object
 *
 */
CAsn1Convert::~CAsn1Convert() {}

/**
 * @brief set map handle
 *
 * @param mapHandle
 */
void CAsn1Convert::SetMapHandle(hadmap::txMapHandle *mapHandle) { m_mapHandle = mapHandle; }

/**
 * @brief convert simulation message into ASN.1 protobuf
 *
 * @param time_stamp
 * @param location
 * @param traffic
 * @param v2x_asn1_2020
 * @return true
 * @return false
 */
bool CAsn1Convert::SimMsgToAsnProto(const int time_stamp, const sim_msg::Location &location,
                                    const sim_msg::Traffic &traffic,
                                    std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020) {
  ProduceBSMProto(time_stamp, location, traffic, v2x_asn1_2020);
  ProduceMAPProto(v2x_asn1_2020);
  ProduceSPATProto(v2x_asn1_2020);
  ProduceRSIProto(v2x_asn1_2020);
  ProduceRSMProto(v2x_asn1_2020);
  return true;
}

/**
 * @brief produce bsm protobuf
 *
 * @param time_stamp
 * @param location
 * @param traffic
 * @param v2x_asn1_2020
 */
void CAsn1Convert::ProduceBSMProto(const int time_stamp, const sim_msg::Location &location,
                                   const sim_msg::Traffic &traffic,
                                   std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020) {
  std::unique_ptr<CAsn1MsgBSM2020> pAsn1MsgBsm(new CAsn1MsgBSM2020());
  pAsn1MsgBsm->data.timestamp = 100;
  for (const auto &car : traffic.cars()) {
    V2XDataBSM::CarBuf carbuf;
    carbuf.car = car;
    carbuf.frameid = 0;
    pAsn1MsgBsm->data.cars.push_back(carbuf);
  }
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgBsm.get())->Convert(v2x_asn1_2020.get());
}

/**
 * @brief produce MAP protobuf
 *
 * @param v2x_asn1_2020
 */
void CAsn1Convert::ProduceMAPProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020) {
  std::unique_ptr<CAsn1MsgMAP2020> pAsn1MsgMap(new CAsn1MsgMAP2020());
  tx_sim::ResetHelper helper(0, 0);
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgMap.get())->Reset(helper);
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgMap.get())->Convert(v2x_asn1_2020.get());
}

/**
 * @brief produce RSI protobuf
 *
 * @param v2x_asn1_2020
 */
void CAsn1Convert::ProduceSPATProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020) {
  std::unique_ptr<CAsn1MsgSPAT2020> pAsn1MsgSpat(new CAsn1MsgSPAT2020());
  // pAsn1MsgSpat->data.scene_file = "D:\\WXWork\\1688855893371377\\Cache\\File\\2023-06\\1212.sim";
  // ((CAsn1Msg *)pAsn1MsgSpat.get())->Init();
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgSpat.get())->Convert(v2x_asn1_2020.get());
}

/**
 * @brief produce RSI protobuf
 *
 * @param v2x_asn1_2020
 */
void CAsn1Convert::ProduceRSIProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020) {
  std::unique_ptr<CAsn1MsgRSI2020> pAsn1MsgRsi(new CAsn1MsgRSI2020());
  tx_sim::ResetHelper helper(0, 0);
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgRsi.get())->Reset(helper);
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgRsi.get())->Convert(v2x_asn1_2020.get());
}

/**
 * @brief produce RSM protobuf
 *
 * @param v2x_asn1_2020
 */
void CAsn1Convert::ProduceRSMProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020) {
  std::unique_ptr<CAsn1MsgRSM2020> pAsn1MsgRsm(new CAsn1MsgRSM2020());
  reinterpret_cast<CAsn1Msg *>(pAsn1MsgRsm.get())->Convert(v2x_asn1_2020.get());
}
