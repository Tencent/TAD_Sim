// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_simulation_loop.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TADSimLoop : public Base::txSimulationLoop {
 public:
  TADSimLoop() TX_DEFAULT;
  virtual ~TADSimLoop() TX_DEFAULT;

  /**
   * @brief 仿真初始化
   *
   * @param helper 初始化助手
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_OVERRIDE;

  /**
   * @brief 仿真重置
   *
   * @param helper 重置助手
   */
  virtual void Reset(tx_sim::ResetHelper& helper) TX_OVERRIDE;

  /**
   * @brief 仿真步行函数
   *
   * @param helper 仿真助手
   */
  virtual void Step(tx_sim::StepHelper& helper) TX_OVERRIDE;

  /**
   * @brief 仿真停止
   *
   * @param helper
   */
  virtual void Stop(tx_sim::StopHelper& helper) TX_OVERRIDE;

 protected:
  Base::txSimulationTemplatePtr m_SimPtr = nullptr;
};
using TADSimLoopPtr = std::shared_ptr<TADSimLoop>;
TX_NAMESPACE_CLOSE(TrafficFlow)
