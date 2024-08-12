// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_event_trigger_param.h"
#include "tx_event.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(TrafficFlow)

using EventHitUtilInfo_t = IEventHandler::EventHitUtilInfo_t;
using EventTriggerParam_t = EventTriggerParam_t;
using EventActionType = Base::ISceneLoader::EventActionType;

/*!\~Chinese
 * @class TAD_VelocityEvent
 * @brief 速度触发的事件类
 */
class TAD_VelocityEvent : public IEvent {
 public:
  using VelocityTimeTriggerItem = std::pair<Base::txFloat, Base::txFloat>;

 public:
  /*!
   * @brief TAD_VelocityEvent 构造函数，用于创建一个 TAD_VelocityEvent 对象。
   * @param[in] _velocityViewPtr 速度视图指针
   */
  explicit TAD_VelocityEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr);

  /**
   * @brief 事件触发的执行函数
   *
   * @return Base::txBool
   */
  virtual Base::txBool TriggerEvent(const EventTriggerParam_t&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件类型
   *
   * @return IEvent::EventType
   */
  virtual IEvent::EventType GetEventType() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件描述
   *
   * @return Base::txString
   */
  virtual Base::txString EventDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加事件监听
   *
   * @return Base::txSize
   */
  virtual Base::txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 移除事件监听
   *
   * @return Base::txSize
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件是否激活
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回事件id
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId Id() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回事件对象的格式化字符串
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件的处理器集合
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件的视图指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  virtual Base::txFloat GetVelocityValue() const TX_NOEXCEPT;

 protected:
  std::vector<VelocityTimeTriggerItem>& Get_vec_pair_time_velocity() TX_NOEXCEPT;
  const std::vector<VelocityTimeTriggerItem>& Get_vec_pair_time_velocity() const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_VelocityEventPtr = std::shared_ptr<TAD_VelocityEvent>;

class TAD_Velocity_EventEvent : public IEvent {
 public:
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;
  using VelocityConditionTriggerItem =
      std::tuple<Base::txFloat, Base::txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt>;

 public:
  explicit TAD_Velocity_EventEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr);

  /**
   * @brief 触发事件
   *
   * @return Base::txBool
   */
  virtual Base::txBool TriggerEvent(const EventTriggerParam_t&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件类型
   *
   * @return IEvent::EventType
   */
  virtual IEvent::EventType GetEventType() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件描述
   *
   * @return Base::txString
   */
  virtual Base::txString EventDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加事件监听器
   *
   * @return Base::txSize
   */
  virtual Base::txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 移除事件监听器
   *
   * @return Base::txSize
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件是否激活
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件id获取
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId Id() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件对象的结构化输出
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件处理器集合
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  virtual Base::txFloat GetVelocityValue() const TX_NOEXCEPT;

 protected:
  std::vector<VelocityConditionTriggerItem>& Get_vec_ttc_tuple_time_velocity_state_projType_triggerIndex() TX_NOEXCEPT;
  const std::vector<VelocityConditionTriggerItem>& Get_vec_ttc_tuple_time_velocity_state_projType_triggerIndex() const
      TX_NOEXCEPT;
  std::vector<VelocityConditionTriggerItem>& Get_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex()
      TX_NOEXCEPT;
  const std::vector<VelocityConditionTriggerItem>& Get_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex()
      const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_Velocity_EventEventPtr = std::shared_ptr<TAD_Velocity_EventEvent>;

TX_NAMESPACE_CLOSE(TrafficFlow)
