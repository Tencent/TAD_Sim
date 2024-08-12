/**
 * @file asn1msg_bsm2020.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <vector>
#include "asn1msg_bsm.h"
#include "v2x_asn1_2020.pb.h"

class CAsn1MsgBSM2020 : public CAsn1MsgBSM {
 public:
  void WritePreJsonExample(const std::string &fpath);

 protected:
  virtual bool Convert(google::protobuf::Message *msg);

 private:
  void Convert_Traffic(v2x_asn1_2020::V2XBSMMsg *pbsmmsg);
  void CoverCar(const std::string &preBSM, v2x_asn1_2020::V2XBSM &pV2XBSM);
};
