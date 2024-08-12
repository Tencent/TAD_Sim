// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_traffic_element_system.h"
TX_NAMESPACE_OPEN(TrafficFlow)
class txPassiveSimTemplate {
 public:
  txPassiveSimTemplate() TX_DEFAULT;
  virtual ~txPassiveSimTemplate() TX_DEFAULT;

 public:
  /**
   * @brief 定义一个虚拟函数，用于创建一个环境对象
   *       该函数属于Base命名空间，返回类型为Base::txBool
   * @param loader 一个ISceneLoaderPtr类型的指针，用于加载场景
   * @param sim_path 一个字符串类型的变量，表示模拟路径
   * @param scene_path 一个字符串类型的变量，表示场景路径
   * @param map_path 一个字符串类型的变量，表示地图路径
   * @return Base::txBool
   */
  virtual Base::txBool CreateEnvPassive(Base::ISceneLoaderPtr loader, const Base::txString sim_path,
                                        const Base::txString scene_path, const Base::txString map_path) TX_NOEXCEPT;
  /**
   * @brief 执行被动仿真
   *
   * @param timeStamp_ms 当前时间戳
   * @param loc_pb_ptr 位置信息指针
   * @param traffic_pb_ptr 交通状态指针
   * @param traj_pb_ptr 轨迹信息指针
   *
   * @return 执行成功返回 true，否则返回 false
   */
  virtual Base::txBool SimulationPassive(const Base::txFloat timeStamp_ms, sim_msg::Location* loc_pb_ptr,
                                         sim_msg::Traffic* traffic_pb_ptr,
                                         sim_msg::Trajectory* traj_pb_ptr) TX_NOEXCEPT;
  /**
   * @brief 创建系统，此函数将初始化模拟系统并创建所有必要的对象和模块以进行仿真.
   *
   * @param sim_path 仿真所在的路径.
   * @param scene_path 场景文件路径.
   * @param map_path 地图文件路径.
   */
  virtual void CreateSystemPassive(const Base::txString sim_path, const Base::txString scene_path,
                                   const Base::txString map_path) TX_NOEXCEPT;

  /**
   * @brief 实现passive模式下的交通数据模拟
   *
   * 在passive模式下，该函数从应用层获取交通数据参数，生成交通信号状态，并存入protobuf交通消息结构体中
   *
   * @param timeMgr 时间管理器，包含当前时间、启动时间等信息
   * @param traffic_pb_ptr 指向Traffic protobuf消息结构体的指针，用于存放模拟交通信号状态数据
   * @return void
   */
  virtual void SimulationTrafficPassive(const Base::TimeParamManager& timeMgr,
                                        sim_msg::Traffic* traffic_pb_ptr) TX_NOEXCEPT;
  /**
   * @brief 在passive模拟中重置系统变量.
   *
   * 该函数用于在passive模拟中重置系统变量.
   *
   * @param _sys_ptr 指向流量系统元素的指针
   * @return void
   */
  virtual void ResetVarsPassive(Base::TrafficElementSystemPtr _sys_ptr) TX_NOEXCEPT;

  /**
   * @brief 获取traffic元素系统的指针
   *
   * @return Base::TrafficElementSystemPtr
   */
  virtual Base::TrafficElementSystemPtr TrafficSystemPtrPassive() TX_NOEXCEPT { return m_TrafficElementSystem_Ptr; }
  /**
   * @brief 更新获取时间管理器指针
   *
   * @param timeStamp_ms  当前时间戳
   * @return Base::TimeParamManager 时间管理器指针
   */
  virtual Base::TimeParamManager MakeTimeMgrPassive(const Base::txFloat timeStamp_ms) TX_NOEXCEPT;
  /**
   * @brief 获取驾驶行为的json字符串
   *
   * @return Base::txString
   */
  virtual Base::txString RawVehicleBehaviorJson() const TX_NOEXCEPT;

 protected:
  // Pointer to the TrafficElementSystem
  Base::TrafficElementSystemPtr m_TrafficElementSystem_Ptr = nullptr;
  // Last step time
  Base::txFloat m_lastStepTime = 0.0;
  // Passed time
  Base::txFloat m_passTime = 0.0;
  // Step count
  Base::txSize m_step_count_ = 0;
  // Maximum step count
  Base::txSize m_max_step_count_ = std::numeric_limits<Base::txSize>::max();
  // Payload
  Base::txString m_payload_;
  // Payload trailer
  Base::txString m_payload_trailer_;
  // Output traffic
  sim_msg::Traffic m_outputTraffic;
  // Inject vehicle behavior database
  Base::txString m_inject_vehicle_behavior_db;
};

using txPassiveSimTemplatePtr = std::shared_ptr<txPassiveSimTemplate>;

TX_NAMESPACE_CLOSE(TrafficFlow)
