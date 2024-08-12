// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_cloud_stand_alone_event_dispatcher.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_collision_detection2d.h"
#include "tx_event_utils.h"
#include "tx_math.h"
#include "tx_spatial_query.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

void InitializeCloudEventDispatcher() TX_NOEXCEPT { CloudEventDispatcher::getInstance().Initialize(); }

#if __TX_Mark__("txEventDispatcher_v_1_0_0_0")
void CloudEventDispatcher::FlushAllEvents(const Base::TimeParamManager& timeMgr,
                                          Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  for (auto itr = m_vec_tuple_eventList_handler.begin(); itr != m_vec_tuple_eventList_handler.end(); ++itr) {
    std::vector<ISceneEventViewerPtr>& refSceneEventPtrVec = std::get<0>(*itr);
    TAD_SceneEvent_1_0_0_0_Ptr _handler = std::get<1>(*itr);
    Base::ITrafficElementPtr curElemPtr = std::dynamic_pointer_cast<Base::ITrafficElement>(_handler);

    if (_NonEmpty_(refSceneEventPtrVec) && NonNull_Pointer(_handler)) {
      for (auto itr_element = refSceneEventPtrVec.begin(); itr_element != refSceneEventPtrVec.end();) {
        ISceneEventViewerPtr sceneEventPtr = *itr_element;
        if (NonNull_Pointer(sceneEventPtr)) {
          SceneEventType conditionType = sceneEventPtr->conditionType();
          if (CallSucc(TriggerEvent(timeMgr, conditionType, sceneEventPtr->conditionVec().front(), curElemPtr,
                                    _elemMgrPtr))) {
            /*_handler->HandlerEvent(sceneEventPtr);*/
            _handler->HandlerEvent(timeMgr, sceneEventPtr);
            itr_element = refSceneEventPtrVec.erase(itr_element);
            continue;
          }
        }
        itr_element++;
      }
    }
  }
}

Base::txBool CloudEventDispatcher::RegisterVehicleSceneEvent(std::vector<ISceneEventViewerPtr> vecSceneEvent,
                                                             TAD_SceneEvent_1_0_0_0_Ptr _handler) TX_NOEXCEPT {
  if (_NonEmpty_(vecSceneEvent) && NonNull_Pointer(_handler)) {
    m_vec_tuple_eventList_handler.emplace_back(std::make_tuple(vecSceneEvent, _handler));
    return true;
  } else {
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerEvent(const Base::TimeParamManager& timeMgr,
                                                const SceneEventType conditionType, IConditionViewerPtr _conditionPtr,
                                                Base::ITrafficElementPtr curElemPtr,
                                                Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT {
  if (_plus_(IConditionViewer::SceneEventType::velocity_trigger) == conditionType) {
    IVelocityTriggerConditionViewerPtr ptr = std::dynamic_pointer_cast<IVelocityTriggerConditionViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else if (_plus_(IConditionViewer::SceneEventType::reach_abs_position) == conditionType) {
    IReachAbsPositionConditionViewerPtr ptr =
        std::dynamic_pointer_cast<IReachAbsPositionConditionViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else if (_plus_(IConditionViewer::SceneEventType::reach_abs_lane) == conditionType) {
    IReachAbsLaneConditionViewerPtr ptr = std::dynamic_pointer_cast<IReachAbsLaneConditionViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else if (_plus_(IConditionViewer::SceneEventType::time_trigger) == conditionType) {
    ITimeTriggerViewerPtr ptr = std::dynamic_pointer_cast<ITimeTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else if (_plus_(IConditionViewer::SceneEventType::ttc_trigger) == conditionType) {
    ITTCTriggerViewerPtr ptr = std::dynamic_pointer_cast<ITTCTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else if (_plus_(IConditionViewer::SceneEventType::distance_trigger) == conditionType) {
    IDistanceTriggerViewerPtr ptr = std::dynamic_pointer_cast<IDistanceTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else if (_plus_(IConditionViewer::SceneEventType::ego_attach_laneid_custom) == conditionType) {
    IEgoAttachLaneUidCustomViewerPtr ptr = std::dynamic_pointer_cast<IEgoAttachLaneUidCustomViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr);
  } else {
    LOG(WARNING) << "un support condition : " << Base::txString(__enum2lpsz__(SceneEventType, conditionType));
    return false;
  }
}
/*
using ElementType = Base::Enums::ElementType;
                        using SceneEventType = Base::Enums::SceneEventType;
                        using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
                        using ConditionDistanceType = Base::Enums::ConditionDistanceType;
                        using ConditionSpeedType = Base::Enums::ConditionSpeedType;
                        using ConditionEquationOp = Base::Enums::ConditionEquationOp;
*/

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       IVelocityTriggerConditionViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr)) {
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->source_type(), _conditionPtr->source_id(), _elemMgrPtr);
    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(souceElemPtr)) {
      Base::txFloat left_v = 0.0;
      if (_plus_(ConditionSpeedType::absolute) == _conditionPtr->speed_type()) {
        left_v = targetElemPtr->GetRawVelocity();
      } else {
        left_v = targetElemPtr->GetRawVelocity() - souceElemPtr->GetRawVelocity();
      }
      const Base::txFloat right_v = _conditionPtr->speed_threshold();
      if (CallSucc(Utils::EquationOperator(_conditionPtr->op(), left_v, _conditionPtr->speed_threshold()))) {
        boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
            _conditionPtr->clearLastValue();
            _conditionPtr->setLastValue(left_v);
            return false;
          } else {
            return true;
          }
        } else {
          _conditionPtr->setLastValue(left_v);
          return false;
        }
      } else {
        _conditionPtr->setLastValue(left_v);
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       IReachAbsPositionConditionViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr)) {
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    if (NonNull_Pointer(targetElemPtr)) {
      Coord::txWGS84 targetPt = targetElemPtr->GetLocation();
      Coord::txWGS84 centerPt(_conditionPtr->point_gps());
      const Base::txFloat left_v = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(targetPt, centerPt);
      const Base::txFloat right_v = _conditionPtr->radius();
      LOG(WARNING) << TX_VARS(left_v) << TX_VARS(right_v);
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
        _conditionPtr->currentTriggerCount()++;
        if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
          _conditionPtr->clearLastValue();
          _conditionPtr->setLastValue(left_v);
          return false;
        } else {
          return true;
        }
      } else {
        _conditionPtr->setLastValue(left_v);
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       IReachAbsLaneConditionViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr)) {
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    if (NonNull_Pointer(targetElemPtr)) {
      const Base::txRoadID _rid = _conditionPtr->road_id();
      const Base::txLaneID _lid = _conditionPtr->lane_id();
      const Base::txFloat right_s = _conditionPtr->longitudinal_offset();
      const Base::txFloat right_t = _conditionPtr->lateral_offset();
      const auto targetLaneInfo = targetElemPtr->GetCurrentLaneInfo();
      if (CallFail(targetLaneInfo.isOnLaneLink) && targetLaneInfo.onLaneUid.roadId == _rid &&
          targetLaneInfo.onLaneUid.laneId == _lid) {
        using ReferenceLine = Geometry::SpatialQuery::RTree2D_NearestReferenceLine;
        Base::Info_Lane_t laneInfo;
        Base::txFloat s, t;
        if (ReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(targetElemPtr->GetLocation(), laneInfo, s, t) &&
            CallFail(laneInfo.isOnLaneLink)) {
          Base::txSectionID endSectionId = laneInfo.onLaneUid.sectionId;
          for (Base::txSectionID secId = 0; secId < endSectionId; ++secId) {
            auto lanePtr =
                HdMap::HadmapCacheConCurrent::GetTxLanePtr(Base::txLaneUId(laneInfo.onLaneUid.roadId, secId, -1));
            if (NonNull_Pointer(lanePtr)) {
              s += lanePtr->getLength();
            }
          }
          const Base::txFloat left_v = std::sqrt((s - right_s) * (s - right_s) + (t - right_t) * (t - right_t));
          const Base::txFloat right_v = _conditionPtr->tolerance();
          boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
          if (CallSucc(Utils::EquationOperator(_plus_(ConditionEquationOp::lt), left_v, right_v))) {
            if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
              _conditionPtr->currentTriggerCount()++;
              if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
                _conditionPtr->clearLastValue();
                _conditionPtr->setLastValue(left_v);
                return false;
              } else {
                return true;
              }
            } else {
              _conditionPtr->setLastValue(left_v);
              return false;
            }
          } else {
            _conditionPtr->setLastValue(left_v);
            return false;
          }
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       ITimeTriggerViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemPtr)) {
    const Base::txFloat passTime = timeMgr.PassTime();
    if (passTime >= _conditionPtr->trigger_time()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       ITTCTriggerViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr = curElemPtr /*_elemMgrPtr->GetEgoArray().front()*/;
    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(souceElemPtr)) {
      const DistanceProjectionType projType =
          ((_plus_(ConditionDistanceType::euclideandistance) == (_conditionPtr->distanceType()))
               ? (_plus_(DistanceProjectionType::Euclidean))
               : (_plus_(DistanceProjectionType::Lane)));
      const Base::txFloat dist = Utils::Compute_TTC_Distance(targetElemPtr, souceElemPtr, projType);
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      const Base::txFloat left_v = dist;
      const Base::txFloat right_v = _conditionPtr->ttc_threshold();
      LOG(WARNING) << TX_VARS(souceElemPtr->Id()) << TX_VARS(left_v) << TX_VARS(right_v);
      if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
        _conditionPtr->currentTriggerCount()++;
        if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
          _conditionPtr->clearLastValue();
          _conditionPtr->setLastValue(left_v);
          return false;
        } else {
          return true;
        }
      } else {
        _conditionPtr->setLastValue(left_v);
        return false;
      }
    } else {
      return false;
    }
  } else {
    LOG(WARNING) << TX_COND(NonNull_Pointer(_conditionPtr)) << TX_COND(NonNull_Pointer(_elemMgrPtr))
                 << TX_VARS((_elemMgrPtr->GetEgoCount()));
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       IDistanceTriggerViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr = nullptr;
    if (_plus_(ElementType::Unknown) == (_conditionPtr->source_type())) {
      souceElemPtr = curElemPtr /*_elemMgrPtr->GetEgoArray().front()*/;
    } else {
      souceElemPtr =
          Utils::QueryTrafficElementPtr(_conditionPtr->source_type(), _conditionPtr->source_id(), _elemMgrPtr);
    }

    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(souceElemPtr)) {
      const DistanceProjectionType projType =
          ((_plus_(ConditionDistanceType::euclideandistance) == (_conditionPtr->distanceType()))
               ? (_plus_(DistanceProjectionType::Euclidean))
               : (_plus_(DistanceProjectionType::Lane)));
      const Base::txFloat dist = Utils::Compute_EGO_Distance(targetElemPtr, souceElemPtr, projType);
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      const Base::txFloat left_v = dist;
      const Base::txFloat right_v = _conditionPtr->distance_threshold();
      if (CallSucc(Utils::EquationOperator(_conditionPtr->op(), left_v, right_v))) {
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
            _conditionPtr->clearLastValue();
            _conditionPtr->setLastValue(left_v);
            return false;
          } else {
            return true;
          }
        } else {
          _conditionPtr->setLastValue(left_v);
          return false;
        }
      } else {
        _conditionPtr->setLastValue(left_v);
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool CloudEventDispatcher::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                       IEgoAttachLaneUidCustomViewerPtr _conditionPtr,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    Base::ITrafficElementPtr souceElemPtr = _elemMgrPtr->GetEgoArray().front();
    if (NonNull_Pointer(souceElemPtr)) {
      const auto laneInfo = souceElemPtr->GetCurrentLaneInfo();
      const auto rightInfo = _conditionPtr->laneUid();
      if (CallFail(laneInfo.isOnLaneLink) && laneInfo.onLaneUid.roadId == rightInfo.roadId &&
          laneInfo.onLaneUid.sectionId == rightInfo.sectionId && laneInfo.onLaneUid.laneId == rightInfo.laneId) {
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

#endif /*__TX_Mark__("txEventDispatcher_v_1_0_0_0")*/
TX_NAMESPACE_CLOSE(TrafficFlow)
