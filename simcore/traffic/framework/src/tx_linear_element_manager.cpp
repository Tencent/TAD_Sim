// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_linear_element_manager.h"

TX_NAMESPACE_OPEN(TrafficFlow)

tbb::mutex LinearElementManager::s_tbbMutex_Vehicles;

Base::txBool LinearElementManager::Initialize(Base::ISceneLoaderPtr _dataSource) TX_NOEXCEPT {
  m_DataSource = _dataSource;
  // 生成所有的交通流元素
  GenerateAllTrafficElement();
  return true;
}

Base::txBool LinearElementManager::Release() TX_NOEXCEPT {
  // 遍历销毁vehicles
  for (auto& refFlowElem : m_vec_VehiclesPtr) {
    refFlowElem->Release();
  }

  // 遍历销毁pedestrian
  for (auto& refFlowElem : m_vec_PedestriansPtr) {
    refFlowElem->Release();
  }

  // 遍历销毁obstacles
  for (auto& refFlowElem : m_vec_ObstaclesPtr) {
    refFlowElem->Release();
  }

  // 遍历销毁signal
  for (auto& refEnvElem : m_vec_SignalPtr) {
    refEnvElem->Release();
  }

  // 遍历销毁ego
  for (auto& refEnvElem : m_vec_EgoPtr) {
    refEnvElem->Release();
  }

  m_vec_VehiclesPtr.clear();
  m_vec_PedestriansPtr.clear();
  m_vec_ObstaclesPtr.clear();
  m_vec_AllElementPtr.clear();
  m_vec_SignalPtr.clear();
  m_vec_EgoPtr.clear();
  return true;
}

LinearElementManager::ITrafficElementPtr LinearElementManager::GetVehiclePtrById(const txSysId _id) TX_NOEXCEPT {
  // 遍历查找
  for (const auto& refVehPtr : m_vec_VehiclesPtr) {
    // 匹配到id返回
    if (NonNull_Pointer(refVehPtr) && (_id == refVehPtr->Id())) {
      return refVehPtr;
    }
  }
  return nullptr;
}

LinearElementManager::ITrafficElementPtr LinearElementManager::GetPedestrianPtrById(const txSysId _id) TX_NOEXCEPT {
  // 遍历查找
  for (const auto& refPedePtr : m_vec_PedestriansPtr) {
    // 匹配到id返回
    if (NonNull_Pointer(refPedePtr) && (_id == refPedePtr->Id())) {
      return refPedePtr;
    }
  }
  return nullptr;
}

LinearElementManager::ITrafficElementPtr LinearElementManager::GetObstaclePtrById(const txSysId _id) TX_NOEXCEPT {
  // 遍历查找
  for (const auto& refObsPtr : m_vec_ObstaclesPtr) {
    // 匹配到id返回
    if (NonNull_Pointer(refObsPtr) && (_id == refObsPtr->Id())) {
      return refObsPtr;
    }
  }
  return nullptr;
}

LinearElementManager::ITrafficElementPtr LinearElementManager::GetSignalPtrById(const txSysId _id) TX_NOEXCEPT {
  // 遍历查找
  for (const auto& refSignal : m_vec_SignalPtr) {
    // 匹配到id返回
    if (NonNull_Pointer(refSignal) && (_id == refSignal->Id())) {
      return refSignal;
    }
  }
  return nullptr;
}

void LinearElementManager::GenerateAllTrafficElement() TX_NOEXCEPT {
  m_vec_AllElementPtr.clear();
  // 直接一次性申请vehicles pedestrian obstacle ego元素空间，以及多余的空间拓展
  m_vec_AllElementPtr.reserve(
      2 * (m_vec_VehiclesPtr.size() + m_vec_PedestriansPtr.size() + m_vec_ObstaclesPtr.size() + m_vec_EgoPtr.size()));
  // 为每类元素集合插入分配好的空间
  m_vec_AllElementPtr.insert(m_vec_AllElementPtr.end(), m_vec_VehiclesPtr.begin(), m_vec_VehiclesPtr.end());
  m_vec_AllElementPtr.insert(m_vec_AllElementPtr.end(), m_vec_PedestriansPtr.begin(), m_vec_PedestriansPtr.end());
  m_vec_AllElementPtr.insert(m_vec_AllElementPtr.end(), m_vec_ObstaclesPtr.begin(), m_vec_ObstaclesPtr.end());
  m_vec_AllElementPtr.insert(m_vec_AllElementPtr.end(), m_vec_EgoPtr.begin(), m_vec_EgoPtr.end());
}

std::vector<Base::ITrafficElementPtr>& LinearElementManager::SearchElementByType(const ElementType _eleType)
    TX_NOEXCEPT {
  // 根据元素类型直接返回对应的集合
  switch (_eleType) {
    case ElementType::TAD_Vehicle: {
      return m_vec_VehiclesPtr;
    }
    case ElementType::TAD_Pedestrian: {
      return m_vec_PedestriansPtr;
    }
    case ElementType::TAD_Immovability: {
      return m_vec_ObstaclesPtr;
    }
    case ElementType::TAD_SignalLight: {
      return m_vec_SignalPtr;
    }
    case ElementType::TAD_Ego: {
      return m_vec_EgoPtr;
    }
  }
  return m_vec_ObstaclesPtr;
}

const std::vector<Base::ITrafficElementPtr>& LinearElementManager::SearchElementByType(const ElementType _eleType) const
    TX_NOEXCEPT {
  // 根据元素类型直接返回对应的不可变集合
  switch (_eleType) {
    case ElementType::TAD_Vehicle: {
      return m_vec_VehiclesPtr;
    }
    case ElementType::TAD_Pedestrian: {
      return m_vec_PedestriansPtr;
    }
    case ElementType::TAD_Immovability: {
      return m_vec_ObstaclesPtr;
    }
    case ElementType::TAD_SignalLight: {
      return m_vec_SignalPtr;
    }
  }
  return m_vec_ObstaclesPtr;
}

Base::txSize LinearElementManager::EarseEgo(const Base::txSysId egoId) TX_NOEXCEPT {
  for (auto itr = std::begin(m_vec_EgoPtr); itr != std::end(m_vec_EgoPtr); ++itr) {
    // 根据id删除
    auto elemPtr = *itr;
    // 匹配到id
    if (NonNull_Pointer(elemPtr) && (_plus_(ElementType::TAD_Ego) == elemPtr->Type()) && (egoId == elemPtr->Id())) {
      // 杀死元素并删除
      elemPtr->Kill();
      m_vec_EgoPtr.erase(itr);
      break;
    }
  }
  return m_vec_EgoPtr.size();
}

Base::txBool LinearElementManager::HasEgoId(const Base::txSysId egoId) const TX_NOEXCEPT {
  for (auto itr = std::begin(m_vec_EgoPtr); itr != std::end(m_vec_EgoPtr); ++itr) {
    // 根据id查找
    auto elemPtr = *itr;
    // 匹配到id返回true
    if (NonNull_Pointer(elemPtr) && (_plus_(ElementType::TAD_Ego) == elemPtr->Type()) && (egoId == elemPtr->Id())) {
      return true;
    }
  }
  return false;
}

#if USE_EgoGroup
LinearElementManager::ITrafficElementPtr LinearElementManager::GetEgoPtrByGroup(
    const txString groupName, const Base::Enums::EgoSubType _egoSubType) TX_NOEXCEPT {
  for (const auto& refEgoPtr : m_vec_EgoPtr) {
    if (NonNull_Pointer(refEgoPtr) && (__strcat__(groupName, _egoSubType) == refEgoPtr->Name())) {
      return refEgoPtr;
    }
  }
  return nullptr;
}
#endif /*USE_EgoGroup*/

Base::txSize LinearElementManager::ResortKillElement() TX_NOEXCEPT {
  Base::txBool needReGenerate = false;

  for (auto itr = std::begin(m_vec_VehiclesPtr); itr != std::end(m_vec_VehiclesPtr);) {
    // 若迭代器为空
    if (Null_Pointer(*itr)) {
      itr = m_vec_VehiclesPtr.erase(itr);
      needReGenerate = true;
    } else if (CallSucc((*itr)->IsEnd())) {  // 如果车辆已经结束
      // LOG(WARNING) << "[20201212_debug] clear kill element " << TX_VARS((*itr)->Id());
      itr = m_vec_VehiclesPtr.erase(itr);
      needReGenerate = true;
    } else if (FLAGS_Kill_After_Stop && CallSucc((*itr)->IsStop())) {  // 如果启用了在停止后杀死车辆，且车辆已经停止
      // LOG(WARNING) << "[20201212_debug] clear IsStop element " << TX_VARS((*itr)->Id());
      itr = m_vec_VehiclesPtr.erase(itr);
      needReGenerate = true;
    } else {
      ++itr;
    }
  }

  if (needReGenerate) {
    // LOG_IF(WARNING, FLAGS_LogLevel_SystemLoop) << oss.str();
    GenerateAllTrafficElement();
  }
  return m_vec_VehiclesPtr.size();
}

Base::txBool LinearElementManager::EraseVehicleById(const txSysId _vehId) TX_NOEXCEPT {
  // 根据id查找并删除
  for (auto ptr : m_vec_VehiclesPtr) {
    // 如果找到了与给定ID匹配的车辆
    if (_vehId == ptr->Id()) {
      ptr->Kill();
      return true;
    }
  }
  // 返回false，表示未找到与给定ID匹配的车辆
  return false;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
