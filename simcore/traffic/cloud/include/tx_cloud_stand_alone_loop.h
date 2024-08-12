// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_simulation_loop.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class CloudStandAloneLoop : public Base::txSimulationLoop {
 public:
  CloudStandAloneLoop() TX_DEFAULT;
  virtual ~CloudStandAloneLoop() TX_DEFAULT;

  /**
   * @brief 初始化
   * @details 初始化模块，给定InitHelper实例来设置初始化参数和注册回调
   * @param helper 初始化参数和回调
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_OVERRIDE;

  /**
   * @brief 重置模块
   * @details 使用提供的ResetHelper实例重置模块
   * @param helper 用于重置模块的帮助器类
   */
  virtual void Reset(tx_sim::ResetHelper& helper) TX_OVERRIDE;

  /**
   * @brief Step
   * @param helper
   * @details Step函数，根据StepHelper实例中的信息执行模块的控制动作
   */
  virtual void Step(tx_sim::StepHelper& helper) TX_OVERRIDE;

  /**
   * @brief Stop 停止功能
   * @param helper 停止帮助器，包含停止所需的所有信息
   * @details 根据 StopHelper 传入的参数，停止当前模块的相关功能
   */
  virtual void Stop(tx_sim::StopHelper& helper) TX_OVERRIDE;

 protected:
  Base::txSimulationTemplatePtr m_SimPtr = nullptr;
};
using CloudStandAloneLoopPtr = std::shared_ptr<CloudStandAloneLoop>;
TX_NAMESPACE_CLOSE(TrafficFlow)
