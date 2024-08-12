/**
 * @file msg.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <map>
#include <memory>
#include "asn1msg/asn1msg_bsm.h"
#include "asn1msg/asn1msg_map.h"
#include "asn1msg/asn1msg_rsi.h"
#include "asn1msg/asn1msg_rsm.h"
#include "asn1msg/asn1msg_spat.h"
#include "base.h"
#include "location.pb.h"
#include "pblog/tx_sim_logger.h"
#include "traffic.pb.h"

enum MsgName {
  MN_BSM = 0,
  MN_MAP = 1,
  MN_RSI = 2,
  MN_RSM = 3,
  MN_SPAT = 4,
};

class Msg : public Base {
 public:
  static Msg &getInstance();

  void Clear();
  CAsn1MsgBSM *getBSM();
  CAsn1MsgMAP *getMAP();
  CAsn1MsgRSI *getRSI();
  CAsn1MsgRSM *getRSM();
  CAsn1MsgSPAT *getSPAT();

 protected:
  void Init(tx_sim::InitHelper &helper);
  void Reset(tx_sim::ResetHelper &helper);
  void Step(tx_sim::StepHelper &helper);
  void Stop(tx_sim::StopHelper &helper);

  std::map<MsgName, std::shared_ptr<CAsn1Msg>> mMessages;

  friend class SimV2X;

 private:
  pblog::txSimLogger m_data_logger;
  bool m_bJson = true;
};

sim_msg::Car location2car(const sim_msg::Location &loc, std::int64_t id);
