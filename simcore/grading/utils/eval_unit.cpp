// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "utils/eval_unit.h"

namespace eval {
EvalMsg::EvalMsg(double t_ms, const std::string &topic, const std::string &payload) {
  if (payload.size() > 0) {
    _sim_time = CSimTime(t_ms);
    _topic = topic;
    _payload = payload;
  }
}
}  // namespace eval
