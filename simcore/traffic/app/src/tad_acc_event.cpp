// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_acc_event.h"
#include <boost/optional.hpp>
#include "tx_event_handler.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(TrafficFlow)

#if __TX_Mark__("TAD_AccEvent::Impl")

/**
 * @brief TAD_AccEvent::Impl 负责事件的内部实现
 *
 * 1. m_vec_Handlers: 用于存储处理器，用于触发相关事件时的回调。
 * 2. m_bAlive: 当设置为 true 时，表示事件尚未结束。
 * 3. m_vec_pair_time_acc: 用于存储事件激活时间和累积时间。
 * 4. m_acc_invalid_type_threshold: 用于存储无效类型时停止事件的条件。
 * 5. m_accValue: 用于存储触发事件时的加速度值。
 * 6. m_endCondition: 用于存储事件结束的条件。
 * 7. _accViewerPtr: 用于存储加速度查看器的实例。
 * 8. m_eventType: 表示事件的类型。
 * 9. vec_pair_time_acc: 用于获取 m_vec_pair_time_acc 的引用。
 * 10. acc_invalid_type_threshold: 用于获取 m_acc_invalid_type_threshold 的引用。
 */
struct TAD_AccEvent::Impl {
 public:
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  std::vector<std::pair<Base::txFloat, Base::txFloat> > m_vec_pair_time_acc;
  std::vector<Base::ISceneLoader::EventEndCondition_t> m_acc_invalid_type_threshold;
  boost::optional<Base::txFloat> m_accValue;
  Base::ISceneLoader::EventEndCondition_t m_endCondition;
  Base::ISceneLoader::IAccelerationViewerPtr _accViewerPtr = nullptr;
  IEvent::EventType m_eventType = IEvent::EventType::eAcc;
  std::vector<std::pair<Base::txFloat, Base::txFloat> >& vec_pair_time_acc() TX_NOEXCEPT { return m_vec_pair_time_acc; }
  const std::vector<std::pair<Base::txFloat, Base::txFloat> >& vec_pair_time_acc() const TX_NOEXCEPT {
    return m_vec_pair_time_acc;
  }
  std::vector<Base::ISceneLoader::EventEndCondition_t>& acc_invalid_type_threshold() TX_NOEXCEPT {
    return m_acc_invalid_type_threshold;
  }
  const std::vector<Base::ISceneLoader::EventEndCondition_t>& acc_invalid_type_threshold() const TX_NOEXCEPT {
    return m_acc_invalid_type_threshold;
  }
  Base::txFloat GetAccValue() const TX_NOEXCEPT {
    // 判断 m_accValue 是否有值
    if (m_accValue) {
      // 如果 m_accValue 有值，则返回 m_accValue 的值
      return *m_accValue;
    } else {
      return 0.0;
    }
  }

  Base::ISceneLoader::EventEndCondition_t GetEndCondition() const TX_NOEXCEPT { return m_endCondition; }

  void ResetAccValue() TX_NOEXCEPT { m_accValue = boost::none; }

  void SetAccValue(Base::txFloat const _acc) TX_NOEXCEPT { m_accValue = _acc; }

  Base::txSysId AccId() const TX_NOEXCEPT {
    // 如果 _accViewerPtr 有值且已初始化，则返回 _accViewerPtr 的 id
    return (_accViewerPtr && (_accViewerPtr->IsInited())) ? (_accViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    // 如果 _accViewerPtr 有值且已初始化，则返回 _accViewerPtr 的 id
    return (_accViewerPtr && (_accViewerPtr->IsInited())) ? (_accViewerPtr->Str()) : ("AccEvent Not Inited.");
  }
};
#endif  // __TX_Mark__("TAD_AccEvent::Impl")

#if __TX_Mark__("TAD_AccEvent")

// 创建实现类的实例
TAD_AccEvent::TAD_AccEvent(Base::ISceneLoader::IAccelerationViewerPtr _accViewPtr)
    : pImpl(std::make_shared<TAD_AccEvent::Impl>()) {
  // 校验参数是否合法
  txAssert(pImpl && _accViewPtr);
  // 将传入的加速度查看器指针存储到实现类中
  pImpl->_accViewerPtr = _accViewPtr;
  // 将时间戳和加速度数据存储到实现类中
  pImpl->m_vec_pair_time_acc = _accViewPtr->timestamp_acceleration_pair_vector();
  pImpl->m_acc_invalid_type_threshold = _accViewPtr->end_condition_vector();
  // 检查 pImpl->m_vec_pair_time_acc 的大小是否等于 pImpl->m_acc_invalid_type_threshold 的大小
  if (pImpl->m_vec_pair_time_acc.size() != pImpl->m_acc_invalid_type_threshold.size()) {
    LOG(WARNING) << "[ERROR] pImpl->m_vec_pair_time_acc.size() != pImpl->m_acc_invalid_type_threshold.size()";
    // 调整 pImpl->m_acc_invalid_type_threshold 的大小，使其与 pImpl->m_vec_pair_time_acc 的大小相等
    pImpl->m_acc_invalid_type_threshold.resize(pImpl->m_vec_pair_time_acc.size());
    // 遍历 pImpl->m_acc_invalid_type_threshold，并为每个元素设置默认值
    for (int i = 0; i < (pImpl->m_vec_pair_time_acc.size()); ++i) {
      pImpl->m_acc_invalid_type_threshold[i].m_IsValid = true;
      pImpl->m_acc_invalid_type_threshold[i].m_type = Base::ISceneLoader::acc_invalid_type::eNone;
      pImpl->m_acc_invalid_type_threshold[i].m_endCondition = 0.0;
    }
  }
}

Base::txFloat TAD_AccEvent::GetAccValue() const TX_NOEXCEPT { return pImpl->GetAccValue(); }

Base::ISceneLoader::EventEndCondition_t TAD_AccEvent::GetEndCondition() const TX_NOEXCEPT {
  return pImpl->GetEndCondition();
}

Base::txBool TAD_AccEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 检查事件是否处于激活状态
  if (IsAlive()) {
    // 确保只有一个事件处理器
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    // 重置累加器的值
    pImpl->ResetAccValue();
    // 获取事件处理器的引用
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    // 获取时间参数
    const Base::TimeParamManager& timeMgr = eventParam._timeMgr;
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger) << TX_VARS_NAME(TAD_AccEvent.size, pImpl->m_vec_pair_time_acc.size());
    for (auto itr = pImpl->m_vec_pair_time_acc.begin(); itr != (pImpl->m_vec_pair_time_acc.end());) {
      // 检查当前时间是否满足事件的触发条件
      if (timeMgr.PassTime() >= (*itr).first) {
        pImpl->SetAccValue((*itr).second);
        // 获取终止条件的索引
        const auto endConditionIdx = std::distance(pImpl->m_vec_pair_time_acc.begin(), itr);
        // 根据索引获取终止条件
        if (endConditionIdx >= 0 && endConditionIdx < (pImpl->m_acc_invalid_type_threshold.size())) {
          pImpl->m_endCondition = pImpl->m_acc_invalid_type_threshold[endConditionIdx];
        } else {
          pImpl->m_endCondition.m_IsValid = false;
        }

        // 事件触发，回调函数处理
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_pair_time_acc).erase(itr);
          // 根据索引移除终止条件
#  if __TX_Mark__("SIM-5334")
          if ((pImpl->m_acc_invalid_type_threshold.size()) > 0 &&
              (endConditionIdx < (pImpl->m_acc_invalid_type_threshold.size()))) {
            auto itr_endCondition = pImpl->m_acc_invalid_type_threshold.begin() + endConditionIdx;
            pImpl->m_acc_invalid_type_threshold.erase(itr_endCondition);
          }
#  endif /*__TX_Mark__("SIM-5334")*/
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

IEvent::EventType TAD_AccEvent::GetEventType() const TX_NOEXCEPT { return IEvent::EventType::eAcc; }

Base::txString TAD_AccEvent::EventDesc() const TX_NOEXCEPT { return Base::txString("TAD_AccEvent"); }

Base::txSize TAD_AccEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_AccEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_AccEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_AccEvent::Id() const TX_NOEXCEPT { return pImpl->AccId(); }

Base::txString TAD_AccEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_AccEvent::EventObjs() const TX_NOEXCEPT { return pImpl->m_vec_Handlers; }

IEvent::IViewerPtr TAD_AccEvent::EventViewPtr() const TX_NOEXCEPT { return pImpl->_accViewerPtr; }

std::vector<TAD_AccEvent::AccTimeTriggerItem>& TAD_AccEvent::Get_vec_pair_time_acc() TX_NOEXCEPT {
  return pImpl->vec_pair_time_acc();
}

const std::vector<TAD_AccEvent::AccTimeTriggerItem>& TAD_AccEvent::Get_vec_pair_time_acc() const TX_NOEXCEPT {
  return pImpl->vec_pair_time_acc();
}

std::vector<Base::ISceneLoader::EventEndCondition_t>& TAD_AccEvent::Get_acc_invalid_type_threshold() TX_NOEXCEPT {
  return pImpl->acc_invalid_type_threshold();
}

const std::vector<Base::ISceneLoader::EventEndCondition_t>& TAD_AccEvent::Get_acc_invalid_type_threshold() const
    TX_NOEXCEPT {
  return pImpl->acc_invalid_type_threshold();
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
#  if __TX_Mark__("TAD_Acc_EventEvent::Impl")
struct TAD_Acc_EventEvent::Impl {
 public:
  using txFloat = Base::txFloat;
  using txUInt = Base::txUInt;
  using EventStateOptional = IEvent::EventStateOptional;
  using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

 public:
  std::vector<IEventHandlerPtr> m_vec_Handlers;
  Base::txBool m_bAlive = true;
  std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, txUInt> >
      m_vec_ttc_tuple_time_acc_state_projType_triggerIndex;
  std::vector<std::tuple<txFloat, txFloat, EventStateOptional, DistanceProjectionType, txUInt> >
      m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex;
  boost::optional<Base::txFloat> m_accValue;
  Base::ISceneLoader::EventEndCondition_t m_endCondition;
  Base::ISceneLoader::IAccelerationViewerPtr _accEventViewerPtr = nullptr;
  IEvent::EventType m_eventType = IEvent::EventType::eAccEvent;
  std::vector<Base::ISceneLoader::EventEndCondition_t> m_acc_invalid_type_threshold_ttc;
  std::vector<Base::ISceneLoader::EventEndCondition_t> m_acc_invalid_type_threshold_egodistance;
  // 获取 m_vec_ttc_tuple_time_acc_state_projType_triggerIndex 的引用
  std::vector<std::tuple<Base::txFloat, Base::txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_ttc_tuple_time_acc_state_projType_triggerIndex() TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_acc_state_projType_triggerIndex;
  }
  const std::vector<
      std::tuple<Base::txFloat, Base::txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_ttc_tuple_time_acc_state_projType_triggerIndex() const TX_NOEXCEPT {
    return m_vec_ttc_tuple_time_acc_state_projType_triggerIndex;
  }
  // 获取 m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex 的引用
  std::vector<std::tuple<Base::txFloat, Base::txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_egodistance_tuple_time_acc_state_projType_triggerIndex() TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex;
  }
  const std::vector<
      std::tuple<Base::txFloat, Base::txFloat, EventStateOptional, DistanceProjectionType, Base::txUInt> >&
  vec_egodistance_tuple_time_acc_state_projType_triggerIndex() const TX_NOEXCEPT {
    return m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex;
  }

  Base::txFloat GetAccValue() const TX_NOEXCEPT {
    // 判断 m_accValue 是否有值
    if (m_accValue) {
      // 如果 m_accValue 有值，则返回 m_accValue 的值
      return *m_accValue;
    } else {
      return 0.0;
    }
  }

  Base::ISceneLoader::EventEndCondition_t GetEndCondition() const TX_NOEXCEPT { return m_endCondition; }

  void ResetAccValue() TX_NOEXCEPT { m_accValue = boost::none; }

  void SetAccValue(Base::txFloat const _acc) TX_NOEXCEPT { m_accValue = _acc; }

  Base::txSysId AccId() const TX_NOEXCEPT {
    return (_accEventViewerPtr && (_accEventViewerPtr->IsInited())) ? (_accEventViewerPtr->id()) : (-1);
  }

  Base::txString Str() const TX_NOEXCEPT {
    return (_accEventViewerPtr && (_accEventViewerPtr->IsInited())) ? (_accEventViewerPtr->Str())
                                                                    : ("Acc_EventEvent Not Inited.");
  }
};
#  endif  // __TX_Mark__("TAD_Acc_EventEvent::Impl")

TAD_Acc_EventEvent::TAD_Acc_EventEvent(Base::ISceneLoader::IAccelerationViewerPtr _accEventViewPtr)
    : pImpl(std::make_shared<TAD_Acc_EventEvent::Impl>()) {
  // 断言 pImpl 和 _accEventViewPtr 不为空
  txAssert(pImpl && _accEventViewPtr);
  // 将 _accEventViewPtr 赋值给 pImpl->_accEventViewerPtr
  pImpl->_accEventViewerPtr = _accEventViewPtr;
  pImpl->m_acc_invalid_type_threshold_egodistance = pImpl->m_acc_invalid_type_threshold_ttc =
      _accEventViewPtr->end_condition_vector();
  // 获取 _accEventViewPtr 的 ttc_acceleration_pair_vector
  const auto& ref_ttc_acceleration_tuple_vector = _accEventViewPtr->ttc_acceleration_pair_vector();

  // 清空 pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex
  pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.clear();
  // 遍历 ref_ttc_acceleration_tuple_vector
  for (const auto& ref_ttc_acc_node : ref_ttc_acceleration_tuple_vector) {
    pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.emplace_back(
        std::make_tuple(std::get<0>(ref_ttc_acc_node), std::get<1>(ref_ttc_acc_node), IEvent::EventStateOptional(),
                        std::get<2>(ref_ttc_acc_node), std::get<3>(ref_ttc_acc_node)));
  }
  /*pImpl->m_vec_ttc_pair_time_acc = _accEventViewPtr->ttc_acceleration_pair_vector();*/
  const auto& ref_egodistance_acceleration_tuple_vector = _accEventViewPtr->egodistance_acceleration_pair_vector();
  /*pImpl->m_vec_egodistance_pair_time_acc = _accEventViewPtr->egodistance_acceleration_pair_vector();*/
  pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.clear();
  // 遍历 ref_egodistance_acceleration_tuple_vector
  for (const auto& ref_egodistance_node : ref_egodistance_acceleration_tuple_vector) {
    // 将 ref_egodistance_node 的元素添加到 pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex
    pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.emplace_back(std::make_tuple(
        std::get<0>(ref_egodistance_node), std::get<1>(ref_egodistance_node), IEvent::EventStateOptional(),
        std::get<2>(ref_egodistance_node), std::get<3>(ref_egodistance_node)));
  }
#  if 0
    if (ref_ttc_acceleration_tuple_vector.size() == ref_egodistance_acceleration_tuple_vector.size()) {
        if (ref_ttc_acceleration_tuple_vector.size() != pImpl->m_acc_invalid_type_threshold.size()) {
            pImpl->m_acc_invalid_type_threshold.resize(ref_ttc_acceleration_tuple_vector.size());
            for (int i = 0; i < (ref_ttc_acceleration_tuple_vector.size()); ++i) {
                pImpl->m_acc_invalid_type_threshold[i].m_IsValid = true;
                pImpl->m_acc_invalid_type_threshold[i].m_type = Base::ISceneLoader::acc_invalid_type::eNone;
                pImpl->m_acc_invalid_type_threshold[i].m_endCondition = 0.0;
            }
        }
    } else {
        LOG(WARNING) << "[ERROR] "
            << TX_VARS_NAME(TTC_COUNT, pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.size())
            << TX_VARS_NAME(EGODIST_COUNT, pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.size())
            << TX_VARS_NAME(END_CONDITION_COUNT, pImpl->m_acc_invalid_type_threshold.size());
        pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.clear();
        pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.clear();
        pImpl->m_acc_invalid_type_threshold.clear();
    }
#  endif
  LOG(INFO) << " AccEvent.id = " << (_accEventViewPtr->id())
            << ", ttc_event.size = " << (pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex).size()
            << ", egodistance_event.size = "
            << (pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex).size();
}

Base::txFloat TAD_Acc_EventEvent::GetAccValue() const TX_NOEXCEPT { return pImpl->GetAccValue(); }

Base::ISceneLoader::EventEndCondition_t TAD_Acc_EventEvent::GetEndCondition() const TX_NOEXCEPT {
  return pImpl->GetEndCondition();
}

IEvent::EventType TAD_Acc_EventEvent::GetEventType() const TX_NOEXCEPT { return IEvent::EventType::eAccEvent; }

Base::txString TAD_Acc_EventEvent::EventDesc() const TX_NOEXCEPT { return Base::txString("TAD_Acc_EventEvent"); }

Base::txSize TAD_Acc_EventEvent::AddEventListener(IEventHandlerPtr _handler) TX_NOEXCEPT {
  pImpl->m_vec_Handlers.emplace_back(_handler);
  return pImpl->m_vec_Handlers.size();
}

Base::txSize TAD_Acc_EventEvent::AbandonEventListener(IEventHandlerPtr) TX_NOEXCEPT {
  txAssert(false);
  return pImpl->m_vec_Handlers.size();
}

Base::txBool TAD_Acc_EventEvent::IsAlive() const TX_NOEXCEPT { return pImpl->m_bAlive; }

Base::txSysId TAD_Acc_EventEvent::Id() const TX_NOEXCEPT { return pImpl->AccId(); }

Base::txString TAD_Acc_EventEvent::Str() const TX_NOEXCEPT { return pImpl->Str(); }

std::vector<IEventHandlerPtr> TAD_Acc_EventEvent::EventObjs() const TX_NOEXCEPT { return pImpl->m_vec_Handlers; }

IEvent::IViewerPtr TAD_Acc_EventEvent::EventViewPtr() const TX_NOEXCEPT { return pImpl->_accEventViewerPtr; }

std::vector<TAD_Acc_EventEvent::AccConditionTriggerItem>&
TAD_Acc_EventEvent::Get_vec_ttc_tuple_time_acc_state_projType_triggerIndex() TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_acc_state_projType_triggerIndex();
}

const std::vector<TAD_Acc_EventEvent::AccConditionTriggerItem>&
TAD_Acc_EventEvent::Get_vec_ttc_tuple_time_acc_state_projType_triggerIndex() const TX_NOEXCEPT {
  return pImpl->vec_ttc_tuple_time_acc_state_projType_triggerIndex();
}

std::vector<TAD_Acc_EventEvent::AccConditionTriggerItem>&
TAD_Acc_EventEvent::Get_vec_egodistance_tuple_time_acc_state_projType_triggerIndex() TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_acc_state_projType_triggerIndex();
}

const std::vector<TAD_Acc_EventEvent::AccConditionTriggerItem>&
TAD_Acc_EventEvent::Get_vec_egodistance_tuple_time_acc_state_projType_triggerIndex() const TX_NOEXCEPT {
  return pImpl->vec_egodistance_tuple_time_acc_state_projType_triggerIndex();
}

Base::txBool TAD_Acc_EventEvent::TriggerEvent(const EventTriggerParam_t& eventParam) TX_NOEXCEPT {
  // 判断对象是否存活，如果存活则继续执行后面的代码，否则直接返回
  if (IsAlive()) {
    txAssert(1 == (pImpl->m_vec_Handlers).size() && (pImpl->m_vec_Handlers[0]));
    pImpl->ResetAccValue();
    auto& refHandler = (pImpl->m_vec_Handlers[0]);
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger) << TX_VARS_NAME(
        TAD_Acc_EventEvent_ttc.size, pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.size());

    for (auto itr = pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.begin();
         itr != (pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 事件触发条件判断:TTC
      if (HitTTC(eventParam, evUtilInfo, std::get<0>(*itr), std::get<2>(*itr), std::get<3>(*itr), std::get<4>(*itr))) {
        pImpl->SetAccValue(std::get<1>(*itr));
        const auto endConditionIdx =
            std::distance(pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.begin(), itr);
        // 判断 endConditionIdx 是否在合法范围内
        if (endConditionIdx >= 0 && endConditionIdx < (pImpl->m_acc_invalid_type_threshold_ttc.size())) {
          // 若 endConditionIdx 在范围内，则将对应的终止条件赋值给 pImpl->m_endCondition
          pImpl->m_endCondition = pImpl->m_acc_invalid_type_threshold_ttc[endConditionIdx];
        } else {
          // 若 endConditionIdx 不在范围内，则将 pImpl->m_endCondition 设置为无效值
          pImpl->m_endCondition.m_IsValid = false;
        }

        // 回调函数处理事件
        if (refHandler->HandlerEvent(*this)) {
          LOG(INFO) << TX_VARS(pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex.size())
                    << TX_VARS(pImpl->m_acc_invalid_type_threshold_ttc.size());
          itr = (pImpl->m_vec_ttc_tuple_time_acc_state_projType_triggerIndex).erase(itr);
#  if __TX_Mark__("SIM-5334")
          if ((pImpl->m_acc_invalid_type_threshold_ttc.size() > 0) &&
              (endConditionIdx < (pImpl->m_acc_invalid_type_threshold_ttc.size()))) {
            auto itr_endCondition = pImpl->m_acc_invalid_type_threshold_ttc.begin() + endConditionIdx;
            pImpl->m_acc_invalid_type_threshold_ttc.erase(itr_endCondition);
          }
#  endif /*__TX_Mark__("SIM-5334")*/
        } else {
          ++itr;
        }
      } else {
        ++itr;
      }
    }
    LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
        << TX_VARS_NAME(TAD_Acc_EventEvent_egodistance.size,
                        pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.size());
    for (auto itr = pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.begin();
         itr != (pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.end());) {
      IEventHandler::EventHitUtilInfo_t evUtilInfo = refHandler->GetEventHitUtilInfo();
      // 事件触发条件判断:EGO Distance
      if (HitEgoDistance(eventParam, evUtilInfo, std::get<0>(*itr), std::get<2>(*itr), std::get<3>(*itr),
                         std::get<4>(*itr))) {
        pImpl->SetAccValue(std::get<1>(*itr));
        const auto endConditionIdx =
            std::distance(pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex.begin(), itr);
        if (endConditionIdx >= 0 && endConditionIdx < (pImpl->m_acc_invalid_type_threshold_egodistance.size())) {
          // 获取指定索引的元素，并将其分配给 pImpl->m_endCondition
          pImpl->m_endCondition = pImpl->m_acc_invalid_type_threshold_egodistance[endConditionIdx];
        } else {
          // 如果索引无效，将 pImpl->m_endCondition 设置为无效状态
          pImpl->m_endCondition.m_IsValid = false;
        }

        // 回调函数处理事件
        if (refHandler->HandlerEvent(*this)) {
          itr = (pImpl->m_vec_egodistance_tuple_time_acc_state_projType_triggerIndex).erase(itr);
#  if __TX_Mark__("SIM-5334")
          if ((pImpl->m_acc_invalid_type_threshold_egodistance.size() > 0) &&
              (endConditionIdx < (pImpl->m_acc_invalid_type_threshold_egodistance.size()))) {
            // 使用迭代器删除指定索引的元素
            auto itr_endCondition = pImpl->m_acc_invalid_type_threshold_egodistance.begin() + endConditionIdx;
            pImpl->m_acc_invalid_type_threshold_egodistance.erase(itr_endCondition);
          }
#  endif /*__TX_Mark__("SIM-5334")*/
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
#endif  // __TX_Mark__("TAD_AccEvent")
TX_NAMESPACE_CLOSE(TrafficFlow)
