// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_linear_element_manager.h"
#include "tx_tc_marco.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class CloudElementManager : public LinearElementManager {
 public:
  /**
   * @brief 获取边缘车辆数量
   * @return txSize 返回边缘车辆数量
   */
  virtual txSize GetEdgeVehicleCount() const TX_NOEXCEPT { return m_vec_edge_VehiclesPtr.size(); }

  /**
   * @brief 通过ID获取指向边缘车辆的指针
   * @param _id 边缘车辆的ID
   * @return ITrafficElementPtr 指向边缘车辆的指针，如果找不到对应的边缘车辆，则返回空指针
   */
  virtual ITrafficElementPtr GetEdgeVehiclePtrById(const txSysId _id) TX_NOEXCEPT;

  /**
   * @brief 添加边缘车辆指针并返回车辆总数
   * @param elemPtr 指向边缘车辆的指针
   * @return txSize 返回车辆总数
   */
  virtual txSize AddEdgeVehiclePtr(ITrafficElementPtr elemPtr) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(s_tbbMutex_Vehicles);
    m_vec_edge_VehiclesPtr.emplace_back(elemPtr);
    return GetVehicleCount();
  }

  /**
   * @brief 通过ID删除指定的边缘车辆
   * @param _vehId 要删除的车辆的ID
   * @return txBool 如果车辆被成功删除，则返回 txTrue；否则，返回 txFalse。
   */
  virtual txBool EraseEdgeVehicleById(const txSysId _vehId) TX_NOEXCEPT;

  /**
   * @brief 删除所有边缘车辆
   * @details 该函数会清空边缘车辆的容器，并保持容器的容量不变。
   */
  virtual void RemoveAllEdgeVehicles() TX_NOEXCEPT {
    m_vec_edge_VehiclesPtr.clear(/*keep the vector capacity unchanged*/);
  }

  /**
   * @brief 获取所有边缘车辆指针的引用
   * @details 该函数返回一个引用，指向一个包含所有边缘车辆指针的vector。可以直接通过该引用修改vector中的元素。
   */
  virtual std::vector<ITrafficElementPtr>& GetAllEdgeVehiclePtr() TX_NOEXCEPT { return m_vec_edge_VehiclesPtr; }

  /**
   * @brief 获取所有边缘车辆指针的引用
   * @details 该函数返回一个引用，指向一个包含所有边缘车辆指针的vector。可以直接通过该引用修改vector中的元素。
   */
  virtual const std::vector<ITrafficElementPtr>& GetAllEdgeVehiclePtr() const TX_NOEXCEPT {
    return m_vec_edge_VehiclesPtr;
  }
#if __cloud_ditw__

  /**
   * @brief 清除所有元素
   * @details 清除所有存在的元素，但不会释放已分配的内存。
   */
  virtual void ClearAllElement() TX_NOEXCEPT;
#endif /*__cloud_ditw__*/

 protected:
  ElemContainer m_vec_edge_VehiclesPtr;
};

using CloudElementManagerPtr = std::shared_ptr<CloudElementManager>;
TX_NAMESPACE_CLOSE(TrafficFlow)
