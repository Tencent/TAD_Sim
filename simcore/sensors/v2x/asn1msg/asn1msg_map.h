/**
 * @file asn1msg_map.h
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

struct V2XDataMAP : V2XData {
  struct RsuBuffer {
    std::int64_t rsuid = 0;
    std::uint64_t frameid = 0;
    double timestampe = 0;
    std::vector<hadmap::junctionpkid> junctions;
    std::string PreRsu;
  };
  std::vector<RsuBuffer> rsuData;
};

class CAsn1MsgMAP : public CAsn1Msg {
 public:
  V2XDataMAP data;

 protected:
  virtual void Clear() { data = V2XDataMAP(); }
};
