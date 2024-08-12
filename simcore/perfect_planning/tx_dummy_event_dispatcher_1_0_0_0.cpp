// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_dummy_event_dispatcher_1_0_0_0.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tad_ego_vehicle_element.h"
#include "tx_collision_detection2d.h"
#include "tx_math.h"
#include "tx_planning_scene_event_loader.h"
#include "tx_spatial_query.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)
#if __TX_Mark__("extern func")
void InitializeDummyEventDispatcher() TX_NOEXCEPT { txDummyEventDispatcher_v_1_0_0_0::getInstance().Initialize(); }

#endif /*__TX_Mark__("extern func")*/

#if __TX_Mark__("txDummyEventDispatcher_v_1_0_0_0")
void txDummyEventDispatcher_v_1_0_0_0::FlushAllEvents(const Base::TimeParamManager& timeMgr,
                                                      Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  m_hasLaneChange = false;
  for (auto itr = m_vec_tuple_eventList_handler.begin(); itr != m_vec_tuple_eventList_handler.end(); ++itr) {
    std::vector<ISceneEventViewerPtr>& refSceneEventPtrVec = std::get<0>(*itr);
    TAD_SceneEvent_1_0_0_0_Ptr _handler = std::get<1>(*itr);
    Base::ITrafficElementPtr curElemPtr = std::dynamic_pointer_cast<Base::ITrafficElement>(_handler);

    if (_NonEmpty_(refSceneEventPtrVec) && NonNull_Pointer(_handler)) {
      LOG(WARNING) << TX_VARS(refSceneEventPtrVec.size());
      for (auto itr_element = refSceneEventPtrVec.begin(); itr_element != refSceneEventPtrVec.end();) {
        ISceneEventViewerPtr sceneEventPtr = *itr_element;
        if (NonNull_Pointer(sceneEventPtr)) {
          SceneEventType conditionType = sceneEventPtr->conditionType();
          if (CallSucc(TriggerEvent(timeMgr, conditionType, sceneEventPtr->conditionVec(), curElemPtr, _elemMgrPtr))) {
            _handler->HandlerEvent(timeMgr, sceneEventPtr);
            m_hasLaneChange = _handler->GetHasLaneChange();
            _handler->GetHasLaneChange() = 0;
            itr_element = refSceneEventPtrVec.erase(itr_element);
            continue;
          }
        }
        itr_element++;
      }
    }
  }
}

Base::txBool txDummyEventDispatcher_v_1_0_0_0::RegisterVehicleSceneEvent(
    std::vector<ISceneEventViewerPtr> vecSceneEvent, TAD_SceneEvent_1_0_0_0_Ptr _handler) TX_NOEXCEPT {
  if (_NonEmpty_(vecSceneEvent) && NonNull_Pointer(_handler)) {
    m_vec_tuple_eventList_handler.emplace_back(std::make_tuple(vecSceneEvent, _handler));
    return true;
  } else {
    return false;
  }
}

Base::txBool txDummyEventDispatcher_v_1_0_0_0::TriggerEvent_Single(const Base::TimeParamManager& timeMgr,
                                                                   const SceneEventType conditionType,
                                                                   IConditionViewerPtr _conditionPtr,
                                                                   Base::ITrafficElementPtr curElemPtr,
                                                                   Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr)) {
    if (_plus_(IConditionViewer::SceneEventType::dummy_speed) == conditionType) {
      using IDummySpeedTriggerConditionViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummySpeedTriggerConditionViewer;
      using IDummySpeedTriggerConditionViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummySpeedTriggerConditionViewerPtr;
      IDummySpeedTriggerConditionViewerPtr ptr =
          std::dynamic_pointer_cast<IDummySpeedTriggerConditionViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummySpeedTriggerConditionViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_relative_speed) == conditionType) {
      using IDummyRelativeSpeedTriggerConditionViewer = SceneLoader::PlanningSceneEvent_SceneLoader::
          IDummySceneEventViewer::IDummyRelativeSpeedTriggerConditionViewer;
      using IDummyRelativeSpeedTriggerConditionViewerPtr = SceneLoader::PlanningSceneEvent_SceneLoader::
          IDummySceneEventViewer::IDummyRelativeSpeedTriggerConditionViewerPtr;
      IDummyRelativeSpeedTriggerConditionViewerPtr ptr =
          std::dynamic_pointer_cast<IDummyRelativeSpeedTriggerConditionViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyRelativeSpeedTriggerConditionViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_reach_position) == conditionType) {
      using IDummyReachAbsPositionConditionViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyReachAbsPositionConditionViewer;
      using IDummyReachAbsPositionConditionViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyReachAbsPositionConditionViewerPtr;
      IDummyReachAbsPositionConditionViewerPtr ptr =
          std::dynamic_pointer_cast<IDummyReachAbsPositionConditionViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyReachAbsPositionConditionViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_ttc_trigger) == conditionType) {
      using IDummyTTCTriggerViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTTCTriggerViewer;
      using IDummyTTCTriggerViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTTCTriggerViewerPtr;
      IDummyTTCTriggerViewerPtr ptr = std::dynamic_pointer_cast<IDummyTTCTriggerViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyTTCTriggerViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_distance_trigger) == conditionType) {
      using IDummyDistanceTriggerViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyDistanceTriggerViewer;
      using IDummyDistanceTriggerViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyDistanceTriggerViewerPtr;
      IDummyDistanceTriggerViewerPtr ptr = std::dynamic_pointer_cast<IDummyDistanceTriggerViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyDistanceTriggerViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_ego_attach_laneid_custom) == conditionType) {
      using IDummyEgoAttachLaneUidCustomViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyEgoAttachLaneUidCustomViewer;
      using IDummyEgoAttachLaneUidCustomViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyEgoAttachLaneUidCustomViewerPtr;
      IDummyEgoAttachLaneUidCustomViewerPtr ptr =
          std::dynamic_pointer_cast<IDummyEgoAttachLaneUidCustomViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyEgoAttachLaneUidCustomViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_time_trigger) == conditionType) {
      using IDummyTimeTriggerViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTimeTriggerViewer;
      using IDummyTimeTriggerViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTimeTriggerViewerPtr;
      IDummyTimeTriggerViewerPtr ptr = std::dynamic_pointer_cast<IDummyTimeTriggerViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyTimeTriggerViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_time_head_way) == conditionType) {
      using IDummyTimeHeadwayTriggerViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTimeHeadwayTriggerViewer;
      using IDummyTimeHeadwayTriggerViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTimeHeadwayTriggerViewerPtr;
      IDummyTimeHeadwayTriggerViewerPtr ptr = std::dynamic_pointer_cast<IDummyTimeHeadwayTriggerViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr, curElemPtr, _elemPtr);
      } else {
        LogWarn << "IDummyTimeHeadwayTriggerViewerPtr is nullptr.";
        return false;
      }
    } else if (_plus_(IConditionViewer::SceneEventType::dummy_element_state) == conditionType) {
      using IDummyElementStateTriggerViewer =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyElementStateTriggerViewer;
      using IDummyElementStateTriggerViewerPtr =
          SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyElementStateTriggerViewerPtr;
      IDummyElementStateTriggerViewerPtr ptr =
          std::dynamic_pointer_cast<IDummyElementStateTriggerViewer>(_conditionPtr);
      if (NonNull_Pointer(ptr)) {
        return ptr->TriggerEvent(timeMgr);
      } else {
        LogWarn << "IDummyElementStateTriggerViewerPtr is nullptr.";
        return false;
      }
    } else {
      LOG(WARNING) << "un support condition : " << Base::txString(__enum2lpsz__(SceneEventType, conditionType));
      return false;
    }
  } else {
    LogWarn << "_conditionPtr is nullptr.";
    return false;
  }
}

Base::txBool txDummyEventDispatcher_v_1_0_0_0::TriggerEvent(const Base::TimeParamManager& timeMgr,
                                                            const SceneEventType conditionType,
                                                            IConditionViewerPtrVec _condition_group_ptr_vec,
                                                            Base::ITrafficElementPtr curElemPtr,
                                                            Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT {
  if (_plus_(IConditionViewer::SceneEventType::conditionGroup) == conditionType) {
    Base::txBool res = true;
    for (auto _conditionPtr : _condition_group_ptr_vec) {
      res = res && TriggerEvent_Single(timeMgr, _conditionPtr->conditionType(), _conditionPtr, curElemPtr, _elemPtr);
    }
    return res;
  } else {
    IConditionViewerPtr _conditionPtr = _condition_group_ptr_vec.front();
    return TriggerEvent_Single(timeMgr, _conditionPtr->conditionType(), _conditionPtr, curElemPtr, _elemPtr);
  }
}
#endif /*__TX_Mark__("txDummyEventDispatcher_v_1_0_0_0")*/
TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
