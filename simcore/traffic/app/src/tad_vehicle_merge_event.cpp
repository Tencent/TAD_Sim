// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_vehicle_merge_event.h"
#include <boost/optional.hpp>
#include "tx_enum_utils.h"
#include "tx_event_handler.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(TrafficFlow)
#if __TX_Mark__("TAD_VehicleMergeEvent::Impl")

struct TAD_VehicleMergeEvent::Impl {
 public:
  // 存储处理事件的处理器
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  // 表示事件是否处于活动状态
  Base::txBool m_bAlive = true;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
      m_vec_pair_time_dir_duration_offset;

  boost::optional<Base::Enums::VehicleMoveLaneState> m_dir;
  boost::optional<Base::txFloat> m_duration;
  boost::optional<Base::txFloat> m_offset;
  Base::ISceneLoader::IMergesViewerPtr _mergeViewerPtr = nullptr;

  void ResetValue() TX_NOEXCEPT {
    m_dir = boost::none;
    m_duration = boost::none;
    m_offset = boost::none;
  }

  Base::Enums::VehicleMoveLaneState GetCurDirection() const TX_NOEXCEPT {
    // 如果m_dir非空，返回m_dir的值
    // 否则返回Base::Enums::VehicleMoveLaneState::eStraight，表示初始值
    return (m_dir) ? (*m_dir) : (_plus_(Base::Enums::VehicleMoveLaneState::eStraight) TX_MARK("initial value"));
  }

  void SetDirValue(const Base::Enums::VehicleMoveLaneState _dir) TX_NOEXCEPT { m_dir = _dir; }

  Base::txFloat GetCurDuration() const TX_NOEXCEPT {
    return (m_duration) ? (*m_duration) : (FLAGS_DefaultMergeDuration);
  }

  void SetDurationValue(const Base::txFloat dur) TX_NOEXCEPT { m_duration = dur; }

  Base::txFloat GetCurOffset() const TX_NOEXCEPT { return (m_offset) ? (*m_offset) : (FLAGS_DefaultMergeOffset); }

  void SetOffsetValue(const Base::txFloat offset) TX_NOEXCEPT { m_offset = offset; }

  Base::txSysId MergeId() const TX_NOEXCEPT {
    return (_mergeViewerPtr && (_mergeViewerPtr->IsInited())) ? (_mergeViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_mergeViewerPtr && (_mergeViewerPtr->IsInited())) ? (_mergeViewerPtr->Str()) : ("MergeEvent Not Inited.");
  }

  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >&
  vec_pair_time_dir_duration_offset() TX_NOEXCEPT {
    return m_vec_pair_time_dir_duration_offset;
  }
  const std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >&
  vec_pair_time_dir_duration_offset() const TX_NOEXCEPT {
    return m_vec_pair_time_dir_duration_offset;
  }
};
#endif  // __TX_Mark__("TAD_VehicleMergeEvent::Impl")

#if __TX_Mark__("TAD_VehicleMergeEvent")

TAD_VehicleMergeEvent::TAD_VehicleMergeEvent(Base::ISceneLoader::IMergesViewerPtr _mergeViewPtr)
    : pImpl(std::make_shared<TAD_VehicleMergeEvent::Impl>()) {
  txAssert(pImpl && _mergeViewPtr);
  pImpl->_mergeViewerPtr = _mergeViewPtr;
  pImpl->m_vec_pair_time_dir_duration_offset = _mergeViewPtr->timestamp_direction_pair_vector();
}

Base::Enums::VehicleMoveLaneState TAD_VehicleMergeEvent::GetDirectionValue() const TX_NOEXCEPT {
  return pImpl->GetCurDirection();
}

Base::txFloat TAD_VehicleMergeEvent::GetCurDuration() const TX_NOEXCEPT { return pImpl->GetCurDuration(); }

Base::txFloat TAD_VehicleMergeEvent::GetCurOffset() const TX_NOEXCEPT { return pImpl->GetCurOffset(); }

Base::txBool TAD_VehicleMergeEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 如果事件仍然存活
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    // 重置事件的值
    pImpl->ResetValue();
    const Base::TimeParamManager& timeMgr = eventParam._timeMgr;
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
        << TX_VARS_NAME(TAD_VehicleMergeEvent.size, pImpl->m_vec_pair_time_dir_duration_offset.size());
    // 遍历事件触发条件
    for (auto itr = pImpl->m_vec_pair_time_dir_duration_offset.begin();
         itr != (pImpl->m_vec_pair_time_dir_duration_offset.end());) {
      LOG_IF(INFO, FLAGS_LogLevel_Merge) << "[vehicle_event_merge]_MergeId = " << (pImpl->MergeId())
                                         << "_passTime = " << (timeMgr.PassTime()) << " >= " << std::get<0>(*itr);
      // 事件触发条件[时间]判断
      if (timeMgr.PassTime() >= std::get<0>(*itr)) {
        pImpl->SetDirValue(__int2enum__(VehicleMoveLaneState, std::get<1>(*itr)));
        TX_MARK("Base::Enums::VehicleMoveLaneState::_from_integral(std::get<1>(*itr))")
        TX_MARK("pVehicle->SwitchLane(stat[direction + 1], true);");
        pImpl->SetDurationValue(std::get<2>(*itr));
        // 设置事件的偏移量
        pImpl->SetOffsetValue(std::get<3>(*itr));
        // 回调处理事件
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_pair_time_dir_duration_offset).erase(itr);
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

IEvent::EventType TAD_VehicleMergeEvent::GetEventType() const TX_NOEXCEPT { return IEvent::EventType::eMerge; }

Base::txString TAD_VehicleMergeEvent::EventDesc() const TX_NOEXCEPT { return Base::txString("TAD_VehicleMergeEvent"); }

Base::txSize TAD_VehicleMergeEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_VehicleMergeEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_VehicleMergeEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_VehicleMergeEvent::Id() const TX_NOEXCEPT { return pImpl->MergeId(); }

Base::txString TAD_VehicleMergeEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_VehicleMergeEvent::EventObjs() const TX_NOEXCEPT { return pImpl->m_vec_Handlers; }

IEvent::IViewerPtr TAD_VehicleMergeEvent::EventViewPtr() const TX_NOEXCEPT { return pImpl->_mergeViewerPtr; }

std::vector<TAD_VehicleMergeEvent::MergeTimeTriggerItem>& TAD_VehicleMergeEvent::Get_vec_pair_time_dir_duration_offset()
    TX_NOEXCEPT {
  return pImpl->vec_pair_time_dir_duration_offset();
}

const std::vector<TAD_VehicleMergeEvent::MergeTimeTriggerItem>&
TAD_VehicleMergeEvent::Get_vec_pair_time_dir_duration_offset() const TX_NOEXCEPT {
  return pImpl->vec_pair_time_dir_duration_offset();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
#  if __TX_Mark__("TAD_VehicleMergeEventEvent::Impl")
struct TAD_VehicleMergeEventEvent::Impl {
 public:
  using txFloat = Base::txFloat;
  using txInt = Base::txInt;
  using EventStateOptional = IEvent::EventStateOptional;
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

 public:
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  std::vector<std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex;
  std::vector<std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex;

  boost::optional<Base::Enums::VehicleMoveLaneState> m_dir;
  boost::optional<Base::txFloat> m_duration;
  boost::optional<Base::txFloat> m_offset;
  Base::ISceneLoader::IMergesViewerPtr _mergeEventViewerPtr = nullptr;

  void ResetValue() TX_NOEXCEPT {
    m_dir = boost::none;
    m_duration = boost::none;
    m_offset = boost::none;
  }

  Base::Enums::VehicleMoveLaneState GetCurDirection() const TX_NOEXCEPT {
    return (m_dir) ? (*m_dir) : (_plus_(Base::Enums::VehicleMoveLaneState::eStraight) TX_MARK("initial value"));
  }

  void SetDirValue(const Base::Enums::VehicleMoveLaneState _dir) TX_NOEXCEPT { m_dir = _dir; }

  Base::txFloat GetCurDuration() const TX_NOEXCEPT {
    return (m_duration) ? (*m_duration) : (FLAGS_DefaultMergeDuration);
  }

  void SetDurationValue(const Base::txFloat dur) TX_NOEXCEPT { m_duration = dur; }

  Base::txFloat GetCurOffset() const TX_NOEXCEPT { return (m_offset) ? (*m_offset) : (FLAGS_DefaultMergeOffset); }

  void SetOffsetValue(const Base::txFloat offset) TX_NOEXCEPT { m_offset = offset; }

  Base::txSysId MergeEventId() const TX_NOEXCEPT {
    return (_mergeEventViewerPtr && (_mergeEventViewerPtr->IsInited())) ? (_mergeEventViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_mergeEventViewerPtr && (_mergeEventViewerPtr->IsInited())) ? (_mergeEventViewerPtr->Str())
                                                                        : ("MergeEvent Not Inited.");
  }

  std::vector<std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >&
  vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex() TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex;
  }
  const std::vector<std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                               Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >&
  vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex() const TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex;
  }

  std::vector<std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >&
  vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex() TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex;
  }

  const std::vector<std::tuple<Base::txFloat, Base::txInt, EventStateOptional, DistanceProjectionType,
                               Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >&
  vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex() const TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex;
  }
};
#  endif  // __TX_Mark__("TAD_VehicleMergeEvent::Impl")

TAD_VehicleMergeEventEvent::TAD_VehicleMergeEventEvent(Base::ISceneLoader::IMergesViewerPtr _mergeEventViewPtr)
    : pImpl(std::make_shared<TAD_VehicleMergeEventEvent::Impl>()) {
  txAssert(pImpl && _mergeEventViewPtr);
  pImpl->_mergeEventViewerPtr = _mergeEventViewPtr;
  const auto& ref_ttc_direction_tuple_vector = _mergeEventViewPtr->ttc_direction_pair_vector();
  /*pImpl->m_vec_ttc_pair_time_dir = _mergeEventViewPtr->ttc_direction_pair_vector();*/
  pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex.clear();
  // 遍历ref_ttc_direction_tuple_vector
  // // 将其元素添加到pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex中
  for (const auto& ref_ttc_node : ref_ttc_direction_tuple_vector) {
    pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex.emplace_back(std::make_tuple(
        std::get<0>(ref_ttc_node), std::get<1>(ref_ttc_node), IEvent::EventStateOptional(), std::get<2>(ref_ttc_node),
        std::get<3>(ref_ttc_node), std::get<4>(ref_ttc_node), std::get<5>(ref_ttc_node)));
  }
  // 获取egodistance_direction_pair_vector
  const auto& ref_egodistance_direction_tuple_vector = _mergeEventViewPtr->egodistance_direction_pair_vector();
  /*pImpl->m_vec_egodistance_pair_time_dir = _mergeEventViewPtr->egodistance_direction_pair_vector();*/
  pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex.clear();
  // 遍历ref_egodistance_direction_tuple_vector
  // // 将其元素添加到pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex中
  for (const auto& ref_egodistance_node : ref_egodistance_direction_tuple_vector) {
    pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex.emplace_back(std::make_tuple(
        std::get<0>(ref_egodistance_node), std::get<1>(ref_egodistance_node), IEvent::EventStateOptional(),
        std::get<2>(ref_egodistance_node), std::get<3>(ref_egodistance_node), std::get<4>(ref_egodistance_node),
        std::get<5>(ref_egodistance_node)));
  }

  LOG(INFO) << " MergeEvent.id = " << (_mergeEventViewPtr->id()) << ", ttc_event.size = "
            << (pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex).size()
            << ", egodistance_event.size = "
            << (pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex).size();
}

Base::Enums::VehicleMoveLaneState TAD_VehicleMergeEventEvent::GetDirectionValue() const TX_NOEXCEPT {
  return pImpl->GetCurDirection();
}

Base::txFloat TAD_VehicleMergeEventEvent::GetCurDuration() const TX_NOEXCEPT { return pImpl->GetCurDuration(); }

Base::txFloat TAD_VehicleMergeEventEvent::GetCurOffset() const TX_NOEXCEPT { return pImpl->GetCurOffset(); }

IEvent::EventType TAD_VehicleMergeEventEvent::GetEventType() const TX_NOEXCEPT {
  return IEvent::EventType::eMergeEvent;
}

Base::txString TAD_VehicleMergeEventEvent::EventDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_VehicleMergeEventEvent");
}

Base::txSize TAD_VehicleMergeEventEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_VehicleMergeEventEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_VehicleMergeEventEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_VehicleMergeEventEvent::Id() const TX_NOEXCEPT { return pImpl->MergeEventId(); }

Base::txString TAD_VehicleMergeEventEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

IEvent::IViewerPtr TAD_VehicleMergeEventEvent::EventViewPtr() const TX_NOEXCEPT { return pImpl->_mergeEventViewerPtr; }

std::vector<IEventHandlerPtr> TAD_VehicleMergeEventEvent::EventObjs() const TX_NOEXCEPT {
  return pImpl->m_vec_Handlers;
}

Base::txBool TAD_VehicleMergeEventEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 如果事件仍然存活
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    // 重置事件的值
    pImpl->ResetValue();
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
        << TX_VARS_NAME(TAD_VehicleMergeEventEvent_ttc.size,
                        pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex.size());
    // 遍历事件触发条件（TTC）
    for (auto itr = pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex.begin();
         itr != (pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 事件触发条件判断:TTC
      if (HitTTC(eventParam, evUtilInfo, std::get<0>(*itr), std::get<2>(*itr), std::get<3>(*itr), std::get<6>(*itr))) {
        pImpl->SetDirValue(__int2enum__(VehicleMoveLaneState, std::get<1>(*itr)));
        TX_MARK("pVehicle->SwitchLane(stat[direction + 1], true);");
        // 设置事件的持续时间
        pImpl->SetDurationValue(std::get<4>(*itr));
        // 设置事件的偏移量
        pImpl->SetOffsetValue(std::get<5>(*itr));
        // 事件触发，回调函数处理
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex).erase(itr);
        } else {
          ++itr;
        }
      } else {
        ++itr;
      }
    }
    // 输出提示信息
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
        << TX_VARS_NAME(TAD_VehicleMergeEventEvent_egodistance.size,
                        pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex.size());
    // 遍历事件触发条件（距离）
    for (auto itr = pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex.begin();
         itr != (pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 事件触发条件判断:距离
      if (HitEgoDistance(eventParam, evUtilInfo, std::get<0>(*itr), std::get<2>(*itr), std::get<3>(*itr),
                         std::get<6>(*itr))) {
        pImpl->SetDirValue(__int2enum__(VehicleMoveLaneState, std::get<1>(*itr)));
        TX_MARK("Base::Enums::VehicleMoveLaneState::_from_integral(std::get<1>(*itr))")
        TX_MARK("pVehicle->SwitchLane(stat[direction + 1], true);");
        pImpl->SetDurationValue(std::get<4>(*itr));
        // 设置事件的偏移量
        pImpl->SetOffsetValue(std::get<5>(*itr));
        // 事件触发，回调函数处理
        if (refHandler->HandlerEvent(*this)) {
          // 如果事件处理成功，从事件触发条件中删除该条件
          itr = (pImpl->m_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex).erase(itr);
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

std::vector<TAD_VehicleMergeEventEvent::MergeConditionTriggerItem>&
TAD_VehicleMergeEventEvent::Get_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex() TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex();
}

const std::vector<TAD_VehicleMergeEventEvent::MergeConditionTriggerItem>&
TAD_VehicleMergeEventEvent::Get_vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex() const TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_dir_state_projType_duration_offset_triggerIndex();
}

std::vector<TAD_VehicleMergeEventEvent::MergeConditionTriggerItem>&
TAD_VehicleMergeEventEvent::Get_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex()
    TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex();
}

const std::vector<TAD_VehicleMergeEventEvent::MergeConditionTriggerItem>&
TAD_VehicleMergeEventEvent::Get_vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex() const
    TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_dir_state_projType_duration_offset_triggerIndex();
}
#endif  // __TX_Mark__("TAD_VehicleMergeEvent")
TX_NAMESPACE_CLOSE(TrafficFlow)
