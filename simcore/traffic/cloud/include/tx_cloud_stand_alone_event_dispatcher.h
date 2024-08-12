// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_scene_event_1_0_0_0.h"
#include "tx_element_manager_base.h"
#include "tx_enum_def.h"
#include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class CloudEventDispatcher {
 public:
  using txBool = Base::txBool;
  using ISceneEventViewerPtr = Base::ISceneLoader::ISceneEventViewerPtr;
  using SceneEventType = Base::ISceneLoader::ISceneEventViewer::IConditionViewer::SceneEventType;
  using IConditionViewer = Base::ISceneLoader::ISceneEventViewer::IConditionViewer;
  using IConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IConditionViewerPtr;

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

  using ElementType = Base::Enums::ElementType;

  using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
  using ConditionDistanceType = Base::Enums::ConditionDistanceType;
  using ConditionSpeedType = Base::Enums::ConditionSpeedType;
  using ConditionEquationOp = Base::Enums::ConditionEquationOp;
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;

 public:
  static CloudEventDispatcher& getInstance() {
    static CloudEventDispatcher instance;
    return instance;
  }

 private:
  CloudEventDispatcher() TX_DEFAULT;
  ~CloudEventDispatcher() TX_DEFAULT;
  CloudEventDispatcher(const CloudEventDispatcher&) TX_DEFAULT;
  CloudEventDispatcher& operator=(const CloudEventDispatcher&) TX_DEFAULT;

 public:
  /**
   * @brief 对象实例初始化函数
   *
   */
  void Initialize() TX_NOEXCEPT { m_vec_tuple_eventList_handler.clear(); }

  /**
   * @brief 清空实例中的events
   *
   */
  void ClearEvent() TX_NOEXCEPT { m_vec_tuple_eventList_handler.clear(); }

  /**
   * @brief 刷新所有事件
   *
   * @param timeMgr 事件参数管理器
   * @param _elemPtr 元素管理器
   */
  void FlushAllEvents(const Base::TimeParamManager& timeMgr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 注册vehicle场景事件
   *
   * @param vecSceneEvent 需要注册的场景事件集合
   * @param _handler 传入一个处理事件的回调函数
   * @return txBool 成功则返回 true，失败则返回 false
   */
  txBool RegisterVehicleSceneEvent(std::vector<ISceneEventViewerPtr> vecSceneEvent,
                                   TAD_SceneEvent_1_0_0_0_Ptr _handler) TX_NOEXCEPT;

  /**
   * @brief 获取所有已注册的事件及其处理回调函数
   *
   * @return 返回一个包含所有已注册事件的 tuple 数组，其中每个 tuple 包含一个事件列表（由 @ref ISceneEventViewerPtr
   * 组成的 vector ）和一个处理回调函数（@ref TAD_SceneEvent_1_0_0_0_Ptr）
   */
  std::vector<std::tuple<std::vector<ISceneEventViewerPtr>, TAD_SceneEvent_1_0_0_0_Ptr> > GetAllEvent() const
      TX_NOEXCEPT {
    return m_vec_tuple_eventList_handler;
  }

 protected:
  /**
   * @brief 触发指定类型的场景事件
   *
   * 此函数用于触发指定类型的场景事件，只有当满足传入的条件时才能触发
   *
   * @param timeMgr 时间参数管理器
   * @param _triggerType 要触发的场景事件类型
   * @param _conditionPtr 指向满足条件的元素查看器的指针
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发场景事件返回 true，否则返回 false
   */
  txBool TriggerEvent(const Base::TimeParamManager& timeMgr, const SceneEventType _triggerType,
                      IConditionViewerPtr _conditionPtr, Base::ITrafficElementPtr curElemPtr,
                      Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数负责触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 IVelocityTriggerConditionViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IVelocityTriggerConditionViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数用于触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 IReachAbsPositionConditionViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IReachAbsPositionConditionViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数用于触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 IReachAbsLaneConditionViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IReachAbsLaneConditionViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数用于触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 ITimeTriggerViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, ITimeTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数用于触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 ITTCTriggerViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, ITTCTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数用于触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 IDistanceTriggerViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IDistanceTriggerViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

  /**
   * @brief 触发特定事件
   *
   * 此函数用于触发特定事件，只有当条件满足时才会触发
   *
   * @param timeMgr 时间参数管理器
   * @param _conditionPtr 指向满足条件的元素查看器的指针 IEgoAttachLaneUidCustomViewerPtr
   * @param curElemPtr 当前所在的流量元素指针
   * @param _elemMgrPtr 所有流量元素管理器的指针
   * @return 函数执行结果，如果成功触发特定事件返回 true，否则返回 false
   */
  txBool TriggerSpecialEvent(const Base::TimeParamManager& timeMgr, IEgoAttachLaneUidCustomViewerPtr _conditionPtr,
                             Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

 protected:
  std::vector<std::tuple<std::vector<ISceneEventViewerPtr>, TAD_SceneEvent_1_0_0_0_Ptr> > m_vec_tuple_eventList_handler;
};

extern void InitializeCloudEventDispatcher() TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(TrafficFlow)
