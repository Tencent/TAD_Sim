// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "SceneLoader/tx_tadloader.h"
#include "tx_element_manager_base.h"
#include "tx_event.h"
#include "tx_event_handler.h"
#include "tx_header.h"

TX_NAMESPACE_OPEN(TrafficFlow)
TODO("change namespace")

class txEventDispatcher {
  using txBool = Base::txBool;

 public:
  static txEventDispatcher& getInstance() {
    static txEventDispatcher instance;
    return instance;
  }

 private:
  txEventDispatcher() TX_DEFAULT;
  ~txEventDispatcher() TX_DEFAULT;
  txEventDispatcher(const txEventDispatcher&) TX_DEFAULT;
  txEventDispatcher& operator=(const txEventDispatcher&) TX_DEFAULT;

 public:
  void Initialize() TX_NOEXCEPT;

  /**
   * @brief 刷新所有事件
   *
   * @param param 事件触发参数
   * @return
   */
  void FlushAllEvents(const EventTriggerParam_t&) TX_NOEXCEPT;

  /**
   * @brief 注册AccEvent
   *
   * @param _accViewPtr Acc视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterAccEvent(Base::ISceneLoader::IAccelerationViewerPtr _accViewPtr,
                          IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册AccEventEvent
   *
   * @param _accViewPtr Acc视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterAccEventEvent(Base::ISceneLoader::IAccelerationViewerPtr _accViewPtr,
                               IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册VelocityEvent
   *
   * @param _mergeViewPtr Merge视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterMergeEvent(Base::ISceneLoader::IMergesViewerPtr _mergeViewPtr, IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册VelocityEventEvent
   *
   * @param _mergeViewPtr Merge视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterMergeEventEvent(Base::ISceneLoader::IMergesViewerPtr _mergeViewPtr,
                                 IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册VelocityEvent
   *
   * @param _velocityViewPtr Velocity视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterVelocityEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr,
                               IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册VelocityEventEvent
   *
   * @param _velocityViewPtr Velocity视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterVelocityEventEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr,
                                    IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册PedestrianTimeVelocityEvent
   *
   * @param _pedestrianEventTimeVelocityViewPtr Pedestrian时间-速度视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterPedestrianTimeVelocityEvent(
      Base::ISceneLoader::IPedestriansEventViewerPtr _pedestrianEventTimeVelocityViewPtr,
      IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 注册PedestrianEventVelocityEvent
   *
   * @param _pedestrianEventEventVelocityViewPtr Pedestrian事件-速度视图指针
   * @param _handler 事件处理器
   * @return txBool
   */
  txBool RegisterPedestrianEventVelocityEvent(
      Base::ISceneLoader::IPedestriansEventViewerPtr _pedestrianEventEventVelocityViewPtr,
      IEventHandlerPtr _handler) TX_NOEXCEPT;

  /**
   * @brief 查询事件
   *
   * @param eventType 事件类型
   * @param elemSysId 元素系统ID
   * @param result_events 查询结果
   * @return txBool
   */
  txBool QueryEventByElementSysId(const IEvent::EventType eventType, const Base::txSysId elemSysId,
                                  std::vector<IEventPtr>& result_events) TX_NOEXCEPT;

 protected:
  std::multimap<IEvent::EventType, IEventPtr> m_multimap_type2event;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
