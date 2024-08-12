// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_logger.h"
#include "tx_marco.h"
#include "tx_type_def.h"
#include "txsim_module.h"
#ifdef max
#  undef max
#  undef min
#endif  // max
#include <limits>
#include "control.pb.h"
#include "control_v2.pb.h"
#include "location.pb.h"
#include "traffic.pb.h"
#include "trajectory.pb.h"
#include "tx_loop.h"
#include "tx_sim_time.h"
#include "tx_traffic_element_system.h"
#include "vehicle_interaction.pb.h"
TX_NAMESPACE_OPEN(Base)

class txSimulationTemplate : public txLoop {
 public:
  /**
   * @brief 初始化函数
   *
   * 初始化函数，设置虚拟模拟器的参数、注册各种事件处理回调函数等。
   *
   * @param helper 初始化辅助对象
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT = 0;

  /**
   * @brief 重置模拟器
   *
   * 重置模拟器，并设置模拟器的初始状态。此函数将被调用在模拟器开始运行前，用于初始化模拟器状态。
   *
   * @param helper 重置辅助对象
   */
  virtual void Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT = 0;

  /**
   * @brief Step
   *
   * 用于在模拟循环中执行步骤。根据`helper`参数进行相应的操作，执行步骤并遵循易错性原则。
   *
   * @param helper StepHelper 类的实例，提供模拟循环所需的各种信息和功能。
   */
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT = 0;

  /**
   * @brief Stop 函数停止模拟
   *
   * 用于在模拟循环结束时，停止模拟并尽快返回控制权。在停止期间，根据 stopHelper 参数进行必要的清理工作。
   *
   * @param helper 停止辅助对象，包含停止所需的必要信息。
   */
  virtual void Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT = 0;

  /**
   * @brief 创建环境
   *
   * 创建环境时调用，初始化环境所需的参数和资源，为模拟循环创建必要的场景等。
   *
   * @param helper 重置辅助对象，包含初始化所需的参数和资源。
   */
  virtual void CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT = 0;

  /**
   * @brief 创建系统
   *
   * 在模拟循环开始时调用，用于初始化系统所需的参数和资源，并准备进入模拟循环的前期操作。
   */
  virtual void CreateSystem() TX_NOEXCEPT = 0;

  /**
   * @brief 获取交通系统的智能指针
   *
   * 该函数用于获取交通系统的智能指针，可以在模拟循环开始前调用该函数。
   * 返回值：智能指针，指向交通系统的实现类对象
   */
  virtual TrafficElementSystemPtr TrafficSystemPtr() TX_NOEXCEPT = 0;

  /**
   * @brief PreSimulation 是在模拟循环开始前调用的虚函数
   *
   * 在函数 PreSimulation 中，可以对模拟进行一些前置操作，比如读取数据或调整模拟参数等。
   *
   * @param helper 辅助类，用于获取和修改模拟数据
   * @param timeMgr 时间管理器类，包含了模拟循环当前的时间信息
   */
  virtual void PreSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief PreSimulation 是在模拟循环开始前调用的虚函数
   *
   * 在函数 PreSimulation 中，可以对模拟进行一些前置操作，比如读取数据或调整模拟参数等。
   *
   * @param helper 辅助类，用于获取和修改模拟数据
   * @param timeMgr 时间管理器类，包含了模拟循环当前的时间信息
   */
  virtual void Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 虚拟函数，模拟循环结束后调用
   *
   * 在虚拟函数 PostSimulation 中，可以根据模拟结果和参数对系统进行控制和操作。
   *
   * @param helper StepHelper 类对象，用于获取和修改模拟中的数据
   * @param timeMgr TimeParamManager 类对象，包含模拟循环的时间信息和参数
   */
  virtual void PostSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 判断模拟模式是否有效
   * @param strScenePath 场景路径
   * @return true 如果模拟模式有效
   * @return false 如果模拟模式无效
   */
  virtual Base::txBool IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT { return true; }

 protected:
  /**
   * @brief 发布消息
   *
   * 这个函数用于将解析好的ProtoBuf消息发布到指定的话题中。
   *
   * @param helper 帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 话题列表，多个话题用英文逗号分隔
   * @param strPB 已解析好的ProtoBuf格式的消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics, const txString& strPB) TX_NOEXCEPT;

  /**
   * @brief 发布Traffic消息
   *
   * 这个函数用于将解析好的ProtoBuf消息发布到指定的话题中。
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param sendTrafficInfo sim_msg::Traffic类型的已解析好的ProtoBuf格式的消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                              const sim_msg::Traffic& sendTrafficInfo) TX_NOEXCEPT;

  /**
   * @brief 发布Location消息
   *
   * 该函数用于根据设定的主题和发送位置，将解析好的sim_msg::Location类型的消息发布到指定的话题中。
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param sendLocation sim_msg::Location类型的已解析好的ProtoBuf格式的消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                              const sim_msg::Location& sendLocation) TX_NOEXCEPT;

  /**
   * @brief 发布Trajectory消息
   *
   * 这个函数用于根据设定的主题和发送位置，将解析好的sim_msg::Trajectory类型的消息发布到指定的话题中。
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param sendTrajectory sim_msg::Trajectory类型的已解析好的ProtoBuf格式的消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                              const sim_msg::Trajectory& sendTrajectory) TX_NOEXCEPT;

  /**
   * @brief 发布VehicleInteraction消息
   *
   * 该函数用于向指定的话题发布VehicleInteraction类型的消息。
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param sendVehicleInteraction sim_msg::VehicleInteraction类型的已解析好的ProtoBuf格式的消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                              const sim_msg::VehicleInteraction& sendVehicleInteraction) TX_NOEXCEPT;

  /**
   * @brief 向指定的Control话题发布控制消息
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param sendControl sim_msg::Control类型的已解析好的ProtoBuf格式的控制消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                              const sim_msg::Control& sendControl) TX_NOEXCEPT;

  /**
   * @brief 向指定的Control_V2话题发布控制消息
   *
   * 该函数向指定的话题发布控制消息。
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param sendControl sim_msg::Control_V2类型的已解析好的ProtoBuf格式的控制消息字符串
   */
  virtual void PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                              const sim_msg::Control_V2& sendControl) TX_NOEXCEPT;

  /**
   * @brief 获取订阅的消息
   *
   * 从指定的话题获取消息，并将其以ProtoBuf格式的字符串形式返回。
   *
   * @param helper tx_sim::StepHelper类型的帮助对象，用于获取和设置模拟过程中的一些信息
   * @param strTopics 字符串类型的话题列表，多个话题用英文逗号分隔
   * @param strPB 字符串类型的ProtoBuf格式的消息字符串
   * @return 无
   */
  virtual void GetSubscribedMessage(tx_sim::StepHelper& helper, const txString& strTopics, txString& strPB) TX_NOEXCEPT;

  /**
   * @brief 重置变量
   *
   * 重置用于模拟的变量，使其回到初始状态。
   *
   * @param TX_NOEXCEPT 保证在异常情况下不会抛出异常
   * @return 无
   */
  virtual void ResetVars() TX_NOEXCEPT = 0;

  /**
   * @brief 创建时间参数管理器
   *
   * 这个虚函数用于创建一个时间参数管理器对象，以便在模拟过程中进行参数的设置和获取。
   *
   * @param time_stamp 当前模拟的时间戳
   * @return 返回一个时间参数管理器对象，用于设置和获取模拟参数
   */
  virtual TimeParamManager MakeTimeMgr(const txFloat time_stamp) TX_NOEXCEPT = 0;
};
using txSimulationTemplatePtr = std::shared_ptr<txSimulationTemplate>;

class txSimulationLoop : public tx_sim::SimModule {
 public:
  enum class SimulationMode : int {
    eNone,
    eWorldSim,
    eLogSim,
    eVisualizer,
    eDummyCar,
    eVirtualCiy,
    eParallelSimulation,
    eScenePreview,
    eDITW,
    eManualVehicle,
    eXoscReplay,
    eFilter, /*eChannel,*/
    eL2wDesktop,
    eADASEgo,
    eVirtualCityEgo,
    eSimEgo,
    eBillboard,
    eDriverAi,
    eDriverUserDefined,
    eDriverTrajFollow,
    eCloudStandAlone,
    eCoSim_Vissim
  };

 public:
  txSimulationLoop() TX_DEFAULT;
  virtual ~txSimulationLoop() TX_DEFAULT;

  /**
   * @brief 初始化函数
   *
   * 这个虚函数用于在模拟过程开始时对时间模拟进行初始化。
   * 需要在模拟开始前调用该函数，初始化各种仿真参数。
   *
   * @param helper 初始化帮助对象，包含各种仿真所需的参数
   */
  virtual void Init(tx_sim::InitHelper& helper) TX_OVERRIDE {}

  /**
   * @brief 重置函数
   *
   * 这个虚函数用于在模拟过程开始时对时间模拟进行重置。
   * 需要在模拟开始前调用该函数，重置各种仿真参数。
   *
   * @param helper 重置帮助对象，包含各种仿真所需的参数
   */
  virtual void Reset(tx_sim::ResetHelper& helper) TX_OVERRIDE {}

  /**
   * @brief Step函数
   *
   * Step函数用于模拟过程中的每一步，提供帮助对象helper以进行模拟操作。
   * @param helper tx_sim::StepHelper类型的帮助对象，包含模拟所需的参数和状态信息。
   */
  virtual void Step(tx_sim::StepHelper& helper) TX_OVERRIDE {}

  /**
   * @brief Stop函数
   *
   * Stop函数用于模拟停止时的操作。通过传递帮助对象helper，可以在此时获取当前模拟状态并进行适当的操作。
   * @param helper tx_sim::StopHelper类型的帮助对象，包含模拟所需的参数和状态信息。
   */
  virtual void Stop(tx_sim::StopHelper& helper) TX_OVERRIDE {}

  /**
   * @brief Get simulation mode
   *
   * @return SimulationMode 当前模拟模式
   */
  virtual SimulationMode SimMode() const TX_NOEXCEPT { return m_SimMode; }

 protected:
  SimulationMode m_SimMode = SimulationMode::eNone;
  Base::txString m_Param_SimMode;
};
using txSimulationLoopPtr = std::shared_ptr<txSimulationLoop>;
TX_NAMESPACE_CLOSE(Base)
