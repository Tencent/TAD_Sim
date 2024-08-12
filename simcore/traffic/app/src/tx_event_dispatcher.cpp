// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_event_dispatcher.h"
#include <map>
#include "tad_acc_event.h"
#include "tad_pedestrian_motion_event.h"
#include "tad_vehicle_merge_event.h"
#include "tad_velocity_event.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

#if __TX_Mark__("txEventDispatcher")

void txEventDispatcher::Initialize() TX_NOEXCEPT { m_multimap_type2event.clear(); }

void txEventDispatcher::FlushAllEvents(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 计数器
  Base::txDiff_t nCnt = 0;
  // eventType是eAcc时
  {
    auto AccEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::eAcc);
    nCnt += std::distance(AccEventItrPair.first, AccEventItrPair.second);
    for (auto itr = AccEventItrPair.first; itr != AccEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是eAccEvent时
  {
    auto Acc_EventEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::eAccEvent);
    nCnt += std::distance(Acc_EventEventItrPair.first, Acc_EventEventItrPair.second);
    for (auto itr = Acc_EventEventItrPair.first; itr != Acc_EventEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是eMerge时
  {
    auto MergeEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::eMerge);
    nCnt += std::distance(MergeEventItrPair.first, MergeEventItrPair.second);
    for (auto itr = MergeEventItrPair.first; itr != MergeEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是eMergeEvent时
  {
    auto Merge_EventEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::eMergeEvent);
    nCnt += std::distance(Merge_EventEventItrPair.first, Merge_EventEventItrPair.second);
    for (auto itr = Merge_EventEventItrPair.first; itr != Merge_EventEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是eVelocity时
  {
    auto VelocityEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::eVelocity);
    nCnt += std::distance(VelocityEventItrPair.first, VelocityEventItrPair.second);
    for (auto itr = VelocityEventItrPair.first; itr != VelocityEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是eVelocityEvent时
  {
    auto Velocity_EventEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::eVelocityEvent);
    nCnt += std::distance(Velocity_EventEventItrPair.first, Velocity_EventEventItrPair.second);
    for (auto itr = Velocity_EventEventItrPair.first; itr != Velocity_EventEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是ePedestrian时
  {
    auto PedestrianEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::ePedestrian);
    nCnt += std::distance(PedestrianEventItrPair.first, PedestrianEventItrPair.second);
    for (auto itr = PedestrianEventItrPair.first; itr != PedestrianEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是ePedestrianShuffle时
  {
    auto PedestrianShuttleEventItrPair = m_multimap_type2event.equal_range(IEvent::EventType::ePedestrianShuttle);
    nCnt += std::distance(PedestrianShuttleEventItrPair.first, PedestrianShuttleEventItrPair.second);
    for (auto itr = PedestrianShuttleEventItrPair.first; itr != PedestrianShuttleEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是ePedestrianTimeVelocity时
  {
    auto PedestrianTimeVelocityEventItrPair =
        m_multimap_type2event.equal_range(IEvent::EventType::ePedestrianTimeVelocity);
    nCnt += std::distance(PedestrianTimeVelocityEventItrPair.first, PedestrianTimeVelocityEventItrPair.second);
    for (auto itr = PedestrianTimeVelocityEventItrPair.first; itr != PedestrianTimeVelocityEventItrPair.second; ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }

  // eventType是ePedestrianEventVelocity时
  {
    auto PedestrianEventVelocityEventItrPair =
        m_multimap_type2event.equal_range(IEvent::EventType::ePedestrianEventVelocity);
    nCnt += std::distance(PedestrianEventVelocityEventItrPair.first, PedestrianEventVelocityEventItrPair.second);
    for (auto itr = PedestrianEventVelocityEventItrPair.first; itr != PedestrianEventVelocityEventItrPair.second;
         ++itr) {
      (*itr).second->TriggerEvent(eventParam);
    }
  }
  /*txAssert((m_multimap_type2event.size()) == nCnt);*/
}

Base::txBool txEventDispatcher::RegisterAccEvent(Base::ISceneLoader::IAccelerationViewerPtr _accViewPtr,
                                                 IEventHandlerPtr _handler) TX_NOEXCEPT {
  if (_accViewPtr && (_accViewPtr->IsInited()) && _handler) {
#  if 1
    // KV中添加元素
    auto itr = m_multimap_type2event.emplace(IEvent::EventType::eAcc,
                                             std::make_shared<TrafficFlow::TAD_AccEvent>(_accViewPtr));
    // 添加事件监听器
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    LogInfo << "[vehicle_event_register_acc]_create_acc_event, element id = " << (_handler->HanderId())
            << ", _Acc id = " << (_accViewPtr->id());
    return true;
#  endif
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterAccEventEvent(Base::ISceneLoader::IAccelerationViewerPtr _accEventViewPtr,
                                                      IEventHandlerPtr _handler) TX_NOEXCEPT {
  if (_accEventViewPtr && (_accEventViewPtr->IsInited()) && _handler) {
#  if 1
    TX_MARK("accEvent has not be created.");
    // KV中添加元素
    auto itr = m_multimap_type2event.emplace(IEvent::EventType::eAccEvent,
                                             std::make_shared<TrafficFlow::TAD_Acc_EventEvent>(_accEventViewPtr));
    // 添加事件监听器
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    LogInfo << "[vehicle_event_register_acc_event]_create_acc_event, element id = " << (_handler->HanderId())
            << ", _Acc_event id = " << (_accEventViewPtr->id());
    return true;
#  endif
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterMergeEvent(Base::ISceneLoader::IMergesViewerPtr _mergeViewPtr,
                                                   IEventHandlerPtr _handler) TX_NOEXCEPT {
  if (_mergeViewPtr && (_mergeViewPtr->IsInited()) && _handler) {
#  if 1
    TX_MARK("mergeEvent has not be created.");

    // KV中添加元素
    auto itr = m_multimap_type2event.emplace(IEvent::EventType::eMerge,
                                             std::make_shared<TrafficFlow::TAD_VehicleMergeEvent>(_mergeViewPtr));
    // 添加事件监听器
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    LogInfo << "[vehicle_event_register_merge]_create_merge_event, element_id = " << (_handler->HanderId())
            << ", _Merge id = " << (_mergeViewPtr->id());
    return true;
#  endif
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterMergeEventEvent(Base::ISceneLoader::IMergesViewerPtr _mergeEventViewPtr,
                                                        IEventHandlerPtr _handler) TX_NOEXCEPT {
  if (_mergeEventViewPtr && (_mergeEventViewPtr->IsInited()) && _handler) {
    TX_MARK("mergeEvent has not be created.");

    // KV中添加元素
    auto itr = m_multimap_type2event.emplace(
        IEvent::EventType::eMergeEvent, std::make_shared<TrafficFlow::TAD_VehicleMergeEventEvent>(_mergeEventViewPtr));
    // 添加事件监听器
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    LogInfo << "[vehicle_event_register_merge_event]_create_merge_event, element_id = " << (_handler->HanderId())
            << ", _Merge_event id = " << (_mergeEventViewPtr->id());
    return true;
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterVelocityEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr,
                                                      IEventHandlerPtr _handler) TX_NOEXCEPT {
  if (_velocityViewPtr && (_velocityViewPtr->IsInited()) && _handler) {
#  if 1
    // KV中添加元素
    auto itr = m_multimap_type2event.emplace(IEvent::EventType::eVelocity,
                                             std::make_shared<TrafficFlow::TAD_VelocityEvent>(_velocityViewPtr));
    // 添加事件监听器
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    LogInfo << "[vehicle_event_register_merge]_create_merge_event, element_id = " << (_handler->HanderId())
            << ", _Merge id = " << (_velocityViewPtr->id());
    return true;
#  endif
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterVelocityEventEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr,
                                                           IEventHandlerPtr _handler) TX_NOEXCEPT {
  // 检查速度视图指针和事件处理器是否有效
  if (_velocityViewPtr && (_velocityViewPtr->IsInited()) && _handler) {
    // 将速度事件添加到多重映射中
    auto itr = m_multimap_type2event.emplace(IEvent::EventType::eVelocityEvent,
                                             std::make_shared<TrafficFlow::TAD_Velocity_EventEvent>(_velocityViewPtr));
    // 将事件处理器添加到速度事件中
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    // 断言事件处理器已成功添加
    txAssert(1 == nSize);
    LogInfo << "[vehicle_event_register_merge_event]_create_merge_event, element_id = " << (_handler->HanderId())
            << ", _Merge_event id = " << (_velocityViewPtr->id());
    return true;
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterPedestrianTimeVelocityEvent(
    Base::ISceneLoader::IPedestriansEventViewerPtr _pedestrianEventTimeVelocityViewPtr,
    IEventHandlerPtr _handler) TX_NOEXCEPT {
  // 检查行人时间速度视图指针和事件处理器是否有效
  if (_pedestrianEventTimeVelocityViewPtr && (_pedestrianEventTimeVelocityViewPtr->IsInited()) && _handler) {
    TX_MARK("Pedestrian time velocity event has not be created.");
    // 将行人时间速度事件添加到多重映射中
    auto itr = m_multimap_type2event.emplace(
        IEvent::EventType::ePedestrianTimeVelocity,
        std::make_shared<TrafficFlow::TAD_PedestrianTimeVelocityEvent>(_pedestrianEventTimeVelocityViewPtr));
    // 将事件处理器添加到行人时间速度事件中
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    // 记录日志信息
    LogInfo << "[pedestrian_event_register_time_velocity_event]_create_time_velocity_event, element id = "
            << (_handler->HanderId())
            << ", _Pedestrian_time_velocity_event id = " << (_pedestrianEventTimeVelocityViewPtr->id());
    return true;
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::RegisterPedestrianEventVelocityEvent(
    Base::ISceneLoader::IPedestriansEventViewerPtr _pedestrianEventEventVelocityViewPtr,
    IEventHandlerPtr _handler) TX_NOEXCEPT {
  // 检查行人事件速度视图指针和事件处理器是否有效
  if (_pedestrianEventEventVelocityViewPtr && (_pedestrianEventEventVelocityViewPtr->IsInited()) && _handler) {
    TX_MARK("Pedestrian event velocity event has not be created.");
    // 将行人事件速度事件添加到多重映射中
    auto itr = m_multimap_type2event.emplace(
        IEvent::EventType::ePedestrianEventVelocity,
        std::make_shared<TrafficFlow::TAD_PedestrianEventVelocityEvent>(_pedestrianEventEventVelocityViewPtr));
    // 将事件处理器添加到行人事件速度事件中
    const Base::txSize nSize = (*itr).second->AddEventListener(_handler);
    txAssert(1 == nSize);
    // 记录日志信息
    LogInfo << "[pedestrian_event_register_event_velocity_event]_create_event_velocity_event, element id = "
            << (_handler->HanderId())
            << ", _Pedestrian_event_velocity_event id = " << (_pedestrianEventEventVelocityViewPtr->id());
    return true;
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher::QueryEventByElementSysId(const IEvent::EventType eventType,
                                                         const Base::txSysId elemSysId,
                                                         std::vector<IEventPtr>& result_events) TX_NOEXCEPT {
  result_events.clear();
  for (const auto& refPair : m_multimap_type2event) {
    // 匹配到事件类型
    if (eventType == refPair.first) {
      // 获取事件引用
      const auto& refElemVec = refPair.second->EventObjs();
      for (const auto& elemPtr : refElemVec) {
        // 匹配到元素系统ID
        if (elemSysId == elemPtr->HanderSysId()) {
          result_events.emplace_back(refPair.second);
        }
      }
    }
  }
  return _NonEmpty_(result_events);
}

#endif  // __TX_Mark__("txEventDispatcher")

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
