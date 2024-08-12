// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "its_sim.pb.h"
#include "location.pb.h"
#include "traffic.pb.h"
#include "tx_assembler_context.h"
#include "tx_element_manager_base.h"
#include "tx_marco.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"
#include "tx_type_def.h"
#if USE_EgoGroup
#  include "union.pb.h"
#endif
#include <list>
#if USE_DITW_Event
#  include "tx_external_injection_event.h"
#endif /*USE_DITW_Event*/
#include "tx_scene_sketch.h"
TX_NAMESPACE_OPEN(Base)

class TrafficElementSystem : public SceneSketchBase {
 public:
  TrafficElementSystem() TX_DEFAULT;
  virtual ~TrafficElementSystem();

  /**
   * @brief 初始化
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 更新系统
   *
   * @param timeMgr 时间管理器
   * @return Base::txBool
   */
  virtual Base::txBool Update(const TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 释放资源
   *
   * @return Base::txBool
   */
  virtual Base::txBool Release() TX_NOEXCEPT;

  /**
   * @brief 是否激活
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT { return m_isAlive; }

  /**
   * @brief 是否支持场景类型
   *
   * @param _sceneType 需要判断的场景类型
   * @return Base::txBool
   */
  virtual Base::txBool IsSupportSceneType(const Base::ISceneLoader::ESceneType _sceneType) const TX_NOEXCEPT = 0;

  /**
   * @brief 清除最后一步数据
   *
   */
  virtual void ClearLastStepData() TX_NOEXCEPT;

 public:
  /**
   * @brief 输出traffic信息
   *
   * @param timeMgr 事件管理器
   * @param[out] outTraffic 输出的结果
   * @return Base::txBool
   */
  virtual Base::txBool FillingTrafficData(Base::TimeParamManager const& timeMgr,
                                          sim_msg::Traffic& outTraffic) TX_NOEXCEPT;

  /**
   * @brief 输出位置信息
   *
   * @param timeMgr 时间管理器
   * @param[out] outLocation 输出的位置信息
   * @return Base::txBool
   */
  virtual Base::txBool FillingLocationData(Base::TimeParamManager const& timeMgr,
                                           sim_msg::Location& outLocation) TX_NOEXCEPT {
    return false;
  }

  /**
   * @brief 更新空间检索信息
   *
   * @return Base::txBool
   */
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT;

  /**
   * @brief 更新主车信息
   *
   * @param timeMgr 当前时间戳
   * @param _egoSubType ego类型: ego or trailer
   * @param egoInfoStr 当前location pb的序列化流字符串
   * @return Base::txBool
   */
  virtual Base::txBool UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                             const Base::Enums::EgoSubType _egoSubType,
                                             const Base::txString& egoInfoStr) TX_NOEXCEPT = 0;
#if USE_EgoGroup
  virtual Base::txBool UpdatePlanningCarHighlight(Base::TimeParamManager const& timeMgr,
                                                  const Base::txString& highlightStr) TX_NOEXCEPT {
    return false;
  }
#endif
  virtual Base::txBool RegisterPlanningCar() TX_NOEXCEPT = 0;

  /**
   * @brief 获取ego的类型
   *
   * @return Base::ISceneLoader::EgoType
   */
  virtual Base::ISceneLoader::EgoType GetEgoType() const TX_NOEXCEPT { return m_EgoType; }

  /**
   * @brief 是否存在ego
   *
   * @return Base::txBool
   */
  virtual Base::txBool HasEgo() const TX_NOEXCEPT { return false; }
  void SetGroupName(Base::txString _strGroupName) TX_NOEXCEPT { m_groupName = _strGroupName; }
  Base::txString GetGroupName() TX_NOEXCEPT { return m_groupName; }

  /**
   * @brief 获取场景加载器
   *
   * @return Base::ISceneLoaderPtr
   */
  virtual Base::ISceneLoaderPtr SceneLoader() TX_NOEXCEPT { return m_SceneDataSource; }
  virtual const Base::ISceneLoaderPtr SceneLoader() const TX_NOEXCEPT { return m_SceneDataSource; }

  /**
   * @brief 获取元素管理器
   *
   * @return Base::IElementManagerPtr
   */
  virtual Base::IElementManagerPtr ElemMgr() TX_NOEXCEPT { return m_ElementMgr; }
  virtual const Base::IElementManagerPtr ElemMgr() const TX_NOEXCEPT { return m_ElementMgr; }

  /**
   * @brief 检查仿真结果是否正确
   *
   * @param sendTrafficInfo
   * @return Base::txBool
   */
  virtual Base::txBool CheckSimulationResult(const sim_msg::Traffic& sendTrafficInfo) TX_NOEXCEPT { return true; }
#if USE_DITW_Event

 public:
  /**
   * @brief 注册仿真事件
   *
   * @param evtPtr 事件指针
   * @return Base::txBool
   */
  virtual Base::txBool RegisterSimulationEvent(txInjectionEventPtr evtPtr) TX_NOEXCEPT;

  /**
   * @brief 更新仿真事件
   *
   * @param timeMgr 时间管理器
   * @return Base::txBool
   */
  virtual Base::txBool UpdateSimulationEvent(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 清空仿真时间
   *
   * @param timeMgr 时间管理器
   */
  virtual void ClearSimulationEvent(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

 protected:
  std::list<txInjectionEventPtr> m_vec_injection_event;
#endif /*USE_DITW_Event*/

 protected:
  /**
   * @brief 检查ego类型
   *
   * @param _egoType
   * @param _egoSize
   * @return Base::txBool
   */
  virtual Base::txBool CheckEgoType(const Base::ISceneLoader::EgoType _egoType,
                                    const Base::txInt _egoSize) const TX_NOEXCEPT;

  /**
   * @brief 获取组装器上下文指针
   *
   * @return Base::IAssemblerContextPtr
   */
  virtual Base::IAssemblerContextPtr AssemblerCtx() TX_NOEXCEPT { return m_AssemblerCtx; }

  /**
   * @brief 创建组装器实例
   *
   */
  virtual void CreateAssemblerCtx() TX_NOEXCEPT = 0;

  /**
   * @brief 创建元素管理器
   *
   */
  virtual void CreateElemMgr() TX_NOEXCEPT = 0;

  /**
   * @brief 刷新场景事件
   *
   * @param timeMgr 时间管理器
   */
  virtual void FlushSceneEvents(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {}

  /**
   * @brief 执行环境感知
   *
   * @param timeMgr 时间管理器
   */
  virtual void ExecuteEnvPerception(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 重置预设的所有car
   *
   * @param _max_car_size
   */
  virtual void ResetPreAllocateCar(const Base::txSize _max_car_size) TX_NOEXCEPT {
    if (m_PreAllocateCars.size() < _max_car_size) {
      m_PreAllocateCars.resize(_max_car_size * 2);
    }
  }

  /**
   * @brief 重置所有预设的vehicel
   *
   * @param _max_car_size
   */
  virtual void ResetPreAllocateTxVehicle(const Base::txSize _max_car_size) TX_NOEXCEPT {
    if (m_PreAllocateTxVehicles.size() < _max_car_size) {
      m_PreAllocateTxVehicles.resize(_max_car_size * 2);
    }
  }

  /**
   * @brief 重置预设的所有obstacle
   *
   * @param _max_obs_size
   */
  virtual void ResetPreAllocateObstacle(const Base::txSize _max_obs_size) TX_NOEXCEPT {
    if (m_PreAllocateObstacles.size() < _max_obs_size) {
      m_PreAllocateObstacles.resize(_max_obs_size * 2);
    }
  }

 protected:
  Base::txBool m_isAlive = false;
  Base::ISceneLoaderPtr m_SceneDataSource = nullptr;
  Base::IElementManagerPtr m_ElementMgr = nullptr;
  Base::IAssemblerContextPtr m_AssemblerCtx = nullptr;
  Base::ISceneLoader::EgoType m_EgoType = Base::ISceneLoader::EgoType::eVehicle;
  std::vector<sim_msg::Car> m_PreAllocateCars;
  std::vector<its::txVehicle> m_PreAllocateTxVehicles;
  std::vector<sim_msg::StaticObstacle> m_PreAllocateObstacles;
  Base::txString m_groupName = "";
};

using TrafficElementSystemPtr = std::shared_ptr<TrafficElementSystem>;

TX_NAMESPACE_CLOSE(Base)
