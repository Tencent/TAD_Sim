// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_vehicle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)
Base::txBool PS_VehicleElement::HandleSimulationEvent(const Base::txFloat v) TX_NOEXCEPT {
  mKinetics.velocity_desired = v;
  LOG_IF(INFO, FLAGS_LogLevel_SimulationEvent) << TX_VARS(Id()) << TX_VARS_NAME(event_velocity, v);
  return true;
}

Base::txBool PS_VehicleElement::AddVehiclePath(hadmap::txPoint startPoint,
                                               const std::vector<int64_t>& roadIds) TX_NOEXCEPT {
  if (!roadIds.empty()) {
    hadmap::txRoadPtr curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(roadIds.back());
    if (Null_Pointer(curRoadPtr) || Null_Pointer(curRoadPtr->getGeometry())) {
      LOG(ERROR) << "Can't find " << roadIds.back() << " in hadmap";
      return false;
    }
    double offset_to_end = std::min(3.0, curRoadPtr->getLength() / 2);
    hadmap::txPoint roadEndPoint = curRoadPtr->getGeometry()->getPoint(curRoadPtr->getLength() - offset_to_end);
    double offset_to_start = std::min(3.0, curRoadPtr->getLength() / 2);
    hadmap::txPoint roadStartPoint = curRoadPtr->getGeometry()->getPoint(offset_to_start);
    /*Base::txHdMapLocateInfo info(roadEndPoint);
    m_deadl
        pImpl->m_deadline.Initialize(info);*/

    bool needInsertMinusOne = true;
    if (roadIds[0] == -1 || (roadIds.size() > 1 && roadIds[1] == -1)) {
      needInsertMinusOne = false;
    }
    if (needInsertMinusOne) {
      std::vector<int64_t> roadIdsWithInsertMinusOne;
      for (int i = 0; i < (roadIds.size() - 1); i++) {
        roadIdsWithInsertMinusOne.push_back(roadIds[i]);
        roadIdsWithInsertMinusOne.push_back(-1);
      }
      roadIdsWithInsertMinusOne.push_back(roadIds.back());
      // LOG(WARNING) << " roadIdsWithInsertMinusOne = ";
      // for(int i = 0; i < roadIdsWithInsertMinusOne.size(); i++)
      //     LOG(WARNING) << roadIdsWithInsertMinusOne[i];
      mRouteAI.Initialize(Id(), 0, Id(), startPoint, {}, roadEndPoint, roadIdsWithInsertMinusOne);
      // pImpl->roadInfoArray.GeneratePath(roadIdsWithInsertMinusOne);
    } else {
      mRouteAI.Initialize(Id(), 0, Id(), startPoint, {}, roadEndPoint, roadIds);
      /*pImpl->roadInfoArray.GeneratePath(roadIds);*/
    }
    mRouteAI.ComputeRoute(mPRandom);
#if 0
        pImpl->roadInfoArray.Reset();

        std::vector<hadmap::txPoint> start_end_pts;
        start_end_pts.push_back(roadStartPoint);
        // printf("startPoint: xy = %.10lf, %.10lf\n", startPoint.x, startPoint.y);
        start_end_pts.push_back(roadEndPoint);
        // printf("roadEndPoint: xy = %.10lf, %.10lf\n", roadEndPoint.x, roadEndPoint.y);
        pImpl->roadInfoArray.SetWayPoints(start_end_pts);
        pImpl->roadInfoArray.SelectRandomLaneLink();
#endif
  }

  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
