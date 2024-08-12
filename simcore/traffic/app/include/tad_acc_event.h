// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_event_trigger_param.h"
#include "tx_event.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(TrafficFlow)

using EventHitUtilInfo_t = IEventHandler::EventHitUtilInfo_t;
using EventTriggerParam_t = EventTriggerParam_t;

class TAD_AccEvent : public IEvent {
 public:
  using AccTimeTriggerItem = std::pair<Base::txFloat, Base::txFloat>;

 public:
  explicit TAD_AccEvent(Base::ISceneLoader::IAccelerationViewerPtr _accViewPtr);

  /*!\~Chinese
   * @brief TriggerEvent 函数触发事件
   * @param[in] EventTriggerParam_t 事件触发参数类型
   * @return Base::txBool 函数返回布尔值
   */
  virtual Base::txBool TriggerEvent(const EventTriggerParam_t&) TX_NOEXCEPT TX_OVERRIDE;

  /*!\~Chinese
   * @brief GetEventType 获取事件类型
   * @return IEvent::EventType 事件类型枚举值
   */
  virtual IEvent::EventType GetEventType() const TX_NOEXCEPT TX_OVERRIDE;

  /*!\~Chinese
   * @brief 获取事件描述
   * @return Base::txString 事件描述
   */
  virtual Base::txString EventDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /*!
   * @brief AddEventListener 添加事件监听器
   * @param[in] eventHandler 事件处理器
   * @return 成功添加的事件处理器数量
   */
  virtual Base::txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /*!
   * @brief AbandonEventListener 移除事件监听器
   * @param[in] eventHandler 事件处理器
   * @return 成功移除的事件处理器数量
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /*!
   * @brief IsAlive 判断对象是否存活
   * @return Base::txBool 存活返回 true，否则返回 false
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件的sysId
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId Id() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前事件视图对象的字符串表示
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前事件所有的处理器
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前事件视图对象指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取Acc值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetAccValue() const TX_NOEXCEPT;

  /**
   * @brief 获取当前事件的结束条件
   *
   * @return Base::ISceneLoader::EventEndCondition_t
   */
  virtual Base::ISceneLoader::EventEndCondition_t GetEndCondition() const TX_NOEXCEPT;

 public:
  std::vector<AccTimeTriggerItem>& Get_vec_pair_time_acc() TX_NOEXCEPT;
  const std::vector<AccTimeTriggerItem>& Get_vec_pair_time_acc() const TX_NOEXCEPT;
  std::vector<Base::ISceneLoader::EventEndCondition_t>& Get_acc_invalid_type_threshold() TX_NOEXCEPT;
  const std::vector<Base::ISceneLoader::EventEndCondition_t>& Get_acc_invalid_type_threshold() const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_AccEventPtr = std::shared_ptr<TAD_AccEvent>;

class TAD_Acc_EventEvent : public IEvent {
 public:
  using AccConditionTriggerItem = std::tuple<Base::txFloat, Base::txFloat, EventStateOptional,
                                             Base::ISceneLoader::DistanceProjectionType, Base::txUInt>;

 public:
  /**
   * @brief 构造函数，用于创建一个新的数据受限的加速事件对象
   * @param _accEventViewPtr 一个指向受限加速视图对象的智能指针
   */
  explicit TAD_Acc_EventEvent(Base::ISceneLoader::IAccelerationViewerPtr _accEventViewPtr);

  /**
   * @brief 触发事件
   *
   * @param[in] EventTriggerParam_t 事件触发参数
   * @return Base::txBool 是否成功触发事件
   */
  virtual Base::txBool TriggerEvent(const EventTriggerParam_t&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件类型
   *
   * @return IEvent::EventType 事件类型
   */
  virtual IEvent::EventType GetEventType() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件描述
   *
   * @return Base::txString 事件描述
   */
  virtual Base::txString EventDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加事件处理器
   *
   * @param handler 事件处理器的智能指针
   * @return txSize 返回处理器数量
   */
  virtual Base::txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 弃用事件处理器
   *
   * @param handler 事件处理器的智能指针
   * @return txSize 返回处理器数量
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查实例是否处于活跃状态
   * @return txBool 如果实例处于活跃状态则返回 true，否则返回 false
   */
  virtual Base::txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回实例的ID
   *
   * @return txSysId 返回实例的ID
   */
  virtual Base::txSysId Id() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件字符串形式
   *
   * @return Base::txString 返回事件的字符串形式
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回所有的事件处理器对象
   *
   * @return std::vector< IEventHandlerPtr > 返回所有的事件处理器对象的容器
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回当前活动的视图器对象
   *
   * @return IViewerPtr 返回当前活动的视图器对象的智能指针
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取事件对应的acc
   *
   * @return 返回事件对应的acc
   */
  virtual Base::txFloat GetAccValue() const TX_NOEXCEPT;

  /**
   * @brief 获取当前事件的结束条件
   *
   * @return Base::ISceneLoader::EventEndCondition_t
   */
  virtual Base::ISceneLoader::EventEndCondition_t GetEndCondition() const TX_NOEXCEPT;

 protected:
  /**
   * @brief 获取vec_ttc_tuple_time_acc_state_projType_triggerIndex
   *
   * @return std::vector< AccConditionTriggerItem >&
   */
  std::vector<AccConditionTriggerItem>& Get_vec_ttc_tuple_time_acc_state_projType_triggerIndex() TX_NOEXCEPT;
  const std::vector<AccConditionTriggerItem>& Get_vec_ttc_tuple_time_acc_state_projType_triggerIndex() const
      TX_NOEXCEPT;

  /**
   * @brief 获取vec_egodistance_tuple_time_acc_state_projType_triggerIndex
   *
   * @return std::vector< AccConditionTriggerItem >&
   */
  std::vector<AccConditionTriggerItem>& Get_vec_egodistance_tuple_time_acc_state_projType_triggerIndex() TX_NOEXCEPT;
  const std::vector<AccConditionTriggerItem>& Get_vec_egodistance_tuple_time_acc_state_projType_triggerIndex() const
      TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_Acc_EventEventPtr = std::shared_ptr<TAD_Acc_EventEvent>;

TX_NAMESPACE_CLOSE(TrafficFlow)
