// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
#include "tx_traffic_element_base.h"

TX_NAMESPACE_OPEN(TrafficFlow)
struct EventTriggerParam_t {
  /**
   * @brief 构造事件触发参数对象
   *
   * @param timeMgr 时间管理器
   * @param logicEgoPtr traffic 元素指针
   */
  EventTriggerParam_t(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr logicEgoPtr) : _timeMgr(timeMgr) {
    _LogicEgoPtr = logicEgoPtr;
  }
  const Base::TimeParamManager& _timeMgr;
  Base::ITrafficElementPtr _LogicEgoPtr = nullptr;
};
TX_NAMESPACE_CLOSE(TrafficFlow)
