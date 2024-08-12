// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "l2w_linear_element_manager.h"
#include "log2world_assembler_context.h"
#include "tad_simrec_loader.h"
#include "tad_stand_alone_traffic_element_system.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class Log2WorldSystem : public TAD_StandAlone_TrafficElementSystem {
 public:
  using ParentClass = TAD_StandAlone_TrafficElementSystem;
  using EgoSubType = Base::Enums::EgoSubType;

 public:
  Log2WorldSystem() TX_DEFAULT;

  /**
   * @brief 系统实例初始化函数
   *
   * @param loader 一个场景加载器指针
   * @return Base::txBool TX_TRUE 表示初始化成功；TX_FALSE 表示初始化失败。
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 切换模拟器
   *
   * 此函数用于切换模拟器的运行状态。如果提供了合法的时间参数，系统将顺利进行模拟。
   * 如果模拟系统无法处理提供的时间参数，则模拟将会失败。
   *
   * @param timeMgr 时间参数管理器，包含了参数分配的时间和周期
   * @return 无
   * @throw 无
   */
  virtual void SwitchSim(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取ego车辆当前位置
   *
   * 根据传入的子类型参数，获取ego或trailer的地理位置。
   *
   * @param _subType 子类型，用于区分不同的对象
   * @return 返回一个包含经纬度信息的 Location 对象
   * @throw 无
   */
  virtual sim_msg::Location GetEgoLocation(const EgoSubType _subType) const TX_NOEXCEPT;

  /**
   * @brief 更新规划车辆数据
   *
   * 根据传入的子类型参数，更新规划车辆的数据。
   *
   * @param timeMgr 时间参数管理器，用于获取当前时间和周期
   * @param _egoSubType 子类型，用于区分不同的规划车辆
   * @return 返回 true，表示更新成功；返回 false，表示更新失败
   * @throw 无
   */
  virtual Base::txBool UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                             const Base::Enums::EgoSubType _egoSubType) TX_NOEXCEPT;

  /**
   * @brief 获取模拟任务场景加载器
   *
   * 返回模拟任务场景加载器的实例。
   *
   * @return 模拟任务场景加载器的实例
   * @throw 无
   */
  virtual SceneLoader::Simrec_SceneLoaderPtr SimrecSceneLoader() TX_NOEXCEPT { return m_l2w_SceneLoader; }

  /**
   * @brief 获取元素管理器
   *
   * 获取元素管理器的实例。
   *
   * @return 多层元素管理器的实例
   * @throw 无
   */
  virtual L2W_LinearElementManagerPtr MultiLayerElemMgr() TX_NOEXCEPT { return m_l2w_elemMgr; }

  /**
   * @brief 从系统数据获取traffic交通数据
   *
   * 使用传入的时间参数管理器来获取交通数据，将数据存入输出结构体中。
   *
   * @param[in] timeMgr 时间参数管理器
   * @param[out] outTraffic 存储多层交通数据的结构体
   * @return 是否成功获取多层交通数据
   * @retval true 成功获取多层交通数据
   * @retval false 获取多层交通数据失败
   * @throw 无
   */
  virtual Base::txBool FillingTrafficDataShadow(Base::TimeParamManager const& timeMgr,
                                                sim_msg::Traffic& outTraffic) TX_NOEXCEPT;

 protected:
  /**
   * @brief 创建装配器实例
   * 初始化对象中的装配器指针
   */
  virtual void CreateAssemblerCtx() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 创建元素管理器
   * 初始化对象的元素管理器指针
   * @return void
   */
  virtual void CreateElemMgr() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新FCW
   *
   * 使用时间参数管理器更新FCW
   * 返回值表示更新是否成功，当成功时返回 true，否则返回 false。
   *
   * @param[in] timeMgr 时间参数管理器
   * @return 更新是否成功
   * @retval true 更新成功
   * @retval false 更新失败
   */
  virtual Base::txBool Update_FCW(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

 protected:
  /**
   * @brief 更新检查生命周期
   *
   * 使用时间参数管理器更新检查元素生命周期，该函数将检查元素的当前状态。
   * 当元素的状态为正常时，返回 true；否则返回 false。
   *
   * @param[in] timeMgr 时间参数管理器
   * @return 元素状态是否正常
   * @retval true 元素状态正常
   * @retval false 元素状态非正常
   */
  virtual Base::txBool Update_CheckLifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新车辆模拟
   *
   * 使用时间参数管理器更新车辆模拟，该函数将根据对象的当前状态更新模拟。
   *
   * @param[in] timeMgr 时间参数管理器
   * @return 是否更新成功
   * @retval true 更新成功
   * @retval false 更新失败
   */
  virtual Base::txBool Update_Vehicle_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Scene::L2W_AssemblerContextPtr m_l2w_AssemblerContextPtr = nullptr;
  SceneLoader::Simrec_SceneLoaderPtr m_l2w_SceneLoader = nullptr;
  L2W_LinearElementManagerPtr m_l2w_elemMgr = nullptr;
};

using Log2WorldSystemPtr = std::shared_ptr<Log2WorldSystem>;

TX_NAMESPACE_CLOSE(TrafficFlow)
