/**
 * @file asn1msg_rsi2020.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <map>
#include "asn1msg_rsi.h"
#include "structs/hadmap_object.h"
#include "v2x_asn1_2020.pb.h"

class CAsn1MsgRSI2020 : public CAsn1MsgRSI {
 public:
  void WritePreJsonExample(const std::string &fpath);

 protected:
  virtual void Reset(tx_sim::ResetHelper &helper) override;
  virtual bool Convert(google::protobuf::Message *msg);

  void CoverRsi(const std::string &preRSU, v2x_asn1_2020::V2XRSIMsg *pMsg);

 private:
  std::map<uint64_t, hadmap::txObjectPtr> m_mapObjects;
};
