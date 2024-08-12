// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/format.hpp>
#include <iomanip>
#include "tad_event_trigger_param.h"
#include "tx_event_handler.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class IEventHandler;
using IEventHandlerPtr = std::shared_ptr<IEventHandler>;

using EventTriggerParam_t = TrafficFlow::EventTriggerParam_t;
using EventHitUtilInfo_t = TrafficFlow::IEventHandler::EventHitUtilInfo_t;
using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

/**
 * @brief 事件统一接口，所有事件都继承自该类
 */
class IEvent {
 public:
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txSysId = Base::txSysId;
  using txString = Base::txString;
  using txSize = Base::txSize;
  using txFloat = Base::txFloat;
  using txUInt = Base::txUInt;
  using IViewerPtr = Base::ISceneLoader::IViewerPtr;

 public:
  class IEventDuration {
   public:
    /**
     * @brief 构造函数，创建一个事件持续时间对象
     * @param[in] _s 事件持续时间开始时间
     * @param[in] _e 事件持续时间结束时间
     */
    IEventDuration(Base::txFloat const _s, Base::txFloat const _e)
        : m_duration_start_time(_s), m_duration_end_time(_e) {}

    /**
     * @brief 默认构造函数
     */
    IEventDuration() : m_duration_start_time(0.0), m_duration_end_time(0.0) {}

    /**
     * @brief 判断当前时间是否在事件持续时间的范围内。
     *
     * @param _curTime 当前时间。
     * @return true 如果当前时间在事件持续时间范围内，否则 false。
     */
    Base::txBool Hit(Base::txFloat const _curTime) const TX_NOEXCEPT {
      TX_MARK("[Closed, Opened)");
      return (m_duration_start_time <= _curTime) && (_curTime < m_duration_end_time);
    }

    /**
     * @brief 获取格式化的Duration字符串表示形式
     *
     * @return Base::txString
     */
    Base::txString Str() const TX_NOEXCEPT {
      return (boost::format("IEventDuration [%1%, %2%)") % m_duration_start_time % m_duration_end_time).str();
    }

    Base::txFloat m_duration_start_time;
    TX_MARK("closed interval, start_time <= current_time ");
    Base::txFloat m_duration_end_time;
    TX_MARK("opened interval, current_time < end_time ");
  };

 public:
  /**
   * @brief 事件类型
   */
  enum class EventType : txInt {
    eAcc,
    eAccEvent,
    eMerge,
    eMergeEvent,
    ePedestrian,
    ePedestrianShuttle,
    ePedestrianTimeVelocity,
    ePedestrianEventVelocity,
    eVelocity,
    eVelocityEvent,
    eUndef
  };
  /*enum class EventState : txInt {eUnActive = 0, eActive = 1, eDead = 2};*/
  enum class TwoWayEventState : txInt { twUndefined = 0, twInCondition = 1, twOutCondition = 2 };
  using EventStateOptional =
      boost::optional<std::tuple<TwoWayEventState /*state*/, Base::txInt /*state change count*/> >;
  friend std::ostream& operator<<(std::ostream& os, const TwoWayEventState& v) TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const EventStateOptional& v) TX_NOEXCEPT;
  virtual ~IEvent() TX_DEFAULT;

  /**
   * @brief 触发事件
   *
   * @param _param 事件参数
   * @return txBool
   */
  virtual txBool TriggerEvent(const EventTriggerParam_t&) TX_NOEXCEPT = 0;

  /**
   * @brief 获取事件类型
   *
   * @return EventType 事件类型
   */
  virtual EventType GetEventType() const TX_NOEXCEPT = 0;

  /**
   * @brief 事件描述
   *
   * @return txString
   */
  virtual txString EventDesc() const TX_NOEXCEPT = 0;

  /**
   * @brief 添加事件监听器
   *
   * @param _handler 事件处理器
   * @return txSize 事件处理器的数量
   */
  virtual txSize AddEventListener(IEventHandlerPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 移除事件监听器
   *
   * @param _handler 事件处理器
   * @return txSize  事件处理器的数量
   */
  virtual txSize AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 检查事件是否处于活跃状态
   *
   * @return txBool 如果事件处于活跃状态则返回 true
   */
  virtual txBool IsAlive() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取event的sysId
   *
   * @return txSysId
   */
  virtual txSysId Id() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取字符串格式化描述
   *
   * @return txString
   */
  virtual txString Str() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取所有的事件处理器
   *
   * @return std::vector< IEventHandlerPtr >
   */
  virtual std::vector<IEventHandlerPtr> EventObjs() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前事件处理器对应的视图指针
   *
   * @return IViewerPtr
   */
  virtual IViewerPtr EventViewPtr() const TX_NOEXCEPT = 0;

 protected:
  /**
   * @brief Compute_TTC_Distance 计算 TTC 距离。
   *
   * @param EventTriggerParam_t 参数：包含 ego 对象和 ego 的相关信息。
   * @param EventHitUtilInfo_t 参数：包含需要评估的元素（如车辆）的相关信息。
   * @param projType 投影类型：选择用于计算 ego 和 元素之间距离的投影方式。
   * @return txFloat 计算得出的 TTC 距离。
   */
  virtual txFloat Compute_TTC_Distance(const EventTriggerParam_t&, const EventHitUtilInfo_t&,
                                       const DistanceProjectionType projType) const TX_NOEXCEPT;

  /**
   * @brief Compute_EGO_Distance 计算车辆与其他实体之间的距离。
   *
   * @param egoInfo 事件触发的车辆信息。
   * @param elementInfo 距离触发的实体信息。
   * @param projType 投影类型
   * @return txFloat 计算得出的距离值。
   */
  virtual txFloat Compute_EGO_Distance(const EventTriggerParam_t&, const EventHitUtilInfo_t&,
                                       const DistanceProjectionType projType) const TX_NOEXCEPT;

  /**
   * @brief IEvent::HitTTC 判断两个事件是否触发ttc碰撞
   *
   * @param egoParam 事件触发的自身元素的相关参数
   * @param elementParam 触发事件的其他元素的相关参数
   * @param threshold 判断ttc碰撞是否发生的阈值
   * @param refState 可选的事件状态，用于在发生碰撞时更新
   * @param projType 投影类型，用于计算距离和投影点
   * @param triggerIndex 触发器索引，用于在多个触发器中选择特定的触发器
   * @return true 发生ttc碰撞
   */
  virtual Base::txBool HitTTC(const EventTriggerParam_t& egoParam, const EventHitUtilInfo_t& elementParam,
                              const Base::txFloat threshold, EventStateOptional& refState,
                              DistanceProjectionType projType, const txUInt triggerIndex) const TX_NOEXCEPT;

  /**
   * @brief IEvent::HitEgoDistance 判断两个事件是否发生距离碰撞
   *
   * @param egoParam 触发事件的自身元素的相关参数
   * @param elementParam 触发事件的其他元素的相关参数
   * @param threshold 判断碰撞是否发生的阈值
   * @param refState 可选的事件状态，用于在发生碰撞时更新
   * @param projType 投影类型，用于计算距离和投影点
   * @param triggerIndex 触发器索引，用于在多个触发器中选择特定的触发器
   * @return true 发生距离碰撞
   */
  virtual Base::txBool HitEgoDistance(const EventTriggerParam_t& egoParam, const EventHitUtilInfo_t& elementParam,
                                      const Base::txFloat threshold, EventStateOptional& refState,
                                      DistanceProjectionType projType, const txUInt triggerIndex) const TX_NOEXCEPT;

  /**
   * @brief IsParamValid 判断触发参数是否合法。
   *
   * @param egoParam 事件触发的自身元素的相关参数。
   * @param elementParam 触发事件的其他元素的相关参数。
   * @return true 参数合法。
   */
  Base::txBool IsParamValid(const EventTriggerParam_t& egoParam,
                            const EventHitUtilInfo_t& elementParam) const TX_NOEXCEPT;
};
using IEventPtr = std::shared_ptr<IEvent>;

TX_NAMESPACE_CLOSE(TrafficFlow)
