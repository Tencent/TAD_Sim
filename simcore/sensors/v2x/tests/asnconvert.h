// Copyright (C) 2022 Tencent Inc.  All rights reserved.

#pragma once
#include <list>
#include <map>
#include <string>
#include "location.pb.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "traffic.pb.h"
#include "v2x_asn1_2020.pb.h"  // for serialization/deserialization of ASN.1 definitions

class CAsn1Convert {
 public:
  CAsn1Convert();
  ~CAsn1Convert();

  void SetMapHandle(hadmap::txMapHandle* mapHandle);

  bool SimMsgToAsnProto(const int time_stamp, const sim_msg::Location& location, const sim_msg::Traffic& traffic,
                        std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020);

  void ProduceMAPProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020);
  void ProduceBSMProto(const int time_stamp, const sim_msg::Location& location, const sim_msg::Traffic& traffic,
                       std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020);
  void ProduceSPATProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020);
  void ProduceRSIProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020);
  void ProduceRSMProto(std::shared_ptr<v2x_asn1_2020::V2X_ASN1> v2x_asn1_2020);

 private:
  hadmap::txMapHandle* m_mapHandle = nullptr;  // hadMap handle
};
