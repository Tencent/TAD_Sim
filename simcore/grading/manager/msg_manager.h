// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "utils/eval_common.h"
#include "utils/eval_unit.h"
#include "utils/eval_utils.h"

namespace eval {
/**
 * @brief "仿真消息管理器", msg manager, gets message from topic or adds to msg_map.
 */
class MsgManager : public NoneCopyable {
 private:
  std::unordered_map<std::string, EvalMsg> _msg_map;
  MsgManager() {}

 public:
  friend CMsgManager;
  virtual ~MsgManager() {}

  void Add(double t_ms, const std::string &topic, const std::string &payload) {
    if (payload.size() > 0) {
      _msg_map[topic] = EvalMsg(t_ms, topic, payload);
    }
  }

  EvalMsg Get(const std::string &topic) {
    if (_msg_map.find(topic) != _msg_map.end()) {
      return _msg_map[topic];
    }
    return EvalMsg();
  }
  inline void Init() { _msg_map.clear(); }
};
}  // namespace eval
