// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_sim_time.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

// @brief 交通规则基类
class ITrafficRules {
 public:
  virtual ~ITrafficRules() TX_DEFAULT;

  /**
   * @brief 是否可以切换到左车道状态
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool CheckCanSwitchRightLaneState(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 是否可以切换到右车道状态
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool CheckCanSwitchLeftLaneState(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("ITrafficRules");
  }
};

using ITrafficRulesPtr = std::shared_ptr<ITrafficRules>;

TX_NAMESPACE_CLOSE(Base)
#undef SerializeInfo
