/**
 * @file asn1msg_bsm.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <vector>
#include "asn1msg.h"
#include "traffic.pb.h"

struct V2XDataBSM : V2XData {
  struct CarBuf {
    std::int64_t obuid = 0;
    std::uint64_t frameid = 0;
    double timestampe;
    sim_msg::Car car;
    std::string preBSM;
  };
  std::vector<V2XDataBSM::CarBuf> cars;
};

class CAsn1MsgBSM : public CAsn1Msg {
 public:
  V2XDataBSM data;

 protected:
  virtual void Clear() { data = V2XDataBSM(); }
};
