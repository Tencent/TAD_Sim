/**
 * @file asnexample.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <fstream>
#include "../asn1msg/asn1msg_bsm2020.h"
#include "../asn1msg/asn1msg_map2020.h"
#include "../asn1msg/asn1msg_rsi2020.h"
#include "../asn1msg/asn1msg_rsm2020.h"
#include "../asn1msg/asn1msg_spat2020.h"
#include "../asn1msg/merge_proto.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "v2x_asn1_2020.pb.h"

namespace v2x_test {

/**
 * @brief Construct a new TEST object
 *
 */
TEST(V2XTest, message_example) {
  CAsn1MsgBSM2020 pAsn1MsgBsm;
  pAsn1MsgBsm.WritePreJsonExample("e:/bsm.json");
  CAsn1MsgMAP2020 pAsn1MsgMap;
  pAsn1MsgMap.WritePreJsonExample("e:/map.json");
  CAsn1MsgRSI2020 pAsn1MsgRsi;
  pAsn1MsgRsi.WritePreJsonExample("e:/rsi.json");
  CAsn1MsgRSM2020 pAsn1MsgRsm;
  pAsn1MsgRsm.WritePreJsonExample("e:/rsm.json");
  CAsn1MsgSPAT2020 pAsn1MsgSpat;
  pAsn1MsgSpat.WritePreJsonExample("e:/spat.json");
  EXPECT_TRUE(true);
}
}  // namespace v2x_test
