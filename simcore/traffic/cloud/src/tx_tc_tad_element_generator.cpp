// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_tad_element_generator.h"
#include <random>
#include "tad_ai_vehicle_element.h"
#include "tx_id_manager.h"
#include "tx_spatial_query.h"
#include "tx_tc_probability_generator.h"
#include "tx_tc_tad_vehicle_ai_element.h"
#if USE_TBB
#  include <tbb/tbb.h>
#endif
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_component.h"
#include "tx_flags.h"

#define VehicleInputInfo LOG_IF(INFO, FLAGS_LogLevel_Vehicle_Input)
#define VehicleInputWarn LOG_IF(WARNING, FLAGS_LogLevel_Vehicle_Input)
TX_NAMESPACE_OPEN(Scene)

Base::txSize TAD_Cloud_ElementGenerator::SceneMaxVehicleSize() const TX_NOEXCEPT {
  if (Utils::VehicleBehaviorCfg::SceneMaxVehicleSize() <= 0) {
    return FLAGS_MAX_VEHICLE_LIMIT_COUNT;
  }
  return Utils::VehicleBehaviorCfg::SceneMaxVehicleSize();
}

Base::txBool InRange(const Base::txVec2& enu_bottom_left, const Base::txVec2& enu_top_right,
                     const Base::txVec2& pt) TX_NOEXCEPT {
  if (enu_bottom_left.x() < enu_top_right.x() && enu_bottom_left.y() < enu_top_right.y()) {
    return (enu_bottom_left.x() < pt.x()) && (pt.x() < enu_top_right.x()) && (enu_bottom_left.y() < pt.y()) &&
           (pt.y() < enu_top_right.y());
  } else if (enu_bottom_left.x() > enu_top_right.x() && enu_bottom_left.y() > enu_top_right.y()) {
    return (enu_bottom_left.x() > pt.x()) && (pt.x() > enu_top_right.x()) && (enu_bottom_left.y() > pt.y()) &&
           (pt.y() > enu_top_right.y());
  }
  return false;
}

Base::txBool TAD_Cloud_ElementGenerator::Initialize(Base::ISceneLoaderPtr _loader,
                                                    const Base::map_range_t& valid_map_range) TX_NOEXCEPT {
  if (_loader) {
    /*step 0 create valid map range.*/
    Coord::txWGS84 ws84_bottom_left(valid_map_range.bottom_left), ws84_top_right(valid_map_range.top_right);
    const Base::txVec2 enu_bottom_left = ws84_bottom_left.ToENU().ENU2D();
    const Base::txVec2 enu_top_right = ws84_top_right.ToENU().ENU2D();

    /*step 1 init location*/
    LocationAgent::ClearLocationAgent();
    std::unordered_map<Base::txInt, Base::ISceneLoader::ILocationViewerPtr> retLocationViewPtr =
        _loader->GetAllLocationData();
    for (const auto& ref_Id_location : retLocationViewPtr) {
      const Base::txInt locId = ref_Id_location.first;
      const Base::ISceneLoader::ILocationViewerPtr locPtr = ref_Id_location.second;
      if (locPtr) {
        LocationAgent::LocationAgentPtr locAgentPtr = std::make_shared<LocationAgent>();
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

    std::vector<TAD_RouteGroupAgent> vec_route_group_agent(nRouteGroupCnt);

    {
      tbb::parallel_for(static_cast<std::size_t>(0), nRouteGroupCnt, [&](const std::size_t idx) {
        vec_route_group_agent[idx].Init(vec_route_group[idx]);
      }); /*lamda function*/
          /* parallel_for */
    }

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
    for (const auto& refVehInput : retVehInputs) {
      const Base::txInt locId = refVehInput.second->location();
      LocationAgent::LocationAgentPtr curLocPtr = LocationAgent::GetLocationAgentById(locId);

      if (NonNull_Pointer(curLocPtr) &&
          CallSucc(InRange(enu_bottom_left, enu_top_right, curLocPtr->vPos().ToENU().ENU2D()))) {
        const Base::txInt viId = refVehInput.first;
        mVehicleInputeVec.emplace_back(VehicleInputeAgent());
        if (CallFail(mVehicleInputeVec.back().Init(_loader, refVehInput.second))) {
          LOG(WARNING) << " Vehicle Inpute id = " << viId << " init failure.";
          LOG(WARNING) << "[ERROR] outter input region : " << (curLocPtr->vPos());
          mVehicleInputeVec.erase(mVehicleInputeVec.end() - 1);
        } else {
          LOG(WARNING) << "[SUCCE] outter input region : " << TX_VARS_NAME(input_pt, (curLocPtr->vPos()))
                       << TX_VARS(ws84_bottom_left) << TX_VARS(ws84_top_right);
          if (CallSucc(InRange(enu_bottom_left, enu_top_right, curLocPtr->vPos().ToENU().ENU2D()))) {
            mVehicleInputeVec.back().SetActive(true);
            LOG(WARNING) << "[SUCCE][Active] input region : " << (viId) << " in map_range active!";
          } else {
            mVehicleInputeVec.back().SetActive(false);
            LOG(WARNING) << "[ERROR][Active] input region : " << (viId) << " not in map_range in-active!"
                         << TX_VARS_NAME(enu_bottom_left, Utils::ToString(enu_bottom_left))
                         << TX_VARS_NAME(enu_top_right, Utils::ToString(enu_top_right))
                         << TX_VARS_NAME(veh_input_pt, Utils::ToString(curLocPtr->vPos().ToENU().ENU2D()));
          }
        }
      } else {
        LOG(WARNING) << "[ERROR][Active]" << TX_VARS(locId)
                     << TX_VARS_NAME(enu_bottom_left, Utils::ToString(enu_bottom_left))
                     << TX_VARS_NAME(enu_top_right, Utils::ToString(enu_top_right));
      }
    }

    std::set<Base::txSysId> setInputId;
    for (const auto& refInputAgent : mVehicleInputeVec) {
      setInputId.insert(refInputAgent.VehInputId());
    }
    Base::txIdManager::RegisterInputRegion(setInputId);

    /*step 4 init vehicle exit*/
    std::unordered_map<Base::txInt, Base::ISceneLoader::ITrafficFlowViewer::VehExitPtr> retVehExits =
        _loader->GetTrafficFlow()->GetAllVehExitData();
    for (const auto& refVehExit : retVehExits) {
      const Base::txInt locId = refVehExit.second->location();
      LocationAgent::LocationAgentPtr curLocPtr = LocationAgent::GetLocationAgentById(locId);
      if (curLocPtr && InRange(enu_bottom_left, enu_top_right, curLocPtr->vPos().ToENU().ENU2D())) {
        const Base::txInt veId = refVehExit.first;
        mVehicleExitVec.emplace_back(VehicleExitAgent());
        if (CallFail(mVehicleExitVec.back().Initialize(veId, static_cast<Coord::txWGS84>((curLocPtr->vPos()).WGS84()),
                                                       refVehExit.second->cover()))) {
          LOG(WARNING) << " Vehicle Exit id = " << veId << " init failure.";
          LOG(WARNING) << "[ERROR] outter exit region : " << (curLocPtr->vPos());
          mVehicleExitVec.erase(mVehicleExitVec.end() - 1);
        } else {
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

Base::txBool TAD_Cloud_ElementGenerator::SafeInputRegion(const Base::txVec2& inputPt,
                                                         const Base::txVec2& nearestPt) const TX_NOEXCEPT {
  if ((inputPt - nearestPt).norm() > FLAGS_tc_input_safe_region_radius) {
    return true;
  } else {
    return false;
  }
}

Base::txBool TAD_Cloud_ElementGenerator::Generate(Base::TimeParamManager const& timeMgr,
                                                  Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  static const std::set<Base::ITrafficElement::ElementType> type_filter_set{
      Base::ITrafficElement::ElementType::TAD_Vehicle, Base::ITrafficElement::ElementType::TAD_Ego};
  const Base::txInt nVehicleSize = _elemMgr->GetVehicleCount();
  static Base::txInt sFrameCnt = 0;
  static Base::txInt sCallCnt = 0;
  static const Base::txSize nMaxVehicleSize = SceneMaxVehicleSize();
  LOG_IF(INFO, (((sFrameCnt++) > 100) ? (sFrameCnt = 0, true) : (false)))
      << "**** " << TX_VARS_NAME(System_Element_Size, nVehicleSize) << " **** " << TX_VARS(nMaxVehicleSize);
  if (nVehicleSize < nMaxVehicleSize) {
#if USE_TBB
    tbb::parallel_for(static_cast<std::size_t>(0), mVehicleInputeVec.size(), [&](const std::size_t idx) {
      VehicleInputeAgent& refInputAgent = mVehicleInputeVec[idx];
      if (CallSucc(refInputAgent.IsValid()) && CallSucc(refInputAgent.IsActive())) {
        if (refInputAgent.IsValidDuration()) {
          if (refInputAgent.UpdatePeriod(timeMgr.RelativeTime())) {
            VehicleInputeAgent::VehicleInitParam_t vehicleInitParam;
            Base::txLaneID vehicleStartLaneId;
            Geometry::SpatialQuery::HashedLaneInfo hashedNode;
            Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr node_ptr = nullptr;
            std::tie(vehicleInitParam, vehicleStartLaneId, hashedNode, node_ptr) =
                refInputAgent.GetNextProbabilityVehicleInitParam();
            TODO("generator lane-wise");
            Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::txSurroundVehiclefo front_nearest_element_info;
            Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::SearchNearestFrontElement(
                -1, -1, 0.0, hashedNode, node_ptr, front_nearest_element_info, FLAGS_tc_input_scan_vision_distance);

            /*const Base::txVec2 startPtLoc = vehicleInitParam.mLocationPtr->vPos().ToENU().ENU2D();*/
            const Base::txLaneUId srcLaneUid = vehicleInitParam.mLocationPtr->GetLaneInfo().onLaneUid;
            const Base::txLaneUId curLaneUid(srcLaneUid.roadId, srcLaneUid.sectionId, vehicleStartLaneId);
            Coord::txWGS84 real_start_pos;

            hadmap::txLanePtr curLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(curLaneUid);
            if (NonNull_Pointer(curLanePtr)) {
              HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(
                  curLaneUid, (vehicleInitParam.mLocationPtr->GetDistanceOnCurve()), real_start_pos);
              const Base::txVec2 startPtLoc = real_start_pos.ToENU().ENU2D();
              VehicleInputInfo << "[vehicle_input_debug]" << TX_VARS(curLaneUid)
                               << TX_VARS_NAME(dist, vehicleInitParam.mLocationPtr->GetDistanceOnCurve())
                               << TX_VARS(real_start_pos);
              Base::SimulationConsistencyPtr front_veh_ptr =
                  Weak2SharedPtr(std::get<_ElementIdx_>(front_nearest_element_info));

              if (Null_Pointer(front_veh_ptr) ||
                  (NonNull_Pointer(front_veh_ptr) &&
                   SafeInputRegion(startPtLoc, front_veh_ptr->StableGeomCenter().ENU2D()))) {
                if (NonNull_Pointer(front_veh_ptr)) {
                  vehicleInitParam.mForceStartV = front_veh_ptr->StableVelocity();
                  VehicleInputInfo << "[vehicle_input_debug]"
                                   << TX_VARS_NAME(InputeRegionId, refInputAgent.VehInputId())
                                   << TX_VARS_NAME(front_veh_id, front_veh_ptr->ConsistencyId())
                                   << TX_VARS_NAME(ForceStartV, *(vehicleInitParam.mForceStartV));
                } else {
                  vehicleInitParam.mForceStartV.clear_value();
                  VehicleInputInfo << "[vehicle_input_debug]"
                                   << TX_VARS_NAME(InputeRegionId, refInputAgent.VehInputId()) << " front is null.";
                }

                TrafficFlow::TAD_Cloud_AI_VehicleElementPtr vehicle =
                    std::make_shared<TrafficFlow::TAD_Cloud_AI_VehicleElement>();
                const Base::txSysId new_sys_id = Base::txIdManager::GenIdPerInput(refInputAgent.VehInputId());
                if (NonNull_Pointer(vehicle) &&
                    vehicle->Initialize(new_sys_id, vehicleInitParam, vehicleStartLaneId, _elemMgr->GetDataSource())) {
#  if 0
                  std::list<Geometry::SpatialQuery::HashedLaneInfo> param;
                  param.emplace_back(hashedNode);
                  HdMap::HashedRoadCacheConCurrent::VehicleContainer results;
                  HdMap::HashedRoadCacheConCurrent::QueryRegisterVehicles(param, results);
                  std::ostringstream oss;
                  oss << "register vehicles : ";
                  for (const auto& refRes : results) {
                    oss << refRes.first << ",";
                  }

                  LOG(INFO) << "[cloud_debug][input_region] " << TX_VARS(hashedNode) << TX_VARS(vehicle->Id())
                            << TX_VARS_NAME(start_loc, real_start_pos)
                            << TX_VARS_NAME(front_veh_id,
                                            (NonNull_Pointer(front_veh_ptr) ? (front_veh_ptr->ConsistencyId()) : (-1)))
                            << TX_VARS_NAME(front_dist,
                                            (NonNull_Pointer(front_veh_ptr)
                                                 ? ((startPtLoc - front_veh_ptr->StablePosition().ENU2D()).norm())
                                                 : (9999.9)))
                            << TX_VARS_NAME(front_pos, (NonNull_Pointer(front_veh_ptr)
                                                            ? (front_veh_ptr->StablePosition().ToWGS84().StrWGS84())
                                                            : (Base::txString("null"))))
                            << oss.str();
#  endif
                  VehicleInputInfo << "[vehicle_input_debug][success]" << timeMgr << TX_VARS(new_sys_id)
                                   << TX_VARS(curLaneUid)
                                   << TX_VARS_NAME(frontId, (NonNull_Pointer(front_veh_ptr))
                                                                ? (front_veh_ptr->ConsistencyId())
                                                                : (-1))
                                   << TX_VARS(real_start_pos) << TX_VARS(vehicle->GeomCenter());
                  _elemMgr->AddVehiclePtr(vehicle);
                } else {
                  VehicleInputWarn << "[vehicle_input_debug][failure][1]" << timeMgr << TX_VARS(new_sys_id)
                                   << TX_VARS(curLaneUid)
                                   << TX_VARS_NAME(frontId, (NonNull_Pointer(front_veh_ptr))
                                                                ? (front_veh_ptr->ConsistencyId())
                                                                : (-1))
                                   << TX_VARS(real_start_pos);
                }
              } else {
                VehicleInputWarn << "[vehicle_input_debug][failure][2]" << timeMgr << TX_VARS(curLaneUid)
                                 << TX_VARS_NAME(frontId, (NonNull_Pointer(front_veh_ptr))
                                                              ? (front_veh_ptr->ConsistencyId())
                                                              : (-1))
                                 << TX_VARS(real_start_pos);
              }
            } else {
              VehicleInputWarn << "[vehicle_input_debug][failure][3]" << timeMgr << TX_VARS(curLaneUid);
            }
          }
        } else {
          LOG_IF(INFO, 0 == sFrameCnt) << "vehicle input duration isvalid. " << TX_VARS(refInputAgent.VehInputId())
                                       << TX_VARS(refInputAgent.CurDuration()) << TX_VARS(refInputAgent.RawDuration());
        }
      } else {
        VehicleInputWarn << "vehicle input error, " << TX_VARS(refInputAgent.VehInputId())
                         << TX_COND(refInputAgent.IsValid()) << TX_COND(refInputAgent.IsActive());
      }
    }); /*lamda function*/
        /* parallel_for */
    _elemMgr->GenerateAllTrafficElement();
#endif /*USE_TBB*/
  } else if ((((sCallCnt++) > 50) ? (sCallCnt = 0, true) : (false))) {
    VehicleInputInfo << " Vehicle fully loaded. " << TX_VARS(nVehicleSize) << TX_VARS(nMaxVehicleSize);
    _elemMgr->ResortKillElement();
    VehicleInputInfo << " ReSortKillVehicle. " << TX_VARS_NAME(nAliveVehicleSize, (_elemMgr->GetVehicleCount()))
                     << TX_VARS(nMaxVehicleSize);
  }
  return true;
}

Base::txBool TAD_Cloud_ElementGenerator::Erase(Base::TimeParamManager const& timeMgr,
                                               Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  Base::txBool needReGenerate = false;
  static std::set<Base::ITrafficElement::ElementType> type_filter_set{Base::ITrafficElement::ElementType::TAD_Vehicle};
  static Base::txInt sCallCnt = 0;
#if USE_TBB

  tbb::parallel_for(static_cast<std::size_t>(0), mVehicleExitVec.size(), [&](const std::size_t idx) {
    VehicleExitAgent& refExitArea = mVehicleExitVec[idx];
    Geometry::SpatialQuery::txHashedRoadArea::VehicleContainer resultElements;
    refExitArea.QueryVehicles(resultElements);
    for (const auto delete_element_ptr : resultElements) {
      delete_element_ptr.second->Kill();
    }
  }); /*lamda function*/
      /* parallel_for */

#endif /*USE_TBB*/
  if ((((sCallCnt++) > 50) ? (sCallCnt = 0, true) : (false))) {
    _elemMgr->ResortKillElement();
  }
  return true;
}

// routing on cloud
Base::txBool TAD_Cloud_ElementGenerator::ReRoute(Base::TimeParamManager const&,
                                                 Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  std::set<Base::ITrafficElement::ElementType> type_filter_set;
  type_filter_set.insert(Base::ITrafficElement::ElementType::TAD_Vehicle);
#if USE_TBB
  tbb::parallel_for(static_cast<std::size_t>(0), mRouteGroupVec.size(), [&](const std::size_t idx) {
    TAD_RouteGroupAgent& refRouteGroupAgent = mRouteGroupVec[idx];
    Geometry::SpatialQuery::txHashedRoadArea::VehicleContainer resultElements;
    refRouteGroupAgent.QueryVehicles(resultElements);
    for (const auto pair_id_re_route_element_ptr : resultElements) {
      const auto re_route_element_ptr = pair_id_re_route_element_ptr.second;
      TrafficFlow::TAD_Cloud_AI_VehicleElementPtr vehicle =
          std::dynamic_pointer_cast<TrafficFlow::TAD_Cloud_AI_VehicleElement>(re_route_element_ptr);
      if (NonNull_Pointer(vehicle) &&
          (((+Base::ISceneLoader::VEHICLE_BEHAVIOR::eTadAI) == vehicle->VehicleBehavior()) ||
           ((+Base::ISceneLoader::VEHICLE_BEHAVIOR::eTadAI_Arterial) == vehicle->VehicleBehavior()))) {
        const Base::txLaneUId& laneUid = re_route_element_ptr->GetCurrentLaneInfo().onLaneUid;
        if (CallSucc(vehicle->IsSupportReRoute()) &&
            (refRouteGroupAgent.RouteGroupId() != vehicle->GetCurRouteGroupId())) {
          LOG_IF(WARNING, FLAGS_LogLevel_Routing) << "ArriveAtStartLineArea id = " << (re_route_element_ptr->Id())
                                                  << " re-route it." << TX_VARS(refRouteGroupAgent.RouteGroupId());
          TrafficFlow::Component::RouteAI newRoute;
          Base::txInt subRouteId = 2;
          newRoute = std::get<2>(refRouteGroupAgent.GetNextProbabilityRoute(subRouteId));
          /*Base::txInt subRouteId = 2;
          const auto& route = ReRouteArea.second.GetNextProbabilityRoute(subRouteId);*/
          vehicle->ReRoute((refRouteGroupAgent.RouteGroupId()), subRouteId, newRoute);
        }
      }
    }
  }); /*lamda function*/
      /* parallel_for */
#else /*USE_TBB*/

#endif /*USE_TBB*/
  return true;
}

Base::txBool TAD_Cloud_ElementGenerator::Release() TX_NOEXCEPT {
  mRouteGroupVec.clear();
  mVehicleInputeVec.clear();
  mVehicleExitVec.clear();
  ParentClass::Release();
  return true;
}

Base::txBool TAD_Cloud_ElementGenerator::ResetGenerator() TX_NOEXCEPT {
  for (auto& refRouteGroupAgent : mRouteGroupVec) {
    refRouteGroupAgent.ResetRoute();
  }

  for (auto& refInputAgent : mVehicleInputeVec) {
    refInputAgent.ResetInputAgent();
  }
  return ParentClass::ResetGenerator();
}

Base::txBool TAD_Cloud_ElementGenerator::ReSetInputAgent(const std::vector<Base::map_range_t>& vec_valid_sim_range)
    TX_NOEXCEPT {
  for (auto& refVehInput : mVehicleInputeVec) {
    refVehInput.SetActive(false);
    if (NonNull_Pointer(refVehInput.RegionLocationPtr())) {
      for (const auto& refSimRange : vec_valid_sim_range) {
        Coord::txWGS84 bottom_left_wgs84;
        bottom_left_wgs84.FromWGS84(refSimRange.bottom_left);
        Coord::txWGS84 top_right_wgs84;
        top_right_wgs84.FromWGS84(refSimRange.top_right);
        if (CallSucc(InRange(bottom_left_wgs84.ToENU().ENU2D(), top_right_wgs84.ToENU().ENU2D(),
                             refVehInput.RegionLocationPtr()->vPos().ToENU().ENU2D()))) {
          refVehInput.SetActive(true);
          LOG(WARNING) << TX_VARS(refVehInput.VehInputId()) << " active.";
        }
      }
    }
  }
  return true;
}

std::vector<Coord::txWGS84> TAD_Cloud_ElementGenerator::InputRegionLocations() const TX_NOEXCEPT {
  std::vector<Coord::txWGS84> res;
  for (const auto& refVehInput : mVehicleInputeVec) {
    if (CallSucc(refVehInput.IsActive()) && NonNull_Pointer(refVehInput.RegionLocationPtr())) {
      res.push_back(refVehInput.RegionLocationPtr()->vPos());
    }
  }
  return res;
}

TX_NAMESPACE_CLOSE(Scene)
