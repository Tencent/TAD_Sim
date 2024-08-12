// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_scene_event_1_0_0_0.h"
#include "tx_element_manager_base.h"
#include "tx_enum_def.h"
#include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class txDummyEventDispatcher_v_1_0_0_0 {
 public:
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using ISceneEventViewerPtr = Base::ISceneLoader::ISceneEventViewerPtr;
  using SceneEventType = Base::ISceneLoader::ISceneEventViewer::IConditionViewer::SceneEventType;
  using IConditionViewer = Base::ISceneLoader::ISceneEventViewer::IConditionViewer;
  using IConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IConditionViewerPtr;
  using IConditionViewerPtrVec = Base::ISceneLoader::ISceneEventViewer::IConditionViewerPtrVec;

  using IVelocityTriggerConditionViewer = Base::ISceneLoader::ISceneEventViewer::IVelocityTriggerConditionViewer;
  using IVelocityTriggerConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IVelocityTriggerConditionViewerPtr;
  using IReachAbsPositionConditionViewer = Base::ISceneLoader::ISceneEventViewer::IReachAbsPositionConditionViewer;
  using IReachAbsPositionConditionViewerPtr =
      Base::ISceneLoader::ISceneEventViewer::IReachAbsPositionConditionViewerPtr;
  using IReachAbsLaneConditionViewer = Base::ISceneLoader::ISceneEventViewer::IReachAbsLaneConditionViewer;
  using IReachAbsLaneConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IReachAbsLaneConditionViewerPtr;
  using ITimeTriggerViewer = Base::ISceneLoader::ISceneEventViewer::ITimeTriggerViewer;
  using ITimeTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::ITimeTriggerViewerPtr;
  using ITTCTriggerViewer = Base::ISceneLoader::ISceneEventViewer::ITTCTriggerViewer;
  using ITTCTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::ITTCTriggerViewerPtr;
  using IDistanceTriggerViewer = Base::ISceneLoader::ISceneEventViewer::IDistanceTriggerViewer;
  using IDistanceTriggerViewerPtr = Base::ISceneLoader::ISceneEventViewer::IDistanceTriggerViewerPtr;
  using IEgoAttachLaneUidCustomViewer = Base::ISceneLoader::ISceneEventViewer::IEgoAttachLaneUidCustomViewer;
  using IEgoAttachLaneUidCustomViewerPtr = Base::ISceneLoader::ISceneEventViewer::IEgoAttachLaneUidCustomViewerPtr;

  using ElementType = Base::Enums::ElementType;

  using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
  using ConditionDistanceType = Base::Enums::ConditionDistanceType;
  using ConditionSpeedType = Base::Enums::ConditionSpeedType;
  using ConditionEquationOp = Base::Enums::ConditionEquationOp;
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;

 public:
  static txDummyEventDispatcher_v_1_0_0_0& getInstance() {
    static txDummyEventDispatcher_v_1_0_0_0 instance;
    return instance;
  }

 private:
  txDummyEventDispatcher_v_1_0_0_0() TX_DEFAULT;
  ~txDummyEventDispatcher_v_1_0_0_0() TX_DEFAULT;
  txDummyEventDispatcher_v_1_0_0_0(const txDummyEventDispatcher_v_1_0_0_0&) TX_DEFAULT;
  txDummyEventDispatcher_v_1_0_0_0& operator=(const txDummyEventDispatcher_v_1_0_0_0&) TX_DEFAULT;

 public:
  void Initialize() TX_NOEXCEPT { m_vec_tuple_eventList_handler.clear(); }
  void ClearEvent() TX_NOEXCEPT { m_vec_tuple_eventList_handler.clear(); }
  void FlushAllEvents(const Base::TimeParamManager& timeMgr, Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;
  txBool RegisterVehicleSceneEvent(std::vector<ISceneEventViewerPtr> vecSceneEvent,
                                   TAD_SceneEvent_1_0_0_0_Ptr _handler) TX_NOEXCEPT;
  txInt GetLaneChange() { return m_hasLaneChange; }

 protected:
  txBool TriggerEvent(const Base::TimeParamManager& timeMgr, const SceneEventType _triggerType,
                      IConditionViewerPtrVec _condition_group_ptr_vec, Base::ITrafficElementPtr curElemPtr,
                      Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;
  txBool TriggerEvent_Single(const Base::TimeParamManager& timeMgr, const SceneEventType _triggerType,
                             IConditionViewerPtr _conditionPtr, Base::ITrafficElementPtr curElemPtr,
                             Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

 protected:
  std::vector<std::tuple<std::vector<ISceneEventViewerPtr>, TAD_SceneEvent_1_0_0_0_Ptr> > m_vec_tuple_eventList_handler;
  txInt m_hasLaneChange = 0;
};

extern void InitializeDummyEventDispatcher() TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(TrafficFlow)
