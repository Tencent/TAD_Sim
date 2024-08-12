// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "log2world_system.h"
#include "worldsim_sim_template.h"
#include "log2world_trigger.pb.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class Log2WorldSimLoop : public WorldsimSimLoop {
  using txString = Base::txString;
  using txFloat = Base::txFloat;
  using txVec3 = Base::txVec3;
  using txPoint = hadmap::txPoint;
  using txUInt = Base::txUInt;
  using txULong = Base::txULong;
  using txWGS84 = Coord::txWGS84;
  using txENU = Coord::txENU;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txLaneID = Base::txLaneID;
  using ParentClass = WorldsimSimLoop;
  enum L2W_Status : txInt { eLogsim = 0, eWorldsim = 1 };

 public:
  /**
   * @brief 初始化模块
   *
   * 该函数用于初始化模块。在该函数中，可以对模块进行初始化操作，例如指定模块的名称、类型等信息，并分配内存等。
   *
   * @param helper 初始化帮助器，包含一些有用的初始化信息
   * @return 无
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 创建系统
   *
   * 此函数用于创建应用程序的系统。在此函数中，应用程序需要初始化各种系统模块，如音频、视频、图形界面等，并提供必要的初始化参数。
   *
   * @param helper 初始化帮助器，包含一些有用的初始化信息
   * @return 无
   */
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 获取log2world系统对象指针
   * @return Log2WorldSystemPtr 返回log2world系统对象指针
   */
  virtual Log2WorldSystemPtr Log2WorldTrafficSystemPtr() TX_NOEXCEPT { return m_l2w_system_ptr; }

 protected:
  /**
   * @brief 重置变量
   * 重置用于存储变量的数据结构
   */
  virtual void ResetVars() TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief PreSimulation 用于仿真前预处理
   *
   * @param helper 用于获取系统上下文信息和调用其他函数
   * @param timeMgr 包含用于计算模拟时间参数和设置模拟速度的函数
   * @return 无返回值
   */
  virtual void PreSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 仿真后置处理
   *
   * 此方法在模拟结束时被调用。
   * 使用步骤帮助管理器和上下文信息进行后续操作。
   *
   * @param helper 用于获取模拟相关信息和调用相关接口的步骤帮助管理器
   * @param timeMgr 包含模拟时间参数和设置模拟速度的函数的时间参数管理器
   */
  virtual void PostSimulation(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 接收ego信息的方法
   *
   * 通过帮助器接收并处理ego的信息，通常是在模拟开始前调用该方法获取ego信息。
   *
   * @param helper 在帮助器中获取模拟相关信息和调用相关接口的步骤帮助管理器
   * @param timeMgr 包含模拟时间参数和设置模拟速度的函数的时间参数管理器
   */
  virtual void ReceiveEgoInfo(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间仿真交通状况
   *
   * 该函数用于根据当前时间模拟交通状况，如流量控制、碰撞检测等。
   * 根据执行环境提供的信息，可以自定义具体实现。
   *
   * @param helper 步骤帮助管理器，带有获取模拟相关信息和调用模拟相关接口的功能
   * @param timeMgr 时间参数管理器，用于获取或设置模拟速度等参数
   */
  virtual void SimulationTraffic(tx_sim::StepHelper& helper,
                                 const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 判断是否是logsim
   *
   * @return txBool 是logsim返回true
   */
  virtual txBool IsLogSim() const TX_NOEXCEPT { return L2W_Status::eLogsim == m_l2w_status; }

  /**
   * @brief 判断是否是worldsim
   *
   * @return txBool 是worldsim返回true
   */
  virtual txBool IsWorldSim() const TX_NOEXCEPT { return L2W_Status::eWorldsim == m_l2w_status; }

  /**
   * @brief 是否需要触发worldsim
   *
   * 当需要触发worldsim时返回 true，否则返回
   * false。这通常是检查时间、环境变量、外部条件等因素，以确定是否需要进行worldsim。
   *
   * @param helper 步骤帮助管理器，包含获取模拟相关信息和调用模拟相关接口的功能
   * @param timeMgr 时间参数管理器，用于获取或设置模拟速度等参数
   * @return txBool 是否需要触发worldsim
   */
  virtual txBool NeedTriggerWorldSim(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 调试特殊时间下，特定功能的开启状态
   *
   * 在特定的特殊时间段内，检查特定的调试功能是否被开启。需要在构造函数中调用 SetTimeParamInfo
   * 方法填充相关时间参数信息。
   *
   * @param timeMgr 时间参数管理器，用于获取或设置模拟速度等参数
   * @return txBool 特定时间是否激活特定的调试功能
   */
  virtual txBool Debug_L2W_Switch_SpecialTime(const Base::TimeParamManager& timeMgr) const TX_NOEXCEPT;

  /**
   * @brief 获取特殊时间开关状态
   * @return bool 返回特殊时间开关状态
   */
  virtual txBool IsSpecialTimeSwitch() const TX_NOEXCEPT { return FLAGS_L2W_Switch_SpecialTime >= 0.0; }

  /**
   * @brief 计算场景类型
   * @param scene_path 场景文件路径
   * @return txString 返回场景类型
   */
  virtual txString ComputeSceneType(const txString scene_path) TX_NOEXCEPT { return "Simrec"; }

  /**
   * @brief 创建场景加载器
   * @param strSceneType 场景类型
   * @return Base::ISceneLoaderPtr 返回场景加载器指针
   */
  virtual Base::ISceneLoaderPtr CreateSceneLoader(const Base::txString strSceneType) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断场景模式是否有效
   * @param strScenePath 场景文件路径
   * @return bool 是否有效
   */
  virtual Base::txBool IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Log2WorldSystemPtr m_l2w_system_ptr = nullptr;
  L2W_Status m_l2w_status = L2W_Status::eLogsim;
  sim_msg::Log2worldTriggerType m_l2w_trigger_cmd = sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_NONE;
  sim_msg::Traffic m_output_ShadowTraffic;
};

using Log2WorldSimLoopPtr = std::shared_ptr<Log2WorldSimLoop>;

TX_NAMESPACE_CLOSE(TrafficFlow)
