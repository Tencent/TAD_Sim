// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_component.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_sim_point.h"
#include "tx_traffic_element_base.h"

TX_NAMESPACE_OPEN(Base)

// @brief 行人类型元素接口
class IPedestrianElement : public ITrafficElement {
 public:
  using PEDESTRIAN_TYPE = Base::Enums::PEDESTRIAN_TYPE;
  using txWGS84 = Coord::txWGS84;
  using txENU = Coord::txENU;
  IPedestrianElement() TX_DEFAULT;
  virtual ~IPedestrianElement() TX_DEFAULT;

 public:
  /** 生命周期相关 **/

  /**
   * @brief 获取行人id
   *
   * @return txSysId
   */
  virtual txSysId Id() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取行人的sysid
   *
   * @return txSysId
   */
  virtual txSysId SysId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.SysId(); }

 public:
  /**
   * @brief 生命周期，获取元素是否激活
   *
   * @return txBool
   */
  virtual txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsAlive(); }

  /**
   * @brief 生命周期，获取元素是否结束
   *
   * @return txBool
   */
  virtual txBool IsEnd() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsEnd(); }

  /**
   * @brief 生命周期是否开始
   *
   * @return txBool
   */
  virtual txBool IsStart() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsStart(); }

  /**
   * @brief 生命周期是否有效
   *
   * @return txBool
   */
  virtual txBool IsValid() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsAlive(); }

  /**
   * @brief 生命周期是否停止
   *
   * @return txBool
   */
  virtual txBool IsStop() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsStop(); }
  /*+*/  // virtual txBool CheckStart(Base::TimeParamManager const&  timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  /*+*/  // virtual txBool CheckEnd(Base::TimeParamManager const&  timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 结束生命周期
   *
   * @return txBool
   */
  virtual txBool Kill() TX_NOEXCEPT TX_OVERRIDE {
    mLifeCycle.SetEnd();
    return IsAlive();
  }

  /**
   * @brief 获取行人的具体类型
   *
   * @return PEDESTRIAN_TYPE
   */
  virtual PEDESTRIAN_TYPE PedestrianType() const TX_NOEXCEPT { return m_pedestrianType; }

 public:
  /**几何相关**/

  /**
   * @brief 获取行人长度
   *
   * @return txFloat
   */
  virtual txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Length(); }

  /**
   * @brief 获取行人宽度
   *
   * @return txFloat
   */
  virtual txFloat GetWidth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Width(); }

  /**
   * @brief 获取行人高度
   *
   * @return txFloat
   */
  virtual txFloat GetHeigth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Height(); }

  /**
   * @brief 获取行人包围盒
   *
   * @return const PolygonArray&
   */
  virtual const PolygonArray& Polygon() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Polygon(); }

 public:
  /**Location相关**/

  /**
   * @brief 获取位置中心经纬度
   *
   * @return txWGS84
   */
  virtual txWGS84 GetLocation() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter(); }

  /**
   * @brief 获取沿曲线的距离
   *
   * 该函数用于计算当前对象沿曲线的沿线位置
   *
   * @return txFloat 当前对象沿曲线的距离
   */
  virtual txFloat DistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.DistanceAlongCurve(); }

  /**
   * @brief 获取当前车道的位移
   *
   * @return txFloat 当前车道的位移
   */
  virtual txFloat LaneOffset() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneOffset(); }

  /**
   * @brief 检查当前对象是否位于车道间的连接处
   *
   * @return txBool 当前对象是否位于车道间的连接处
   */
  virtual txBool IsOnLaneLink() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLaneLink(); }

  /**
   * @brief 检查当前对象是否位于车道中
   *
   * @return txBool 当前对象是否位于车道中
   */
  virtual txBool IsOnLane() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLane(); }

  /**
   * @brief 获取当前车辆所在的车道信息
   *
   * @return const Info_Lane_t& 当前车辆所在车道的信息
   */
  virtual const Info_Lane_t& GetCurrentLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }

  /**
   * @brief 获取行人元素的交叉旋转角度
   *
   * 此函数获取行人元素的交叉旋转角度。
   *
   * @return Unit::txDegree 行人元素的交叉旋转角度，单位为度数。
   */
  virtual Unit::txDegree GetCrossRot() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.CrossRot(); }

  /**
   * @brief 获取行人所在车道的方向向量
   *
   * @return txVec3 行人所在车道的方向向量
   */
  virtual txVec3 GetLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取行人在自身坐标系中的本地坐标
   *
   * @return std::array< txVec3, 2 > 行人在自身坐标系中的本地坐标 (x, y, z)，其中 (x, y) 表示行人在车道上的位置，z
   * 表示行人高度
   */
  virtual const std::array<txVec3, 2> LocalCoords() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LocalCoords(); }

  /**
   * @brief 获取行人在场景中的高度
   *
   * @return txFloat 行人在场景中的高度
   */
  virtual txFloat Altitude() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.Altitude(); }

 public:
  /**
   * @brief 获取行人的速度
   *
   * @return txFloat 返回行人的速度
   */
  virtual txFloat GetVelocity() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_velocity; }

  /**
   * @brief GetAcc 获取行人的加速度
   * @return txFloat 返回行人的加速度
   */
  virtual txFloat GetAcc() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_acceleration; }

  /**
   * @brief GetDisplacement 获取行人的平移距离
   * @return txFloat 返回行人的平移距离
   */
  virtual txFloat GetDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_displacement; }

  /**
   * @brief GetLateralVelocity 获取行人的侧向速度
   * @return txFloat 返回行人的侧向速度
   */
  virtual txFloat GetLateralVelocity() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_LateralVelocity; }

  /**
   * @brief 获取行人的侧偏移量
   *
   * @return txFloat 侧偏移量
   */
  virtual txFloat GetLateralDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_LateralDisplacement; }

  /**
   * @brief Compute_Displacement 计算行人的平移距离
   * @param timeMgr 时间管理器参数，包含相对时间等信息
   * @return txFloat 返回行人的平移距离
   */
  virtual txFloat Compute_Displacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    mKinetics.m_displacement = GetVelocity() * timeMgr.RelativeTime();
    return GetDisplacement();
  }

  /**
   * @brief 获取行人的目标方向
   * @return txVec3 返回行人的目标方向
   */
  virtual txVec3 GetTargetDir() const TX_NOEXCEPT = 0;

  /**
   * @brief GetRawWayPoints 获取行人元素的原始点集，以WGS84坐标系表示
   * @return std::vector<Coord::txWGS84> 返回行人元素的原始点集
   */
  virtual std::vector<Coord::txWGS84> GetRawWayPoints() const TX_NOEXCEPT = 0;

 public:
  /*+*/  // virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;
  /*+*/  // virtual txBool FillingElement(txFloat const, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;
  /*+*/  // virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const&  timeMgr) const TX_NOEXCEPT
         // TX_OVERRIDE;

 public:
  /**
   * @brief RelocateTracker 尝试重新定位行人轨迹指示器
   * @param pLane 需要考虑的车道指针
   * @param _timestamp 发生重新定位的时间戳，以seconds为单位
   * @return txBool 如果重定位成功，则返回 true，否则返回 false
   */
  virtual Base::txBool RelocateTracker(hadmap::txLanePtr pLane, const txFloat& _timestamp) TX_NOEXCEPT {
    return mLocation.RelocateTracker(pLane, _timestamp);
  }

  /**
   * @brief 将轨迹追踪器在给定车道链接上重新定位
   *
   * 此函数尝试在给定车道链接上重新定位轨迹追踪器。如果成功，则返回 true；否则返回 false。
   *
   * @param pLaneLink 需要考虑的车道链接指针
   * @param _timestamp 发生重新定位的时间戳，以秒为单位
   * @return txBool 如果重定位成功，则返回 true；否则返回 false
   */
  virtual Base::txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink, const txFloat& _timestamp) TX_NOEXCEPT {
    return mLocation.RelocateTracker(pLaneLink, _timestamp);
  }

 protected:
  Base::Component::Identity mIdentity;
  Base::Component::LifeCycle mLifeCycle;
  Base::Component::GeometryData mGeometryData;
  Base::Component::Location mLocation;
  Base::Component::Kinetics mKinetics;
  PEDESTRIAN_TYPE m_pedestrianType = PEDESTRIAN_TYPE::human;
  Base::txString m_pedestrianTypeStr = "human";
  Base::txInt m_pedestrianTypeId = 0;
};
using IPedestrianElementPtr = std::shared_ptr<IPedestrianElement>;
using WEAK_IPedestrianElementPtr = std::weak_ptr<IPedestrianElement>;
TX_NAMESPACE_CLOSE(Base)
