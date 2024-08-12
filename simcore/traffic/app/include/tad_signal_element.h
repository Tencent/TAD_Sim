// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "tad_signal_phase_period.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_signal_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_SignalLightElement : public Base::ISignalLightElement {
 public:
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using AdjoinContainerType = HashedLaneInfoOrthogonalList::AdjoinContainerType;
  using ParentClass = Base::ISignalLightElement;

 public:
  TAD_SignalLightElement() TX_DEFAULT;
  virtual ~TAD_SignalLightElement();

  /**
   * @brief 初始化signal对象相关
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新当前signal的信息状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool Update(const Base::TimeParamManager &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放当前signal的资源
   *
   * @return Base::txBool
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取signal对象的traffic信息
   *
   * @return Base::txBool
   */
  virtual Base::txBool FillingElement(Base::TimeParamManager const &, sim_msg::Traffic &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新空间查询
   *
   * @return Base::txBool
   */
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE { return true; };

  /**
   * @brief 获取元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_SignalLight; }

  /**
   * @brief 获取元素对象格式化字符串
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

  virtual Base::ISignalLightElement::SIGN_LIGHT_COLOR_TYPE GetCurrentSignLightColorType() const TX_NOEXCEPT TX_OVERRIDE;
#if __TX_Mark__("ISimulationConsistency")

 public:
  /**
   * @brief 获取元素id
   *
   * @return txSysId
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取元素几何中心点坐标
   *
   * @return Coord::txENU ENU坐标系的坐标
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter().GetENU(); }

  /**
   * @brief 获取后轴中心的ENU坐标
   *
   * @return Coord::txENU enu坐标点
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return StableGeomCenter(); }

  /**
   * @brief 获取信号元素稳定在道路中线上的位置，以WGS84坐标系的经纬度表示
   *
   * @return Coord::txWGS84 信号元素稳定在道路中线上的经纬度坐标
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.PosOnLaneCenterLinePos();
  }

  /**
   * @brief 获取信号元素的稳定在道路上的车道信息
   *
   * @return Base::Info_Lane_t 包含信号元素在道路上的车道信息
   */
  virtual Base::Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }

  /**
   * @brief 获取信号元素的稳定在道路上的车道方向
   *
   * @return Base::txVec3 包含信号元素在道路上的车道方向
   */
  virtual Base::txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取信号元素的稳定在道路上的车道方向
   *
   * @return Unit::txDegree 包含信号元素在道路上的车道方向
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE {
    return Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());
  }

  /**
   * @brief 获取信号元素的稳定速度
   *
   * @return txFloat 返回信号元素的稳定速度
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return GetVelocity(); }

  /**
   * @brief 获取信号元素的稳定加速度
   *
   * @return txFloat 返回信号元素的稳定加速度
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return GetAcc(); }

  /**
   * @brief 获取信号元素沿着曲线的距离倒数
   *
   * @return txFloat 返回信号元素的距离倒数
   */
  virtual txFloat StableInvertDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastInvertDistanceAlongCurve();
  }

  /**
   * @brief 获取信号元素沿着曲线的距离
   *
   * @return txFloat 返回信号元素的距离
   */
  virtual txFloat StableDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastDistanceAlongCurve();
  }

  /**
   * @brief 保存信号元素的稳定状态
   *
   * 该函数用于保存信号元素的稳定状态，以便在接下来的执行过程中恢复该状态。
   * 此函数通常在信号元素的生命周期内被多次调用。
   *
   * @note 该函数必须在保存状态后才能被调用，否则可能导致未定义的行为。
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 获取当前稳定的哈希后的车道信息
   *
   * 该函数用于获取当前稳定的哈希后的车道信息。
   * 稳定的车道信息是在执行过程中保持不变的。
   *
   * @return 返回当前稳定的哈希后的车道信息
   */
  virtual const HashedLaneInfo &StableHashedLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return m_curHashedLaneInfo; }
#endif /*__TX_Mark__("ISimulationConsistency")*/

 public:
  /**
   * @brief 生成信号控制车道信息列表
   * @param map_roadid2signalId 道路ID到信号ID的映射关系
   * @return 返回生成结果，成功为true，失败为false
   */
  virtual Base::txBool GenerateSignalControlLaneInfoList(const RoadId2SignalIdSet &map_roadid2signalId) TX_NOEXCEPT;

  /**
   * @brief 检查控制相位
   *
   * @param phase
   * @return txBool
   */
  virtual txBool CheckControlPhase(txString phase) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 生成控制相位
   *
   * @param control_phase 具有格式的字符串
   */
  void GenerateControlPhases(Base::txString control_phase) TX_NOEXCEPT;

  /**
   * @brief 生成控制车道连接
   *
   * @param lanes 车道
   */
  void GenerateControlLaneLinks(const hadmap::txLanes &lanes) TX_NOEXCEPT;

 public:
  /**
   * @brief 计划当前信号控制的具体步骤
   *
   * 该函数用于获取当前信号控制的具体步骤，例如“红绿灯：绿色行驶”。
   *
   * @return 返回字符串，包含当前信号控制的具体步骤
   */
  virtual Base::txString plan() const TX_NOEXCEPT { return m_signal_control_plan; }

  /**
   * @brief 获取控制信号灯的junction信息
   *
   * @return Base::txString 字符串格式
   */
  virtual Base::txString junction() const TX_NOEXCEPT { return m_signal_control_junction; }

  /**
   * @brief 获取控制信号的编号
   *
   * @return Base::txString 字符串格式
   */
  virtual Base::txString phaseNumber() const TX_NOEXCEPT { return m_signal_control_phaseNumber; }

  /**
   * @brief 获取控制信号的head
   *
   * @return Base::txString 字符串格式
   */
  virtual Base::txString signalHead() const TX_NOEXCEPT { return m_signal_control_signalHead; }

  /**
   * @brief 获取控制信号的eventID
   *
   * @return Base::txString 字符串格式
   */
  virtual Base::txString eventId() const TX_NOEXCEPT { return m_signal_control_eventId; }

 protected:
  using ISceneLoader = Base::ISceneLoader;
  using ISignlightsViewer = ISceneLoader::ISignlightsViewer;
  using ISignlightsViewerPtr = Base::ISceneLoader::ISignlightsViewerPtr;
  using IRouteViewer = Base::ISceneLoader::IRouteViewer;
  using IRouteViewerPtr = Base::ISceneLoader::IRouteViewerPtr;
  using TAD_SignalPhasePeriod = Scene::TAD_SignalPhasePeriod;

  ISignlightsViewerPtr _elemAttrViewPtr = nullptr;
  IRouteViewerPtr _elemRouteViewPtr = nullptr;
  TAD_SignalPhasePeriod m_SignalPhasePeriod;
  // Unit::txDegree m_rot;
  // std::vector< sim_msg::Phase > m_vec_control_phases;

  // Base::txString m_signal_control_plan;
  // Base::txString m_signal_control_junction;
  // Base::txString m_signal_control_phaseNumber;
  // Base::txString m_signal_control_signalHead;
  // Base::txString m_signal_control_eventId;

  // Coord::txWGS84 m_show_wgs84;
};
using TAD_SignalLightElementPtr = std::shared_ptr<TAD_SignalLightElement>;
TX_NAMESPACE_CLOSE(TrafficFlow)
