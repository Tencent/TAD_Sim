// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_element_manager_base.h"
#include "tx_traffic_element_base.h"
#if USE_TBB
#  include <tbb/mutex.h>
#endif /*USE_TBB*/
TX_NAMESPACE_OPEN(TrafficFlow)

// @brief 线性元素管理器
class LinearElementManager : public Base::IElementManager {
 public:
  using ElementType = Base::ITrafficElement::ElementType;
  using ITrafficElementPtr = Base::ITrafficElementPtr;
  using txBool = Base::txBool;
  using txSize = Base::txSize;
  using txSysId = Base::txSysId;
  using txString = Base::txString;
  using TimeParamManager = Base::TimeParamManager;
  using ElemContainer = std::vector<ITrafficElementPtr>;

 public:
  LinearElementManager() TX_DEFAULT;
  virtual ~LinearElementManager() { Release(); }

  /**
   * @brief 初始化函数
   *
   * @return txBool
   */
  virtual txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新函数
   *
   * @return txBool
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 释放资源
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Vehicle Count object
   *
   * @return txSize
   */
  virtual txSize GetVehicleCount() const TX_NOEXCEPT TX_OVERRIDE { return m_vec_VehiclesPtr.size(); }

  /**
   * @brief Get the Vehicle Ptr By Index object
   *
   * @param idx
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetVehiclePtrByIndex(const txSize idx) TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_VehiclesPtr.at(idx);
  }

  /**
   * @brief Get the Vehicle Ptr By Id object
   *
   * @param _id
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetVehiclePtrById(const txSysId _id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加vehicle
   *
   * @param elemPtr
   * @return txSize
   */
  virtual txSize AddVehiclePtr(ITrafficElementPtr elemPtr) TX_NOEXCEPT TX_OVERRIDE {
    tbb::mutex::scoped_lock lock(s_tbbMutex_Vehicles);
    m_vec_VehiclesPtr.emplace_back(elemPtr);
    return GetVehicleCount();
  }

  /**
   * @brief 根据sysid删除vehicle
   *
   * @param _vehId
   * @return txBool
   */
  virtual txBool EraseVehicleById(const txSysId _vehId) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 删除所有的vehicle
   *
   */
  virtual void KillAllVehicles() TX_NOEXCEPT TX_OVERRIDE { m_vec_VehiclesPtr.clear(); }

  /**
   * @brief Get the Pedestrian Count object
   *
   * @return txSize
   */
  virtual txSize GetPedestrianCount() const TX_NOEXCEPT TX_OVERRIDE { return m_vec_PedestriansPtr.size(); }

  /**
   * @brief Get the Pedestrian Ptr By Index object
   *
   * @param _idx
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetPedestrianPtrByIndex(const txSize _idx) TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_PedestriansPtr.at(_idx);
  }

  /**
   * @brief Get the Pedestrian Ptr By Id object
   *
   * @param _id
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetPedestrianPtrById(const txSysId _id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加行人
   *
   * @param _elemPtr
   * @return txSize
   */
  virtual txSize AddPedestrianPtr(ITrafficElementPtr _elemPtr) TX_NOEXCEPT TX_OVERRIDE {
    m_vec_PedestriansPtr.emplace_back(_elemPtr);
    return GetPedestrianCount();
  }

  /**
   * @brief Get the Obstacle Count
   *
   * @return txSize
   */
  virtual txSize GetObstacleCount() const TX_NOEXCEPT TX_OVERRIDE { return m_vec_ObstaclesPtr.size(); }

  /**
   * @brief Get the Obstacle Ptr By Index
   *
   * @param _idx
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetObstaclePtrByIndex(const txSize _idx) TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_ObstaclesPtr.at(_idx);
  }

  /**
   * @brief 根据sysid获取障碍物
   *
   * @param _id
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetObstaclePtrById(const txSysId _id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加障碍物
   *
   * @param _elemPtr
   * @return txSize
   */
  virtual txSize AddObstaclePtr(ITrafficElementPtr _elemPtr) TX_NOEXCEPT TX_OVERRIDE {
    m_vec_ObstaclesPtr.emplace_back(_elemPtr);
    return GetObstacleCount();
  }

  /**
   * @brief 获取信号灯数量
   *
   * @return txSize
   */
  virtual txSize GetSignalCount() const TX_NOEXCEPT TX_OVERRIDE { return m_vec_SignalPtr.size(); }

  /**
   * @brief 根据索引获取signal
   *
   * @param _idx 索引
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetSignalPtrByIndex(const txSize _idx) TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_SignalPtr.at(_idx);
  }

  /**
   * @brief 根据sysid获取signal
   *
   * @param _id
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetSignalPtrById(const txSysId _id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加signal
   *
   * @param _elemPtr
   * @return txSize
   */
  virtual txSize AddSignalPtr(ITrafficElementPtr _elemPtr) TX_NOEXCEPT TX_OVERRIDE {
    m_vec_SignalPtr.emplace_back(_elemPtr);
    return GetSignalCount();
  }
#if USE_EgoGroup
  virtual ITrafficElementPtr GetEgoPtrByGroup(const txString groupName,
                                              const Base::Enums::EgoSubType _egoSubType) TX_NOEXCEPT TX_OVERRIDE;
#endif /*USE_EgoGroup*/

  /**
   * @brief 获取ego元素数量
   *
   * @return txSize
   */
  virtual txSize GetEgoCount() const TX_NOEXCEPT TX_OVERRIDE { return m_vec_EgoPtr.size(); }

  /**
   * @brief 添加ego
   *
   * @param elemPtr
   * @return txSize
   */
  virtual txSize AddEgoPtr(ITrafficElementPtr elemPtr) TX_NOEXCEPT TX_OVERRIDE {
    m_vec_EgoPtr.emplace_back(elemPtr);
    return m_vec_EgoPtr.size();
  }

  /**
   * @brief 获取所有ego
   *
   */
  virtual std::vector<ITrafficElementPtr /*1. header; 2. trailer */> GetEgoArray() TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_EgoPtr;
  }

  /**
   * @brief 根据egoid删除ego
   *
   * @param egoId 当前ego的id
   * @return txSize 返回删除后ego的元素
   */
  virtual txSize EarseEgo(const Base::txSysId egoId) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据egoid判断是否存在
   *
   * @param egoId 当前ego的id
   * @return txBool true为存在
   */
  virtual txBool HasEgoId(const Base::txSysId egoId) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取relative obs数量
   *
   * @return txSize
   */
  virtual txSize GetRelativeObsCount() const TX_NOEXCEPT TX_OVERRIDE { return m_vec_RelativeObsPtr.size(); }

  /**
   * @brief 添加relative obs
   *
   * @param elemPtr
   * @return txSize
   */
  virtual txSize AddRelativeObsPtr(Base::ITrafficElementPtr elemPtr) TX_NOEXCEPT TX_OVERRIDE {
    m_vec_RelativeObsPtr.emplace_back(elemPtr);
    return GetRelativeObsCount();
  }

  /**
   * @brief 获取relative obs集合
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetRelativeObsArray() TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_RelativeObsPtr;
  }

  /**
   * @brief 根据类型获取元素集合
   *
   * @param _eleType
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& SearchElementByType(const ElementType _eleType) TX_NOEXCEPT TX_OVERRIDE;
  virtual const std::vector<ITrafficElementPtr>& SearchElementByType(const ElementType _eleType) const TX_NOEXCEPT
      TX_OVERRIDE;

  /**
   * @brief 获取所有元素
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllElement() TX_NOEXCEPT TX_OVERRIDE { return m_vec_AllElementPtr; }
  virtual const std::vector<ITrafficElementPtr>& GetAllElement() const TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_AllElementPtr;
  }

  /**
   * @brief 获取所有signal
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllSignal() TX_NOEXCEPT TX_OVERRIDE { return m_vec_SignalPtr; }
  virtual const std::vector<ITrafficElementPtr>& GetAllSignal() const TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_SignalPtr;
  }

  /**
   * @brief 获取所有vehicle
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllVehiclePtr() TX_NOEXCEPT TX_OVERRIDE { return m_vec_VehiclesPtr; }
  virtual const std::vector<ITrafficElementPtr>& GetAllVehiclePtr() const TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_VehiclesPtr;
  }

  /**
   * @brief 获取所有行人
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllPedestrianPtr() TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_PedestriansPtr;
  }
  virtual const std::vector<ITrafficElementPtr>& GetAllPedestrianPtr() const TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_PedestriansPtr;
  }

  /**
   * @brief 获取所有obstacle
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllObstaclePtr() TX_NOEXCEPT TX_OVERRIDE { return m_vec_ObstaclesPtr; }
  virtual const std::vector<ITrafficElementPtr>& GetAllObstaclePtr() const TX_NOEXCEPT TX_OVERRIDE {
    return m_vec_ObstaclesPtr;
  }

  /**
   * @brief 生成所有traffic元素
   *
   */
  virtual void GenerateAllTrafficElement() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重排删除元素
   *
   * @return Base::txSize
   */
  virtual Base::txSize ResortKillElement() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  ElemContainer m_vec_VehiclesPtr;
  ElemContainer m_vec_PedestriansPtr;
  ElemContainer m_vec_ObstaclesPtr;
  ElemContainer m_vec_AllElementPtr;
  ElemContainer m_vec_SignalPtr;
  ElemContainer m_vec_EgoPtr;
  ElemContainer m_vec_RelativeObsPtr;
  static tbb::mutex s_tbbMutex_Vehicles;
};
TX_NAMESPACE_CLOSE(TrafficFlow)
