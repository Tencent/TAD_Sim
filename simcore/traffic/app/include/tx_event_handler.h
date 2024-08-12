// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_obb.h"
#include "tx_sim_point.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class IEvent;
// @brief 事件处理器基类接口
class IEventHandler {
 public:
  struct EventHitUtilInfo_t {
    Base::txInt _ElemId;
    Coord::txENU _ElementGeomCenter;
    Base::txFloat _Velocity = 0.0;
    Geometry::OBB2D::PolygonArray _vecPolygon;
  };
  using EventHandlerType = Base::Enums::EventHandlerType;

 public:
  virtual ~IEventHandler() TX_DEFAULT;

  /**
   * @brief 获取事件处理器类型
   *
   * @return EventHandlerType
   */
  virtual EventHandlerType GetEventHandlerType() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取处理器的描述信息
   *
   * @return Base::txString
   */
  virtual Base::txString HandlerDesc() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取事件点击工具信息
   *
   * @return EventHitUtilInfo_t
   */
  virtual EventHitUtilInfo_t GetEventHitUtilInfo() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取处理器的ID
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId HanderId() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取处理器的系统ID
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId HanderSysId() const TX_NOEXCEPT = 0;

  /**
   * @brief 处理事件，关键的执行函数
   *
   * @return Base::txBool
   */
  virtual Base::txBool HandlerEvent(IEvent &) TX_NOEXCEPT = 0;
};

using IEventHandlerPtr = std::shared_ptr<IEventHandler>;
TX_NAMESPACE_CLOSE(TrafficFlow)
