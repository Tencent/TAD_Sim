// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_simulation_loop.h"
#include "tx_tc_template.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class CloudStandAloneTemplate : public Base::txSimulationTemplate, public PluginTemplate {
 public:
  /**
   * @brief Init 初始化
   * @param helper 初始化帮助器，包含初始化所需的所有信息
   * @details 根据 InitHelper 传入的参数，初始化当前模块的相关功能
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重置模块
   * @param helper 重置帮助器，包含重置所需的所有信息
   * @details 根据 ResetHelper 传入的参数，重置当前模块的相关功能
   */
  virtual void Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Step 执行步骤
   * @param helper 步骤帮助器，包含执行所需的所有信息
   * @details 根据 StepHelper 传入的参数，执行当前模块的相关功能
   */
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Stop 停止当前模块
   * @param helper StopHelper 类型的参数，包含停止所需的所有信息
   * @details 根据 StopHelper 传入的参数，停止当前模块的相关功能
   */
  virtual void Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief CreateEnv 创建环境
   * @param helper tx_sim::ResetHelper 类型的参数，包含初始化所需的所有信息
   * @details 根据 tx_sim::ResetHelper 传入的参数，创建并初始化一个环境
   */
  virtual void CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 创建系统
   * @details 创建系统的初始化操作
   */
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE { CreateSystemPlugin(); }

  /**
   * @brief 获取traffic系统指针
   * @return Base::TrafficElementSystemPtr 返回流量系统指针
   */
  virtual Base::TrafficElementSystemPtr TrafficSystemPtr() TX_NOEXCEPT TX_OVERRIDE { return CloudTrafficSystemPtr(); }

  /**
   * @brief 对象模拟步骤
   *
   * @param helper StepHelper类型的参数，包含辅助信息
   * @param timeMgr Base::TimeParamManager类型的参数，包含时间信息
   * @return TX_NOEXCEPT 函数不抛出异常
   * @return TX_OVERRIDE 函数重写基类函数
   */
  virtual void Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重置变量
   *
   * @param TX_NOEXCEPT 函数不抛出异常
   * @param TX_OVERRIDE 函数重写基类函数
   */
  virtual void ResetVars() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 构造一个时间管理器实例
   *
   * @param time_stamp 用于设置时间戳的变量
   * @return Base::TimeParamManager 构造好的时间管理器实例
   */
  virtual Base::TimeParamManager MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 解析初始化参数
   *
   * 从初始化帮助器对象中解析初始化参数
   *
   * @param helper 初始化帮助器对象的引用
   */
  virtual void ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT;

  /**
   * @brief PreSimulation 准备模拟
   *
   * 每个模拟循环开始之前需要调用的方法
   *
   * @param helper tx_sim::StepHelper 对象的引用，用于访问step阶段辅助信息
   * @param timeMgr const Base::TimeParamManager& 时间参数管理器的引用，用于访问当前模拟的时间信息
   */
  virtual void PreSimulation(tx_sim::StepHelper& helper,
                             const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 结束模拟时执行的钩子函数
   *
   * 每个模拟循环结束之后需要调用的函数，通常用于清理资源
   * 或执行一些需要在模拟结束时执行的操作
   *
   * @param helper tx_sim::StepHelper 对象的引用，用于访问当前步骤的辅助信息
   * @param timeMgr const Base::TimeParamManager& 时间参数管理器的引用，用于访问当前模拟的时间信息
   */
  virtual void PostSimulation(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 接收ego信息的回调函数
   *
   * 本函数在模拟循环的每个步骤中被调用，用于接收驾驶员的控制信息和传感器信息，
   * 以及车辆的定位信息、速度、位置等属性。
   *
   * @param helper tx_sim::StepHelper 对象的引用，用于访问当前步骤的辅助信息
   * @param timeMgr const Base::TimeParamManager& 时间参数管理器的引用，用于访问当前模拟的时间信息
   */
  virtual void ReceiveEgoInfo(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  /**********************************************************/
#if 0

 public:
  virtual void CreateEnv(const Base::txBool res_ego, sim_msg::Location& loc) TX_NOEXCEPT;
  virtual TrafficFlow::CloudTrafficElementSystemPtr CloudTrafficSystemPtr() TX_NOEXCEPT {
    return m_TrafficElementSystemPtr;
  }
  virtual TrafficFlow::ParallelSimulation_TrafficElementSystemPtr ParallelSimulationSystemPtr() TX_NOEXCEPT {
    return m_ps_TrafficElementSystemPtr;
  }
  virtual void Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txBool ResetHdMapCache(const Base::txBool _f) TX_NOEXCEPT {
    resetHdMapCache = _f;
    return resetHdMapCache;
  }
  virtual Base::txBool IsResetHdMapCache() const TX_NOEXCEPT { return resetHdMapCache; }
  Scene::InitInfoWrap& InputeParams() TX_NOEXCEPT { return inputeParams; }
  const Scene::InitInfoWrap& InputeParams() const TX_NOEXCEPT { return inputeParams; }

  virtual Base::TimeParamManager MakeTimeMgrRelativeTimeInSecond(const Base::txFloat relativeTimeInSec) TX_NOEXCEPT;
  virtual Base::TimeParamManager MakeTimeMgrAbsTimeInMillisecondSecond(const uint64_t abs_time_ms) TX_NOEXCEPT;
  virtual Base::TimeParamManager GetTimeMgr() const TX_NOEXCEPT { return m_cloud_time_mgr; }

  virtual void SimulationTraffic(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txBool initialize_l2w_sim(const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */,
                                          const its::txStatusRecord& status_pb_record /* size = 1 */,
                                          const std::string _event_json) TX_NOEXCEPT;
  virtual Base::txBool inject_traffic_event(const std::string _event_json) TX_NOEXCEPT;

 protected:
  virtual Base::ISceneLoaderPtr CreateSceneLoader() const TX_NOEXCEPT;

 protected:
  Base::txBool isParallelSimulation() const TX_NOEXCEPT { return inputeParams.isParallelSimulation(); }
  Base::txBool isVirtualCity() const TX_NOEXCEPT { return inputeParams.isVirtualCity(); }
#  if __pingsn__

 protected:
  Base::txBool loadVehicles(const its::txSimStatus& startStatus) TX_NOEXCEPT;

 public:
  void TrafficRecord_PartialClear() TX_NOEXCEPT { trafficRecord_.PartialClear(); }
  void TrafficRecord_RecordInputVehicles(const int64_t frame_id, its::txVehicles& info) TX_NOEXCEPT {
    trafficRecord_.RecordInputVehicles(info);
  }
  void TrafficRecord_GetCurTrafficState(const int64_t frame_id, its::txSimStatus& sceneSnapshot) TX_NOEXCEPT;
  Base::txBool GetMapTrafficInfo(const int64_t frame_id, sim_msg::Traffic& traffic) TX_NOEXCEPT;
  void TrafficRecord_RecordInputVehicle(const its::txVehicle& vehicleMsg) TX_NOEXCEPT;
  Base::txBool addVehicleToElementManager(Base::txSysId vehId, Base::txInt vehType, Base::txLaneID vehicleStartLaneId,
                                          const std::vector<int64_t>& roadIds, Base::txFloat startV,
                                          Base::txFloat roadStartPointX, Base::txFloat roadStartPointY,
                                          Base::txFloat roadStartPointZ) TX_NOEXCEPT;
  its::txSimStat GetSimStatMsg() const TX_NOEXCEPT;
#  endif /*__pingsn__*/

 protected:
  TrafficFlow::CloudTrafficElementSystemPtr m_TrafficElementSystemPtr = nullptr;
  TrafficFlow::ParallelSimulation_TrafficElementSystemPtr m_ps_TrafficElementSystemPtr = nullptr;
  Base::txFloat m_lastStepTime = 0.0;
  Base::txFloat m_passTime = 0.0;
  Base::txSize m_step_count_ = 0;
  Base::txSize m_max_step_count_ = std::numeric_limits<Base::txSize>::max();
  Base::txString m_payload_;
  Base::txString m_payload_trailer_;
  sim_msg::Traffic m_outputTraffic;
  Base::txBool resetHdMapCache = true;
  Scene::InitInfoWrap inputeParams;
  sim_msg::VehicleGeometoryList vehGeomList;
  TrafficFlow::TrafficRecord trafficRecord_;
  Base::TimeParamManager m_cloud_time_mgr;
#  if __pingsn__
  sim_msg::Traffic trafficMsg_;
  uint64_t cur_AbsTime = 0; /*unix timestamp*/
#  endif /*__pingsn__*/

#endif
};

using CloudStandAloneTemplatePtr = std::shared_ptr<CloudStandAloneTemplate>;

TX_NAMESPACE_CLOSE(TrafficFlow)
