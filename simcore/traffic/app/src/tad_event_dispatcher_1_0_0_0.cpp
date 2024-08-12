// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_event_dispatcher_1_0_0_0.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tad_ego_vehicle_element.h"
#include "tad_relative_trajectory_follow_element.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tx_collision_detection2d.h"
#include "tx_event_utils.h"
#include "tx_math.h"
#include "tx_spatial_query.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)
#if __TX_Mark__("extern func")
void InitializeEventDispatcher() TX_NOEXCEPT {
  txEventDispatcher::getInstance().Initialize();
  txEventDispatcher_v_1_0_0_0::getInstance().Initialize();
  TAD_SceneEvent_1_0_0_0::ClearSceneEventStates();
}

#endif /*__TX_Mark__("extern func")*/

#if __TX_Mark__("txEventDispatcher_v_1_0_0_0")
void txEventDispatcher_v_1_0_0_0::FlushAllEvents(const Base::TimeParamManager& timeMgr,
                                                 Base::IElementManagerPtr _elemMgrPtr,
                                                 Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 场景加载器和元素管理器不为空
  if (NonNull_Pointer(_elemMgrPtr) && NonNull_Pointer(_sceneLoader)) {
    // 遍历事件处理器集合
    for (auto itr = m_vec_tuple_eventList_handler.begin(); itr != m_vec_tuple_eventList_handler.end(); ++itr) {
      std::vector<ISceneEventViewerPtr>& refSceneEventPtrVec = std::get<0>(*itr);
      TAD_SceneEvent_1_0_0_0_Ptr _handler = std::get<1>(*itr);
      Base::ITrafficElementPtr curElemPtr = std::dynamic_pointer_cast<Base::ITrafficElement>(_handler);

      // 非空判断
      if (_NonEmpty_(refSceneEventPtrVec) && NonNull_Pointer(_handler)) {
        // 遍历场景事件集合
        for (auto itr_element = refSceneEventPtrVec.begin(); itr_element != refSceneEventPtrVec.end();) {
          ISceneEventViewerPtr sceneEventPtr = *itr_element;
          // 场景事件不为空
          if (NonNull_Pointer(sceneEventPtr)) {
            SceneEventType conditionType = sceneEventPtr->conditionType();
            // 触发事件成功
            if (CallSucc(TriggerEvent(timeMgr, conditionType, sceneEventPtr->conditionVec(), curElemPtr, _elemMgrPtr,
                                      _sceneLoader))) {
              if (NonNull_Pointer(sceneEventPtr) && NonNull_Pointer(sceneEventPtr->action()) &&
                  CallSucc(sceneEventPtr->action()->IsValid())) {
                // 获取场景事件的动作列表
                auto& refActionVec = sceneEventPtr->action()->actionList();
                // 遍历动作列表
                for (auto& refAction : refActionVec) {
                  using SceneEventActionType =
                      Base::ISceneLoader::ISceneEventViewer::IActionViewer::SceneEventActionType;
                  // 如果动作类型为 lateralDistance
                  if (_plus_(SceneEventActionType::lateralDistance) == refAction.action_type()) {
                    // 查询并设置目标交通元素指针
                    Base::ITrafficElementPtr targetElemPtr = Utils::QueryTrafficElementPtr(
                        refAction.target_element_type(), refAction.target_element_id(), _elemMgrPtr);
                    refAction.set_target_element_ptr(targetElemPtr);
                  }
                }
              }

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
}

Base::txBool txEventDispatcher_v_1_0_0_0::RegisterVehicleSceneEvent(std::vector<ISceneEventViewerPtr> vecSceneEvent,
                                                                    TAD_SceneEvent_1_0_0_0_Ptr _handler) TX_NOEXCEPT {
  if (_NonEmpty_(vecSceneEvent) && NonNull_Pointer(_handler)) {
    // 添加场景事件到处理器集合
    m_vec_tuple_eventList_handler.emplace_back(std::make_tuple(vecSceneEvent, _handler));
    return true;
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerEvent_Single(const Base::TimeParamManager& timeMgr,
                                                              const SceneEventType conditionType,
                                                              IConditionViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // velocity事件触发
  if (_plus_(IConditionViewer::SceneEventType::velocity_trigger) == conditionType) {
    IVelocityTriggerConditionViewerPtr ptr = std::dynamic_pointer_cast<IVelocityTriggerConditionViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 位置事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::reach_abs_position) == conditionType) {
    IReachAbsPositionConditionViewerPtr ptr =
        std::dynamic_pointer_cast<IReachAbsPositionConditionViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 车道事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::reach_abs_lane) == conditionType) {
    IReachAbsLaneConditionViewerPtr ptr = std::dynamic_pointer_cast<IReachAbsLaneConditionViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 时间事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::time_trigger) == conditionType) {
    ITimeTriggerViewerPtr ptr = std::dynamic_pointer_cast<ITimeTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // ttc事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::ttc_trigger) == conditionType) {
    ITTCTriggerViewerPtr ptr = std::dynamic_pointer_cast<ITTCTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 距离事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::distance_trigger) == conditionType) {
    IDistanceTriggerViewerPtr ptr = std::dynamic_pointer_cast<IDistanceTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 车道ID事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::ego_attach_laneid_custom) == conditionType) {
    IEgoAttachLaneUidCustomViewerPtr ptr = std::dynamic_pointer_cast<IEgoAttachLaneUidCustomViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 车头时距事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::timeheadway_trigger) == conditionType) {
    ITimeHeadwayTriggerViewerPtr ptr = std::dynamic_pointer_cast<ITimeHeadwayTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
    // 元素状态事件触发
  } else if (_plus_(IConditionViewer::SceneEventType::element_state) == conditionType) {
    IElementStateTriggerViewerPtr ptr = std::dynamic_pointer_cast<IElementStateTriggerViewer>(_conditionPtr);
    return TriggerSpecialEvent(timeMgr, ptr, curElemPtr, _elemPtr, _sceneLoader);
  } else {
    LOG(WARNING) << "un support condition : " << Base::txString(__enum2lpsz__(SceneEventType, conditionType));
    return false;
  }
}

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerEvent(const Base::TimeParamManager& timeMgr,
                                                       const SceneEventType conditionType,
                                                       IConditionViewerPtrVec _condition_group_ptr_vec,
                                                       Base::ITrafficElementPtr curElemPtr,
                                                       Base::IElementManagerPtr _elemPtr,
                                                       Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (_plus_(IConditionViewer::SceneEventType::conditionGroup) == conditionType) {
    Base::txBool res = true;
    // 遍历条件组
    for (auto _conditionPtr : _condition_group_ptr_vec) {
      // 根据条件类型触发事件
      res = res && TriggerEvent_Single(timeMgr, _conditionPtr->conditionType(), _conditionPtr, curElemPtr, _elemPtr,
                                       _sceneLoader);
    }
    return res;
  } else {
    // 单个条件触发事件
    IConditionViewerPtr _conditionPtr = _condition_group_ptr_vec.front();
    return TriggerEvent_Single(timeMgr, conditionType, _conditionPtr, curElemPtr, _elemPtr, _sceneLoader);
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

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              IVelocityTriggerConditionViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 触发条件视图指针不为空
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr)) {
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->source_type(), _conditionPtr->source_id(), _elemMgrPtr);
    // 判断目标元素和源元素是否有效
    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(souceElemPtr)) {
      Base::txFloat left_v = 0.0;
      // 判断速度类型
      if (_plus_(ConditionSpeedType::absolute) == _conditionPtr->speed_type()) {
        // 获取绝对速度
        left_v = targetElemPtr->GetRawVelocity();
      } else {
        left_v = targetElemPtr->GetRawVelocity() - souceElemPtr->GetRawVelocity();
      }
      const Base::txFloat right_v = _conditionPtr->speed_threshold();
      // 判断边界类型，并且比较速度
      if (CallSucc(Utils::EquationOperator(_conditionPtr->op(), left_v, _conditionPtr->speed_threshold()))) {
        boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
        // 如果边界操作成功
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          // 如果当前触发计数小于触发计数
          if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
            LogInfo << "[VelocityTrigger][un-trigger]" << TX_VARS(left_v)
                    << TX_VARS_NAME(op, _conditionPtr->boundaryType()._to_string()) << TX_VARS(right_v)
                    << TX_VARS_NAME(CurTriggerCnt, _conditionPtr->currentTriggerCount())
                    << TX_VARS_NAME(TotalTriggerCnt, _conditionPtr->trigger_count());
            _conditionPtr->clearLastValue();
            // 设置上一个值
            _conditionPtr->setLastValue(left_v);
            return false;
          } else {
            LogInfo << "[VelocityTrigger][done-trigger]" << TX_VARS(left_v)
                    << TX_VARS_NAME(op, _conditionPtr->boundaryType()._to_string()) << TX_VARS(right_v)
                    << TX_VARS_NAME(CurTriggerCnt, _conditionPtr->currentTriggerCount())
                    << TX_VARS_NAME(TotalTriggerCnt, _conditionPtr->trigger_count());
            return true;
          }
        } else {
          // 设置上一个值
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

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              IReachAbsPositionConditionViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 如果条件指针和元素管理器指针有效
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr)) {
    // 查询目标交通元素指针
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    // 如果目标交通元素指针有效
    if (NonNull_Pointer(targetElemPtr)) {
      // 获取目标元素位置和条件中心点位置
      Coord::txWGS84 targetPt = targetElemPtr->GetLocation();
      Coord::txWGS84 centerPt(_conditionPtr->point_gps());
      // 计算目标元素与中心点之间的欧几里得距离
      const Base::txFloat left_v = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(targetPt, centerPt);
      const Base::txFloat right_v = _conditionPtr->radius();
      LOG(WARNING) << TX_VARS(left_v) << TX_VARS(right_v);
      // 获取上一个值列表
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      if (CallSucc(Utils::EquationOperator(_plus_(ConditionEquationOp::eq), left_v, right_v))) {
        // 如果边界操作成功
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          // 如果当前触发计数小于触发计数
          if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
            _conditionPtr->clearLastValue();
            // 设置上一个值
            _conditionPtr->setLastValue(left_v);
            return false;
          } else {
            return true;
          }
        } else {
          // 设置上一个值
          _conditionPtr->setLastValue(left_v);
          return false;
        }
      } else {
        // 设置上一个值
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

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              IReachAbsLaneConditionViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 如果条件指针和元素管理器指针有效
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr)) {
    // 查询目标交通元素指针
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    // 如果目标交通元素指针有效
    if (NonNull_Pointer(targetElemPtr)) {
      // 获取条件中的道路ID、车道ID、纵向偏移和横向偏移
      const Base::txRoadID _rid = _conditionPtr->road_id();
      const Base::txLaneID _lid = _conditionPtr->lane_id();
      const Base::txFloat right_s = _conditionPtr->longitudinal_offset();
      const Base::txFloat right_t = _conditionPtr->lateral_offset();
      // 获取目标交通元素的当前车道信息
      const auto targetLaneInfo = targetElemPtr->GetCurrentLaneInfo();
      // 如果目标交通元素不在车道链接上且道路ID和车道ID与条件中的相同
      if (CallFail(targetLaneInfo.isOnLaneLink) && targetLaneInfo.onLaneUid.roadId == _rid &&
          targetLaneInfo.onLaneUid.laneId == _lid) {
        using ReferenceLine = Geometry::SpatialQuery::RTree2D_NearestReferenceLine;
        Base::Info_Lane_t laneInfo;
        Base::txFloat s, t;
        // 如果能够获取目标元素的纵横向坐标
        if (ReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(targetElemPtr->GetLocation(), laneInfo, s, t) &&
            CallFail(laneInfo.isOnLaneLink)) {
          // 计算目标元素在道路上的纵向坐标
          Base::txSectionID endSectionId = laneInfo.onLaneUid.sectionId;
          for (Base::txSectionID secId = 0; secId < endSectionId; ++secId) {
            auto lanePtr =
                HdMap::HadmapCacheConCurrent::GetTxLanePtr(Base::txLaneUId(laneInfo.onLaneUid.roadId, secId, -1));
            if (NonNull_Pointer(lanePtr)) {
              s += lanePtr->getLength();
            }
          }
          // 计算目标元素与条件中心点的欧几里得距离
          const Base::txFloat left_v = std::sqrt((s - right_s) * (s - right_s) + (t - right_t) * (t - right_t));
          const Base::txFloat right_v = _conditionPtr->tolerance();
          boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
          // 如果等式操作成功
          if (CallSucc(Utils::EquationOperator(_plus_(ConditionEquationOp::lt), left_v, right_v))) {
            if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
              // 增加当前触发计数
              _conditionPtr->currentTriggerCount()++;
              if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
                _conditionPtr->clearLastValue();
                // 设置上一个值
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

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              ITimeTriggerViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 如果条件指针和元素管理器指针有效
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemPtr)) {
    const Base::txFloat passTime = timeMgr.PassTime();
    // 判断比较结果
    const Base::txBool op_result =
        Utils::EquationOperator(_conditionPtr->op(), passTime, _conditionPtr->trigger_time());
    LOG(INFO) << "[time_trigger] " << TX_COND(op_result) << TX_VARS_NAME(op, _conditionPtr->op()._to_string())
              << TX_VARS(passTime) << TX_VARS_NAME(trigger_time, _conditionPtr->trigger_time());
    if (CallSucc(op_result)) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              ITTCTriggerViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    // 获取目标交通元素指针
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr = curElemPtr /*_elemMgrPtr->GetEgoArray().front()*/;

    const DistanceProjectionType projType = Utils::DistType2ProjType(_conditionPtr->distanceType(), _sceneLoader);

    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(souceElemPtr) &&
        Utils::IsValidProj(souceElemPtr, targetElemPtr, projType)) {
      // 计算当前TTC
      const Base::txFloat dist = Utils::Compute_TTC_Distance(targetElemPtr, souceElemPtr, projType);
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      const Base::txFloat left_v = dist;
      const Base::txFloat right_v = _conditionPtr->ttc_threshold();
      LOG(WARNING) << TX_VARS(souceElemPtr->Id()) << TX_VARS(left_v) << TX_VARS(right_v);
      // 如果比较公式成立
      if (CallSucc(Utils::EquationOperator(_conditionPtr->op(), left_v, _conditionPtr->ttc_threshold()))) {
        // 边界条件比较
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          // 判断触发次数是否达到阈值
          if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
            _conditionPtr->clearLastValue();
            // 设置上一个值
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
    LOG(WARNING) << TX_COND(NonNull_Pointer(_conditionPtr)) << TX_COND(NonNull_Pointer(_elemMgrPtr))
                 << TX_VARS((_elemMgrPtr->GetEgoCount()));
    return false;
  }
}

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              IDistanceTriggerViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    // 获取目标交通元素指针
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr = nullptr;
    if (_plus_(ElementType::Unknown) == (_conditionPtr->source_type())) {
      souceElemPtr = curElemPtr /*_elemMgrPtr->GetEgoArray().front()*/;
    } else {
      souceElemPtr =
          Utils::QueryTrafficElementPtr(_conditionPtr->source_type(), _conditionPtr->source_id(), _elemMgrPtr);
    }

    // 距离类型获取
    const DistanceProjectionType projType = Utils::DistType2ProjType(_conditionPtr->distanceType(), _sceneLoader);

    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(souceElemPtr) &&
        Utils::IsValidProj(souceElemPtr, targetElemPtr, projType)) {
      // 计算距离值
      const Base::txFloat dist = Utils::Compute_EGO_Distance(targetElemPtr, souceElemPtr, projType);
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      const Base::txFloat left_v = dist;
      const Base::txFloat right_v = _conditionPtr->distance_threshold();
      // 距离与阈值比较判断
      if (CallSucc(Utils::EquationOperator(_conditionPtr->op(), left_v, right_v))) {
        // 边界条件判断
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          // 触发次数判断
          if (_conditionPtr->currentTriggerCount() < _conditionPtr->trigger_count()) {
            _conditionPtr->clearLastValue();
            // 设置上一个值
            _conditionPtr->setLastValue(left_v);
            return false;
          } else {
            return true;
          }
        } else {
          // 设置上一个值
          _conditionPtr->setLastValue(left_v);
          return false;
        }
      } else {
        // 设置上一个值
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

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              IEgoAttachLaneUidCustomViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    // 获取目标交通元素指针
    Base::ITrafficElementPtr souceElemPtr = _elemMgrPtr->GetEgoArray().front();
    if (NonNull_Pointer(souceElemPtr)) {
      // 获取车道信息
      const auto laneInfo = souceElemPtr->GetCurrentLaneInfo();
      const auto rightInfo = _conditionPtr->laneUid();
      // 到达车道判断
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

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              ITimeHeadwayTriggerViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr curElemPtr,
                                                              Base::IElementManagerPtr _elemMgrPtr,
                                                              Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (NonNull_Pointer(_conditionPtr) && NonNull_Pointer(_elemMgrPtr) && (_elemMgrPtr->GetEgoCount() > 0)) {
    // 获取目标元素和当前元素
    Base::ITrafficElementPtr targetElemPtr =
        Utils::QueryTrafficElementPtr(_conditionPtr->target_type(), _conditionPtr->target_id(), _elemMgrPtr);
    Base::ITrafficElementPtr souceElemPtr = nullptr;
    if (_plus_(ElementType::Unknown) == (_conditionPtr->source_type())) {
      TX_MARK("SIM EVENT");
      souceElemPtr = curElemPtr /*_elemMgrPtr->GetEgoArray().front()*/;
    } else {
      souceElemPtr =
          Utils::QueryTrafficElementPtr(_conditionPtr->source_type(), _conditionPtr->source_id(), _elemMgrPtr);
    }

    // 计算THW
    const auto sign = Utils::Compute_EGO_Distance(targetElemPtr, souceElemPtr, DistanceProjectionType::Lane);
    LOG(INFO) << TX_VARS(sign);
    // 获取当前元素速度
    const Base::txFloat srcVelocity = souceElemPtr->StableVelocity();
    // 当前元素速度有效
    if (sign <= 0.0 && Math::isNotZero(srcVelocity) && srcVelocity > 0.0) {
      Base::txFloat dist = 0.0;
      /*souceElem behind of targetElem*/
      const auto alongroute = _conditionPtr->alongroute();
      const auto freespace = _conditionPtr->freespace();

      const Base::txFloat targetLength = targetElemPtr->GetLength();
      const Base::txFloat sourceLength = souceElemPtr->GetLength();
      if (CallSucc(alongroute)) {
        /*lane proj*/
        if (CallSucc(freespace)) {
          /*bounding box*/
          dist = std::fabs(sign);
        } else {
          /*geom center*/
          dist = std::fabs(sign) - 0.5 * (targetLength + sourceLength);
          dist = (dist > 0.0) ? (dist) : (0.0);
        }
      } else {
        /*abs dist*/
        if (CallSucc(freespace)) {
          /*bounding box*/
          Base::txVec2 nouse_intersectPt1, nouse_intersectPt2;
          dist = Geometry::CD2D::smallest_dist_between_ego_and_obsVehicle(
              targetElemPtr->GetPolygon(), souceElemPtr->GetPolygon(), nouse_intersectPt1, nouse_intersectPt2);
        } else {
          /*geom center*/
          dist = Coord::txENU::EuclideanDistanceLocal_2D(targetElemPtr->StableGeomCenter(),
                                                         souceElemPtr->StableGeomCenter());
        }
      }
      // 如果距离小于0，输出警告信息
      if (dist < 0.0) {
        LogWarn << "[Error] dist < 0.0 ";
      }
      // 计算源元素到达目标元素所需的时间
      const Base::txFloat left_v = dist / srcVelocity;
      // 获取thw阈值
      const Base::txFloat right_v = _conditionPtr->time_head_way_threshold();
      LOG(WARNING) << TX_VARS(souceElemPtr->Id()) << TX_VARS(left_v) << TX_VARS(right_v);
      boost::optional<std::list<Base::txFloat> > lastValueList = _conditionPtr->lastValue();
      // thw条件判断
      if (CallSucc(Utils::EquationOperator(_conditionPtr->op(), left_v, right_v))) {
        // 边界条件判断
        if (CallSucc(Utils::BoundaryOperator(_conditionPtr->boundaryType(), lastValueList, left_v, right_v))) {
          _conditionPtr->currentTriggerCount()++;
          // 若不满足触发次数
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
    LOG(WARNING) << TX_COND(NonNull_Pointer(_conditionPtr)) << TX_COND(NonNull_Pointer(_elemMgrPtr))
                 << TX_VARS((_elemMgrPtr->GetEgoCount()));
    return false;
  }
}

Base::txBool txEventDispatcher_v_1_0_0_0::TriggerSpecialEvent(const Base::TimeParamManager& timeMgr,
                                                              IElementStateTriggerViewerPtr _conditionPtr,
                                                              Base::ITrafficElementPtr /*curElemPtr*/,
                                                              Base::IElementManagerPtr /*_elemPtr*/,
                                                              Base::ISceneLoaderPtr /*_sceneLoader*/) TX_NOEXCEPT {
  // 如果条件指针有效
  if (NonNull_Pointer(_conditionPtr)) {
    // 获取事件ID和动作ID
    const auto e_a = _conditionPtr->eventId_actionId();
    const auto state = _conditionPtr->state();
    // 查询场景事件状态信息
    TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventStateInfo queryRes;
    if (CallSucc(TrafficFlow::TAD_SceneEvent_1_0_0_0::QuerySceneEventStates(e_a, queryRes))) {
      // 如果状态匹配
      if (state == queryRes.state) {
        LogInfo << "query success. state match." << TX_VARS(e_a) << TX_VARS_NAME(state, state._to_string());
        return true;
      } else {
        // 输出日志信息
        LogInfo << "query success. but state not match" << TX_VARS(e_a) << TX_VARS_NAME(need_state, state._to_string())
                << TX_VARS_NAME(cur_state, queryRes.state._to_string());
        return false;
      }
    } else {
      LogInfo << "query failure. " << TX_VARS(e_a);
      return false;
    }
  } else {
    LogWarn << "_conditionPtr is nullptr.";
    return false;
  }
}

#endif /*__TX_Mark__("txEventDispatcher_v_1_0_0_0")*/
TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
