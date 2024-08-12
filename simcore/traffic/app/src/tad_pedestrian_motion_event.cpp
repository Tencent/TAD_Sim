// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_pedestrian_motion_event.h"
#include "tx_event_handler.h"

TX_NAMESPACE_OPEN(TrafficFlow)

#if __TX_Mark__("TAD_PedestrianTimeVelocityEvent")

#  if __TX_Mark__("TAD_PedestrianTimeVelocityEvent::Impl")
struct TAD_PedestrianTimeVelocityEvent::Impl {
 public:
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > m_vec_pair_time_direction_velocity;
  boost::optional<Base::txFloat> m_velocityValue;
  boost::optional<Base::txInt> m_directionValue;
  Base::ISceneLoader::IPedestriansEventViewerPtr _time_velocityViewerPtr = nullptr;
  IEvent::EventType m_eventType = IEvent::EventType::ePedestrianTimeVelocity;
  // 获取时间、方向和速度的元组向量
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >& vec_pair_time_direction_velocity() TX_NOEXCEPT {
    return m_vec_pair_time_direction_velocity;
  }
  // 获取时间、方向和速度的元组向量
  const std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >& vec_pair_time_direction_velocity() const
      TX_NOEXCEPT {
    return m_vec_pair_time_direction_velocity;
  }
  std::pair<Base::txInt, Base::txFloat> GetValue() const TX_NOEXCEPT {
    std::pair<Base::txInt, Base::txFloat> retPair;
    // 如果方向值有效，则将方向值赋给retPair.first，否则将0赋给retPair.first
    if (m_directionValue) {
      retPair.first = *m_directionValue;
    } else {
      retPair.first = 0.0;
    }

    // 如果速度值有效，则将速度值赋给retPair.second，否则将0赋给retPair.second
    if (m_velocityValue) {
      retPair.second = *m_velocityValue;
      /*return *m_velocityValue;*/
    } else {
      retPair.second = 0.0;
      /*return 0.0;*/
    }

    return retPair;
  }

  void ResetValue() TX_NOEXCEPT {
    m_velocityValue = boost::none;
    m_directionValue = boost::none;
  }

  void SetValue(const Base::txInt _dir, const Base::txFloat _velocity) TX_NOEXCEPT {
    m_velocityValue = _velocity;
    m_directionValue = _dir;
  }

  Base::txSysId VelocityId() const TX_NOEXCEPT {
    return (_time_velocityViewerPtr && (_time_velocityViewerPtr->IsInited())) ? (_time_velocityViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_time_velocityViewerPtr && (_time_velocityViewerPtr->IsInited())) ? (_time_velocityViewerPtr->Str())
                                                                              : ("VelocityEvent Not Inited.");
  }
};
#  endif /*__TX_Mark__("TAD_PedestrianTimeVelocityEvent::Impl")*/

TAD_PedestrianTimeVelocityEvent::TAD_PedestrianTimeVelocityEvent(
    Base::ISceneLoader::IPedestriansEventViewerPtr _time_velocityViewPtr)
    : pImpl(std::make_shared<TAD_PedestrianTimeVelocityEvent::Impl>()) {
  txAssert(pImpl && _time_velocityViewPtr);
  // 如果速度值有效，则将速度值赋给retPair.second，否则将0赋给retPair.second
  pImpl->_time_velocityViewerPtr = _time_velocityViewPtr;
  pImpl->m_vec_pair_time_direction_velocity = _time_velocityViewPtr->timestamp_direction_speed_tuple_vector();
}

std::pair<Base::txInt, Base::txFloat> TAD_PedestrianTimeVelocityEvent::GetValue() const TX_NOEXCEPT {
  return pImpl->GetValue();
}

Base::txBool TAD_PedestrianTimeVelocityEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 事件活跃
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    pImpl->ResetValue();
    // 获取事件处理器
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    const Base::TimeParamManager& timeMgr = eventParam._timeMgr;
    for (auto itr = pImpl->m_vec_pair_time_direction_velocity.begin();
         itr != (pImpl->m_vec_pair_time_direction_velocity.end());) {
      // 事件触发条件：时间到达
      if (timeMgr.PassTime() >= std::get<0>(*itr)) {
        pImpl->SetValue(std::get<1>(*itr), std::get<2>(*itr));

        // 事件触发，回调函数进行处理
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_pair_time_direction_velocity).erase(itr);
        } else {
          ++itr;
        }
      } else {
        ++itr;
      }
    }
  }
  return true;
}

IEvent::EventType TAD_PedestrianTimeVelocityEvent::GetEventType() const TX_NOEXCEPT {
  return IEvent::EventType::ePedestrianTimeVelocity;
}

Base::txString TAD_PedestrianTimeVelocityEvent::EventDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_PedestrianTimeVelocityEvent");
}

Base::txSize TAD_PedestrianTimeVelocityEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_PedestrianTimeVelocityEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_PedestrianTimeVelocityEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_PedestrianTimeVelocityEvent::Id() const TX_NOEXCEPT { return pImpl->VelocityId(); }

Base::txString TAD_PedestrianTimeVelocityEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_PedestrianTimeVelocityEvent::EventObjs() const TX_NOEXCEPT {
  return pImpl->m_vec_Handlers;
}

IEvent::IViewerPtr TAD_PedestrianTimeVelocityEvent::EventViewPtr() const TX_NOEXCEPT {
  return pImpl->_time_velocityViewerPtr;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >&
TAD_PedestrianTimeVelocityEvent::Get_vec_pair_time_direction_velocity() TX_NOEXCEPT {
  return pImpl->vec_pair_time_direction_velocity();
}

const std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >&
TAD_PedestrianTimeVelocityEvent::Get_vec_pair_time_direction_velocity() const TX_NOEXCEPT {
  return pImpl->vec_pair_time_direction_velocity();
}
#endif /*__TX_Mark__("TAD_PedestrianTimeVelocityEvent")*/

#if __TX_Mark__("TAD_PedestrianEventVelocityEvent")

#  if __TX_Mark__("TAD_PedestrianEventVelocityEvent::Impl")

struct TAD_PedestrianEventVelocityEvent::Impl {
 public:
  using txInt = Base::txInt;
  using txFloat = Base::txFloat;
  using EventStateOptional = IEvent::EventStateOptional;
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

 public:
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >
      m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex;
  std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >
      m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex;
  boost::optional<Base::txFloat> m_velocityValue;
  boost::optional<Base::txFloat> m_directionValue;
  Base::ISceneLoader::IPedestriansEventViewerPtr _event_velocityViewerPtr = nullptr;
  IEvent::EventType m_eventType = IEvent::EventType::ePedestrianEventVelocity;

  /**
   * @brief 获取时间、方向、速度、事件状态、距离投影类型和触发索引的元组向量
   *
   * @return std::vector< std::tuple< txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt
   * > >&
   */
  std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex;
  }
  const std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex() const TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex;
  }

  /**
   * @brief 获取时间、方向、速度、事件状态、距离投影类型和触发索引的元组向量
   *
   * @return std::vector< std::tuple< txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt
   * > >&
   */
  std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex;
  }
  const std::vector<std::tuple<txFloat, txInt, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex() const TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex;
  }

  std::pair<Base::txInt, Base::txFloat> GetValue() const TX_NOEXCEPT {
    std::pair<Base::txInt, Base::txFloat> retPair;
    // 如果方向值有效，则将方向值赋给retPair.first，否则将0赋给retPair.first
    if (m_directionValue) {
      retPair.first = *m_directionValue;
    } else {
      retPair.first = 0.0;
    }

    // 如果速度值有效，则将速度值赋给retPair.second，否则将0赋给retPair.second
    if (m_velocityValue) {
      retPair.second = *m_velocityValue;
      /*return *m_velocityValue;*/
    } else {
      retPair.second = 0.0;
      /*return 0.0;*/
    }

    return retPair;
  }

  void ResetValue() TX_NOEXCEPT {
    m_velocityValue = boost::none;
    m_directionValue = boost::none;
  }

  void SetValue(const Base::txInt _dir, const Base::txFloat _velocity) TX_NOEXCEPT {
    m_velocityValue = _velocity;
    m_directionValue = _dir;
  }

  Base::txSysId VelocityId() const TX_NOEXCEPT {
    return (_event_velocityViewerPtr && (_event_velocityViewerPtr->IsInited())) ? (_event_velocityViewerPtr->id())
                                                                                : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_event_velocityViewerPtr && (_event_velocityViewerPtr->IsInited()))
               ? (_event_velocityViewerPtr->Str())
               : ("_event_velocityViewerPtr Not Inited.");
  }
};
#  endif /*__TX_Mark__("TAD_PedestrianEventVelocityEvent::Impl")*/

TAD_PedestrianEventVelocityEvent::TAD_PedestrianEventVelocityEvent(
    Base::ISceneLoader::IPedestriansEventViewerPtr _event_velocityViewPtr)
    : pImpl(std::make_shared<TAD_PedestrianEventVelocityEvent::Impl>()) {
  // 如果速度值有效，则将速度值赋给retPair.second，否则将0赋给retPair.second
  txAssert(pImpl && _event_velocityViewPtr);
  pImpl->_event_velocityViewerPtr = _event_velocityViewPtr;

  // 获取TTC事件的时间、方向和速度的元组向量
  const auto& ref_ttc_speed_tuple_vector = _event_velocityViewPtr->ttc_direction_speed_pair_vector();
  pImpl->m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex.clear();
  // 遍历TTC事件的时间、方向和速度的元组向量
  for (const auto& ref_ttc_node : ref_ttc_speed_tuple_vector) {
    pImpl->m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex.emplace_back(
        std::make_tuple(std::get<0>(ref_ttc_node), std::get<1>(ref_ttc_node), std::get<2>(ref_ttc_node),
                        IEvent::EventStateOptional(), std::get<3>(ref_ttc_node), std::get<4>(ref_ttc_node)));
  }

  // 获取EgoDistance事件的时间、方向和速度的元组向量
  const auto& ref_egodistance_speed_tuple_vector = _event_velocityViewPtr->egodistance_direction_speed_pair_vector();
  /*pImpl->m_vec_egodistance_pair_time_velocity = _velocityViewPtr->egodistance_speed_pair_vector();*/
  pImpl->m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex.clear();
  // 遍历EgoDistance事件的时间、方向和速度的元组向量
  for (const auto& ref_egodistance_node : ref_egodistance_speed_tuple_vector) {
    pImpl->m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex.emplace_back(std::make_tuple(
        std::get<0>(ref_egodistance_node), std::get<1>(ref_egodistance_node), std::get<2>(ref_egodistance_node),
        IEvent::EventStateOptional(), std::get<3>(ref_egodistance_node), std::get<4>(ref_egodistance_node)));
  }

  LOG(INFO) << " PedestrianEventVelocityEvent.id = " << (_event_velocityViewPtr->id()) << ", ttc_event.size = "
            << (pImpl->m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex).size()
            << ", egodistance_event.size = "
            << (pImpl->m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex).size();
}

std::pair<Base::txInt, Base::txFloat> TAD_PedestrianEventVelocityEvent::GetValue() const TX_NOEXCEPT {
  return pImpl->GetValue();
}

IEvent::EventType TAD_PedestrianEventVelocityEvent::GetEventType() const TX_NOEXCEPT {
  return IEvent::EventType::ePedestrianEventVelocity;
}

Base::txString TAD_PedestrianEventVelocityEvent::EventDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_PedestrianEventVelocityEvent");
}

Base::txSize TAD_PedestrianEventVelocityEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_PedestrianEventVelocityEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_PedestrianEventVelocityEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_PedestrianEventVelocityEvent::Id() const TX_NOEXCEPT { return pImpl->VelocityId(); }

Base::txString TAD_PedestrianEventVelocityEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_PedestrianEventVelocityEvent::EventObjs() const TX_NOEXCEPT {
  return pImpl->m_vec_Handlers;
}

IEvent::IViewerPtr TAD_PedestrianEventVelocityEvent::EventViewPtr() const TX_NOEXCEPT {
  return pImpl->_event_velocityViewerPtr;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, TAD_PedestrianEventVelocityEvent::EventStateOptional,
                       TAD_PedestrianEventVelocityEvent::DistanceProjectionType, Base::txUInt> >&
TAD_PedestrianEventVelocityEvent::Get_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex();
}

const std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, TAD_PedestrianEventVelocityEvent::EventStateOptional,
               TAD_PedestrianEventVelocityEvent::DistanceProjectionType, Base::txUInt> >&
TAD_PedestrianEventVelocityEvent::Get_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex() const
    TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex();
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, TAD_PedestrianEventVelocityEvent::EventStateOptional,
                       TAD_PedestrianEventVelocityEvent::DistanceProjectionType, Base::txUInt> >&
TAD_PedestrianEventVelocityEvent::Get_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex()
    TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex();
}

const std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, TAD_PedestrianEventVelocityEvent::EventStateOptional,
               TAD_PedestrianEventVelocityEvent::DistanceProjectionType, Base::txUInt> >&
TAD_PedestrianEventVelocityEvent::Get_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex() const
    TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex();
}

Base::txBool TAD_PedestrianEventVelocityEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 事件是否活跃
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    pImpl->ResetValue();
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    // 获取事件处理器的事件命中实用信息
    for (auto itr = pImpl->m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex.begin();
         itr != (pImpl->m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 事件触发条件: TTC
      if (HitTTC(eventParam, evUtilInfo, std::get<0>(*itr), std::get<3>(*itr), std::get<4>(*itr), std::get<5>(*itr))) {
        pImpl->SetValue(std::get<1>(*itr), std::get<2>(*itr));

        // 事件已经触发，回调进行处理
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_ttc_tuple_time_direction_velocity_state_projType_triggerIndex).erase(itr);
        } else {
          ++itr;
        }
      } else {
        ++itr;
      }
    }

    for (auto itr = pImpl->m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex.begin();
         itr != (pImpl->m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 事件触发条件: EgoDistance
      if (HitEgoDistance(eventParam, evUtilInfo, std::get<0>(*itr), std::get<3>(*itr), std::get<4>(*itr),
                         std::get<5>(*itr))) {
        pImpl->SetValue(std::get<1>(*itr), std::get<2>(*itr));

        // 事件已经触发，回调进行处理
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_egodistance_tuple_time_direction_velocity_state_projType_triggerIndex).erase(itr);
        } else {
          ++itr;
        }
      } else {
        ++itr;
      }
    }
  }
  return true;
}

#endif /*__TX_Mark__("TAD_PedestrianEventVelocityEvent")*/
TX_NAMESPACE_CLOSE(TrafficFlow)
