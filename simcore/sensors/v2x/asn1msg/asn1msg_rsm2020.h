/**
 * @file asn1msg_rsm2020.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "asn1msg_rsm.h"
#include "v2x_asn1_2020.pb.h"

class CAsn1MsgRSM2020 : public CAsn1MsgRSM {
 public:
  void WritePreJsonExample(const std::string &fpath);

 protected:
  virtual bool Convert(google::protobuf::Message *msg);
  void CoverRSM(const std::string &preRSU, v2x_asn1_2020::V2XRSMMsg *pMsg);

 private:
  void Convert_RSM(v2x_asn1_2020::V2XRSMMsg *pbsmmsg, const V2XDataRSM::RsuBuff &buffer);
  void LoadCarInfo(v2x_asn1_2020::V2XRSMMsg *pbsmmsg, const sim_msg::Car &car, const Eigen::Vector3d &pos);
  void LoadDynamicObstacleInfo(v2x_asn1_2020::V2XRSMMsg *pbsmmsg, const sim_msg::DynamicObstacle &dynamicObstacle,
                               const Eigen::Vector3d &pos);
};
