// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_element_generator.h"
#include "tad_ai_vehicle_element.h"
#include "tx_id_manager.h"
#include "tx_parallel_def.h"
#include "tx_probability_generator.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
TX_NAMESPACE_OPEN(Scene)

// 元素生成器的初始化函数
Base::txBool TAD_ElementGenerator::Initialize(Base::ISceneLoaderPtr _loader,
                                              const Base::map_range_t& valid_map_range) TX_NOEXCEPT {
  // 判断loader不为空
  if (NonNull_Pointer(_loader)) {
    /*step 0 create valid map range.*/
    /*step 1 init location*/
    LocationAgent::ClearLocationAgent();
    std::unordered_map<Base::txInt, Base::ISceneLoader::ILocationViewerPtr> retLocationViewPtr =
        _loader->GetAllLocationData();
    // 遍历 retLocationViewPtr，为每个有效的位置创建一个 LocationAgent 对象，并将其添加到 LocationAgent 管理器中
    for (const auto& ref_Id_location : retLocationViewPtr) {
      // 取KV对中的键值对
      const Base::txInt locId = ref_Id_location.first;
      const Base::ISceneLoader::ILocationViewerPtr locPtr = ref_Id_location.second;
      // 若V存在
      if (locPtr) {
        LocationAgent::LocationAgentPtr locAgentPtr = std::make_shared<LocationAgent>();
        // 初始化LocationAgent并添加到 LocationAgent 管理器中
        locAgentPtr->Init(locPtr);
        LocationAgent::AddLocationAgent(locAgentPtr);
      }
    }

    /*step 2 init route group*/
#if USE_TBB
    mRouteGroupVec.clear();
    std::unordered_map<Base::txInt, Base::ISceneLoader::ITrafficFlowViewer::RouteGroupPtr> retRouteGroup =
        _loader->GetTrafficFlow()->GetAllRouteGroupData();
    std::vector<Base::ISceneLoader::ITrafficFlowViewer::RouteGroupPtr> vec_route_group;
    for (const auto& refRoute : retRouteGroup) {
      vec_route_group.push_back(refRoute.second);
    }
    const Base::txSize nRouteGroupCnt = vec_route_group.size();

    // 创建一个 TAD_RouteGroupAgent 对象的向量，用于存储路线组代理
    std::vector<TAD_RouteGroupAgent> vec_route_group_agent(nRouteGroupCnt);

    // 使用 TBB 并行库初始化路线组代理
    {
      tbb::parallel_for(static_cast<std::size_t>(0), nRouteGroupCnt, [&](const std::size_t idx) {
        vec_route_group_agent[idx].Init(vec_route_group[idx]);
      }); /*lamda function*/
          /* parallel_for */
    }

    // 将有效的路线组代理添加到 mRouteGroupVec 中
    for (Base::txInt idx = 0; idx < vec_route_group_agent.size(); ++idx) {
      const auto& refRouteAgent = vec_route_group_agent[idx];
      const auto rgId = vec_route_group[idx]->id();
      if (refRouteAgent.IsValid()) {
        mRouteGroupVec.emplace_back(refRouteAgent);
      } else {
        LOG(WARNING) << " Route Group id = " << rgId << " init failure.";
      }
    }
#endif /*USE_TBB*/

    /*step 3 init vehicle input*/
    std::unordered_map<Base::txInt, Base::ISceneLoader::ITrafficFlowViewer::VehInputPtr> retVehInputs =
        _loader->GetTrafficFlow()->GetAllVehInputData();
    // 遍历 retVehInputs，为每个有效的车辆输入创建一个 VehicleInputeAgent 对象，并将其添加到 mVehicleInputeVec 中
    for (const auto& refVehInput : retVehInputs) {
      const Base::txInt locId = refVehInput.second->location();
      LocationAgent::LocationAgentPtr curLocPtr = LocationAgent::GetLocationAgentById(locId);

      // 若当前位置存在
      if (NonNull_Pointer(curLocPtr)) {
        const Base::txInt viId = refVehInput.first;
        // 创建一个 VehicleInputeAgent 对象并初始化添加
        mVehicleInputeVec.emplace_back(VehicleInputeAgent());
        if (CallFail(mVehicleInputeVec.back().Init(_loader, refVehInput.second))) {
          LOG(WARNING) << " Vehicle Inpute id = " << viId << " init failure.";
          LOG(WARNING) << "[ERROR] outter input region : " << (curLocPtr->vPos());
          mVehicleInputeVec.erase(mVehicleInputeVec.end() - 1);
        } else {
          LOG(WARNING) << "[SUCCE] outter input region : " << (curLocPtr->vPos());
        }
      } else {
        LOG(WARNING) << "[ERROR] outter input region : " << (curLocPtr->vPos());
      }
    }

    // 获取vehInputId集合
    std::set<Base::txSysId> setInputId;
    // 遍历添加
    for (const auto& refInputAgent : mVehicleInputeVec) {
      setInputId.insert(refInputAgent.VehInputId());
    }
    Base::txIdManager::RegisterInputRegion(setInputId);

    /*step 4 init vehicle exit*/
    std::unordered_map<Base::txInt, Base::ISceneLoader::ITrafficFlowViewer::VehExitPtr> retVehExits =
        _loader->GetTrafficFlow()->GetAllVehExitData();
    for (const auto& refVehExit : retVehExits) {
      // 获取车辆出口的位置 ID
      const Base::txInt locId = refVehExit.second->location();
      // 根据位置 ID 获取位置指针
      LocationAgent::LocationAgentPtr curLocPtr = LocationAgent::GetLocationAgentById(locId);
      // 判断位置指针是否有效
      if (NonNull_Pointer(curLocPtr)) {
        // 获取车辆出口的 ID
        const Base::txInt veId = refVehExit.first;
        mVehicleExitVec.emplace_back(VehicleExitAgent());
        // 初始化 VehicleExitAgent 对象
        if (CallFail(
                mVehicleExitVec.back().Initialize(veId, static_cast<Coord::txWGS84>((curLocPtr->vPos()).WGS84())))) {
          // 如果初始化失败，输出警告信息
          LOG(WARNING) << " Vehicle Exit id = " << veId << " init failure.";
          LOG(WARNING) << "[ERROR] outter exit region : " << (curLocPtr->vPos());
          mVehicleExitVec.erase(mVehicleExitVec.end() - 1);
        } else {
          // 如果初始化成功，输出成功信息
          LOG(WARNING) << "[SUCCE] outter exit region : " << (curLocPtr->vPos());
        }
      } else {
        LOG(WARNING) << "[ERROR] outter exit region : " << (curLocPtr->vPos());
      }
    }
    return true;
  } else {
    return false;
  }
}

Base::txBool TAD_ElementGenerator::SafeInputRegion(const Base::txVec2& inputPt,
                                                   const Base::txVec2& nearestPt) const TX_NOEXCEPT {
  // 如果输入点到最近路口或路线组的距离大于安全区域半径，则返回 true
  if ((inputPt - nearestPt).norm() > FLAGS_vehicle_input_safe_region_radius) {
    return true;
  } else {
    return false;
  }
}

// 场景允许的最大车辆数
Base::txSize TAD_ElementGenerator::SceneMaxVehicleSize() const TX_NOEXCEPT {
  if (Utils::VehicleBehaviorCfg::SceneMaxVehicleSize() <= 0) {
    return FLAGS_MAX_VEHICLE_LIMIT_COUNT;
  }
  return Utils::VehicleBehaviorCfg::SceneMaxVehicleSize();
}

Base::txBool TAD_ElementGenerator::Generate(Base::TimeParamManager const& timeMgr,
                                            Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  static const std::set<Base::ITrafficElement::ElementType> type_filter_set{
      Base::ITrafficElement::ElementType::TAD_Vehicle, Base::ITrafficElement::ElementType::TAD_Ego};
  const Base::txInt nVehicleSize = _elemMgr->GetVehicleCount();
  static Base::txInt sFrameCnt = 0;
  static Base::txInt sCallCnt = 0;
  static const Base::txSize nMaxVehicleSize = SceneMaxVehicleSize();
  LOG_IF(INFO, (((sFrameCnt++) > 100) ? (sFrameCnt = 0, true) : (false)))
      << "**** " << TX_VARS_NAME(System_Element_Size, nVehicleSize) << " ****" << TX_VARS(nMaxVehicleSize);
  if (nVehicleSize < nMaxVehicleSize) {
#if USE_TBB
    tbb::parallel_for(static_cast<std::size_t>(0), mVehicleInputeVec.size(), [&](const std::size_t idx) {
      VehicleInputeAgent& refInputAgent = mVehicleInputeVec[idx];
      // 如果输入代理有效
      if (refInputAgent.IsValid()) {
        // 如果输入代理的持续时间有效
        if (refInputAgent.IsValidDuration()) {
          // 更新输入代理的周期
          if (refInputAgent.UpdatePeriod(timeMgr.RelativeTime())) {
            // 获取下一个概率车辆的初始化参数
            VehicleInputeAgent::VehicleInitParam_t vehicleInitParam;
            Base::txLaneID vehicleStartLaneId;
            Geometry::SpatialQuery::HashedLaneInfo hashedNode;
            Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr node_ptr = nullptr;
            std::tie(vehicleInitParam, vehicleStartLaneId, hashedNode, node_ptr) =
                refInputAgent.GetNextProbabilityVehicleInitParam();
            TODO("generator lane-wise");
            Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::txSurroundVehiclefo front_nearest_element_info;
            // 搜索最近的前方元素
            Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::SearchNearestFrontElement(
                -1, -1, 0.0, hashedNode, node_ptr, front_nearest_element_info,
                FLAGS_vehicle_input_scan_vision_distance);
            const Base::txVec2 startPtLoc = vehicleInitParam.mLocationPtr->vPos().ToENU().ENU2D();
            // 获取最近的车辆指针
            Base::SimulationConsistencyPtr front_veh_ptr = Weak2SharedPtr(
                std::get<_ElementIdx_>(front_nearest_element_info));  // (CallFail(front_veh_weak_ptr.expired())) ?
                                                                      // (front_veh_weak_ptr.lock()) : (nullptr);

            // 如果最近的车辆指针为空或者起始点与最近车辆的距离大于安全区域半径
            if (Null_Pointer(front_veh_ptr) ||
                (NonNull_Pointer(front_veh_ptr) &&
                 SafeInputRegion(startPtLoc, front_veh_ptr->StableGeomCenter().ENU2D()))) {
              // 如果最近的车辆指针不为空
              if (NonNull_Pointer(front_veh_ptr)) {
                // 设置起始速度为最近车辆的速度
                vehicleInitParam.mForceStartV = front_veh_ptr->StableVelocity();
                LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
                    << "[vehicle_input_debug]" << TX_VARS_NAME(InputeRegionId, refInputAgent.VehInputId())
                    << TX_VARS_NAME(front_veh_id, front_veh_ptr->ConsistencyId())
                    << TX_VARS_NAME(ForceStartV, *(vehicleInitParam.mForceStartV));
              } else {
                // 如果最近的车辆指针为空，清除起始速度
                vehicleInitParam.mForceStartV.clear_value();
                LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
                    << "[vehicle_input_debug]" << TX_VARS_NAME(InputeRegionId, refInputAgent.VehInputId())
                    << " front is null.";
              }

              // 创建新的车辆元素
              TrafficFlow::TAD_AI_VehicleElementPtr vehicle = std::make_shared<TrafficFlow::TAD_AI_VehicleElement>();
              // 如果车辆元素指针有效且初始化成功
              if (NonNull_Pointer(vehicle) &&
                  vehicle->Initialize(Base::txIdManager::GenIdPerInput(refInputAgent.VehInputId()), vehicleInitParam,
                                      vehicleStartLaneId, _elemMgr->GetDataSource())) {
                /*LOG(WARNING) << "create vehicle element success. " << TX_VARS(vehicle->Id())
                    << TX_VARS_NAME(input_id, refVehInput.first);*/
                // 将车辆元素添加到元素管理器
                _elemMgr->AddVehiclePtr(vehicle);
              } else {
                LOG(WARNING) << " Create Vehicle Element Failure.";
              }
            } else {
              // refVehInput.second.ResetPeriod();
            }
          }
        } else {
          LOG_IF(INFO, 0 == sFrameCnt) << "vehicle input duration isvalid. " << TX_VARS(refInputAgent.VehInputId())
                                       << TX_VARS(refInputAgent.CurDuration()) << TX_VARS(refInputAgent.RawDuration());
        }
      } else {
        LOG(WARNING) << "refInputAgent.IsValid() = FALSE, " << TX_VARS(refInputAgent.VehInputId());
      }
    }); /*lamda function*/
        /* parallel_for */
    _elemMgr->GenerateAllTrafficElement();
#else  /*USE_TBB*/
    for (auto& refVehInput : mVehicleInputeMap) {
      if (refVehInput.second.IsValid()) {
        if (refVehInput.second.UpdatePeriod(timeMgr.RelativeTime())) {
          VehicleInputeAgent::VehicleInitParam_t vehicleInitParam;
          Base::txLaneID vehicleStartLaneId;
          Geometry::SpatialQuery::HashedLaneInfo hashedNode;
          Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr node_ptr = nullptr;
          std::tie(vehicleInitParam, vehicleStartLaneId, hashedNode, node_ptr) =
              refVehInput.second.GetNextProbabilityVehicleInitParam();
          TODO("generator lane-wise");
          Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::txSurroundVehiclefo front_nearest_element_info;
          Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::SearchNearestFrontElement(
              -1, -1, 0.0, hashedNode, node_ptr, front_nearest_element_info, FLAGS_vehicle_input_scan_vision_distance);
          const Base::txVec2 startPtLoc = vehicleInitParam.mLocationPtr->vPos().ToENU().ENU2D();
          Base::IVehicleElementPtr front_veh_ptr = std::get<_ElementIdx_>(front_nearest_element_info);

          if (Null_Pointer(front_veh_ptr) || (NonNull_Pointer(front_veh_ptr) &&
                                              SafeInputRegion(startPtLoc, front_veh_ptr->StablePosition().ENU2D()))) {
            if (NonNull_Pointer(front_veh_ptr)) {
              vehicleInitParam.mStartV = front_veh_ptr->StableVelocity();
              LOG_IF(INFO, FLAGS_LogLevel_Kinetics) << TX_VARS_NAME(InputeRegionId, refVehInput.second.VehInputId())
                                                    << TX_VARS_NAME(front_veh_id, front_veh_ptr->Id())
                                                    << TX_VARS_NAME(ResetMaxV, vehicleInitParam.mStartV);
            }

            TrafficFlow::TAD_AI_VehicleElementPtr vehicle = std::make_shared<TrafficFlow::TAD_AI_VehicleElement>();
            if (NonNull_Pointer(vehicle) &&
                vehicle->Initialize(Base::txIdManager::GenIdPerInput(refVehInput.first), vehicleInitParam,
                                    vehicleStartLaneId, _elemMgr->GetDataSource())) {
              /*LOG(WARNING) << "create vehicle element success. " << TX_VARS(vehicle->Id())
                  << TX_VARS_NAME(input_id, refVehInput.first);*/
              _elemMgr->AddVehiclePtr(vehicle);
            } else {
              LOG(WARNING) << " Create Vehicle Element Failure.";
            }
          } else {
            // refVehInput.second.ResetPeriod();
          }
        }
        /*else {
            LOG(WARNING) << "refVehInput.second.UpdatePeriod(timeMgr.RelativeTime()) false.";
        }*/
      } else {
        LOG(WARNING) << "refVehInput.second.IsValid() = FALSE";
      }
    }
    _elemMgr->GenerateAllTrafficElement();
#endif /*USE_TBB*/
  } else if ((((sCallCnt++) > 50) ? (sCallCnt = 0, true) : (false))) {
    // 如果调用次数大于 50，则重置调用次数并输出警告信息
    LOG(WARNING) << " Vehicle fully loaded. " << TX_VARS(nVehicleSize) << TX_VARS(nMaxVehicleSize);
    // 对元素管理器中的车辆进行排序和清理
    _elemMgr->ResortKillElement();
    LOG(WARNING) << " ReSortKillVehicle. " << TX_VARS_NAME(nAliveVehicleSize, (_elemMgr->GetVehicleCount()))
                 << TX_VARS(nMaxVehicleSize);
  }
  return true;
}

Base::txBool TAD_ElementGenerator::Erase(Base::TimeParamManager const& timeMgr,
                                         Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  Base::txBool needReGenerate = false;
  static std::set<Base::ITrafficElement::ElementType> type_filter_set{Base::ITrafficElement::ElementType::TAD_Vehicle};
  static Base::txInt sCallCnt = 0;
#if USE_TBB
  // 使用 tbb::parallel_for 并行遍历 mVehicleExitVec
  tbb::parallel_for(static_cast<std::size_t>(0), mVehicleExitVec.size(), [&](const std::size_t idx) {
    VehicleExitAgent& refExitArea = mVehicleExitVec[idx];
    // 定义一个用于存储查询结果的车辆容器
    Geometry::SpatialQuery::txHashedRoadArea::VehicleContainer resultElements;
    refExitArea.QueryVehicles(resultElements);
    // 遍历查询结果中的车辆
    for (const auto delete_element_ptr : resultElements) {
      delete_element_ptr.second->Kill();
    }
  }); /*lamda function*/
      /* parallel_for */
#else
  for (auto& refExitArea : mVehicleExitVec) {
    Geometry::SpatialQuery::txHashedRoadArea::VehicleContainer resultElements;
    refExitArea.QueryVehicles(resultElements);
    for (const auto delete_element_ptr : resultElements) {
      {
        delete_element_ptr.second->Kill();
      }
    }
  }

#endif /*USE_TBB*/
  if ((((sCallCnt++) > 50) ? (sCallCnt = 0, true) : (false))) {
    // 对元素管理器中的车辆进行排序和清理
    _elemMgr->ResortKillElement();
  }
  return true;
}

Base::txBool TAD_ElementGenerator::ReRoute(Base::TimeParamManager const&,
                                           Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  std::set<Base::ITrafficElement::ElementType> type_filter_set;
  type_filter_set.insert(Base::ITrafficElement::ElementType::TAD_Vehicle);

  // 如果使用 TBB 并行库
#if USE_TBB
  // 使用 tbb::parallel_for 并行遍历 mRouteGroupVec
  tbb::parallel_for(static_cast<std::size_t>(0), mRouteGroupVec.size(), [&](const std::size_t idx) {
    // 获取当前路线组代理
    TAD_RouteGroupAgent& refRouteGroupAgent = mRouteGroupVec[idx];
    Geometry::SpatialQuery::txHashedRoadArea::VehicleContainer resultElements;
    // 查询车辆
    refRouteGroupAgent.QueryVehicles(resultElements);
    // 遍历查询结果中的车辆
    for (const auto pair_id_re_route_element_ptr : resultElements) {
      const auto re_route_element_ptr = pair_id_re_route_element_ptr.second;
      // 将车辆元素指针转换为 TAD_AI_VehicleElement 类型的智能指针
      TrafficFlow::TAD_AI_VehicleElementPtr vehicle =
          std::dynamic_pointer_cast<TrafficFlow::TAD_AI_VehicleElement>(re_route_element_ptr);
      // 如果车辆元素指针有效且车辆行为为 TAD_AI 或 TAD_AI_Arterial
      if (NonNull_Pointer(vehicle) &&
          (((_plus_(Base::ISceneLoader::VEHICLE_BEHAVIOR::eTadAI)) == vehicle->VehicleBehavior()) ||
           ((_plus_(Base::ISceneLoader::VEHICLE_BEHAVIOR::eTadAI_Arterial)) == vehicle->VehicleBehavior()))) {
        const Base::txLaneUId& laneUid = re_route_element_ptr->GetCurrentLaneInfo().onLaneUid;
        // 如果车辆支持重新路由且当前路线组 ID 与车辆的当前路线组 ID 不同
        if (CallSucc(vehicle->IsSupportReRoute()) &&
            (refRouteGroupAgent.RouteGroupId() != vehicle->GetCurRouteGroupId())) {
          LOG_IF(WARNING, FLAGS_LogLevel_Routing) << "ArriveAtStartLineArea id = " << (re_route_element_ptr->Id())
                                                  << " re-route it." << TX_VARS(refRouteGroupAgent.RouteGroupId());
          // 获取新路线信息
          TrafficFlow::Component::RouteAI newRoute;
          Base::txInt subRouteId = 2;
          if (8002 == refRouteGroupAgent.RouteGroupId()) {
            subRouteId = laneUid.laneId * -1 - 1;
            newRoute = std::get<2>(refRouteGroupAgent.GetSpecialRoute(subRouteId));
          } else {
            subRouteId = 2;
            newRoute = std::get<2>(refRouteGroupAgent.GetNextProbabilityRoute(subRouteId));
          }
          /*Base::txInt subRouteId = 2;
          const auto& route = ReRouteArea.second.GetNextProbabilityRoute(subRouteId);*/
          vehicle->ReRoute((refRouteGroupAgent.RouteGroupId()), subRouteId, newRoute);
        }
      }
    }
  });  /*lamda function*/
       /* parallel_for */
#endif /*USE_TBB*/

  return true;
}

// 释放资源
Base::txBool TAD_ElementGenerator::Release() TX_NOEXCEPT {
  mRouteGroupVec.clear();
  mVehicleInputeVec.clear();
  mVehicleExitVec.clear();
  ParentClass::Release();
  return true;
}

Base::txBool TAD_ElementGenerator::ResetGenerator() TX_NOEXCEPT {
  // 遍历路线组集合
  for (auto& refRouteGroupAgent : mRouteGroupVec) {
    // 重置路线组代理
    refRouteGroupAgent.ResetRoute();
  }

  // 遍历车辆输入集合
  for (auto& refInputAgent : mVehicleInputeVec) {
    refInputAgent.ResetInputAgent();
  }
  // 调用父类的 ResetGenerator 方法
  return ParentClass::ResetGenerator();
}

std::vector<Coord::txWGS84> TAD_ElementGenerator::InputRegionLocations() const TX_NOEXCEPT {
  std::vector<Coord::txWGS84> res;
  // 遍历车辆输入集合
  for (const auto& refVehInput : mVehicleInputeVec) {
    if (CallSucc(refVehInput.IsActive()) && NonNull_Pointer(refVehInput.RegionLocationPtr())) {
      res.push_back(refVehInput.RegionLocationPtr()->vPos());
    }
  }
  return res;
}

TX_NAMESPACE_CLOSE(Scene)
