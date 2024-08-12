// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_cloud_element_manager.h"

TX_NAMESPACE_OPEN(TrafficFlow)

CloudElementManager::ITrafficElementPtr CloudElementManager::GetEdgeVehiclePtrById(const txSysId _id) TX_NOEXCEPT {
  for (const auto& refVehPtr : m_vec_edge_VehiclesPtr) {
    if (NonNull_Pointer(refVehPtr) && (_id == refVehPtr->Id())) {
      return refVehPtr;
    }
  }
  return nullptr;
}

CloudElementManager::txBool CloudElementManager::EraseEdgeVehicleById(const txSysId _vehId) TX_NOEXCEPT {
  for (auto itr = std::begin(m_vec_edge_VehiclesPtr); itr != m_vec_edge_VehiclesPtr.end();) {
    if (NonNull_Pointer((*itr)) && _vehId == (*itr)->Id()) {
      (*itr)->Kill();
      m_vec_edge_VehiclesPtr.erase(itr);
      return true;
    }
  }
  return false;
}

void CloudElementManager::ClearAllElement() TX_NOEXCEPT {
  m_vec_VehiclesPtr.clear();
  m_vec_PedestriansPtr.clear();
  m_vec_ObstaclesPtr.clear();
  m_vec_AllElementPtr.clear();
  m_vec_SignalPtr.clear();
  m_vec_EgoPtr.clear();
  m_vec_edge_VehiclesPtr.clear();
}

TX_NAMESPACE_CLOSE(TrafficFlow)
