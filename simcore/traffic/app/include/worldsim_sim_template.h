// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "passive_sim_template.h"
#include "tx_simulation_loop.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class WorldsimSimLoop : public Base::txSimulationTemplate, public txPassiveSimTemplate {
 public:
  // The following are virtual functions in the base class (e.g., Base or tx_sim)

  /**
   * @brief The Init function is called when the simulation starts
   *
   * @param helper
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief The Reset function is called when the simulation is reset
   *
   * @param helper
   */
  virtual void Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief The Step function is called every time the simulation advances by one time step
   *
   * @param helper
   */
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief The Stop function is called when the simulation stops
   *
   * @param helper
   */
  virtual void Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief The CreateEnv function is called when the simulation is reset
   *
   * @param helper
   */
  virtual void CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief The CreateSystem function is called when the simulation starts
   *
   */
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief The TrafficSystemPtr function returns a pointer to the traffic element system
   *
   * @return Base::TrafficElementSystemPtr
   */
  virtual Base::TrafficElementSystemPtr TrafficSystemPtr() TX_NOEXCEPT TX_OVERRIDE { return TrafficSystemPtrPassive(); }

  /**
   * @brief The Simulation function is called every time the simulation advances by one time step
   *
   * @param helper
   * @param timeMgr
   */
  virtual void Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual void ResetVars() TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::TimeParamManager MakeTimeMgr(const Base::txFloat time_stamp_ms) TX_NOEXCEPT TX_OVERRIDE {
    return MakeTimeMgrPassive(time_stamp_ms);
  }
  // The following virtual functions are part of a simulation context.

  /**
   * @brief 在模拟前执行的虚函数
   *
   * 虚拟函数 PreSimulation 提供了在每个模拟步骤前进行的初始化操作。
   * 函数接受两个参数：tx_sim::StepHelper 类型的引用和 Base::TimeParamManager 类型的引用。
   *
   * @param helper 模拟步骤辅助类对象的引用
   * @param timeMgr 时间参数管理器对象的引用
   */
  virtual void PreSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 在模拟后执行的虚函数
   *
   * PostSimulation 函数在模拟结束后执行，用于执行与模拟结束相关的操作。
   *
   * @param helper 模拟步骤辅助类对象的引用
   * @param timeMgr 时间参数管理器对象的引用
   */
  virtual void PostSimulation(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 接收ego信息的虚函数，用以更新simulation
   *
   * @param helper 模拟步骤辅助类对象的引用
   * @param timeMgr 时间参数管理器对象的引用
   */
  virtual void ReceiveEgoInfo(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  // SimulationTraffic is called during the simulation to simulate traffic behavior.
  // It can be used to update the simulation state with traffic behavior information.
  /**
   * @brief 模拟交通流行为的虚函数，用于更新仿真的状态信息
   *
   * @param helper 模拟步骤辅助类对象的引用
   * @param timeMgr 时间参数管理器对象的引用
   */
  virtual void SimulationTraffic(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 仿真模式是否有效
   *
   * @param strScenePath 场景路径
   * @return Base::txBool
   */
  virtual Base::txBool IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 创建场景加载器的虚函数
   *
   * @param strSceneType 场景类型
   * @return Base::ISceneLoaderPtr
   */
  virtual Base::ISceneLoaderPtr CreateSceneLoader(const Base::txString strSceneType) const TX_NOEXCEPT;

  /**
   * @brief 解析初始化参数
   *
   * @param helper 模拟步骤辅助类对象的引用
   */
  virtual void ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT;
};

using WorldsimSimLoopPtr = std::shared_ptr<WorldsimSimLoop>;

// chomper
class ChomperGATemplate : public txPassiveSimTemplate {
  using ParentClass = txPassiveSimTemplate;
};
using ChomperGATemplatePtr = std::shared_ptr<ChomperGATemplate>;
TX_NAMESPACE_CLOSE(TrafficFlow)
