/**
 * @file v2x_test.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "../v2x.h"
#include <fstream>
#include "../asn1msg/asn1msg_spat2020.h"
#include "../hmap.h"
#include "../sensors.h"
#include "asnconvert.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"

namespace v2x_test {

/**
 * @brief Construct a new TEST object
 *
 */
TEST(V2XTest, test) {
  LoadMap(
      "C:/Users/kekesong/AppData/Roaming/tadsim/data/scenario/hadmap/"
      "umc_map_20220929.xodr");

  CAsn1Convert m_asn1Convert;
  std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020 = std::make_shared<v2x_asn1_2020::V2X_ASN1>();
  m_asn1Convert.ProduceMAPProto(v2x_asn1_2020);

  sim_msg::Location location;
  sim_msg::Traffic traffic;
  double time_stamp;
  // m_asn1Convert.ProduceMAPProto();
  //  m_asn1Convert.ProduceBSMProto(time_stamp, location, traffic);
}

/**
 * @brief Construct a new TEST object
 *
 */
TEST(V2XTest, convert_spat2020) {
  v2x_asn1_2020::V2XRSIMsg rsi;
  rsi.set_rsuid(1);

  std::string buffer;
  google::protobuf::util::MessageToJsonString(rsi, &buffer);

  std::cout << "\n\n\n" << buffer << "\n\n\n";

  v2x_asn1_2020::V2XBSM pV2XBSM;
  pV2XBSM.set_msgcount(122);
  pV2XBSM.mutable_position3d()->set_longitude(113.4332323 * 1E7);
  pV2XBSM.mutable_position3d()->set_latitude(40.23255344 * 1E7);
  pV2XBSM.mutable_position3d()->set_elevation(54.34 * 10);
  pV2XBSM.set_speed(4.876);
  pV2XBSM.mutable_vehiclesize()->set_vehiclewidth(170);     // m -> cm
  pV2XBSM.mutable_vehiclesize()->set_vehicleheight(195.6);  // m -> cm
  pV2XBSM.mutable_vehiclesize()->set_vehiclelength(570.4);  // m -> cm

  google::protobuf::util::MessageToJsonString(pV2XBSM, &buffer);

  std::cout << "\n\n\n" << buffer << "\n\n\n";

  // if (!google::protobuf::util::JsonStringToMessage(preBSM, &preV2XBSM).ok()) {

  return;
  CAsn1Convert asn1convert;
  std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020(new v2x_asn1_2020::V2X_ASN1());
  asn1convert.ProduceSPATProto(v2x_asn1_2020);
}

/**
 * @brief Construct a new TEST object
 *
 */
TEST(V2XTest, convert_rsi2020) {
  LoadMap("C:\\Users\\fangccheng\\AppData\\Roaming\\tadsim\\data\\scenario\\hadmap\\umc_map_20220929.xodr");

  CAsn1Convert m_asn1Convert;
  std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020 = std::make_shared<v2x_asn1_2020::V2X_ASN1>();
  m_asn1Convert.ProduceRSIProto(v2x_asn1_2020);
}

/**
 * @brief Construct a new TEST object
 *
 */
TEST(V2XTest, min) { std::cout << std::min<int>({1, 2, 3}) << std::endl; }

}  // namespace v2x_test
