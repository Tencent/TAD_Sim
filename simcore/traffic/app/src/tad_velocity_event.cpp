// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_velocity_event.h"
#include <boost/optional.hpp>
#include "tx_event_handler.h"
#include "tx_scene_loader.h"
TX_NAMESPACE_OPEN(TrafficFlow)

#if __TX_Mark__("TAD_VelocityEvent::Impl")
struct TAD_VelocityEvent::Impl {
 public:
  // 事件处理器集合，事件发生时进行一种或多种处理
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  // 多个事件触发条件参数集合
  std::vector<std::pair<Base::txFloat, Base::txFloat> > m_vec_pair_time_velocity;
  boost::optional<Base::txFloat> m_velocityValue;
  Base::ISceneLoader::IVelocityViewerPtr _velocityViewerPtr = nullptr;
  IEvent::EventType m_eventType = IEvent::EventType::eVelocity;
  std::vector<std::pair<Base::txFloat, Base::txFloat> >& vec_pair_time_velocity() TX_NOEXCEPT {
    return m_vec_pair_time_velocity;
  }
  const std::vector<std::pair<Base::txFloat, Base::txFloat> >& vec_pair_time_velocity() const TX_NOEXCEPT {
    return m_vec_pair_time_velocity;
  }
  Base::txFloat GetVelocityValue() const TX_NOEXCEPT {
    if (m_velocityValue) {
      return *m_velocityValue;
    } else {
      return 0.0;
    }
  }

  void ResetVelocityValue() TX_NOEXCEPT { m_velocityValue = boost::none; }

  void SetVelocityValue(const Base::txFloat _velocity) TX_NOEXCEPT { m_velocityValue = _velocity; }

  Base::txSysId VelocityId() const TX_NOEXCEPT {
    return (_velocityViewerPtr && (_velocityViewerPtr->IsInited())) ? (_velocityViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_velocityViewerPtr && (_velocityViewerPtr->IsInited())) ? (_velocityViewerPtr->Str())
                                                                    : ("VelocityEvent Not Inited.");
  }
};
#endif /*__TX_Mark__("TAD_VelocityEvent::Impl")*/

#if __TX_Mark__("TAD_VelocityEvent")
TAD_VelocityEvent::TAD_VelocityEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr)
    : pImpl(std::make_shared<TAD_VelocityEvent::Impl>()) {
  txAssert(pImpl && _velocityViewPtr);
  pImpl->_velocityViewerPtr = _velocityViewPtr;
  pImpl->m_vec_pair_time_velocity = _velocityViewPtr->timestamp_speed_pair_vector();
}

Base::txFloat TAD_VelocityEvent::GetVelocityValue() const TX_NOEXCEPT { return pImpl->GetVelocityValue(); }

Base::txBool TAD_VelocityEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    pImpl->ResetVelocityValue();
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
        << TX_VARS_NAME(TAD_VelocityEvent.size, pImpl->m_vec_pair_time_velocity.size());
    const Base::TimeParamManager& timeMgr = eventParam._timeMgr;
    for (auto itr = pImpl->m_vec_pair_time_velocity.begin(); itr != (pImpl->m_vec_pair_time_velocity.end());) {
      // 判断是否到达该事件的发生时间点
      if (timeMgr.PassTime() >= (*itr).first) {
        pImpl->SetVelocityValue((*itr).second);

        // 此时触发事件，调用事件处理器进行处理
        if (refHandler->HandlerEvent(*this)) {
          // 事件处理成功，移除事件
          itr = (pImpl->m_vec_pair_time_velocity).erase(itr);
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

IEvent::EventType TAD_VelocityEvent::GetEventType() const TX_NOEXCEPT { return IEvent::EventType::eVelocity; }

Base::txString TAD_VelocityEvent::EventDesc() const TX_NOEXCEPT { return Base::txString("TAD_VelocityEvent"); }

Base::txSize TAD_VelocityEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_VelocityEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_VelocityEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_VelocityEvent::Id() const TX_NOEXCEPT { return pImpl->VelocityId(); }

Base::txString TAD_VelocityEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_VelocityEvent::EventObjs() const TX_NOEXCEPT { return pImpl->m_vec_Handlers; }

IEvent::IViewerPtr TAD_VelocityEvent::EventViewPtr() const TX_NOEXCEPT { return pImpl->_velocityViewerPtr; }

std::vector<TAD_VelocityEvent::VelocityTimeTriggerItem>& TAD_VelocityEvent::Get_vec_pair_time_velocity() TX_NOEXCEPT {
  return pImpl->vec_pair_time_velocity();
}

const std::vector<TAD_VelocityEvent::VelocityTimeTriggerItem>& TAD_VelocityEvent::Get_vec_pair_time_velocity() const
    TX_NOEXCEPT {
  return pImpl->vec_pair_time_velocity();
}
#endif /*TAD_VelocityEvent*/

#if __TX_Mark__("TAD_Velocity_EventEvent::Impl")

struct TAD_Velocity_EventEvent::Impl {
 public:
  using txFloat = Base::txFloat;
  using EventStateOptional = IEvent::EventStateOptional;
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

 public:
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >
      m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex;
  std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >
      m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex;
  boost::optional<Base::txFloat> m_velocityValue;

  Base::ISceneLoader::IVelocityViewerPtr _velocityViewerPtr = nullptr;
  IEvent::EventType m_eventType = IEvent::EventType::eVelocityEvent;
  std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_ttc_tuple_time_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex;
  }
  const std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_ttc_tuple_time_velocity_state_projType_triggerIndex() const TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex;
  }

  std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_egodistance_tuple_time_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex;
  }

  const std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_egodistance_tuple_time_velocity_state_projType_triggerIndex() const TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex;
  }
  // 定义一个成员函数GetVelocityValue，用于获取速度值
  Base::txFloat GetVelocityValue() const TX_NOEXCEPT {
    if (m_velocityValue) {
      return *m_velocityValue;
    } else {
      return 0.0;
    }
  }

  void ResetVelocityValue() TX_NOEXCEPT { m_velocityValue = boost::none; }

  void SetVelocityValue(const Base::txFloat _velocity) TX_NOEXCEPT { m_velocityValue = _velocity; }

  Base::txSysId VelocityId() const TX_NOEXCEPT {
    return (_velocityViewerPtr && (_velocityViewerPtr->IsInited())) ? (_velocityViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_velocityViewerPtr && (_velocityViewerPtr->IsInited())) ? (_velocityViewerPtr->Str())
                                                                    : ("Velocity_EventEvent Not Inited.");
  }
};

#endif /*__TX_Mark__("TAD_Velocity_EventEvent::Impl")*/

#if __TX_Mark__("TAD_Velocity_EventEvent")

TAD_Velocity_EventEvent::TAD_Velocity_EventEvent(Base::ISceneLoader::IVelocityViewerPtr _velocityViewPtr)
    : pImpl(std::make_shared<TAD_Velocity_EventEvent::Impl>()) {
  txAssert(pImpl && _velocityViewPtr);
  pImpl->_velocityViewerPtr = _velocityViewPtr;

  const auto& ref_ttc_speed_tuple_vector = _velocityViewPtr->ttc_speed_pair_vector();
  pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex.clear();
  // 遍历TTC速度对的向量
  for (const auto& ref_ttc_node : ref_ttc_speed_tuple_vector) {
    pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex.emplace_back(
        std::make_tuple(std::get<0>(ref_ttc_node), std::get<1>(ref_ttc_node), IEvent::EventStateOptional(),
                        std::get<2>(ref_ttc_node), std::get<3>(ref_ttc_node)));
  }

  const auto& ref_egodistance_speed_tuple_vector = _velocityViewPtr->egodistance_speed_pair_vector();
  /*pImpl->m_vec_egodistance_pair_time_velocity = _velocityViewPtr->egodistance_speed_pair_vector();*/
  pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex.clear();
  // 遍历距离速度对的向量
  for (const auto& ref_egodistance_node : ref_egodistance_speed_tuple_vector) {
    // 将距离速度对添加到pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex
    pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex.emplace_back(std::make_tuple(
        std::get<0>(ref_egodistance_node), std::get<1>(ref_egodistance_node), IEvent::EventStateOptional(),
        std::get<2>(ref_egodistance_node), std::get<3>(ref_egodistance_node)));
  }

  LOG(INFO) << " VelocityEventEvent.id = " << (_velocityViewPtr->id())
            << ", ttc_event.size = " << (pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex).size()
            << ", egodistance_event.size = "
            << (pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex).size();
}

Base::txFloat TAD_Velocity_EventEvent::GetVelocityValue() const TX_NOEXCEPT { return pImpl->GetVelocityValue(); }

IEvent::EventType TAD_Velocity_EventEvent::GetEventType() const TX_NOEXCEPT {
  return IEvent::EventType::eVelocityEvent;
}

Base::txString TAD_Velocity_EventEvent::EventDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_Velocity_EventEvent");
}

Base::txSize TAD_Velocity_EventEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_Velocity_EventEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_Velocity_EventEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_Velocity_EventEvent::Id() const TX_NOEXCEPT { return pImpl->VelocityId(); }

Base::txString TAD_Velocity_EventEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_Velocity_EventEvent::EventObjs() const TX_NOEXCEPT { return pImpl->m_vec_Handlers; }

IEvent::IViewerPtr TAD_Velocity_EventEvent::EventViewPtr() const TX_NOEXCEPT { return pImpl->_velocityViewerPtr; }

std::vector<TAD_Velocity_EventEvent::VelocityConditionTriggerItem>&
TAD_Velocity_EventEvent::Get_vec_ttc_tuple_time_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_velocity_state_projType_triggerIndex();
}

const std::vector<TAD_Velocity_EventEvent::VelocityConditionTriggerItem>&
TAD_Velocity_EventEvent::Get_vec_ttc_tuple_time_velocity_state_projType_triggerIndex() const TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_velocity_state_projType_triggerIndex();
}

std::vector<TAD_Velocity_EventEvent::VelocityConditionTriggerItem>&
TAD_Velocity_EventEvent::Get_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex() TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_velocity_state_projType_triggerIndex();
}

const std::vector<TAD_Velocity_EventEvent::VelocityConditionTriggerItem>&
TAD_Velocity_EventEvent::Get_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex() const TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_velocity_state_projType_triggerIndex();
}
Base::txBool TAD_Velocity_EventEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    pImpl->ResetVelocityValue();
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger) << TX_VARS_NAME(
        TAD_Velocity_EventEvent_ttc.size, pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex.size());
    // 遍历当前类型所有事件
    for (auto itr = pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex.begin();
         itr != (pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 命中TTC，满足事件触发
      if (HitTTC(eventParam, evUtilInfo, std::get<0>(*itr), std::get<2>(*itr), std::get<3>(*itr), std::get<4>(*itr))) {
        pImpl->SetVelocityValue(std::get<1>(*itr));

        // 事件触发，调用回调函数处理
        if (refHandler->HandlerEvent(*this)) {
          // 事件处理成功，移除事件
          itr = (pImpl->m_vec_ttc_tuple_time_velocity_state_projType_triggerIndex).erase(itr);
        } else {
          ++itr;
        }
      } else {
        ++itr;
      }
    }
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
        << TX_VARS_NAME(TAD_Velocity_EventEvent_egodistance.size,
                        pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex.size());
    for (auto itr = pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex.begin();
         itr != (pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 命中EgoDistance，满足事件触发
      if (HitEgoDistance(eventParam, evUtilInfo, std::get<0>(*itr), std::get<2>(*itr), std::get<3>(*itr),
                         std::get<4>(*itr))) {
        pImpl->SetVelocityValue(std::get<1>(*itr));

        // 事件触发，同样调用回调函数处理
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_egodistance_tuple_time_velocity_state_projType_triggerIndex).erase(itr);
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
#endif /*__TX_Mark__("TAD_Velocity_EventEvent")*/
TX_NAMESPACE_CLOSE(TrafficFlow)
