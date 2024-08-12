// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <cstddef>
#include <iterator>
#include <vector>
#include "tx_header.h"
#include "tx_system_module.h"
#include "tx_traffic_element_base.h"
#include "tx_type_traits.h"
TX_NAMESPACE_OPEN(Base)

// @brief 元素管理器基类接口
class IElementManager : public ISystemModule {
 public:
  enum { nMaxEgoSize = 50 };
  using ElementType = ITrafficElement::ElementType;

 public:
  virtual ~IElementManager() TX_DEFAULT;

  /**
   * @brief 获取场景加载器指针
   *
   * @return ISceneLoaderPtr
   */
  virtual ISceneLoaderPtr GetDataSource() TX_NOEXCEPT { return m_DataSource; }

  /**
   * @brief 初始化
   *
   * @return txBool
   */
  virtual txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 资源释放
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT = 0;

  /**
   * @brief 获取vehicle数量
   *
   * @return txSize
   */
  virtual txSize GetVehicleCount() const TX_NOEXCEPT = 0;

  /**
   * @brief 根据索引获取vehicle元素
   *
   * @param _idx 索引
   * @return ITrafficElementPtr 返回的元素指针
   */
  virtual ITrafficElementPtr GetVehiclePtrByIndex(const Base::txSize _idx) TX_NOEXCEPT = 0;

  /**
   * @brief 根据sysid获取vehicle元素指针
   *
   * @param _id 元素sysid
   * @return ITrafficElementPtr 返回的元素指针
   */
  virtual ITrafficElementPtr GetVehiclePtrById(const txSysId _id) TX_NOEXCEPT = 0;

  /**
   * @brief 添加vehicle元素
   *
   * @param _elemPtr 需要添加的vehcile元素
   * @return * txSize vehicle元素数量
   */
  virtual txSize AddVehiclePtr(Base::ITrafficElementPtr _elemPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 根据vehicle的sysid从元素管理器删除对应vehicle
   *
   * @param _vehId vehicle的sysid
   * @return txBool
   */
  virtual txBool EraseVehicleById(const txSysId _vehId) TX_NOEXCEPT = 0;

  /**
   * @brief 销毁所有车辆对象
   *
   */
  virtual void KillAllVehicles() TX_NOEXCEPT = 0;

  /**
   * @brief 获取行人数量
   *
   * @return txSize
   */
  virtual txSize GetPedestrianCount() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取指定索引的行人对象
   *
   * 此函数会返回指定索引的行人对象，以便进一步操作。如果索引无效，则返回nullptr。
   *
   * @param _idx 行人对象的索引
   * @return 指定索引的行人对象，如果索引无效则返回nullptr
   */
  virtual ITrafficElementPtr GetPedestrianPtrByIndex(const Base::txSize _idx) TX_NOEXCEPT = 0;

  /**
   * @brief 根据给定的行人ID获取行人对象的指针
   * @param _id 行人ID
   * @return 返回指向行人对象的指针，如果找不到对应的行人，则返回nullptr
   */
  virtual ITrafficElementPtr GetPedestrianPtrById(const Base::txSysId _id) TX_NOEXCEPT = 0;

  /**
   * @brief 向容器中添加一个行人对象指针
   *
   * @param _elemPtr 行人对象指针
   * @return 返回行人在容器中的序号，如果插入失败则返回 0
   */
  virtual txSize AddPedestrianPtr(Base::ITrafficElementPtr _elemPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取静态障碍物的数量
   *
   * @return txSize
   */
  virtual txSize GetObstacleCount() const TX_NOEXCEPT = 0;

  /**
   * @brief 根据索引获取静态障碍物指针
   *
   * @param _idx 索引
   * @return ITrafficElementPtr 静态障碍物
   */
  virtual ITrafficElementPtr GetObstaclePtrByIndex(const Base::txSize _idx) TX_NOEXCEPT = 0;

  /**
   * @brief 根据obstacle id获取元素
   *
   * @param _id id号
   * @return ITrafficElementPtr 返回的静态障碍物指针
   */
  virtual ITrafficElementPtr GetObstaclePtrById(const Base::txSysId _id) TX_NOEXCEPT = 0;

  /**
   * @brief 添加obstacle元素
   *
   * @param _elemPtr 需要添加的元素
   * @return txSize
   */
  virtual txSize AddObstaclePtr(Base::ITrafficElementPtr _elemPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取信号灯数量
   *
   * @return txSize
   */
  virtual txSize GetSignalCount() const TX_NOEXCEPT = 0;

  /**
   * @brief 根据索引获取信号灯
   *
   * @param _idx 索引
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetSignalPtrByIndex(const Base::txSize _idx) TX_NOEXCEPT = 0;

  /**
   * @brief 根据信号灯id获取信号灯元素
   *
   * @param _id 信号灯id
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetSignalPtrById(const Base::txSysId _id) TX_NOEXCEPT = 0;

  /**
   * @brief 添加信号灯
   *
   * @param _elemPtr
   * @return txSize
   */
  virtual txSize AddSignalPtr(Base::ITrafficElementPtr _elemPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取ego数量
   *
   * @return txSize
   */
  virtual txSize GetEgoCount() const TX_NOEXCEPT = 0;

  /**
   * @brief 添加ego元素
   *
   * @param elemPtr 需要添加的ego
   * @return txSize
   */
  virtual txSize AddEgoPtr(Base::ITrafficElementPtr elemPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取所有的ego元素，header and trailer
   *
   */
  virtual std::vector<ITrafficElementPtr /*1. header; 2. trailer */> GetEgoArray() TX_NOEXCEPT = 0;

  /**
   * @brief 根据ego id删除ego元素
   *
   * @param egoId 提供的id
   * @return txSize
   */
  virtual txSize EarseEgo(const Base::txSysId egoId) TX_NOEXCEPT = 0;

  /**
   * @brief 根据egoid判断是否存在ego
   *
   * @param egoId 提供的egoid
   * @return txBool
   */
  virtual txBool HasEgoId(const Base::txSysId egoId) const TX_NOEXCEPT = 0;
#if USE_EgoGroup
  virtual ITrafficElementPtr GetEgoPtrByGroup(const txString groupName,
                                              const Base::Enums::EgoSubType _egoSubType) TX_NOEXCEPT = 0;
#endif /*USE_EgoGroup*/

  /**
   * @brief Get the Relative Obs Count object
   *
   * @return txSize
   */
  virtual txSize GetRelativeObsCount() const TX_NOEXCEPT = 0;

  /**
   * @brief 添加相对障碍物
   *
   * @param elemPtr 障碍物指针
   * @return txSize
   */
  virtual txSize AddRelativeObsPtr(Base::ITrafficElementPtr elemPtr) TX_NOEXCEPT = 0;

  /**
   * @brief Get the Relative Obs Array object
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetRelativeObsArray() TX_NOEXCEPT = 0;

  /**
   * @brief 根据类型获取所有对应类型的元素
   *
   * @param _eleType
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& SearchElementByType(const ElementType _eleType) TX_NOEXCEPT = 0;
  virtual const std::vector<ITrafficElementPtr>& SearchElementByType(const ElementType _eleType) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取所有元素
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllElement() TX_NOEXCEPT = 0;
  virtual const std::vector<ITrafficElementPtr>& GetAllElement() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取所有信号灯
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllSignal() TX_NOEXCEPT = 0;
  virtual const std::vector<ITrafficElementPtr>& GetAllSignal() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取所有vehicles
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllVehiclePtr() TX_NOEXCEPT = 0;
  virtual const std::vector<ITrafficElementPtr>& GetAllVehiclePtr() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取所有行人
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllPedestrianPtr() TX_NOEXCEPT = 0;
  virtual const std::vector<ITrafficElementPtr>& GetAllPedestrianPtr() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the All Obstacle Ptr object获取所有的障碍物
   *
   * @return std::vector<ITrafficElementPtr>&
   */
  virtual std::vector<ITrafficElementPtr>& GetAllObstaclePtr() TX_NOEXCEPT = 0;
  virtual const std::vector<ITrafficElementPtr>& GetAllObstaclePtr() const TX_NOEXCEPT = 0;

  /**
   * @brief 生成所有的traffic元素
   *
   */
  virtual void GenerateAllTrafficElement() TX_NOEXCEPT = 0;

  /**
   * @brief 排序杀死的元素
   *
   * @return txSize
   */
  virtual txSize ResortKillElement() TX_NOEXCEPT = 0;

 protected:
  ISceneLoaderPtr m_DataSource = nullptr;
};

using IElementManagerPtr = std::shared_ptr<IElementManager>;
TX_NAMESPACE_CLOSE(Base)
