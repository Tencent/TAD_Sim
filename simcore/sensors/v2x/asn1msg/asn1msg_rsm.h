/**
 * @file asn1msg_rsm.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <Eigen/Core>
#include "asn1msg.h"
#include "traffic.pb.h"

struct V2XDataRSM : V2XData {
  struct RsuBuff {
    std::int64_t rsuid = 0;
    std::uint64_t frameid = 0;
    Eigen::Vector3d pos;
    double timestampe;
    std::vector<sim_msg::Car> participant_cars;
    std::vector<sim_msg::DynamicObstacle> participant_dynamicObstacles;
    std::string PreRsu;
  };
  std::vector<RsuBuff> rsu;
};

class CAsn1MsgRSM : public CAsn1Msg {
 public:
  V2XDataRSM data;

 protected:
  virtual void Clear() { data = V2XDataRSM(); }
};
