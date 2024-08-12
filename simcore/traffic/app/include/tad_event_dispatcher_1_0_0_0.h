// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_scene_event_1_0_0_0.h"
#include "tx_element_manager_base.h"
#include "tx_enum_def.h"
#include "tx_event_dispatcher.h"
#include "tx_scene_loader.h"
#include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class txEventDispatcher_v_1_0_0_0 {
 public:
  using txBool = Base::txBool;
  using ISceneEventViewerPtr = Base::ISceneLoader::ISceneEventViewerPtr;
  using SceneEventType = Base::ISceneLoader::ISceneEventViewer::IConditionViewer::SceneEventType;
  using IConditionViewer = Base::ISceneLoader::ISceneEventViewer::IConditionViewer;
  using IConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IConditionViewerPtr;
  using IConditionViewerPtrVec = Base::ISceneLoader::ISceneEventViewer::IConditionViewerPtrVec;

  using IVelocityTriggerConditionViewer = Base::ISceneLoader::ISceneEventViewer::IVelocityTriggerConditionViewer;
  using IVelocityTriggerConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IVelocityTriggerConditionViewerPtr;
  using IReachAbsPositionConditionViewer = Base::ISceneLoader::ISceneEventViewer::IReachAbsPositionConditionViewer;
  using IReachAbsPositionConditionViewerPtr =
      Base::ISceneLoader::ISceneEventViewer::IReachAbsPositionConditionViewerPtr;
  using IReachAbsLaneConditionViewer = Base::ISceneLoader::ISceneEventViewer::IReachAbsLaneConditionViewer;
  using IReachAbsLaneConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IReachAbsLaneConditionViewerPtr;
  using ITimeTriggerViewer = Base::ISceneLoader::ISceneEventViewer::ITimeTriggerViewer;
  using ITimeTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::ITimeTriggerViewerPtr;
  using ITTCTriggerViewer = Base::ISceneLoader::ISceneEventViewer::ITTCTriggerViewer;
  using ITTCTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::ITTCTriggerViewerPtr;
  using IDistanceTriggerViewer = Base::ISceneLoader::ISceneEventViewer::IDistanceTriggerViewer;
  using IDistanceTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::IDistanceTriggerViewerPtr;
  using IEgoAttachLaneUidCustomViewer = Base::ISceneLoader::ISceneEventViewer::IEgoAttachLaneUidCustomViewer;
  using IEgoAttachLaneUidCustomViewerPtr = Base::ISceneLoader::ISceneEventViewer::IEgoAttachLaneUidCustomViewerPtr;

  using ITimeHeadwayTriggerViewer = Base::ISceneLoader::ISceneEventViewer::ITimeHeadwayTriggerViewer;
  using ITimeHeadwayTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::ITimeHeadwayTriggerViewerPtr;
  using IElementStateTriggerViewer = Base::ISceneLoader::ISceneEventViewer::IElementStateTriggerViewer;
  using IElementStateTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::IElementStateTriggerViewerPtr;

  using ElementType = Base::Enums::ElementType;

  using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
  using ConditionDistanceType = Base::Enums::ConditionDistanceType;
  using ConditionSpeedType = Base::Enums::ConditionSpeedType;
  using ConditionEquationOp = Base::Enums::ConditionEquationOp;
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;

 public:
  /**
   * @brief 单例模式，获取dispatch实例
   *
   * @return txEventDispatcher_v_1_0_0_0&
   */
  static txEventDispatcher_v_1_0_0_0& getInstance() {
    static txEventDispatcher_v_1_0_0_0 instance;
    return instance;
  }

 private:
  txEventDispatcher_v_1_0_0_0() TX_DEFAULT;
  ~txEventDispatcher_v_1_0_0_0() TX_DEFAULT;
  txEventDispatcher_v_1_0_0_0(const txEventDispatcher_v_1_0_0_0&) TX_DEFAULT;
  txEventDispatcher_v_1_0_0_0& operator=(const txEventDispatcher_v_1_0_0_0&) TX_DEFAULT;

 public:
  /**
   * @brief 初始化事件处理器
   *
   */
  void Initialize() TX_NOEXCEPT { m_vec_tuple_eventList_handler.clear(); }

  /**
   * @brief 清空事件处理器
   *
   */
  void ClearEvent() TX_NOEXCEPT { m_vec_tuple_eventList_handler.clear(); }

  /**
   * @brief 刷新所有事件
   * @param timeMgr 时间参数管理器
   * @param _elemPtr 元素管理器指针
   * @param _sceneLoader 场景加载器指针
   */
  void FlushAllEvents(const Base::TimeParamManager& timeMgr, Base::IElementManagerPtr _elemPtr,
                      Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 注册场景事件处理器
   *
   * @param vecSceneEvent 场景事件列表
   * @param _handler 事件处理器
   * @return true 注册成功
   * @return false 注册失败
   */
  txBool RegisterVehicleSceneEvent(std::vector<ISceneEventViewerPtr> vecSceneEvent,
                                   TAD_SceneEvent_1_0_0_0_Ptr _handler) TX_NOEXCEPT;

  /**
   * @brief 获取所有事件处理器
   *
   * @return std::vector< std::tuple< std::vector< ISceneEventViewerPtr >, TAD_SceneEvent_1_0_0_0_Ptr > >
   */
  std::vector<std::tuple<std::vector<ISceneEventViewerPtr>, TAD_SceneEvent_1_0_0_0_Ptr> > GetAllEvent() const
      TX_NOEXCEPT {
    return m_vec_tuple_eventList_handler;
  }

 protected:
  /**
   * @brief 触发事件
   *
   * @param timeMgr 时间参数管理器
   * @param triggerType 触发类型
   * @param condition_group_ptr_vec 事件绑定的条件组指针集合
   * @param curElemPtr 当前事件处理元素指针
   * @param elemMgrPtr 元素管理器指针
   * @param sceneLoader 场景加载器指针
   * @return true 触发成功
   */
  txBool TriggerEvent(const Base::TimeParamManager& timeMgr, const SceneEventType _triggerType,
                      IConditionViewerPtrVec _condition_group_ptr_vec, Base::ITrafficElementPtr curElemPtr,
                      Base::IElementManagerPtr _elemMgrPtr, Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发单个事件
   *
   * @param timeMgr 时间参数管理器
   * @param _triggerType 触发类型
   * @param _conditionPtr 事件条件指针
   * @param curElemPtr 当前事件处理元素指针
   * @param _elemMgrPtr 元素管理器指针
   * @param _sceneLoader 场景加载器指针
   * @return true 触发成功
   */
  txBool TriggerEvent_Single(const Base::TimeParamManager& timeMgr, const SceneEventType _triggerType,
                             IConditionViewerPtr _conditionPtr, Base::ITrafficElementPtr curElemPtr,
                             Base::IElementManagerPtr _elemMgrPtr, Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件，主要包括Velocity,ReachAbsPosition,Time,TTC等各类事件
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 特殊事件条件组指针
   * @param curElemPtr 当前事件处理元素指针
   * @param _elemPtr 元素管理器指针
   * @param _sceneLoader 场景加载器指针
   * @return true 触发成功
   * @return false 触发失败
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IVelocityTriggerConditionViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            到达绝对位置条件视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IReachAbsPositionConditionViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            到达绝对车道条件视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IReachAbsLaneConditionViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            时间视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, ITimeTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            TTC视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, ITTCTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            距离视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IDistanceTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            车道UID视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IEgoAttachLaneUidCustomViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            车头时距视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, ITimeHeadwayTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 触发特殊事件
   *
   * 触发特殊事件，根据传入的条件和当前元素，判断是否满足触发条件。
   * 如果满足触发条件，则返回 true，否则返回 false。
   *
   * @param timeMgr                  当前时间参数管理器
   * @param _conditionPtr            元素状态视图
   * @param curElemPtr                当前元素指针
   * @param _elemPtr                  元素管理器指针
   * @param _sceneLoader              场景加载器指针
   * @return                         是否触发特殊事件
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IElementStateTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr,
                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

 protected:
  std::vector<std::tuple<std::vector<ISceneEventViewerPtr>, TAD_SceneEvent_1_0_0_0_Ptr> > m_vec_tuple_eventList_handler;
};

extern void InitializeEventDispatcher() TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(TrafficFlow)
