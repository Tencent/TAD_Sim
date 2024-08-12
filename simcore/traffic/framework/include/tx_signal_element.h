// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "tx_component.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_traffic_element_base.h"

#define _CloseControlSignal_ (0)
TX_NAMESPACE_OPEN(Base)

class ISignalLightElement : public Base::ITrafficElement {
 public:
  using SIGN_LIGHT_COLOR_TYPE = Base::Enums::SIGN_LIGHT_COLOR_TYPE;
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using AdjoinContainerType = HashedLaneInfoOrthogonalList::AdjoinContainerType;

 public:
  ISignalLightElement() TX_DEFAULT;
  virtual ~ISignalLightElement() TX_DEFAULT;

  /**
   * @brief 获取当前信号元素的当前灯光颜色类型
   * @return 当前灯光颜色类型
   */
  virtual SIGN_LIGHT_COLOR_TYPE GetCurrentSignLightColorType() const TX_NOEXCEPT = 0;

 public:
  /**
   * @brief 获取信号元素的唯一标识符
   * @return 信号元素的唯一标识符
   */
  virtual txSysId Id() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取信号元素的系统标识符
   * @return 信号元素的系统标识符
   */
  virtual txSysId SysId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.SysId(); }

 public:
  /**
   * @brief 判断信号元素是否处于活跃状态
   * @return 如果信号元素处于活跃状态，则返回 true，否则返回 false
   */
  virtual txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsAlive(); }

  /**
   * @brief 检查信号元素是否已结束
   * @return 如果信号元素已结束，则返回 true，否则返回 false
   */
  virtual txBool IsEnd() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsEnd(); }

  /**
   * @brief 判断信号元素是否处于开始状态
   * @return 如果信号元素处于开始状态，则返回 true，否则返回 false
   */
  virtual txBool IsStart() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsStart(); }

  /**
   * @brief 检查信号元素是否有效
   * @return 如果信号元素有效，则返回 true，否则返回 false
   */
  virtual txBool IsValid() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsAlive(); }

  /**
   * @brief 判断信号元素是否停止
   *
   * 该函数用于判断信号元素是否停止，如果信号元素处于停止状态，则返回 true，否则返回 false。
   * @return 如果信号元素处于停止状态，则返回 true，否则返回 false。
   */
  virtual txBool IsStop() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsStop(); }

  /**
   * @brief 检查信号元素是否开始
   * @param TimeParamManager 参数管理器
   * @return txBool 是否启动成功
   *
   * 检查信号元素是否启动成功。如果启动成功，则返回 true，否则返回 false。
   */
  virtual txBool CheckStart(Base::TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE {
    mLifeCycle.SetStart();
    return IsAlive();
  }

  /**
   * @brief 检查信号元素是否结束
   *
   * 该函数用于检查信号元素是否已经结束。如果信号元素已结束，则返回 true，否则返回 false。
   * @param timeParamManager 参数管理器
   * @return 是否结束
   */
  virtual txBool CheckEnd(Base::TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsEnd(); }

  /**
   * @brief 虚函数: 信号元素开始时被调用
   *
   * 此虚函数用于扩展此类信号元素的功能。默认实现什么也不做。
   * 可以用来设置信号元素的初始状态等。
   *
   * @param timeParamManager 参数管理器
   */
  virtual void OnStart(Base::TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 信号元素结束时的回调函数。
   *
   * 此函数可以被扩展，以处理信号元素结束时的其他操作，例如，重置信号元素状态等。
   */
  virtual void OnEnd() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 信号元素结束时的回调函数。
   *
   * 此函数可以被扩展，以处理信号元素结束时的其他操作，例如，重置信号元素状态等。
   */
  virtual txBool Kill() TX_NOEXCEPT TX_OVERRIDE {
    mLifeCycle.SetStop();
    return IsAlive();
  }

 public:
  /**
   * @brief 获取信号元素的长度。
   * @return 信号元素的长度，单位为米。
   */
  virtual txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Length(); }

  /**
   * @brief 获取信号元素的宽度。
   * @return 信号元素的宽度，单位为米。
   */
  virtual txFloat GetWidth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Width(); }

  /**
   * @brief 获取信号元素的高度。
   * @return 信号元素的高度，单位为米。
   */
  virtual txFloat GetHeigth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Height(); }

  /**
   * @brief 获取信号元素的包围盒。
   * @return 返回一个引用，指向信号元素的多边形数组。
   */
  virtual const PolygonArray &Polygon() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Polygon(); }

 public:
  /**
   * @brief 获取信号元素的地理坐标。
   *
   * @return 返回一个WGS84地理坐标，表示信号元素的中心点。
   *
   */
  virtual Coord::txWGS84 GetLocation() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter(); }

  /**
   * @brief 计算信号元素在曲线上的距离
   *
   * 此函数用于计算信号元素在曲线上的距离。此距离可以用于调整信号元素的播放位置。
   *
   * @return 返回信号元素在曲线上的距离，单位为米。
   */
  virtual txFloat DistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.DistanceAlongCurve(); }

  /**
   * @brief 获取信号元素所在车道的偏移量
   *
   * 该函数用于获取信号元素所在车道的偏移量。该值通常用于调整信号元素的播放位置，以确保其与车道的中心点对齐。
   *
   * @return 返回信号元素所在车道的偏移量，单位为米
   */
  virtual txFloat LaneOffset() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneOffset(); }

  /**
   * @brief 检查信号元素是否位于车道联接区域
   *
   * 该函数用于确定信号元素是否位于车道联接区域，该区域包含了将车道改变方向所需的信号元素。
   *
   * @return 如果信号元素位于车道联接区域则返回 true，否则返回 false
   */
  virtual txBool IsOnLaneLink() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLaneLink(); }

  /**
   * @brief 检查信号元素是否位于车道中
   *
   * 该函数用于确定信号元素是否位于车道中，该区域通常用于绿灯车流通行的信号元素。
   *
   * @return 如果信号元素位于车道中则返回 true，否则返回 false
   */
  virtual txBool IsOnLane() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLane(); }

  /**
   * @brief 获取当前信号元素所在的车道信息
   *
   * 此函数用于获取当前信号元素所在的车道信息。此信息常用于描述绿灯且能够通过的车道。
   *
   * @return 当前信号元素所在的车道信息
   */
  virtual const Base::Info_Lane_t &GetCurrentLaneInfo() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LaneLocInfo();
  }

  /**
   * @brief 获取当前信号元素所在的交叉角度
   *
   * 此函数用于获取当前信号元素所在的交叉角度，用于描述绿灯信号元素可通过的车道。
   *
   * @return 当前信号元素所在的交叉角度
   */
  virtual Unit::txDegree GetCrossRot() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.CrossRot(); }

  /**
   * @brief 获取当前信号元素所在的车道方向
   *
   * 此函数用于获取当前信号元素所在的车道方向，以方便进行路径判断等操作。
   *
   * @return 当前信号元素所在的车道方向
   */
  virtual Base::txVec3 GetLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取当前信号元素的局部坐标系，包括左侧和右侧坐标的最小和最大值
   *
   * 此函数用于获取当前信号元素的局部坐标系，也就是信号元素的坐标范围。
   *
   * @return 一个包含两个元素的数组，第一个元素是左侧坐标的最小值，第二个元素是右侧坐标的最大值
   */
  virtual const std::array<Base::txVec3, 2> LocalCoords() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LocalCoords();
  }

  /**
   * @brief 获取当前信号元素的海拔高度
   *
   * 此函数用于获取当前信号元素所处的海拔高度。
   *
   * @return 返回当前信号元素的海拔高度，单位为米
   */
  virtual txFloat Altitude() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.Altitude(); }

  /**
   * @brief 获取控制阶段
   *
   * 获取信号元素的控制阶段，用于确定哪些阶段需要控制。
   *
   * @return 一个包含所有控制阶段的列表，每个阶段的编号从0开始，按照阶段的执行顺序排列。
   */
  virtual std::vector<sim_msg::Phase> GetControlPhases() const TX_NOEXCEPT { return m_vec_control_phases; }

  /**
   * @brief 检查信号元素是否可以控制该阶段
   *
   * 该函数用于检查信号元素是否可以控制指定阶段。如果可以，返回 true，否则返回 false。
   *
   * @param phase 要检查的阶段名称
   * @return 返回一个布尔值，如果信号元素可以控制该阶段，则返回 true，否则返回 false。
   */
  virtual txBool CheckControlPhase(txString phase) const TX_NOEXCEPT { return true; }

  /**
   * @brief 获取控制线链接信息
   *
   * 这个函数用于获取指定车道的控制线链接信息。
   *
   * @param _laneUid 指定车道的ID
   * @return hadmap::txLaneLinks 返回一个包含该车道的控制线链接信息的对象
   */
  virtual hadmap::txLaneLinks GetControlLaneLinks(const txLaneUId _laneUid) TX_NOEXCEPT {
    return m_map_control_lanelinks[_laneUid];
  }

 public:
  /**
   * @brief 获取信号元素的速度
   *
   * 此函数用于获取信号元素的速度。
   *
   * @return txFloat 返回信号元素的速度，以米/秒为单位
   */
  virtual txFloat GetVelocity() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取信号元素的加速度
   *
   * 此函数用于获取信号元素的加速度。
   *
   * @return txFloat 返回信号元素的加速度，以米/秒²为单位
   */
  virtual txFloat GetAcc() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取信号元素的位移量
   *
   * 此函数用于获取信号元素的位移量。
   *
   * @return txFloat 返回信号元素的位移量，以米为单位
   */
  virtual txFloat GetDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取信号元素的侧向速度
   *
   * 此函数用于获取信号元素的侧向速度。
   *
   * @return txFloat 返回信号元素的侧向速度，以米/秒为单位
   */
  virtual txFloat GetLateralVelocity() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取信号元素的侧向位移
   *
   * 此函数用于获取信号元素的侧向位移。
   *
   * @return txFloat 返回信号元素的侧向位移，以米为单位
   */
  virtual txFloat GetLateralDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief Compute_Displacement
   *
   * 此函数用于计算信号元素的位移。
   *
   * @param timeMgr
   * @return txFloat 返回信号元素的位移，单位为米。
   */
  virtual txFloat Compute_Displacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 计算信号元素的侧向位移
   *
   * 此函数用于计算信号元素的侧向位移。
   *
   * @param timeMgr 时间参数管理器
   * @return txFloat 返回信号元素的侧向位移，单位为米。
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return 0.0;
  }

 public:
  /**
   * @brief GetKineticsInfo 获取信号元素的动力学信息
   *
   * 该函数用于获取信号元素的动力学信息，包括相位、速率和位移等信息。
   *
   * @param timeMgr 时间参数管理器，用于获取当前时间和步长等参数
   * @return KineticsInfo_t 返回一个包含动力学信息的结构体，包括相位、速率和位移等信息
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const &timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取信号元素的切换车道状态
   *
   * 该函数用于获取信号元素的切换车道状态，返回值可能是直线、右转、左转等。
   *
   * @return 返回一个 VehicleMoveLaneState 枚举值，表示信号元素的切换车道状态
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return _plus_(VehicleMoveLaneState::eStraight);
  }

 protected:
  Unit::txDegree m_rot;
  std::vector<sim_msg::Phase> m_vec_control_phases;
  std::map<Base::txLaneUId, hadmap::txLaneLinks> m_map_control_lanelinks;
  txString m_signal_control_plan;
  txString m_signal_control_junction;
  txString m_signal_control_phaseNumber;
  txString m_signal_control_signalHead;
  txString m_signal_control_eventId;
  Coord::txWGS84 m_show_wgs84;

#if USE_SignalByLane

 public:
  using RoadId2SignalIdSet = std::map<Base::txRoadID, std::set<Base::txSysId>>;
  struct SignalControlLaneOp {
    SignalControlLaneOp(const RoadId2SignalIdSet &_map, const std::set<Base::txRoadID> _ignoreRoadIdSet)
        : map_roadid2signalId(_map), ignoreRoadIdSet(_ignoreRoadIdSet) {}

    Base::txBool check(const Base::txRoadID beCheckRoadId) const {
      if (_Contain_(ignoreRoadIdSet, beCheckRoadId)) {
        TX_MARK("beCheckRoadId is ignoreRoadId, whereis dst signal id in this road. continue iterator.");
        return true;
      } else {
        if (_Contain_(map_roadid2signalId, beCheckRoadId)) {
          TX_MARK("road id beCheckRoadId, which contain signals. finish iterator.");
          return false;
        } else {
          TX_MARK("current road id do not contain signal. continue iterator.");
          return true;
        }
      }
    }

   protected:
    const RoadId2SignalIdSet &map_roadid2signalId;
    const std::set<Base::txRoadID> ignoreRoadIdSet /*maybe set*/;
  };

  /**
   * @brief GenerateSignalByLaneCache 根据指定的车道信息生成信号
   *
   * 此函数根据给定的车道信息生成一个信号位置。根据指定的车道信息，绕道查询相应的车道信息，通过车道信息获取信号位置。
   *
   * @param control_lane 需要生成信号的车道字符串
   * @param signalLoc 指定信号所在的绕道车道信息ID
   * @param _Control_laneUid_one_ring 存放所有相同车道的排列位置
   * @param signalPosition 返回信号位置
   * @return 返回true表示生成成功，返回false表示生成失败
   */
  virtual Base::txBool GenerateSignalByLaneCache(Base::txString control_lane, const Base::txLaneUId signalLoc,
                                                 hadmap::txLanes &_Control_laneUid_one_ring,
                                                 Coord::txWGS84 &signalPosition) TX_NOEXCEPT;

  /**
   * @brief 通过绕道车道信息生成信号位置
   *
   * 根据指定的绕道车道信息，通过车道信息获取信号位置。
   *
   * @param _Control_laneUid_one_ring 指定的绕道车道信息
   * @param map_roadid2signalId 存储绕道车道信息与对应信号ID的映射关系
   * @return 返回生成信号位置
   */
  virtual void GenerateSignalByLaneCache_Internal(const hadmap::txLanes &_Control_laneUid_one_ring,
                                                  const RoadId2SignalIdSet &map_roadid2signalId) TX_NOEXCEPT;
  /*!
   @brief    Public signals to veh within a certain distance of the upstream lane of the signal light
  */
  virtual void PublicSignals(const Base::TimeParamManager &) TX_NOEXCEPT;
  AdjoinContainerType mControlOrthogonalNodeMap;
  std::vector<std::pair<Base::txLaneUId, hadmap::txPoint>> m_control_lane_set_from_conf;
  std::unordered_set<Base::Info_Lane_t, Utils::Info_Lane_t_HashCompare> m_ControlLaneHashSet;
#endif /*USE_SignalByLane*/
#if USE_HashedRoadNetwork

 protected:
  /**
   * @brief 初始化组件
   *
   * 在组件初始化过程中，通过该函数进行注册信息。
   *
   * @param _location 组件所在的位置
   */
  virtual void RegisterInfoOnInit(const Component::Location &_location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 更新哈希键的车道信息
   *
   * 在每次组件更新时，都会将其当前位置信息更新至已有的车道信息中。
   *
   * @param _location 当前组件所在的位置
   */
  virtual void UpdateHashedLaneInfo(const Component::Location &_location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 注销信息的操作
   *
   * 此函数用于在某些情况下，从信号系统中注销某些信息。
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE {}
#endif /*USE_HashedRoadNetwork*/

 public:
  /**
   * @brief 调整跟踪器的位置
   * @param pLane 需要跟踪的车道指针
   * @param _timestamp 当前的时间戳
   * @return 是否成功重定位跟踪器
   */
  virtual Base::txBool RelocateTracker(hadmap::txLanePtr pLane, const txFloat &_timestamp) TX_NOEXCEPT {
    return mLocation.RelocateTracker(pLane, _timestamp);
  }

  /**
   * @brief 使用给定的时间戳重新定位跟踪器
   * @param pLaneLink 车道链接指针
   * @param _timestamp 当前的时间戳
   * @return 是否成功重定位跟踪器
   */
  virtual Base::txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink, const txFloat &_timestamp) TX_NOEXCEPT {
    return mLocation.RelocateTracker(pLaneLink, _timestamp);
  }

 protected:
  Base::Component::Identity mIdentity;
  Base::Component::GeometryData mGeometryData;
  Base::Component::Location mLocation;
  Base::Component::LifeCycle mLifeCycle;
};

using ISignalLightElementPtr = std::shared_ptr<ISignalLightElement>;
TX_NAMESPACE_CLOSE(Base)
