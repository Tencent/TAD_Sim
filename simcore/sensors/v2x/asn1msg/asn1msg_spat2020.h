/**
 * @file asn1msg_spat2020.h
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
#include <set>
#include "asn1msg_spat.h"
#include "v2x_asn1_2020.pb.h"

class CAsn1MsgSPAT2020 : public CAsn1MsgSPAT {
 public:
  void WritePreJsonExample(const std::string& fpath);

 protected:
  virtual bool Convert(google::protobuf::Message* msg) override;
  virtual void Reset(tx_sim::ResetHelper& helper);

  void CoverSPAT(const std::string& preRSU, v2x_asn1_2020::V2XSPATMsg* pMsg);

 private:
  void Convert_byJunc(v2x_asn1_2020::V2XSPATMsg* pSpatMsg, const V2XDataSPAT::SpatBuff& buffer);

  bool GetSignLights(std::string traffic_xml);
  std::string m_traffic_xml;
  struct SignLight {
    int start_t = 0;
    int time_green = 0;
    int time_yellow = 0;
    int time_red = 0;
    int phase = 0;
    bool operator<(const SignLight& rhs) const { return phase < rhs.phase; }
  };
  std::map<hadmap::junctionpkid, std::set<SignLight>> m_mapLights;
  std::map<hadmap::junctionpkid, size_t> mapHdJunctionToV2XNode;
};
