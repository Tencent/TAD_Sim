// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_element_generator.h"

TX_NAMESPACE_OPEN(Scene)

Base::txBool ParallelSimulation_ElementGenerator::Initialize(Base::ISceneLoaderPtr _loader,
                                                             const Base::map_range_t& valid_map_range) TX_NOEXCEPT {
#if __pingsn__
  /*step 5 init*/
  Base::txLaneID vehicleStartLaneId;
  /*mVehicleSeparateInput.Init(_loader);*/
#endif /*__pingsn__*/
  return TAD_Cloud_ElementGenerator::Initialize(_loader, valid_map_range);
}

Base::txBool ParallelSimulation_ElementGenerator::Generate(its::txVehicles& info,
                                                           Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  return mVehicleSeparateInput.Init(info, _loader);
}

ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::VehicleInfo_t
ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::generateVehicleInfo(
    Base::txSysId vehId64, Base::txInt vehType, int64_t startTime, Base::txLaneID vehicleStartLaneId,
    const std::vector<int64_t>& roadIds, Base::ISceneLoaderPtr _loader, Base::txFloat startV,
    Base::txFloat roadStartPointX, Base::txFloat roadStartPointY, Base::txFloat roadStartPointZ) TX_NOEXCEPT {
  VehicleInfo_t vehicleInfo;
  // vehicleInfo.vehicleInitParam = vehicleInitParam;

  vehicleInfo.vehId64 = vehId64;
  vehicleInfo.startTime = startTime;
  vehicleInfo.vehicleStartLaneId = vehicleStartLaneId;
  vehicleInfo.roadIds = roadIds;

  hadmap::txPoint roadStartPoint(roadStartPointX, roadStartPointY, roadStartPointZ);
  LocationAgent::LocationAgentPtr locAgentPtr = std::make_shared<LocationAgent>();

  locAgentPtr->Init(roadStartPoint);
  vehicleInfo.vehicleInitParam.mLocationPtr = locAgentPtr;

  vehicleInfo.vehicleInitParam.mVehTypePtr = _loader->GetTrafficFlow()->GetVehType(vehType);
  vehicleInfo.vehicleInitParam.mBehPtr = _loader->GetTrafficFlow()->GetBeh(1);
  // TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehTypePtr vehType1Ptr =
  //     std::make_shared<TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType>();
  // VehType vehType(1, 1, 4.5, 1.8, 1.5, Base::ISceneLoader::VEHICLE_BEHAVIOR::TAD_TRAFFIC_VEHICLE_BEHAVIOR);
  // vehType1Ptr->Init(vehType)  ;

  vehicleInfo.vehicleInitParam.SetStartV_Cloud(startV);
  vehicleInfo.vehicleInitParam.SetMaxV_Cloud(15);
  return vehicleInfo;
}

bool compareByStartTime(const ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::VehicleInfo_t& a,
                        const ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::VehicleInfo_t& b) {
  return a.startTime < b.startTime;
}

std::vector<ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::VehicleInfo_t>
ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::getVehicleListFromPb(
    its::txVehicles& vehicles, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  std::vector<VehicleInfo_t> mVehicleListFromPB;

  LOG(INFO) << "Simu Input vehicles:" << vehicles.DebugString() << std::endl;
  LOG(INFO) << "traffic.cars_size(): " << vehicles.vehicle_size();
  for (int i = 0; i < vehicles.vehicle_size(); i++) {
    const its::txVehicle& vehicle = vehicles.vehicle(i);

    std::vector<int64_t> roadIds = {vehicle.route().road_id().begin(), vehicle.route().road_id().end()};
    if (roadIds.size() == 0) {
      LOG(WARNING) << "traffic" << vehicle.vehicle_id() << " route is empty";
      continue;
    }
    if (roadIds[0] == -1) {
      roadIds.erase(roadIds.begin());
    }
    std::cout << vehicle.DebugString() << std::endl;

    hadmap::txMapHandle* hdr = HdMap::HadmapCacheConCurrent::GetMapHandler();
    hadmap::txRoadPtr curRoadPtr = nullptr;
    hadmap::getRoad(hdr, roadIds[0], true, curRoadPtr);
#if 0
        int lane_id = -1;
        hadmap::txPoint roadStartPoint = curRoadPtr->getSections()[0]->
            get(lane_id)->getGeometry()->getStart();
        // printf("roadStartID:::::::::::::::%lld\n", curRoadPtr->getId());
        // printf("roadStartPoint:::::::::::::::%.10f,%.10f\n", roadStartPoint.x,roadStartPoint.y);

        mVehicleListFromPB.push_back(generateVehicleInfo(vehicle.vehicle_id(), vehicle.vehicle_type(),
            vehicle.start_time(), lane_id, roadIds, _loader, 12,
            roadStartPoint.x, roadStartPoint.y, roadStartPoint.z));
#else
    hadmap::txSectionPtr curSectionPtr = curRoadPtr->getSections()[0];
    hadmap::txLanePtr curLanePtr = nullptr;
    for (auto lanePtr : curSectionPtr->getLanes()) {
      if (Utils::IsDrivingLane(lanePtr->getLaneType())) {
        curLanePtr = lanePtr;
        break;
      }
    }
    hadmap::txPoint roadStartPoint = curLanePtr->getGeometry()->getStart();
    mVehicleListFromPB.push_back(generateVehicleInfo(vehicle.vehicle_id(), vehicle.vehicle_type(), vehicle.start_time(),
                                                     curLanePtr->getId(), roadIds, _loader, 12, roadStartPoint.x,
                                                     roadStartPoint.y, roadStartPoint.z));
#endif
  }

  std::sort(mVehicleListFromPB.begin(), mVehicleListFromPB.end(), compareByStartTime);
  return mVehicleListFromPB;
}

Base::txBool ParallelSimulation_ElementGenerator::VehicleSeparateInputAgent::Init(
    its::txVehicles& info, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  mValid = true;
  mCurIndex = 0;
  mVehicleList = getVehicleListFromPb(info, _loader);
  if (mVehicleList.size() > 0)
    mVehicleBornTime = mVehicleList[0].startTime;
  else
    mValid = false;
  return true;
}

TX_NAMESPACE_CLOSE(Scene)
