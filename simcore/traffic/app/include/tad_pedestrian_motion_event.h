// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "SceneLoader/tx_tadloader.h"
#include "tad_event_trigger_param.h"
#include "tx_event.h"
#include "tx_event_handler.h"

TX_NAMESPACE_OPEN(TrafficFlow)

using EventActionType = Base::ISceneLoader::EventActionType;

class TAD_PedestrianTimeVelocityEvent : public IEvent {
 public:
  explicit TAD_PedestrianTimeVelocityEvent(Base::ISceneLoader::IPedestriansEventViewerPtr _time_velocityViewPtr);

  /**
   * @brief 事件触发
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
   * @brief 事件描述
   *
   * @return Base::txString
   */
  virtual Base::txString EventDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加事件监听
   *
   * @return Base::txSize 事件处理器的个数
   */
  virtual Base::txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 移除事件监听
   *
   * @return Base::txSize 移除后事件处理器的个数
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件是否存活
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件ID
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId Id() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件字符串描述
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取所有事件处理对象
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件视图指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取事件中value
   *
   * @return std::pair<Base::txInt, Base::txFloat>
   */
  virtual std::pair<Base::txInt, Base::txFloat> GetValue() const TX_NOEXCEPT;

 protected:
  /**
   * @brief 获取事件中vec_pair_time_direction_velocity
   *
   * @return std::vector< std::tuple< Base::txFloat, Base::txInt, Base::txFloat > >&
   */
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >& Get_vec_pair_time_direction_velocity()
      TX_NOEXCEPT;
  const std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >& Get_vec_pair_time_direction_velocity()
      const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_PedestrianTimeVelocityEventPtr = std::shared_ptr<TAD_PedestrianTimeVelocityEvent>;

class TAD_PedestrianEventVelocityEvent : public IEvent {
 public:
  explicit TAD_PedestrianEventVelocityEvent(Base::ISceneLoader::IPedestriansEventViewerPtr _event_velocityViewPtr);

  /**
   * @brief 事件触发
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
   * @return Base::txSize 事件处理器的个数
   */
  virtual Base::txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 移除事件监听
   *
   * @return Base::txSize 移除后事件处理器的个数
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件是否存活
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件ID获取
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId Id() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件字符串描述
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取所有事件处理对象
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件视图指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取事件中value属性值
   *
   * @return std::pair<Base::txInt, Base::txFloat>
   */
  virtual std::pair<Base::txInt, Base::txFloat> GetValue() const TX_NOEXCEPT;

 protected:
  using txFloat = Base::txFloat;
  using txInt = Base::txInt;
  using txUInt = Base::txUInt;
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

  /**
   * @brief 获取vec_ttc tuple_time_direction_velocity_state_projType_triggerIndex
   *
   * @return std::vector< std::tuple< txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt
   * > >&
   */
  std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  Get_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex() TX_NOEXCEPT;
  const std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  Get_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex() const TX_NOEXCEPT;

  /**
   * @brief 获取vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex
   *
   * @return std::vector< std::tuple< txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt
   * > >&
   */
  std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  Get_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex() TX_NOEXCEPT;
  const std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  Get_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex() const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_PedestrianEventVelocityEventPtr = std::shared_ptr<TAD_PedestrianEventVelocityEvent>;

TX_NAMESPACE_CLOSE(TrafficFlow)
