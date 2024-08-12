/**
 * @file asn1msg_rsi.h
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
#include "types/map_defs.h"

struct V2XDataRSI : V2XData {
  struct RsuBuff {
    std::int64_t rsuid = 0;
    std::uint64_t frameid = 0;
    double timestampe;
    std::vector<hadmap::objectpkid> objs;
    std::string PreRsu;
  };
  std::vector<RsuBuff> rsu;
};

class CAsn1MsgRSI : public CAsn1Msg {
 public:
  V2XDataRSI data;

 protected:
  virtual void Clear() { data = V2XDataRSI(); }
};
