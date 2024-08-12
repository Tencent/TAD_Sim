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

class TAD_VehicleMergeEvent : public IEvent {
 public:
  using MergeTimeTriggerItem =
      std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/>;

 public:
  explicit TAD_VehicleMergeEvent(Base::ISceneLoader::IMergesViewerPtr _mergeViewPtr);

  /**
   * @brief 事件触发的处理函数
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
   * @brief 事件是否活跃
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
   * @brief 返回处理器集合
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回事件视图指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  virtual Base::Enums::VehicleMoveLaneState GetDirectionValue() const TX_NOEXCEPT;

  /**
   * @brief 获取事件持续时长
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetCurDuration() const TX_NOEXCEPT;
  virtual Base::txFloat GetCurOffset() const TX_NOEXCEPT;

 protected:
  std::vector<MergeTimeTriggerItem>& Get_vec_pair_time_dir_duration_offset() TX_NOEXCEPT;
  const std::vector<MergeTimeTriggerItem>& Get_vec_pair_time_dir_duration_offset() const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_VehicleMergeEventPtr = std::shared_ptr<TAD_VehicleMergeEvent>;

class TAD_VehicleMergeEventEvent : public IEvent {
 public:
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;
  using MergeConditionTriggerItem = std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                                               Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt>;

 public:
  explicit TAD_VehicleMergeEventEvent(Base::ISceneLoader::IMergesViewerPtr _mergeViewPtr);

  /**
   * @brief 事件触发的检测以及处理函数
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
   * @brief 移除事件处理器
   *
   * @return Base::txSize
   */
  virtual Base::txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回事件是否激活
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
   * @brief 获取所有的事件处理器集合
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
  virtual Base::Enums::VehicleMoveLaneState GetDirectionValue() const TX_NOEXCEPT;

  /**
   * @brief 获取事件的持续时间
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetCurDuration() const TX_NOEXCEPT;
  virtual Base::txFloat GetCurOffset() const TX_NOEXCEPT;

 protected:
  std::vector<MergeConditionTriggerItem>& Get_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex()
      TX_NOEXCEPT;
  const std::vector<MergeConditionTriggerItem>& Get_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex()
      const TX_NOEXCEPT;

  std::vector<MergeConditionTriggerItem>&
  Get_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex() TX_NOEXCEPT;

  const std::vector<MergeConditionTriggerItem>&
  Get_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex() const TX_NOEXCEPT;

 protected:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
using TAD_VehicleMergeEventEventPtr = std::shared_ptr<TAD_VehicleMergeEventEvent>;
TX_NAMESPACE_CLOSE(TrafficFlow)
