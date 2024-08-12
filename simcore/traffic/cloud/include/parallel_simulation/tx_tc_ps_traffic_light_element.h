// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_signal_phase_period.h"
#include "tx_header.h"
#include "tx_signal_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class PS_SignalLightElement : public Base::ISignalLightElement {
 public:
  PS_SignalLightElement() TX_DEFAULT;
  virtual ~PS_SignalLightElement() TX_DEFAULT;

  /**
   * @brief 初始化交通灯元素
   *
   * 初始化交通灯元素的状态为待评估状态。
   *
   * @param[in] viewerPtr 交通灯元素的数据和呈现
   * @param[in] sceneLoaderPtr 数据加载器，用于获取交通灯元素的数据
   * @return true 初始化成功
   * @return false 初始化失败，需要重试
   */
  virtual Base::txBool Initialize(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }

  /**
   * @brief 更新元素
   *
   * 更新当前元素的状态，如需要则执行相应的更新操作。
   *
   * @param[in] param 时间相关参数管理器
   * @return true 更新成功
   * @return false 更新失败，需要重试
   */
  virtual Base::txBool Update(const Base::TimeParamManager&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放资源
   *
   * 释放与此对象相关的所有资源。
   *
   * @return true 成功释放资源
   * @return false 未能释放资源，需要重试
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 对特定信号实现的元素进行填充操作
   *
   * @param timeMgr 当前信号相关时间参数管理器
   * @param traffic 用于填充的空间查询对象
   *
   * @return true：成功进行填充操作。false：未成功进行填充操作。
   */
  virtual Base::txBool FillingElement(Base::TimeParamManager const& timeMgr, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 用于填充空间查询
   *
   * 在这个函数中，我们决定是否需要为这个元素填充空间查询。
   *
   * @return true 表示需要填充空间查询
   * @return false 表示不需要填充空间查询
   */
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 初始化并行模拟
   *
   * 使用交通信号灯ID和交通网络拓扑结构，初始化并行模拟。
   *
   * @param light_id 交通信号灯ID
   * @param vecRoadTopo 交通网络拓扑结构，包含所有道路的起点和终点
   * @return true 初始化成功
   * @return false 初始化失败
   */
  virtual Base::txBool Initialize_ParallelSimulation(
      const Base::txSysId light_id,
      const std::vector<std::tuple<Base::txRoadID, Base::txRoadID> >& vecRoadTopo) TX_NOEXCEPT;

  /**
   * @brief 更新并行模拟
   *
   * 使用交通信号灯颜色更新并行模拟。
   *
   * @param _color 交通信号灯颜色
   * @return true 更新成功
   * @return false 更新失败
   */
  virtual Base::txBool Update_ParallelSimulation(const SIGN_LIGHT_COLOR_TYPE _color) TX_NOEXCEPT;

  /**
   * @brief 获取元素类型
   *
   * 返回当前元素的类型
   *
   * @return ElementType 元素类型
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::PS_SignalLight; }

  /**
   * @brief 获取元素类型的字符串表示
   *
   * 返回当前元素的类型的字符串表示形式，用于展示和调试
   *
   * @return String 元素类型的字符串表示
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE { return "PS_SignalLight"; }

  /**
   * @brief 获取当前信号灯颜色类型
   *
   * 返回当前信号灯的颜色类型，以便展示和调试
   *
   * @return SIGN_LIGHT_COLOR_TYPE 当前信号灯颜色类型
   */
  virtual Base::ISignalLightElement::SIGN_LIGHT_COLOR_TYPE GetCurrentSignLightColorType() const TX_NOEXCEPT
      TX_OVERRIDE {
    return t_traffic_light_color;
  }
#if __TX_Mark__("ISimulationConsistency")

 public:
  /**
   * @brief 获取当前信号灯元素的一致性ID
   *
   * 返回当前信号灯元素的一致性ID，以便进行子类对象之间的一致性检查
   *
   * @return txSysId 当前信号灯元素的一致性ID
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取当前信号灯元素的稳定轴心位置
   *
   * 返回当前信号灯元素的稳定轴心位置，该位置不受信号灯元素在网络中的移动影响，通常用于进行子类对象之间的一致性检查
   *
   * @return Coord::txENU 当前信号灯元素的稳定轴心位置
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter().GetENU(); }

  /**
   * @brief 获取当前元素稳定的后轴中心
   *
   * @return Coord::txENU 当前信号灯元素的稳定后中心位置
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return StableGeomCenter(); }

  /**
   * @brief 获取当前信号灯元素在道路上的稳定位置
   *
   * 返回当前信号灯元素在道路上的稳定位置，即当前信号灯元素在路上中心线的位置，该位置不受信号灯元素在网络中的移动影响，通常用于进行子类对象之间的一致性检查
   *
   * @return Coord::txWGS84 当前信号灯元素在道路上的稳定位置
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.PosOnLaneCenterLinePos();
  }

  /**
   * @brief 获取当前信号灯元素的稳定的车道信息
   *
   * 返回当前信号灯元素的稳定车道信息，即该元素所在车道的方向。该方向信息不受信号灯元素在网络中的移动影响，通常用于进行子类对象之间的一致性检查。
   *
   * @return Base::Info_Lane_t 当前信号灯元素的稳定车道信息
   */
  virtual Base::Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }

  /**
   * @brief 获取当前元素所在lane的方向
   *
   * @return Base::txVec3 当前信号灯元素的稳定车道方向信息
   */
  virtual Base::txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取当前元素的稳定朝向
   *
   * @return Unit::txDegree 角度
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE {
    return Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());
  }

  /**
   * @brief 获取当前元素稳定的速度
   *
   * @return txFloat 稳定速度返回
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return GetVelocity(); }

  /**
   * @brief 获取当前元素稳定的加速度
   *
   * @return txFloat 返回稳定的加速度
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return GetAcc(); }

  /**
   * @brief 获取当前元素沿曲线的稳定距离倒数
   *
   * @return txFloat 返回的距离倒数
   */
  virtual txFloat StableInvertDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastInvertDistanceAlongCurve();
  }

  /**
   * @brief 获取当前信号灯元素的稳定离路径起点的距离
   *
   * 此函数返回当前信号灯元素与路径起点之间的距离，可以保证该值不随信号灯元素在网络中的移动而变化。
   *
   * @return txFloat 当前信号灯元素与路径起点之间的距离
   */
  virtual txFloat StableDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastDistanceAlongCurve();
  }

  /**
   * @brief 保存当前元素的稳定状态
   *
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE {}
#endif /*__TX_Mark__("ISimulationConsistency")*/

 public:
  /**
   * @brief 获取稳定的hash车道信息
   *
   * @return const HashedLaneInfo& 返回的稳定车道信息
   */
  virtual const HashedLaneInfo& StableHashedLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return m_curHashedLaneInfo; }

  /**
   * @brief 根据道路与信号灯的关联关系生成信号灯控制车道信息列表
   *
   * 此函数根据传入的一个关联关系映射表（map_roadid2signalId），在其中查找各道路对应的信号灯ID，然后将这些信号灯ID与车道信息关联起来，生成一个列表供后续处理。
   *
   * @param map_roadid2signalId 道路与信号灯ID的关联关系映射表
   * @return Base::txBool 函数执行成功返回 true，否则返回 false
   */
  virtual Base::txBool GenerateSignalControlLaneInfoList(const RoadId2SignalIdSet& map_roadid2signalId) TX_NOEXCEPT;

 protected:
  SIGN_LIGHT_COLOR_TYPE t_traffic_light_color = +SIGN_LIGHT_COLOR_TYPE::eGreen;
  hadmap::txLanes one_ring_lanes;
};

using PS_SignalLightElementPtr = std::shared_ptr<PS_SignalLightElement>;
TX_NAMESPACE_CLOSE(TrafficFlow)
