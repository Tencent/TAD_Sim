// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_ai_vehicle_element.h"
#include "tx_component.h"
#include "tx_tadsim_flags.h"
#include "tx_traj_manager_base.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class L2W_VehicleElement : public TAD_AI_VehicleElement {
 public:
  enum class L2W_Status : Base::txInt { l2w_log = 1, l2w_world = 2 };
  using ParentClass = TAD_AI_VehicleElement;

  /**
   * @brief 初始化元素
   *
   * @param ViewerPtr 玩家游戏角色
   * @param sceneLoaderPtr 场景加载器
   * @return txBool 初始化成功返回 true，否则返回 false
   */
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断当前元素是否支持指定的行为
   *
   * 此函数用于判断当前元素是否支持指定的行为。
   *
   * @param behavior 指定的行为
   * @return txBool 当前元素支持指定行为时返回 true，否则返回 false
   */
  virtual txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新元素状态
   *
   * 更新元素状态，通常用于控制动画、逻辑等相关操作。
   *
   * @param timeMgr 时间参数管理器，包含了当前时间等信息
   * @return txBool 更新成功时返回 true，否则返回 false
   */
  virtual txBool Update(TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief FillingElement 使用给定的时间戳和汽车获取traffic信息，
   *
   * @param[in] timeStamp 时间戳，单位为秒。
   * @param[in,out] refTraffic traffic存储对象
   * @return 获取成功返回true
   */
  virtual Base::txBool FillingElement(Base::TimeParamManager const& timeMgr,
                                      sim_msg::Traffic& refTraffic) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief FillingElement 使用给定的时间戳和汽车实例更新汽车元素，并返回更新后的汽车实例指针。
   *
   * @param[in] timeStamp 时间戳，单位为秒。
   * @param[in,out] pSimVehicle 要更新的汽车实例指针。
   * @return 返回更新后的汽车实例指针，如果更新失败则返回空指针。
   */
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief PreFillingElement 在设定大小前执行操作.
   *
   * 执行此函数时, 实现根据需要修改输入参数(atomicSize)或者对函数执行前的处理, 进而阻止元素的填充。
   * @param[in,out] a_size 设定元素大小，若函数返回false则表示设定大小失败。
   * @return TX_TRUE 表示继续执行元素填充；TX_FALSE 表示不执行元素填充。
   */
  virtual txBool PreFillingElement(AtomicSize& a_size) TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief IsLogSim 获取该VehicleElement是否为Log Sim元素
   * @return TX_TRUE 表示该VehicleElement为Log Sim元素；TX_FALSE 表示该VehicleElement不是Log Sim元素。
   */
  virtual Base::txBool IsLogSim() const TX_NOEXCEPT { return L2W_Status::l2w_log == m_l2w_status; }

  /**
   * @brief 该元素是否是worldsim状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsWorldSim() const TX_NOEXCEPT { return L2W_Status::l2w_world == m_l2w_status; }

  /**
   * @brief 切换成log2world
   *
   */
  virtual void SwitchLog2World() TX_NOEXCEPT;

  /**
   * @brief logsim的pb是否准备好
   *
   * @return txBool 准备好返回true
   */
  virtual txBool IsLogSimPbReady() const TX_NOEXCEPT { return m_logsim_pb_ready; }

  /**
   * @brief Set the Log Sim Pb Ready Flag
   *
   * @param _f 设置的状态
   */
  virtual void SetLogSimPbReadyFlag(const txBool _f) TX_NOEXCEPT { m_logsim_pb_ready = _f; }

 protected:
  /**
   * @brief 切换absorblane
   *
   * @return Base::txBool
   */
  Base::txBool SwitchAbsorbLane() const TX_NOEXCEPT { return FLAGS_L2W_SwitchAbsorb; }

  /**
   * @brief 更新logsim函数
   *
   * @return Base::txBool 返回更新是否成功
   */
  virtual Base::txBool UpdateLogSim(TimeParamManager const&) TX_NOEXCEPT;

  /**
   * @brief 更新logsim的vehicle
   *
   * @param timeMgr
   * @return Base::txBool
   */
  virtual Base::txBool UpdateLogSimVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 生命周期开始前准备
   *
   * @param timeMgr 事件参数管理器
   */
  virtual void OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 生命周期是否结束
   *
   * @param timeMgr 时间参数管理器
   * @return txBool 返回是否结束
   */
  virtual txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  L2W_Status m_l2w_status = L2W_Status::l2w_log;
  txTrajManagerBasePtr m_traj_mgr = nullptr;
  txBool m_logsim_pb_ready = false;
  txFloat m_scene_start_time_ms = 0.0;
};

using L2W_VehicleElementPtr = std::shared_ptr<L2W_VehicleElement>;

#if __TX_Mark__("shadow")
class L2W_VehicleElement_Shadow : public L2W_VehicleElement {
 public:
  using ParentClass = L2W_VehicleElement;

  /**
   * @brief 更新空间查询
   *
   * @return txBool 更新成功返回true
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化时注册位置信息
   *
   * @param _location 位置信息
   */
  virtual void RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 更新hash过的车道信息
   *
   * @param _location 位置信息
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief kill时注销信息
   *
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 切换为log2world模式
   *
   */
  virtual void SwitchLog2World() TX_NOEXCEPT TX_OVERRIDE {}
};
using L2W_VehicleElement_ShadowPtr = std::shared_ptr<L2W_VehicleElement_Shadow>;
#endif /*__TX_Mark__("shadow")*/

TX_NAMESPACE_CLOSE(TrafficFlow)
