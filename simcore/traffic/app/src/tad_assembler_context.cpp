// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_assembler_context.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tad_ai_vehicle_element.h"
#include "tad_ego_vehicle_element.h"
#include "tad_element_generator.h"
#include "tad_event_dispatcher_1_0_0_0.h"
#include "tad_map_obstacle_element.h"
#include "tad_obstacle_element.h"
#include "tad_pedestrian_element.h"
#include "tad_relative_trajectory_follow_element.h"
#include "tad_signal_element.h"
#include "tad_traj_follow_vehicle_element.h"
#include "tad_user_defined_vehicle_element.h"
#include "its_sim.pb.h"
#include "tx_event_dispatcher.h"
#include "tx_profile_utils.h"
#include "tx_serialization.h"
#include "tx_tadsim_flags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(Scene)

Base::txBool TAD_AssemblerContext::GenerateScene(Base::IElementManagerPtr _elemMgr,
                                                 Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  // 检查 _elemMgr 和 _loader 是否非空
  if (NonNull_Pointer(_elemMgr) && NonNull_Pointer(_loader)) {
    // 生成流量流元素
    Base::txInt const retCnt_FlowElem = GenerateTrafficFlowElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_FlowElem > 0) << "GenerateTrafficFlowElement " << retCnt_FlowElem;
    LOG_IF(WARNING, retCnt_FlowElem <= 0) << "GenerateTrafficFlowElement " << retCnt_FlowElem;

    // 生成交通环境元素
    Base::txInt const retCnt_EnvElem = GenerateTrafficEnvironmentElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_EnvElem > 0) << "GenerateTrafficEnvironmentElement " << retCnt_EnvElem;
    LOG_IF(WARNING, retCnt_EnvElem <= 0) << "GenerateTrafficEnvironmentElement " << retCnt_EnvElem;

    // 生成交通参考元素
    Base::txInt const retCnt_RefElem = GenerateTrafficReferenceElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_RefElem > 0) << "GenerateTrafficReferenceElement " << retCnt_RefElem;
    LOG_IF(WARNING, retCnt_RefElem <= 0) << "GenerateTrafficReferenceElement " << retCnt_RefElem;

    return true;
  } else {
    LogWarn << "Param Error.";
    return false;
  }
}

Base::txBool TAD_AssemblerContext::GenerateEgo(Base::IElementManagerPtr _elemMgr,
                                               Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  // 检查传入的参数是否为空
  if (NonNull_Pointer(_elemMgr) && NonNull_Pointer(_loader)) {
#if USE_EgoGroup
    for (auto& egoGroup : _loader->GetAllEgoGroups()) {
      const auto egoType = _loader->GetEgoType(egoGroup);
      auto egoDataPtr = _loader->GetEgoData(egoGroup);

      TrafficFlow::TAD_EgoVehicleElementPtr ego = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();
      TrafficFlow::TAD_EgoVehicleElementPtr egoTrailer = nullptr;
      auto trailerDataPtr = _loader->GetEgoTrailerData(egoGroup);

      if (_plus_(Base::ISceneLoader::EgoType::eTruck) == egoType) {
        egoTrailer = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();
        egoTrailer->SetSubType(TrafficFlow::TAD_EgoVehicleElement::EgoSubType::eFollower);
      }
      if (NonNull_Pointer(ego) && CallSucc(ego->Initialize(egoDataPtr, _loader)) &&
          CallSucc(((egoTrailer) ? (egoTrailer->Initialize(trailerDataPtr, _loader)) : (true)))) {
        _elemMgr->AddEgoPtr(ego);
        if (_plus_(Base::ISceneLoader::EgoType::eTruck) == egoType && egoTrailer) {
          _elemMgr->AddEgoPtr(egoTrailer);
        }
      } else {
        LogWarn << "Create Ego Element Failure." << TX_VARS(egoGroup);  // Just record failed init ego
      }
    }
    LogInfo << "Create Ego Element Success.";
    return true;
#else  /*USE_EgoGroup*/
    const auto egoType = _loader->GetEgoType();
    // 获取车辆数据指针
    auto egoDataPtr = _loader->GetEgoData();
    // 创建自身车辆元素实例
    TrafficFlow::TAD_EgoVehicleElementPtr ego = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();
    // 创建跟随车辆元素实例（如果适用）
    TrafficFlow::TAD_EgoVehicleElementPtr egoTrailer = nullptr;
    auto trailerDataPtr = _loader->GetEgoTrailerData();
    if (_plus_(Base::ISceneLoader::EgoType::eTruck) == egoType) {
      egoTrailer = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();
      egoTrailer->SetSubType(TrafficFlow::TAD_EgoVehicleElement::EgoSubType::eFollower);
    }

    // 初始化车辆元素实例
    if (NonNull_Pointer(ego) && CallSucc(ego->Initialize(egoDataPtr, _loader)) &&
        CallSucc(((egoTrailer) ? (egoTrailer->Initialize(trailerDataPtr, _loader)) : (true)))) {
      // 将车辆元素实例添加到元素管理器中
      _elemMgr->AddEgoPtr(ego);
      // 如果是大型货车，则还需要添加车尾元素实例
      if (_plus_(Base::ISceneLoader::EgoType::eTruck) == egoType && egoTrailer) {
        _elemMgr->AddEgoPtr(egoTrailer);
      }
      LogInfo << "Create Vehicle Element Success.";
      return true;
    } else {
      LogWarn << "Create Vehicle Element Failure.";
      return false;
    }
#endif /*USE_EgoGroup*/
  } else {
    LogWarn << " Param Error.";
    return false;
  }
}

Base::txInt TAD_AssemblerContext::GenerateTrafficFlowElement(Base::IElementManagerPtr _elemMgr,
                                                             Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  // 它将各种类型的traffic流元素分别生成，并返回生成的元素数量
  Base::txInt retCnt = 0;
  // 生成traffic中的vehicle
  retCnt += GenerateTrafficFlowElement_Vehicle(_elemMgr, _loader);
  // 生成traffic中的pedestrian
  retCnt += GenerateTrafficFlowElement_Pedestrian(_elemMgr, _loader);
  // 生成traffic中的obstacle
  retCnt += GenerateTrafficFlowElement_Obstacle(_elemMgr, _loader);
  return retCnt;
}

Base::txInt TAD_AssemblerContext::GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr _elemMgr,
                                                                     Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;
  // 获取所有的vehicel data
  auto res = _loader->GetAllVehicleData();
  // 遍历场景中所有的vehicle
  for (auto& pair_id_viewerPtr : res) {
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(pair_id_viewerPtr.second);
    // 判断vehicle视图指针不为空
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      // 获取vehicle的behav用来进行类型判断
      const Base::Enums::VEHICLE_BEHAVIOR behavorType = _elemAttrViewPtr->behaviorEnum();
      // 若vehicle behav是用户定义
      if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eUserDefined)) == behavorType) {
        // 实例化一个TAD_UserDefined_VehicleElement对象
        TrafficFlow::TAD_UserDefined_VehicleElementPtr vehicle =
            std::make_shared<TrafficFlow::TAD_UserDefined_VehicleElement>();
        // 初始化vehicle 成功
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          // 添加到元素管理器
          _elemMgr->AddVehiclePtr(vehicle);
          // 切换到场景事件指针
          TrafficFlow::TAD_SceneEvent_1_0_0_0_Ptr sceneEventPtr =
              std::dynamic_pointer_cast<TrafficFlow::TAD_SceneEvent_1_0_0_0>(vehicle);
          // 如果指针不为空
          if (NonNull_Pointer(sceneEventPtr)) {
            // 如果场景事件使用的是1.0.0.0
            if (CallSucc(sceneEventPtr->UseSceneEvent_1_0_0_0())) {
              LogInfo << "[vehicle_event][scene_event_1.0.0.0] vehicle_id:" << (*vehicle).Id()
                      << TX_VARS_NAME(sceneEventCnt, sceneEventPtr->SceneEvent().size());
              // 注册场景事件txEventDispatcher_v_1_0_0_0
              TrafficFlow::txEventDispatcher_v_1_0_0_0::getInstance().RegisterVehicleSceneEvent(
                  sceneEventPtr->SceneEvent(), sceneEventPtr);
            } else {
              // 否则
              LogInfo << "[vehicle_event][deprecated] vehicle_id:" << (*vehicle).Id()
                      << ", Merge : " << ((vehicle->getMergeViewPtr()) ? (vehicle->getMergeViewPtr()->Str()) : "Unset.")
                      << ", Merge_Event : "
                      << ((vehicle->getMergeEventViewPtr()) ? (vehicle->getMergeEventViewPtr()->Str()) : "Unset.")
                      << ", Acc : " << ((vehicle->getAccViewPtr()) ? (vehicle->getAccViewPtr()->Str()) : "Unset")
                      << ", Acc_Event : "
                      << ((vehicle->getAccEventViewPtr()) ? (vehicle->getAccEventViewPtr()->Str()) : "Unset")
                      << ", Velocity : "
                      << ((vehicle->getVelocityViewPtr()) ? (vehicle->getVelocityViewPtr()->Str()) : "Unset ")
                      << ", Velocity_Event : "
                      << ((vehicle->getVelocityEventViewPtr()) ? (vehicle->getVelocityEventViewPtr()->Str())
                                                               : "Unset ");
              if (_plus_(Base::ISceneLoader::VEHICLE_BEHAVIOR::eUserDefined) == vehicle->VehicleBehavior()) {
                // 否则注册以下事件到txEventDispatcher
                txEventDispatcher::getInstance().RegisterAccEvent(vehicle->getAccViewPtr(), vehicle);
                txEventDispatcher::getInstance().RegisterAccEventEvent(vehicle->getAccEventViewPtr(), vehicle);
                txEventDispatcher::getInstance().RegisterMergeEvent(vehicle->getMergeViewPtr(), vehicle);
                txEventDispatcher::getInstance().RegisterMergeEventEvent(vehicle->getMergeEventViewPtr(), vehicle);
                txEventDispatcher::getInstance().RegisterVelocityEvent(vehicle->getVelocityViewPtr(), vehicle);
                txEventDispatcher::getInstance().RegisterVelocityEventEvent(vehicle->getVelocityEventViewPtr(),
                                                                            vehicle);
              } else {
                LOG(FATAL) << " vehicle behavior is not user_defined, do not response any event. "
                           << TX_VARS(vehicle->Id());
              }
            }
          } else {
            LogWarn << "un support scene event 1.0.0.0" << TX_VARS_NAME(vehId, (*vehicle).Id());
          }
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
        // 这里判断是否AI模式
      } else if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI)) == behavorType ||
                 (_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI_Arterial)) == behavorType) {
        TrafficFlow::TAD_AI_VehicleElementPtr vehicle = std::make_shared<TrafficFlow::TAD_AI_VehicleElement>();
        // 初始化成功
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          // 添加到元素管理器
          _elemMgr->AddVehiclePtr(vehicle);
          LogInfo << "[vehicle_event][TadAI] vehicle_id:" << (*vehicle).Id();
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
      } else if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTrajectoryFollow)) == behavorType) {  // 是否TrajectoryFollow
        TrafficFlow::TAD_TrajFollow_VehicleElementPtr vehicle =
            std::make_shared<TrafficFlow::TAD_TrajFollow_VehicleElement>();
        // 检查 vehicle 是否为非空指针 并初始化
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          // 添加vehicle
          _elemMgr->AddVehiclePtr(vehicle);
          // 判断事件触发标志位
          if (FLAGS_VehTrajEventTrigger) {
            TrafficFlow::TAD_SceneEvent_1_0_0_0_Ptr sceneEventPtr =
                std::dynamic_pointer_cast<TrafficFlow::TAD_SceneEvent_1_0_0_0>(vehicle);
            // 判断场景事件指针不为空
            if (NonNull_Pointer(sceneEventPtr)) {
              if (CallSucc(sceneEventPtr->UseSceneEvent_1_0_0_0())) {
                LogInfo << "[vehicle_event][trajectory_follow][scene_event_1.0.0.0] vehicle_id:" << (*vehicle).Id()
                        << TX_VARS_NAME(sceneEventCnt, sceneEventPtr->SceneEvent().size());
                // 注册场景事件
                TrafficFlow::txEventDispatcher_v_1_0_0_0::getInstance().RegisterVehicleSceneEvent(
                    sceneEventPtr->SceneEvent(), sceneEventPtr);
              }
            } else {
              LogWarn << "un support scene event 1.0.0.0" << TX_VARS_NAME(vehId, (*vehicle).Id());
            }
          } else {
            LogWarn << "vehicle trajectory follow ignore event trigger." << TX_VARS_NAME(VehId, vehicle->Id());
          }
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
#ifdef InjectMode
        // 若vehicle模式是eInjectMode
      } else if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eInjectMode)) == behavorType) {
        TrafficFlow::InjectModeVehicleElementPtr vehicle = std::make_shared<TrafficFlow::InjectModeVehicleElement>();
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          // 添加元素到element manager中
          _elemMgr->AddVehiclePtr(vehicle);
          LogInfo << "[vehicle_event][InjectMode] vehicle_id:" << (*vehicle).Id();
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
#endif /*InjectMode*/
      } else {
        LogWarn << "unsupport behavior " << behavorType._to_string() << TX_VARS_NAME(VehId, _elemAttrViewPtr->id());
      }
    } else {
      LogWarn << "dynamic_pointer_cast IVehiclesViewer error.";
    }
  }
  LogInfo << "[Create_Vehicle]_Count = " << retCnt;
  return retCnt;
}

// 生成traffic中所有的pedestrians
Base::txInt TAD_AssemblerContext::GenerateTrafficFlowElement_Pedestrian(Base::IElementManagerPtr _elemMgr,
                                                                        Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;

  // 根据场景获取其中所有的pedestrian数据
  auto res = _loader->GetAllPedestrianData();
  // 遍历添加pedestrian
  for (auto& entity : res) {
    Base::ISceneLoader::IPedestriansViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IPedestriansViewer>(entity.second);
    // 若行人视图指针不为空
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      const Base::Enums::PEDESTRIAN_TYPE pedestrianType =
          __lpsz2enum__(PEDESTRIAN_TYPE, _elemAttrViewPtr->type().c_str());
      // 跳过
      if (false && CallSucc(TrafficFlow::TAD_PedestrianElement::IsTransparentObject(pedestrianType))) {
        TX_MARK("vehicle trajectory follow");
        TrafficFlow::TAD_TrajFollow_VehicleElementPtr vehicle =
            std::make_shared<TrafficFlow::TAD_TrajFollow_VehicleElement>();
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->InitializePedestrian(_elemAttrViewPtr, _loader))) {
          _elemMgr->AddVehiclePtr(vehicle);
          TrafficFlow::TAD_SceneEvent_1_0_0_0_Ptr sceneEventPtr =
              std::dynamic_pointer_cast<TrafficFlow::TAD_SceneEvent_1_0_0_0>(vehicle);
          if (NonNull_Pointer(sceneEventPtr)) {
            if (CallSucc(sceneEventPtr->UseSceneEvent_1_0_0_0())) {
              LogInfo << "[vehicle_event][trajectory_follow][scene_event_1.0.0.0] vehicle_id:" << (*vehicle).Id()
                      << TX_VARS_NAME(sceneEventCnt, sceneEventPtr->SceneEvent().size());
              TrafficFlow::txEventDispatcher_v_1_0_0_0::getInstance().RegisterVehicleSceneEvent(
                  sceneEventPtr->SceneEvent(), sceneEventPtr);
            }
          } else {
            LogWarn << "un support scene event 1.0.0.0" << TX_VARS_NAME(vehId, (*vehicle).Id());
          }
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
      } else {
        TX_MARK("normal pedestrian");
        TrafficFlow::TAD_PedestrianElementPtr pedestrian = std::make_shared<TrafficFlow::TAD_PedestrianElement>();
        // 初始化成功
        if (NonNull_Pointer(pedestrian) && CallSucc(pedestrian->Initialize(entity.second, _loader))) {
          // 添加pedestrian到管理器中
          _elemMgr->AddPedestrianPtr(pedestrian);
          // LOG(INFO) << "[pedestrian]" << (*pedestrian); txPAUSE;
          auto tad_pede_viewptr = pedestrian->getPedestriansViewPtr();
          // 若行人视图指针存在
          if (tad_pede_viewptr) {
            if (tad_pede_viewptr->hadDirection()) {
              TX_MARK("direction1 direction1_t direction1_v ");
              LOG(FATAL) << "deprecated pedestrian scene.";
            } else {
              // 注册到event Dispatch实例中去
              txEventDispatcher::getInstance().RegisterPedestrianTimeVelocityEvent(
                  pedestrian->getPedestriansTimeVelocityViewPtr(), pedestrian);
              txEventDispatcher::getInstance().RegisterPedestrianEventVelocityEvent(
                  pedestrian->getPedestriansEventVelocityViewPtr(), pedestrian);
              TrafficFlow::TAD_SceneEvent_1_0_0_0_Ptr sceneEventPtr =
                  std::dynamic_pointer_cast<TrafficFlow::TAD_SceneEvent_1_0_0_0>(pedestrian);
              // 如果 sceneEventPtr 不为空，且 UseSceneEvent_1_0_0_0() 和 HasSceneEvent() 调用成功
              if (NonNull_Pointer(sceneEventPtr) && CallSucc(sceneEventPtr->UseSceneEvent_1_0_0_0()) &&
                  CallSucc(sceneEventPtr->HasSceneEvent())) {
                // 输出日志，表示 pedestrian_event 的 scene_event_1.0.0.0 已经使用
                LogInfo << "[pedestrian_event][scene_event_1.0.0.0] pede_id:" << (*pedestrian).Id()
                        << TX_VARS_NAME(sceneEventCnt, sceneEventPtr->SceneEvent().size());
                TrafficFlow::txEventDispatcher_v_1_0_0_0::getInstance().RegisterVehicleSceneEvent(
                    sceneEventPtr->SceneEvent(), sceneEventPtr);
              } else {
                // 如果 sceneEventPtr 为空，或者 UseSceneEvent_1_0_0_0() 或 HasSceneEvent() 调用失败，输出日志
                LogWarn << "[pedestrian_event][scene_event_1.0.0.0] pede_id:" << (*pedestrian).Id()
                        << " sceneEvent_1.0.0.0 is null.";
              }
            }
          }
          ++retCnt;
        } else {
          LogWarn << " Create Pedestrian Element Failure.";
        }
      }
    }
  }
  LogInfo << "[Create_Pedestrian]_Count = " << retCnt;
  return retCnt;
}

// 生成交通流中所有的静态障碍物
Base::txInt TAD_AssemblerContext::GenerateTrafficFlowElement_Obstacle(Base::IElementManagerPtr _elemMgr,
                                                                      Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;

  // 获取所有的静态障碍物数据
  auto res = _loader->GetAllObstacleData();
  for (auto& entity : res) {
    const Base::txString strBehavorType = entity.second->behavior();
    if (("RelativeTrajectoryFollow") == strBehavorType || ("AbsoluteTrajectoryFollow") == strBehavorType) {
      // 创建一个 TrafficFlow::TAD_RelativeTrajectoryFollowElement 对象
      TrafficFlow::TAD_RelativeTrajectoryFollowElementPtr relativeObs =
          std::make_shared<TrafficFlow::TAD_RelativeTrajectoryFollowElement>();
      // 如果 relativeObs 不为空，且 Initialize() 和 UseSceneEvent_1_0_0_0() 调用成功
      if (NonNull_Pointer(relativeObs) && CallSucc(relativeObs->Initialize(entity.second, _loader)) &&
          CallSucc(relativeObs->UseSceneEvent_1_0_0_0())) {
        // 将 relativeObs 添加到 _elemMgr
        _elemMgr->AddRelativeObsPtr(relativeObs);
        LogInfo << "[vehicle_event][scene_event_1.0.0.0] relative_id:" << (*relativeObs).Id()
                << TX_VARS_NAME(sceneEventCnt, relativeObs->SceneEvent().size());
        // 使用 TrafficFlow::txEventDispatcher_v_1_0_0_0 的单例注册 VehicleSceneEvent
        TrafficFlow::txEventDispatcher_v_1_0_0_0::getInstance().RegisterVehicleSceneEvent(relativeObs->SceneEvent(),
                                                                                          relativeObs);
        ++retCnt;
      }
    } else {
      /*Vertex = 6, VertexList = 7,*/
      const auto obs_type = __lpsz2enum__(STATIC_ELEMENT_TYPE, entity.second->type().c_str());
      if (_plus_(Base::Enums::STATIC_ELEMENT_TYPE::Vertex) == obs_type) {
        TrafficFlow::TAD_VertexElementPtr obstacle = std::make_shared<TrafficFlow::TAD_VertexElement>();
        // 若obstacle存在且成功初始化
        if (obstacle && obstacle->Initialize(entity.second, _loader)) {
          // 添加到元素管理器中
          _elemMgr->AddObstaclePtr(obstacle);
          ++retCnt;
        } else {
          LogWarn << ", Create Vertex Obstacle Element Failure.";
        }
      } else if (_plus_(Base::Enums::STATIC_ELEMENT_TYPE::VertexList) == obs_type) {
        const Base::txSysId vtx_id_base = entity.second->id();
        const Base::txString strVtxList = entity.second->behavior();
        const auto res = float_float_pair_parser(strVtxList);
        Base::txSysId curIdx = 0;
        for (const auto& pair : res) {
          TrafficFlow::TAD_VertexElementPtr obstacle = std::make_shared<TrafficFlow::TAD_VertexElement>();
          // 若obstacle存在且成功初始化
          if (obstacle && obstacle->Initialize(vtx_id_base + curIdx, pair.first, pair.second)) {
            // 添加到元素管理器中
            _elemMgr->AddObstaclePtr(obstacle);
            ++retCnt;
          } else {
            LogWarn << ", Create Vertex Obstacle Element Failure.";
          }
          ++curIdx;
        }
      } else {
        TrafficFlow::TAD_ObstacleElementPtr obstacle = std::make_shared<TrafficFlow::TAD_ObstacleElement>();
        // 若obstacle存在且成功初始化
        if (obstacle && obstacle->Initialize(entity.second, _loader)) {
          // 添加到元素管理器中
          _elemMgr->AddObstaclePtr(obstacle);
          ++retCnt;
        } else {
          LogWarn << ", Create Obstacle Element Failure.";
        }
      }
    }
  }

  if (FLAGS_USE_MAP_OBJECT) {
    const auto envelope = HdMap::HadmapCacheConCurrent::GetEnvelope();
    hadmap::txObjects objs;
    const std::vector<hadmap::OBJECT_TYPE> obj_types;
    auto res = hadmap::getObjects(txMapHdr, envelope, obj_types, objs);
    for (const auto objPtr : objs) {
      if (NonNull_Pointer(objPtr)) {
        TrafficFlow::TAD_MapObstacleElementPtr map_obs_ptr = std::make_shared<TrafficFlow::TAD_MapObstacleElement>();
        // 若map_obs_ptr存在且成功初始化
        if (NonNull_Pointer(map_obs_ptr) && CallSucc(map_obs_ptr->Initialize_txObject(objPtr))) {
          // 添加到元素管理器中
          _elemMgr->AddObstaclePtr(map_obs_ptr);
          ++retCnt;
        }

        LOG_IF(INFO,
               FLAGS_LogLevel_MapObject && (hadmap::OBJECT_TYPE::OBJECT_TYPE_TrafficSign == objPtr->getObjectType()))
            << TX_VARS(objPtr->getId()) << TX_VARS(objPtr->getName());
      }
    }
  } else {
    LOG(WARNING) << TX_COND(FLAGS_USE_MAP_OBJECT) << ", unsupport map objects.";
  }
  LogInfo << "[Create_Obstacle]_Count = " << retCnt;

  if (FLAGS_Enable_Sketch) {
    const auto envelope = HdMap::HadmapCacheConCurrent::GetEnvelope();
    hadmap::txObjects objs;
    std::vector<hadmap::OBJECT_TYPE> obj_types;
    // 添加地图元素类型
    obj_types.push_back(hadmap::OBJECT_TYPE::OBJECT_TYPE_TrafficSign);
    obj_types.push_back(hadmap::OBJECT_TYPE::OBJECT_TYPE_ParkingSpace);
    LOG(INFO) << TX_VARS(obj_types.size());
    auto res = hadmap::getObjects(txMapHdr, envelope, obj_types, objs);
    // 遍历各种地图元素
    for (const auto objPtr : objs) {
      // 元素存在且匹配元素类型
      // 对于trafficsign元素
      if (NonNull_Pointer(objPtr) && (hadmap::OBJECT_TYPE::OBJECT_TYPE_TrafficSign == objPtr->getObjectType())) {
        Coord::txWGS84 objPt;
        objPt.FromWGS84(objPtr->getPos());
        // 将 objPtr 插入到 RTree2D_Signal 中
        Geometry::SpatialQuery::RTree2D_Signal::getInstance().Insert_Sign(objPt.ToENU().ENU2D(), objPtr);
        // 如果 objPtr 的类型为 OBJECT_TYPE_TrafficSign，则输出日志
        LOG_IF(INFO, (hadmap::OBJECT_TYPE::OBJECT_TYPE_TrafficSign == objPtr->getObjectType()))
            << TX_VARS(objPtr->getId()) << TX_VARS(objPtr->getName());
      }
      // 对于parkingspace类型
      if (NonNull_Pointer(objPtr) && (hadmap::OBJECT_TYPE::OBJECT_TYPE_ParkingSpace == objPtr->getObjectType())) {
        // 将 objPtr 的位置从 WGS84 坐标系转换为 ENU 坐标系
        Coord::txWGS84 objPt;
        objPt.FromWGS84(objPtr->getPos());
        // 将 objPtr 插入到 RTree2D_Signal 中
        Geometry::SpatialQuery::RTree2D_Signal::getInstance().Insert_Park(objPt.ToENU().ENU2D(), objPtr);
        LOG_IF(INFO, (hadmap::OBJECT_TYPE::OBJECT_TYPE_ParkingSpace == objPtr->getObjectType()))
            << TX_VARS(objPtr->getId()) << TX_VARS(objPtr->getName());
      }
    }
  }
  return retCnt;
}

// 生成交通流中所有的环境元素
Base::txInt TAD_AssemblerContext::GenerateTrafficEnvironmentElement(Base::IElementManagerPtr _elemMgr,
                                                                    Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  /*Traffic Light Data*/
  Base::txInt retCnt = 0;
  // 获取所有信号灯数据
  auto res = _loader->GetAllSignlightData();
  // 获取当前活动计划
  const Base::txString activePlan = _loader->activePlan();
  LogWarn << "[signal_control_configure] " << TX_VARS(activePlan);
  // 遍历所有信号灯数据
  for (auto& tl : res) {
    // 如果信号灯数据不为空，且活动计划与信号灯数据的计划相同
    if (NonNull_Pointer(tl.second) && CallSucc(activePlan == tl.second->plan())) {
      // 创建一个 TrafficFlow::TAD_SignalLightElement 对象
      TrafficFlow::TAD_SignalLightElementPtr signal_light = std::make_shared<TrafficFlow::TAD_SignalLightElement>();
      // 若signlight元素存在且初始化
      if (NonNull_Pointer(signal_light) && signal_light->Initialize(tl.second, _loader)) {
        // 添加到元素管理器
        _elemMgr->AddSignalPtr(signal_light);
#if __TX_Mark__("cloud search circle signal")
        // 将 signal_light 插入到 RTree2D_Signal 中
        Geometry::SpatialQuery::RTree2D_Signal::getInstance().Insert_Signal(signal_light->GetLocation().ToENU().ENU2D(),
                                                                            signal_light);
        LogInfo << TX_VARS_NAME(signalId, (signal_light->Id())) << TX_VARS_NAME(signalPos, signal_light->GetLocation());
#endif /*USE_RTree*/
        ++retCnt;
      } else {
        LogWarn << " Create Signal Light Element Failure.";
      }
    } else {
      LogWarn << "[signal_control_configure] ignore signal id : " << (tl.second->id()) << TX_VARS(activePlan)
              << TX_VARS(tl.second->plan());
    }
  }
  Base::ISignalLightElement::RoadId2SignalIdSet roadId_with_signal;
  // 获取所有信号灯元素
  auto valid_signals = _elemMgr->GetAllSignal();
  // 遍历所有信号灯元素
  for (auto signalPtr : valid_signals) {
    // 将 signalPtr 转换为 TrafficFlow::TAD_SignalLightElement 类型
    auto tad_signal_ptr = std::dynamic_pointer_cast<TrafficFlow::TAD_SignalLightElement>(signalPtr);
    // 如果 tad_signal_ptr 不为空
    if (NonNull_Pointer(tad_signal_ptr)) {
      // 将信号灯的 ID 插入到 roadId_with_signal 中，以 roadId 为键
      roadId_with_signal[tad_signal_ptr->GetCurrentLaneInfo().onLaneUid.roadId].insert(tad_signal_ptr->Id());
    } else {
      // 如果转换失败，输出日志
      LogWarn << " Create Signal Light Element Failure.";
    }
  }
  // 遍历 roadId_with_signal
  for (const auto& ref_pair : roadId_with_signal) {
    std::ostringstream oss;
    // 将 ref_pair.second 中的信号灯 ID 复制到 oss 中，以逗号分隔
    std::copy(ref_pair.second.begin(), ref_pair.second.end(), std::ostream_iterator<Base::txSysId>(oss, ","));
    LogInfo << "[TAD_Signal_Road]" << TX_VARS_NAME(RoadId, ref_pair.first) << TX_VARS_NAME(SignalIds, oss.str());
  }

  // 将 ref_pair.second 中的信号灯 ID 复制到 oss 中，以逗号分隔
  for (auto signalPtr : valid_signals) {
    // 将 signalPtr 转换为 TrafficFlow::TAD_SignalLightElement 类型
    auto tad_signal_ptr = std::dynamic_pointer_cast<TrafficFlow::TAD_SignalLightElement>(signalPtr);
    // 如果 tad_signal_ptr 不为空
    if (NonNull_Pointer(tad_signal_ptr)) {
      // 生成信号控制车道信息列表
      if (CallFail(tad_signal_ptr->GenerateSignalControlLaneInfoList(roadId_with_signal))) {
        LogWarn << "[TAD_Signal_Road] " << TX_VARS_NAME(SignalId, tad_signal_ptr->Id()) << " initialize failure.";
      } else {
        // 检查信号灯的开始时间
        tad_signal_ptr->CheckStart(Base::TimeParamManager(0, 0, 0, 0));
        LogInfo << "[Signal]" << TX_VARS_NAME(SignalId, tad_signal_ptr->Id()) << " create success.";
      }
    } else {
      LogWarn << " Create Signal Light Element Failure.";
    }
  }
  return retCnt;
}

Base::txInt TAD_AssemblerContext::GenerateTrafficReferenceElement(Base::IElementManagerPtr,
                                                                  Base::ISceneLoaderPtr) TX_NOEXCEPT {
  return 0;
}

Base::txBool TAD_AssemblerContext::Init(Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  // 检查场景加载器是否为空
  if (NonNull_Pointer(_loader)) {
    LOG(INFO) << "TAD_AssemblerContext::Init";

    // 获取车流观察者指针
    Base::ISceneLoader::ITrafficFlowViewerPtr tf_ptr = _loader->GetTrafficFlow();

    // 检查车流观察者指针是否为空
    if (NonNull_Pointer(tf_ptr)) {
      // 初始化交通元素生成器智能指针
      m_TAD_VehicleElementGenerator = std::make_shared<Scene::TAD_ElementGenerator>();

      // 检查交通元素生成器智能指针是否为空
      if (NonNull_Pointer(m_TAD_VehicleElementGenerator)) {
        // 初始化交通元素生成器
        m_TAD_VehicleElementGenerator->Initialize(_loader, Base::map_range_t());
        return true;
      } else {
        return false;
      }
    } else {
      LOG(WARNING) << "TAD_AssemblerContext::Init, ITrafficFlowViewerPtr is null.";
      return false;
    }
  } else {
    LOG(WARNING) << "TAD_AssemblerContext::Init, _loader is null.";
    return false;
  }
}

Base::txBool TAD_AssemblerContext::DynamicGenerateScene(const Base::TimeParamManager& timeMgr,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::ISceneLoaderPtr) TX_NOEXCEPT {
  // 使用 NonNull_Pointer 宏来检查 m_TAD_VehicleElementGenerator 指针是否为空
  if (NonNull_Pointer(m_TAD_VehicleElementGenerator)) {
    // 调用 m_TAD_VehicleElementGenerator 的 Generate 方法来动态生成场景
    return m_TAD_VehicleElementGenerator->Generate(timeMgr, _elemMgr);
  }
  return true;
}

Base::txBool TAD_AssemblerContext::DynamicChangeScene(const Base::TimeParamManager& timeMgr,
                                                      Base::IElementManagerPtr _elemMgr,
                                                      Base::ISceneLoaderPtr) TX_NOEXCEPT {
  // 当 m_TAD_VehicleElementGenerator 和 _elemMgr 都不为空时，尝试删除和重定向元素
  if (NonNull_Pointer(m_TAD_VehicleElementGenerator) && NonNull_Pointer(_elemMgr)) {
    Base::txBool bRet = true;
    bRet = bRet && (m_TAD_VehicleElementGenerator->Erase(timeMgr, _elemMgr));
    bRet = bRet && (m_TAD_VehicleElementGenerator->ReRoute(timeMgr, _elemMgr));
    return bRet;
  }
  return true;
}

Base::txBool TAD_AssemblerContext::ResetContext() TX_NOEXCEPT {
  // 如果 m_TAD_VehicleElementGenerator 不为空指针
  if (NonNull_Pointer(m_TAD_VehicleElementGenerator)) {
    // 调用 m_TAD_VehicleElementGenerator 的 ResetGenerator 方法，并返回其结果
    return m_TAD_VehicleElementGenerator->ResetGenerator();
  } else {
    LOG(WARNING) << "ResetContext failure. Generator is nullptr.";
    return false;
  }
}

Base::txBool TAD_AssemblerContext::TestSerialization(const Base::TimeParamManager& timeMgr,
                                                     Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(_elemMgr)) {
#if 0
    using sysId2Serialization = tbb::concurrent_hash_map<Base::txSysId, its::txVehicle>;
    using OutputArchive = cereal::BinaryOutputArchive;
    using InputArchive = cereal::BinaryInputArchive;
    sysId2Serialization map_sysId2String;
    auto& refAllVehicle = _elemMgr->GetAllVehiclePtr();
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicle.size(),
      [&](const std::size_t idx) {
      TrafficFlow::TAD_AI_VehicleElementPtr ai_veh_ptr =
                                    std::dynamic_pointer_cast<TrafficFlow::TAD_AI_VehicleElement>(refAllVehicle[idx]);
      if (NonNull_Pointer(ai_veh_ptr)) {
        {
          std::stringstream oss;
          oss << "D:/log/serialization/" << (ai_veh_ptr->SysId()) << "_" << timeMgr.AbsTime() << ".json";
          std::ofstream os(oss.str());
          os << ai_veh_ptr->json_archive();
          os.close();
        }

        sysId2Serialization::accessor a;
        map_sysId2String.insert(a, ai_veh_ptr->SysId());
        a->second.set_serialization_archive(ai_veh_ptr->binary_archive());
        // *(a->second.mutable_serialization_archive()) = ss.str();
      }
    }); /*lamda function*/
        /* parallel_for */

    for (const auto& ref : map_sysId2String) {
      LOG(INFO) << TX_VARS_NAME(SysId, ref.first) << TX_VARS_NAME(size, ref.second.serialization_archive().size());
    }

    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicle.size(),
      [&](const std::size_t idx) {
      TrafficFlow::TAD_AI_VehicleElementPtr ai_veh_ptr =
                    std::dynamic_pointer_cast<TrafficFlow::TAD_AI_VehicleElement>(refAllVehicle[idx]);
      if (NonNull_Pointer(ai_veh_ptr)) {
        sysId2Serialization::const_accessor ca;
        if (CallSucc(map_sysId2String.find(ca, ai_veh_ptr->SysId()))) {
          std::stringstream ss(ca->second.serialization_archive());
          InputArchive arin(ss);
          arin(*ai_veh_ptr);
        }
      }
    }); /*lamda function*/
        /* parallel_for */
#endif
    return true;
  } else {
    return false;
  }
}

TX_NAMESPACE_CLOSE(Scene)
