/**
 * @file asn1msg.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <google/protobuf/message.h>
#include "txsim_module.h"

struct V2XData {
  double timestamp = 0;
};

class CAsn1Msg {
 public:
  CAsn1Msg() = default;
  virtual ~CAsn1Msg() = default;

 protected:
  virtual void Init(tx_sim::InitHelper& helper) {}
  virtual void Reset(tx_sim::ResetHelper& helper) {}
  // virtual void Step(tx_sim::StepHelper& helper) {}
  // virtual void Stop(tx_sim::StopHelper& helper) {}

  virtual bool Convert(google::protobuf::Message* msg) = 0;
  virtual void Clear() = 0;

  friend class Msg;
  friend class CAsn1Convert;  // FOR TESTS
};
