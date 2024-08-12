// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_pedestrian_trajectory.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_pedestrian_element.h"
#include "tx_scene_loader.h"
#include "tx_sim_point.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class Cloud_PedestrianElement : public Base::IPedestrianElement {
 public:
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using AdjoinContainerType = HashedLaneInfoOrthogonalList::AdjoinContainerType;

  Cloud_PedestrianElement() TX_DEFAULT;
  virtual ~Cloud_PedestrianElement() TX_DEFAULT;

  /**
   * @brief 初始化方法
   *
   * 初始化函数，以准备启动场景的任何对象，如加载模型、纹理和其他可能的相关数据。
   *
   * @param[in] ViewerPtr：场景查看器的指针
   * @param[in] SceneLoaderPtr：场景加载器的指针
   * @return Base::txBool：如果初始化成功返回true，否则返回false
   */
  virtual Base::txBool Initialize(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return false;
  }

  /**
   * @brief 在云端初始化行人元素
   * @param pedId 行人ID
   * @param fromLocInfo 从位置的车道信息
   * @param from_s 从位置的相对车道坐标
   * @param toLocInfo 到位置的车道信息
   * @param to_s 到位置的相对车道坐标
   * @return 初始化成功返回true，否则返回false
   */
  virtual Base::txBool InitializeOnCloud(const Base::txSysId pedId, const Base::Info_Lane_t& fromLocInfo,
                                         const Base::txFloat from_s, const Base::Info_Lane_t& toLocInfo,
                                         const Base::txFloat to_s) TX_NOEXCEPT;

  /**
   * @brief 更新行人元素状态
   *
   * 根据传入的时间参数管理器，更新行人元素状态。
   *
   * @param timeMgr 时间参数管理器，包含当前时间等信息
   * @return bool 更新成功返回true，否则返回false
   */
  virtual Base::txBool Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放行人元素
   *
   * 释放行人元素的内存资源，包括销毁动画和资源。
   *
   * @return bool 释放成功返回 true，否则返回 false
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 填充行人元素
   *
   * 使用传入的时间参数管理器和交通消息对象，填充行人元素，并返回填充结果。
   *
   * @param timeMgr 时间参数管理器，包含当前时间等信息
   * @param traffic 交通消息对象，用于存储和更新行人元素的信息
   * @return bool 填充成功返回 true，否则返回 false
   */
  virtual Base::txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 填充空间查询
   *
   * 使用传入的时间参数管理器和交通消息对象，填充空间查询，并返回填充结果。
   *
   * @param timeMgr 时间参数管理器，包含当前时间等信息
   * @param traffic 交通消息对象，用于存储和更新行人元素的信息
   * @return bool 填充成功返回 true，否则返回 false
   */
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 获取行人元素的动力学信息
   *
   * 根据传入的时间参数管理器，返回一个结构体，包含行人元素的动力学信息，如速度、加速度等。
   *
   * @param timeMgr 时间参数管理器，包含当前时间等信息
   * @return KineticsInfo_t 行人元素的动力学信息
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE {
    return KineticsInfo_t();
  }

  /**
   * @brief 获取行人元素类型
   *
   * 返回行人元素的类型。
   *
   * @return ElementType 返回行人元素的类型
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return _plus_(ElementType::TAD_Pedestrian); }

  /**
   * @brief 检查行人元素是否开始
   *
   * 检查行人元素是否开始。行人元素的开始通常是指其沿指定路线移动。
   *
   * @param timeMgr 时间参数管理器，包含游戏当前时间等信息
   * @return true 行人元素已开始
   * @return false 行人元素未开始
   */
  virtual Base::txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查行人元素是否结束
   *
   * 检查行人元素是否结束。当行人元素到达预设位置或等待一段时间后，通常会结束动作。
   *
   * @param timeMgr 时间参数管理器，包含游戏当前时间等信息
   * @return true 行人元素已结束
   * @return false 行人元素未结束
   */
  virtual Base::txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 行人元素开始执行
   *
   * @param timeMgr
   */
  virtual void OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 结束行人元素事件
   *
   * 当行人元素完成指定动作后，触发此事件。
   *
   * @param timeMgr 时间参数管理器，包含游戏当前时间等信息
   */
  virtual void OnEnd() TX_NOEXCEPT TX_OVERRIDE {}

 public:
  /**
   * @brief 计算行人元素的侧位移
   * @param[in] timeMgr 时间参数管理器，包含游戏当前时间等信息
   * @return 返回行人元素的侧位移，单位：米
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return 0.0;
  }

  /**
   * @brief 获取行人元素在切换车道时的状态
   * @return 返回行人元素在切换车道时的状态，可选值为 tx_tc_VehicleMoveLaneState 中定义的枚举类型
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return VehicleMoveLaneState::eStraight;
  }

 protected:
  /**
   * @brief 根据给定的起点、中间点和终点坐标生成车道切换路径点
   *
   * @param startPos 起点的坐标
   * @param midPoint 车道切换过程中的各个中间点坐标
   * @param refEndPoint 车道切换结束时的坐标
   */
  void GenerateWayPoints(const Coord::txWGS84& startPos,
                         const std::vector<std::pair<Base::txFloat, Base::txFloat> >& midPoint,
                         const std::pair<Base::txFloat, Base::txFloat>& refEndPoint) TX_NOEXCEPT;
#if __TX_Mark__("ISimulationConsistency")

 public:
  /**
   * @brief 获取一个实例的一致性ID
   *
   * @return 返回实例的一致性ID
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取一个实例的最后车辆后架中心点的坐标
   *
   * @return 返回实例的最后车辆后架中心点的坐标
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastRearAxleCenter(); }

  /**
   * @brief 获取一个实例的稳定几何中心点的坐标
   *
   * @return 返回实例的稳定几何中心点的坐标
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastGeomCenter(); }

  /**
   * @brief 获取一个实例的稳定位置，保证该位置在当前道路中心线上
   *
   * @return 返回一个WGS84坐标系中的经纬度坐标
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.PosOnLaneCenterLinePos();
  }

  /**
   * @brief 获取一个实例的稳定车道信息
   *
   * @return 返回一个包含当前稳定车道信息的结构体，用于表示该实例的稳定车道
   */
  virtual Base::Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }

  /**
   * @brief 获取稳定车道的方向
   *
   * @return Base::txVec3
   */
  virtual Base::txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取实例稳定的朝向
   *
   * @return Unit::txDegree 度
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE {
    return Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());
  }

  /**
   * @brief 获取实例稳定的速度
   *
   * @return txFloat 返回的速度大小
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return GetVelocity(); }

  /**
   * @brief 获取实例稳定的加速度
   *
   * @return txFloat 返回的加速度大小
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return GetAcc(); }

  /**
   * @brief 保存稳定状态
   *
   * 保存当前实例的稳定状态，以便在应用崩溃或重新启动后，可以恢复到相同的状态。
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前稳定的哈希车道信息
   *
   * 返回当前实例的稳定哈希车道信息。该信息可用于重建当前状态。
   *
   * @return 返回一个包含当前稳定哈希车道信息的引用
   */
  virtual const HashedLaneInfo& StableHashedLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return m_curHashedLaneInfo; }
#endif /*__TX_Mark__("ISimulationConsistency")*/

#if __TX_Mark__("Log.Debug")
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE { return ""; }
  friend std::ostream& operator<<(std::ostream& os, Cloud_PedestrianElement const& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
#endif  // #if __TX_Mark__("Log.Debug")
#if USE_HashedRoadNetwork

 protected:
  /**
   * @brief 初始化时注册信息
   *
   * @param _location 位置信息
   */
  virtual void RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 更新hash车道信息
   *
   * @param _location 位置信息
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief kill时注销信息
   *
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE {}
#endif /*USE_HashedRoadNetwork*/

 public:
  /**
   * @brief 通过缓存生成指定车道的信号
   *
   * 此函数用于根据缓存的信息生成指定车道的信号。函数输入为车道的起始位置信息和位置坐标，输出为一个空操作。
   *
   * @param from_locInfo 车道的起始位置信息
   * @param from_s 车道的起始位置坐标
   */
  virtual void GenerateSignalByLaneCache(const Base::Info_Lane_t from_locInfo, const Base::txFloat from_s) TX_NOEXCEPT;

  /**
   * @brief 通过缓存快速生成折线段落子路线的信号
   *
   * 根据传入车道信息，逐个生成下一环的信号。
   *
   * @param _Control_laneUid_one_ring 传入的一环的缓存
   * @param from_s 起始位置
   */
  virtual void GenerateSignalByLaneCache_Internal(const hadmap::txLanes& _Control_laneUid_one_ring,
                                                  const Base::txFloat from_s) TX_NOEXCEPT;

  /**
   * @brief 获取公共行人信息
   *
   * 该函数用于获取指定道路区域内的公共行人信息，包括行人的道路方向、行驶速度等信息。
   *
   * @param[in] manager 时间参数管理器，用于获取当前时间等信息
   * @param[out] list 用于存储公共行人信息的容器
   * @return 无
   */
  virtual void PublicPedestrians(const Base::TimeParamManager&) TX_NOEXCEPT;

  /**
   * @brief 获取影响车辆列表
   *
   * 该函数用于获取指定道路区域内的影响车辆列表，包括车道、方向、道路方向、行驶速度等信息。
   *
   * @param[out] vehicleList 用于存储影响车辆列表的容器
   * @return 无
   */
  virtual void GetInflunceVehicles(HashedLaneInfoOrthogonalList::VehicleContainer&) TX_NOEXCEPT;

  /**
   * @brief 获取目标方向
   *
   * 获取目标方向，该方向通常用于控制行人移动方向。
   *
   * @return 目标方向
   */
  virtual Base::txVec3 GetTargetDir() const TX_NOEXCEPT TX_OVERRIDE { return m_vTargetDir; }

  /**
   * @brief 获取原始的路径点集合
   *
   * 获取折线路径点的集合，该点集合表示该折线路径的起点、终点和中间点的经纬度坐标。
   *
   * @return 经纬度坐标点集合
   */
  virtual std::vector<Coord::txWGS84> GetRawWayPoints() const TX_NOEXCEPT { return m_polyline_traj.GetRawWayPoints(); }

 protected:
  PolyLineTraj m_polyline_traj;
  Base::txVec3 m_vTargetDir;
  Coord::txWGS84 m_start_pt;
  Coord::txWGS84 m_end_pt;
  AdjoinContainerType mControlOrthogonalNodeMap;
  Base::txCountDownTimer mSilentMgr;
};
using Cloud_PedestrianElementPtr = std::shared_ptr<Cloud_PedestrianElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
