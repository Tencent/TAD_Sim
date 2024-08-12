/**
 * @file asn1msg_spat.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "asn1msg.h"
#include "traffic.pb.h"
#include "types/map_defs.h"

struct V2XDataSPAT : V2XData {
  struct SpatBuff {
    std::int64_t rsuid = 0;
    std::uint64_t frameid = 0;
    double timestampe;
    std::vector<hadmap::junctionpkid> juncs;
    std::string PreRsu;
  };
  std::vector<SpatBuff> spat;
};

class CAsn1MsgSPAT : public CAsn1Msg {
 public:
  V2XDataSPAT data;

 protected:
  virtual void Clear() { data = V2XDataSPAT(); }
};
