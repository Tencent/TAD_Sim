// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <vector>
#include "HdMap/tx_road_network.h"
#include "tx_dead_line_area_manager.h"
#include "tx_enum_def.h"
#include "tx_hadmap_utils.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_map_tracker.h"
#include "tx_obb.h"
#include "tx_serialization.h"
#if USE_HashedRoadNetwork
#  include "HdMap/tx_hashed_lane_info.h"
#endif /*USE_HashedRoadNetwork*/

TX_NAMESPACE_OPEN(Base)

class IVehicleElement;
class IPedestrianElement;
class ISignalLightElement;

TX_NAMESPACE_OPEN(Component)

// @brief 仿真组件基类
class txComponent {
 public:
  virtual ~txComponent() TX_DEFAULT;
};

// @brief 身份标识基类
class Identity : public txComponent {
 public:
  Identity() TX_DEFAULT;
  ~Identity() TX_DEFAULT;

  /**
   * @brief 获取当前的系统唯一id
   *
   * @return Base::txSysId
   */
  Base::txSysId SysId() const TX_NOEXCEPT { return m_sysId; }
  Base::txSysId& SysId() TX_NOEXCEPT { return m_sysId; }

  /**
   * @brief 当前的系统id是否有效
   *
   * @return Base::txBool
   */
  Base::txBool IsSysIdInited() const TX_NOEXCEPT { return FLAGS_invalid_id != m_sysId; }

  /**
   * @brief 获取当前类型中分配的id
   *
   * @return Base::txSysId
   */
  Base::txSysId Id() const TX_NOEXCEPT { return m_id; }
  Base::txSysId& Id() TX_NOEXCEPT { return m_id; }

  Base::txString Name() const TX_NOEXCEPT { return m_name; }
  Base::txString& Name() TX_NOEXCEPT { return m_name; }

  /**
   * @brief 当前对象的描述
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT { return ""; }
  friend std::ostream& operator<<(std::ostream& os, const Identity& v) TX_NOEXCEPT {
    os << "{" << TX_VARS_NAME(Id, v.Id()) << TX_VARS_NAME(SysId, v.SysId()) << TX_VARS_NAME(Name, v.Name()) << "}";
    return os;
  }

  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("SysId", m_sysId), _MAKE_NVP_("Id", m_id));
  }

 protected:
  Base::txSysId m_sysId = FLAGS_invalid_id;
  Base::txSysId m_id = -1;
  Base::txString m_name = "";
};

// @brief 位置信息
class Location : public txComponent {
  using txFloat = Base::txFloat;
  using txVec3 = Base::txVec3;
  using txPoint = hadmap::txPoint;
  using txUInt = Base::txUInt;
  using txULong = Base::txULong;
  using txWGS84 = Coord::txWGS84;
  using txENU = Coord::txENU;
  using txBool = Base::txBool;
  using txLaneID = Base::txLaneID;

 public:
  static Base::txUInt GetLaneCount(const Base::txRoadID roadId, const Base::txSectionID sectionId) TX_NOEXCEPT;
  static Info_Lane_t GetCurrentLaneInfoByParam(const HdMap::MapTrackerPtr pTracker, Base::txBool& bCurInLaneLinkState,
                                               Unit::txDegree& crossRot) TX_NOEXCEPT;

 public:
  /*txWGS84& vPos() TX_NOEXCEPT { return m_vPos; }
  const txWGS84& vPos() const TX_NOEXCEPT { return m_vPos; }*/

  /**
   * @brief 几何中心的WGS坐标
   *
   * @return txWGS84&
   */
  txWGS84& GeomCenter() TX_NOEXCEPT { return m_geom_center; }
  const txWGS84& GeomCenter() const TX_NOEXCEPT { return m_geom_center; }

  /**
   * @brief 获取m_rear_axle_center
   *
   * @return txWGS84&
   */
  txWGS84& RearAxleCenter() TX_NOEXCEPT { return m_rear_axle_center; }
  const txWGS84& RearAxleCenter() const TX_NOEXCEPT { return m_rear_axle_center; }

  /**
   * @brief 获取元素位于车道中心线上点的经纬度
   *
   * @return txWGS84&
   */
  txWGS84& PosOnLaneCenterLinePos() TX_NOEXCEPT { return m_OnLanePos; }
  const txWGS84& PosOnLaneCenterLinePos() const TX_NOEXCEPT { return m_OnLanePos; }

  /**
   * @brief 获取m_PosWithoutOffset
   *
   * @return txWGS84&
   */
  txWGS84& PosWithLateralWithoutOffset() TX_NOEXCEPT { return m_PosWithoutOffset; }
  const txWGS84& PosWithLateralWithoutOffset() const TX_NOEXCEPT { return m_PosWithoutOffset; }

  /**
   * @brief 获取ENU下当前元素的方向
   *
   * @return Unit::txDegree
   */
  Unit::txDegree heading() const TX_NOEXCEPT { return Utils::GetLaneAngleFromVectorOnENU(vLaneDir()) + fAngle(); }
  Unit::txDegree heading_for_front_region_on_ENU() const TX_NOEXCEPT { return heading(); }
  Unit::txDegree rot_for_display() const TX_NOEXCEPT {
    return heading(); /* Utils::DisplayerGetLaneAngleFromVector(vLaneDir()) + fAngle();*/
  }

  /**
   * @brief 获取车道方向
   *
   * @return txVec3&
   */
  txVec3& vLaneDir() TX_NOEXCEPT { return m_vLaneDir; }
  const txVec3& vLaneDir() const TX_NOEXCEPT { return m_vLaneDir; }

  /**
   * @brief 获取角度值的引用
   *
   * 通过返回角度值的引用，可以方便地更改该角度值。
   *
   * @return Unit::txDegree& 角度值的引用
   */
  Unit::txDegree& fAngle() TX_NOEXCEPT { return m_fAngle; }
  const Unit::txDegree& fAngle() const TX_NOEXCEPT { return m_fAngle; }

  /**
   * @brief 获取本地坐标系中 X 轴的坐标向量
   *
   * 返回一个坐标向量，表示本地坐标系中 X 轴的方向。
   *
   * @return txVec3 本地坐标系中 X 轴的坐标向量
   */
  txVec3& LocalCoord_AxisX() TX_NOEXCEPT { return m_vLocalCoord[LocalCoord_X]; }
  const txVec3& LocalCoord_AxisX() const TX_NOEXCEPT { return m_vLocalCoord[LocalCoord_X]; }

  /**
   * @brief 获取本地坐标系中 Y 轴的坐标向量
   *
   * 返回一个坐标向量，表示本地坐标系中 Y 轴的方向。
   *
   * @return txVec3 本地坐标系中 Y 轴的坐标向量
   */
  txVec3& LocalCoord_AxisY() TX_NOEXCEPT { return m_vLocalCoord[LocalCoord_Y]; }
  const txVec3& LocalCoord_AxisY() const TX_NOEXCEPT { return m_vLocalCoord[LocalCoord_Y]; }

  /**
   * @brief 获取本地坐标系的两个坐标向量
   *
   * 返回一个包含两个元素的数组，分别代表本地坐标系中 X 轴和 Y 轴的坐标向量。
   *
   * @return std::array< txVec3, 2 > 本地坐标系的两个坐标向量
   */
  const std::array<txVec3, 2> LocalCoords() const TX_NOEXCEPT { return m_vLocalCoord; }

  /**
   * @brief 获取当前高度值
   *
   * 返回一个float类型的数值，表示当前组件的高度值。
   *
   * @return txFloat 当前组件的高度值
   */
  txFloat Altitude() const TX_NOEXCEPT;

  /**
   * @brief 获取交叉旋转角度
   *
   * 返回一个Unit::txDegree类型的值，表示组件的交叉旋转角度。
   *
   * @return Unit::txDegree 组件的交叉旋转角度
   */
  Unit::txDegree& CrossRot() TX_NOEXCEPT { return m_CrossRot; }
  const Unit::txDegree& CrossRot() const TX_NOEXCEPT { return m_CrossRot; }
  // Base::txLaneID& CurLaneIndex() TX_NOEXCEPT { return m_curLaneIndex; }

  /**
   * @brief 获取当前组件所在的车道索引
   *
   * 返回一个Base::txLaneID类型的值，表示当前组件所在的车道索引。
   *
   * @return Base::txLaneID 当前组件所在的车道索引
   */
  const Base::txLaneID CurLaneIndex() const TX_NOEXCEPT { return Utils::ComputeLaneIndex(m_laneInfo.onLaneUid.laneId); }

  /**
   * @brief 获取下一个车道的索引
   *
   * @return Base::txLaneID&
   */
  Base::txLaneID& NextLaneIndex() TX_NOEXCEPT { return m_nextLaneIndex; }
  const Base::txLaneID NextLaneIndex() const TX_NOEXCEPT { return m_nextLaneIndex; }

  /**
   * @brief 获取当前组件所在的车道信息
   *
   * 返回一个Info_Lane_t类型的值，表示当前组件所在的车道信息。
   *
   * @return Info_Lane_t 当前组件所在的车道信息
   */
  Info_Lane_t& LaneLocInfo() TX_NOEXCEPT { return m_laneInfo; }
  const Info_Lane_t& LaneLocInfo() const TX_NOEXCEPT { return m_laneInfo; }

  /**
   * @brief 检查组件是否在车道内
   *
   * 返回一个布尔值，表示组件是否在车道内。
   *
   * @return txBool 组件是否在车道内
   */
  const txBool IsOnLane() const TX_NOEXCEPT { return !(IsOnLaneLink()); }

  /**
   * @brief 检查组件是否在车道连接处
   *
   * 返回一个布尔值，表示组件是否在车道连接处。
   *
   * @return txBool 组件是否在车道连接处
   */
  const txBool IsOnLaneLink() const TX_NOEXCEPT { return m_laneInfo.isOnLaneLink; }

  /**
   * @brief 更新车道位置信息
   *
   * 该方法用于更新当前组件所在的车道信息。
   *
   */
  void UpdateLaneLocInfo() TX_NOEXCEPT;

  /**
   * @brief 获取当前组件沿着曲线的距离
   *
   * 该方法返回当前组件在曲线上的位置。
   *
   * @return txFloat& 当前组件沿着曲线的距离
   */
  txFloat& DistanceAlongCurve() TX_NOEXCEPT { return m_distanceAlongCurve; }
  const txFloat DistanceAlongCurve() const TX_NOEXCEPT { return m_distanceAlongCurve; }

  /**
   * @brief 获取当前组件的车道偏移量
   *
   * 该方法返回当前组件与车道中心的偏移量。
   *
   * @return txFloat& 当前组件与车道中心的偏移量
   */
  txFloat& LaneOffset() TX_NOEXCEPT { return m_l_offset; }
  const txFloat LaneOffset() const TX_NOEXCEPT { return m_l_offset; }

  /**
   * @brief 获取当前组件的中心线偏移量
   *
   * 该方法返回当前组件与车道中心的偏移量。
   *
   * @return txFloat& 当前组件与车道中心的偏移量
   */
  txFloat& CenterLineOffset() TX_NOEXCEPT { return m_centerline_offset; }
  const txFloat CenterLineOffset() const TX_NOEXCEPT { return m_centerline_offset; }

  /**
   * @brief 判断当前组件是否处于最终车道
   *
   * 该方法返回一个布尔值，指示当前组件是否处于最终车道。
   *
   * @return txBool& 一个布尔值引用，表示当前组件是否处于最终车道
   */
  txBool& IsOnFinalLane() TX_NOEXCEPT { return m_bInFinalLane; }
  const txBool IsOnFinalLane() const TX_NOEXCEPT { return m_bInFinalLane; }

  /**
   * @brief 获取当前组件是否处于边界车道状态
   *
   * 该方法返回一个布尔值引用，表示当前组件是否处于边界车道状态。
   *
   * @return txBool& 一个布尔值引用，表示当前组件是否处于边界车道状态
   */
  txBool& IsOnBoundaryLane() TX_NOEXCEPT { return m_bInBoundaryLane; }
  const txBool IsOnBoundaryLane() const TX_NOEXCEPT { return m_bInBoundaryLane; }

  /**
   * @brief 获取当前组件所追踪的位置器
   *
   * 该方法返回一个 HdMap::MapTrackerPtr 类型的引用，表示当前组件所追踪的位置器。
   *
   * @return const HdMap::MapTrackerPtr& 一个指向 HdMap::MapTrackerPtr 的常量引用
   */
  const HdMap::MapTrackerPtr tracker() const TX_NOEXCEPT { return m_tracker; }

  /**
   * @brief 初始化组件位置器
   *
   * 初始化 HdMap::MapTracker 对象，并为指定 ID 的组件分配一个新的位置器对象。如果指定的 ID
   * 对应的组件不存在或者组件已有位置器，则返回 false。
   *
   * @param _id 组件 ID
   * @return txBool 如果初始化成功，返回 true；否则返回 false
   */
  txBool InitTracker(const txInt _id) TX_NOEXCEPT;

 protected:
  friend class Base::IVehicleElement;
  friend class Base::IPedestrianElement;
  friend class Base::ISignalLightElement;

  /**
   * @brief 关于车道对象的信息更新和地图同步处理
   *
   * 此函数根据当前传入的时间戳检测指定时间范围内的交通状态更新，同时更新存储在车道对象中的数据。
   * 函数会检查组件是否已分配位置器。如果是，则更新位置器。在函数执行过程中，还会检查新的交通状态更新是否涉及到不同的车道，
   * 如果有涉及到不同车道的更新，则调用 `RelocateTracker()` 函数更新指定车道的位置器。
   *
   * @param pLane 指向要更新的车道的指针
   * @param _timestamp 要更新的时间戳
   * @return txBool 如果更新成功，则返回 true；否则返回 false
   */
  txBool RelocateTracker(hadmap::txLanePtr pLane, const txFloat& _timestamp) TX_NOEXCEPT;

  /**
   * @brief 处理车道关联的更新和同步。
   *
   * 当给定的时间戳范围内有交通状态更新发生时，此函数根据新的交通状态更新指定车道。
   * 此外，函数还会检查组件是否已分配位置器。如果是，则更新位置器。在函数执行过程中，还会检查新的交通状态更新是否涉及到不同的车道，
   * 如果有涉及到不同车道的更新，则调用 `RelocateTracker()` 函数更新指定车道的位置器。
   *
   * @param pLaneLink 指向要更新的车道关联指针
   * @param _timestamp 要更新的时间戳
   * @return txBool 如果更新成功，则返回 true；否则返回 false
   */
  txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink, const txFloat& _timestamp) TX_NOEXCEPT;

 public:
  /**
   * @brief Set the On Lane Link Without Link Id object
   *
   * @param isOnLink 设置的值
   */
  void SetOnLaneLinkWithoutLinkId(Base::txBool isOnLink) TX_NOEXCEPT { m_laneInfo.isOnLaneLink = isOnLink; }

  /**
   * @brief 检查是否在最左侧车道
   *
   * @return txBool 处于最左侧车道返回true
   */
  txBool IsOnLeftestLane() const TX_NOEXCEPT;

  /**
   * @brief 检查是否在最右侧车道
   *
   * @return txBool 处于最右侧车道返回true
   */
  txBool IsOnRightestLane() const TX_NOEXCEPT;

  /**
   * @brief 返回对象的字符串表示
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const Location& v) TX_NOEXCEPT {
    os << "{" << v.Str() << "}";
    return os;
  }

  /**
   * @brief 更新新移动点位置
   *
   * 更新移动点位置，不带偏移量。
   *
   * @return void
   */
  void UpdateNewMovePointWithoutOffset() TX_NOEXCEPT;
#if USE_HashedRoadNetwork
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using HashedLaneInfoPtr = Geometry::SpatialQuery::HashedLaneInfoPtr;

  /**
   * @brief 获取哈希化的车道信息
   *
   * 根据给定的元素ID，获取该元素对应的哈希化车道信息。
   *
   * @param _elemId 给定的元素ID
   * @return 返回哈希化的车道信息
   */
  HashedLaneInfo GetHashedLaneInfo(const Base::txSysId _elemId) const TX_NOEXCEPT;

  /**
   * @brief 获取左侧的哈希车道信息
   *
   * 根据给定的元素ID，获取该元素左侧的哈希车道信息。
   *
   * @param _elemId 给定的元素ID
   * @return 返回左侧的哈希车道信息
   */
  HashedLaneInfo GetLeftHashedLaneInfo(const Base::txSysId _elemId) const TX_NOEXCEPT;

  /**
   * @brief 获取右侧的哈希车道信息
   *
   * 根据给定的元素ID，获取该元素右侧的哈希车道信息。
   *
   * @param _elemId 给定的元素ID
   * @return 返回右侧的哈希车道信息
   */
  HashedLaneInfo GetRightHashedLaneInfo(const Base::txSysId _elemId) const TX_NOEXCEPT;
#endif /*USE_HashedRoadNetwork*/

 public:
  /*const txENU& LastPosition() const TX_NOEXCEPT { return m_lastPos; }*/

  /**
   * @brief 获取最后一次行驶的道路信息
   *
   * 返回一个Info_Lane_t类型的值，表示最后一次行驶时的道路信息。
   *
   * @return 最后一次行驶的道路信息
   */
  const Info_Lane_t& LastLaneInfo() const TX_NOEXCEPT { return m_last_laneInfo; }

  /**
   * @brief 获取最后一次行驶的道路方向
   *
   * 返回一个txVec3类型的值，表示最后一次行驶时的道路方向。
   *
   * @return 最后一次行驶的道路方向
   */
  const txVec3& LastLaneDir() const TX_NOEXCEPT { return m_last_vLaneDir; }
  /*txENU& LastPosition() TX_NOEXCEPT { return m_lastPos; }*/

  /**
   * @brief 获取最后一次道路信息
   *
   * 返回一个Info_Lane_t类型的值，表示最后一次行驶时的道路信息。
   *
   * @return 最后一次道路信息
   */
  Info_Lane_t& LastLaneInfo() TX_NOEXCEPT { return m_last_laneInfo; }

  /**
   * @brief 获取最后一次道路方向
   *
   * 返回一个txVec3类型的值，表示最后一次行驶时的道路方向。
   *
   * @return 最后一次道路方向
   */
  txVec3& LastLaneDir() TX_NOEXCEPT { return m_last_vLaneDir; }

  /**
   * @brief 获取最后一次朝向
   *
   * 返回一个Unit::txDegree类型的值，表示最后一次行驶时的朝向。
   *
   * @return 最后一次朝向
   */
  const Unit::txDegree& LastHeading() const TX_NOEXCEPT { return m_last_heading; }
  Unit::txDegree& LastHeading() TX_NOEXCEPT { return m_last_heading; }

  /**
   * @brief 获取最后一次沿车道位置
   *
   * 返回一个txWGS84类型的结构体，表示最后一次沿着指定车道行驶的坐标点的地理坐标。
   *
   * @return 最后一次沿车道位置
   */
  const txWGS84& LastOnLanePos() const TX_NOEXCEPT { return m_lastOnLanePos; }
  txWGS84& LastOnLanePos() TX_NOEXCEPT { return m_lastOnLanePos; }

  /**
   * @brief 获取当前车辆在车道连接上的时间戳
   *
   * 返回当前车辆在车道连接上的时间戳，该值用于识别车道内不同车辆的相对位置。
   *
   * @return 当前车辆在车道连接上的时间戳
   */
  txFloat TimeStampOnLanelink() const TX_NOEXCEPT { return m_TimeStampOnLanelink; }

  /**
   * @brief 获取上次的几何中心
   *
   * @return const txENU& enu坐标点
   */
  const txENU& LastGeomCenter() const TX_NOEXCEPT { return m_last_geom_center; }
  txENU& LastGeomCenter() TX_NOEXCEPT { return m_last_geom_center; }

  /**
   * @brief 获取最后一次的后轴中心
   *
   * 获取后轴中心。
   *
   * @return 返回后轴中心enu坐标点
   */
  const txENU& LastRearAxleCenter() const TX_NOEXCEPT { return m_last_rear_axle_center; }
  txENU& LastRearAxleCenter() TX_NOEXCEPT { return m_last_rear_axle_center; }

  /**
   * @brief 获取沿着曲线的距离倒数
   *
   * @return const txFloat 距离
   */
  const txFloat LastInvertDistanceAlongCurve() const TX_NOEXCEPT { return m_last_InvertDistanceAlongCurve; }
  txFloat& LastInvertDistanceAlongCurve() TX_NOEXCEPT { return m_last_InvertDistanceAlongCurve; }

  /**
   * @brief 获取上次沿着曲线的距离
   *
   * @return const txFloat 距离
   */
  const txFloat LastDistanceAlongCurve() const TX_NOEXCEPT { return m_last_DistanceAlongCurve; }
  txFloat& LastDistanceAlongCurve() TX_NOEXCEPT { return m_last_DistanceAlongCurve; }

 protected:
  /*txWGS84 m_vPos;
  txENU m_lastPos;*/
  txWGS84 m_geom_center;
  txWGS84 m_rear_axle_center;
  txENU m_last_geom_center;
  txENU m_last_rear_axle_center;
  Unit::txDegree m_last_heading;
  txWGS84 m_OnLanePos;
  txWGS84 m_lastOnLanePos;
  txWGS84 m_PosWithoutOffset;
  txVec3 m_vLaneDir /*= Utils::Axis_Enu_Up()*/;
  txVec3 m_last_vLaneDir /*= Utils::Axis_Enu_Up()*/;
  Unit::txDegree m_fAngle;
  Unit::txDegree m_CrossRot;
  Base::txLaneID m_curLaneIndex = 0;
  Base::txLaneID m_nextLaneIndex = 0;
  Info_Lane_t m_laneInfo;
  Info_Lane_t m_last_laneInfo;
  txFloat m_distanceAlongCurve = 0.0;
  TX_MARK("状态很难保持，累积会有误差");
  txFloat m_l_offset = 0.0;
  txFloat m_centerline_offset = 0.0;
  txBool m_bInFinalLane = false;
  txBool m_bInBoundaryLane = false;
  HdMap::MapTrackerPtr m_tracker = nullptr;
  enum { LocalCoord_X = 0, LocalCoord_Y = 1 };
  std::array<txVec3, 2> m_vLocalCoord; /*0 = LocalCoord_X; 1 = LocalCoord_Y*/
  Base::txFloat m_TimeStampOnLanelink = 0.0;
  Base::txFloat m_last_DistanceAlongCurve = 0.0;
  Base::txFloat m_last_InvertDistanceAlongCurve = 0.0;

 public:
  template <class Archive>
  void save(Archive& archive) const {
    /*archive(_MAKE_NVP_("vPos", m_vPos));
    archive(_MAKE_NVP_("lastPos", m_lastPos));*/
    archive(_MAKE_NVP_("geom_center", m_geom_center));
    archive(_MAKE_NVP_("last_geom_center", m_last_geom_center));
    archive(_MAKE_NVP_("rear_axle_center", m_rear_axle_center));
    archive(_MAKE_NVP_("last_rear_axle_center", m_last_rear_axle_center));

    archive(_MAKE_NVP_("OnLanePos", m_OnLanePos));
    archive(_MAKE_NVP_("lastOnLanePos", m_lastOnLanePos));
    archive(_MAKE_NVP_("PosWithoutOffset", m_PosWithoutOffset));

    archive(_MAKE_NVP_("vLaneDir", m_vLaneDir));
    archive(_MAKE_NVP_("last_vLaneDir", m_last_vLaneDir));
    archive(_MAKE_NVP_("fAngle", m_fAngle));
    archive(_MAKE_NVP_("CrossRot", m_CrossRot));
    archive(_MAKE_NVP_("curLaneIndex", m_curLaneIndex));
    archive(_MAKE_NVP_("nextLaneIndex", m_nextLaneIndex));
    archive(_MAKE_NVP_("laneInfo", m_laneInfo));
    archive(_MAKE_NVP_("last_laneInfo", m_last_laneInfo));
    archive(_MAKE_NVP_("distanceAlongCurve", m_distanceAlongCurve));
    archive(_MAKE_NVP_("l_offset", m_l_offset));
    archive(_MAKE_NVP_("centerline_offset", m_centerline_offset));
    archive(_MAKE_NVP_("bInFinalLane", m_bInFinalLane));

    archive(_MAKE_NVP_("bInBoundaryLane", m_bInBoundaryLane));
    if (NonNull_Pointer(m_tracker)) {
      archive(_MAKE_NVP_("m_tracker_is_not_nullptr", true));
      archive(_MAKE_NVP_("m_tracker", (*m_tracker)));
    } else {
      archive(_MAKE_NVP_("m_tracker_is_not_nullptr", false));
    }
    archive(_MAKE_NVP_("vLocalCoord", m_vLocalCoord));
    archive(_MAKE_NVP_("TimeStampOnLanelink", m_TimeStampOnLanelink));
  }

  template <class Archive>
  void load(Archive& archive) {
    /*archive(_MAKE_NVP_("vPos", m_vPos));
    archive(_MAKE_NVP_("lastPos", m_lastPos));*/
    archive(_MAKE_NVP_("geom_center", m_geom_center));
    archive(_MAKE_NVP_("last_geom_center", m_last_geom_center));
    archive(_MAKE_NVP_("rear_axle_center", m_rear_axle_center));
    archive(_MAKE_NVP_("last_rear_axle_center", m_last_rear_axle_center));

    archive(_MAKE_NVP_("OnLanePos", m_OnLanePos));
    archive(_MAKE_NVP_("lastOnLanePos", m_lastOnLanePos));
    archive(_MAKE_NVP_("PosWithoutOffset", m_PosWithoutOffset));

    archive(_MAKE_NVP_("vLaneDir", m_vLaneDir));
    archive(_MAKE_NVP_("last_vLaneDir", m_last_vLaneDir));
    archive(_MAKE_NVP_("fAngle", m_fAngle));
    archive(_MAKE_NVP_("CrossRot", m_CrossRot));
    archive(_MAKE_NVP_("curLaneIndex", m_curLaneIndex));
    archive(_MAKE_NVP_("nextLaneIndex", m_nextLaneIndex));
    archive(_MAKE_NVP_("laneInfo", m_laneInfo));
    archive(_MAKE_NVP_("last_laneInfo", m_last_laneInfo));
    archive(_MAKE_NVP_("distanceAlongCurve", m_distanceAlongCurve));
    archive(_MAKE_NVP_("l_offset", m_l_offset));
    archive(_MAKE_NVP_("centerline_offset", m_centerline_offset));
    archive(_MAKE_NVP_("bInFinalLane", m_bInFinalLane));

    archive(_MAKE_NVP_("bInBoundaryLane", m_bInBoundaryLane));
    {
      txBool m_tracker_is_not_nullptr = false;
      archive(_MAKE_NVP_("m_tracker_is_not_nullptr", m_tracker_is_not_nullptr));
      if (CallSucc(m_tracker_is_not_nullptr)) {
        m_tracker = std::make_shared<HdMap::MapTracker>(0);
        archive(_MAKE_NVP_("m_tracker", (*m_tracker)));
      } else {
        m_tracker = nullptr;
      }
    }

    archive(_MAKE_NVP_("vLocalCoord", m_vLocalCoord));
    archive(_MAKE_NVP_("TimeStampOnLanelink", m_TimeStampOnLanelink));
  }
};

// @brief 几何数据基类
class GeometryData : public txComponent {
 public:
  using PolygonArray = Geometry::OBB2D::PolygonArray;
  using Object_Type = Base::Enums::Object_Type;

  /**
   * @brief 获取几何物体的类型 人 车 障碍物...
   *
   * @return const Object_Type
   */
  const Object_Type ObjectType() const TX_NOEXCEPT { return m_type; }
  Object_Type& ObjectType() TX_NOEXCEPT { return m_type; }

  /**
   * @brief 获取几何物体长度
   *
   * @return const txFloat
   */
  const txFloat Length() const TX_NOEXCEPT { return m_length; }
  txFloat& Length() TX_NOEXCEPT { return m_length; }

  /**
   * @brief 获取几何物体宽度
   *
   * @return const txFloat
   */
  const txFloat Width() const TX_NOEXCEPT { return m_width; }
  txFloat& Width() TX_NOEXCEPT { return m_width; }

  /**
   * @brief 获取几何物体高度
   *
   * @return const txFloat
   */
  const txFloat Height() const TX_NOEXCEPT { return m_height; }
  txFloat& Height() TX_NOEXCEPT { return m_height; }

  /**
   * @brief 获取包围盒
   *
   * @return const PolygonArray&
   */
  const PolygonArray& Polygon() const TX_NOEXCEPT { return m_vecPolygon; }
  PolygonArray& Polygon() TX_NOEXCEPT { return m_vecPolygon; }

  /**
   * @brief 获取几何物体是否有效
   *
   * @return txBool
   */
  txBool IsValid() const TX_NOEXCEPT { return m_isValid; }

  /**
   * @brief Set the In Valid object
   *
   */
  void SetInValid() TX_NOEXCEPT { m_isValid = false; }

  /**
   * @brief Set the In Valid object
   *
   * @param _f param valid or not
   */
  void SetInValid(Base::txBool _f) TX_NOEXCEPT { m_isValid = _f; }

  /**
   * @brief 计算包围盒
   *
   * @param Pos
   * @param frontAxis3d_hasNormalized
   */
  void ComputePolygon(const Coord::txENU& Pos, const Base::txVec3& frontAxis3d_hasNormalized) TX_NOEXCEPT;

  /**
   * @brief 几何描述
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const GeometryData& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

 protected:
  Object_Type m_type = Object_Type::Unknown;
  txFloat m_length = 4.5;
  txFloat m_width = 1.8;
  txFloat m_height = 1.5;
  PolygonArray m_vecPolygon;
  txBool m_isValid = true;

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("ObjectType", m_type), _MAKE_NVP_("Length", m_length), _MAKE_NVP_("Width", m_width),
       _MAKE_NVP_("Height", m_height), _MAKE_NVP_("vecPolygon", m_vecPolygon), _MAKE_NVP_("isValid", m_isValid));
  }
};

// @brief 动力属性基类
class Kinetics : public txComponent {
 public:
  const txFloat gravitational_acceleration_m_ss = 9.8;
  txFloat m_acceleration = 0.0;
  txFloat m_lateral_acceleration = 0.0;
  txFloat m_velocity = 8.0;
  txFloat m_show_velocity = 0.0;
  txFloat m_LateralVelocity = 0.0;

  txFloat m_displacement = 0.0;
  txFloat m_LateralDisplacement = 0.0;
  txFloat TX_MARK("m/s") m_velocity_max = FLAGS_vehicle_max_speed;
  txFloat velocity_desired = FLAGS_vehicle_max_speed;
  txFloat velocity_comfort = 25.0;
  txFloat raw_velocity_max = FLAGS_vehicle_max_speed;
  txInt m_direction = 0;
  Unit::txDegree m_gear_angle;
  txFloat acceleration_desired = 0.0;

  /**
   * @brief 限制加速度
   *
   */
  void LimitAcceleration() TX_NOEXCEPT { m_acceleration = Math::Clamp(m_acceleration, m_max_dec_m_ss, m_max_acc_m_ss); }

  /**
   * @brief 限制速度
   *
   */
  void LimitVelocity() TX_NOEXCEPT { m_velocity = Math::Clamp(m_velocity, 0.0, m_velocity_max); }

 public:
  /**
   * @brief 获取上次速度
   *
   * @return txFloat
   */
  txFloat LastVelocity() const TX_NOEXCEPT { return m_last_velocity; }

  /**
   * @brief 获取上次加速度
   *
   * @return txFloat
   */
  txFloat LastAcceleration() const TX_NOEXCEPT { return m_last_acceleration; }
  txFloat& LastVelocity() TX_NOEXCEPT { return m_last_velocity; }
  txFloat& LastAcceleration() TX_NOEXCEPT { return m_last_acceleration; }

  /**
   * @brief 获取m_max_acc_m_ss
   *
   * @return txFloat
   */
  txFloat max_acc_m_ss() const TX_NOEXCEPT { return m_max_acc_m_ss; }

  /**
   * @brief Set and Return the Max Acc object
   *
   * @param maxAcc
   * @return txFloat
   */
  txFloat SetMaxAcc(const txFloat maxAcc) TX_NOEXCEPT {
    m_max_acc_m_ss = maxAcc;
    return max_acc_m_ss();
  }

  /**
   * @brief 获取max_dec_m_ss
   *
   * @return txFloat
   */
  txFloat max_dec_m_ss() const TX_NOEXCEPT { return m_max_dec_m_ss; }

  /**
   * @brief Set and Return the Max Dec object
   *
   * @param maxDec
   * @return txFloat
   */
  txFloat SetMaxDec(const txFloat maxDec) TX_NOEXCEPT {
    m_max_dec_m_ss = maxDec;
    return max_dec_m_ss();
  }

 protected:
  txFloat m_last_velocity = 8.0;
  txFloat m_last_acceleration = 0.0;
  txFloat m_max_acc_m_ss = gravitational_acceleration_m_ss;
  txFloat m_max_dec_m_ss = gravitational_acceleration_m_ss;

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("Acceleration", m_acceleration), _MAKE_NVP_("Velocity", m_velocity),
       _MAKE_NVP_("show_velocity", m_show_velocity), _MAKE_NVP_("LateralVelocity", m_LateralVelocity),
       _MAKE_NVP_("Displacement", m_displacement), _MAKE_NVP_("LateralDisplacement", m_LateralDisplacement),
       _MAKE_NVP_("LateralAcceleration", m_lateral_acceleration), _MAKE_NVP_("Velocity_Max", m_velocity_max),
       _MAKE_NVP_("Velocity_Desired", velocity_desired), _MAKE_NVP_("Velocity_Comfort", velocity_comfort),
       _MAKE_NVP_("raw_velocity_max", raw_velocity_max), _MAKE_NVP_("Direction", m_direction),
       _MAKE_NVP_("max_acc_m_ss", m_max_acc_m_ss), _MAKE_NVP_("max_dec_m_ss", m_max_dec_m_ss));
  }
};

// @brief 生命周期基类
class LifeCycle : public txComponent {
 public:
  /**
   * @brief 重置对象
   *
   */
  void ResetState() TX_NOEXCEPT {
    m_bStart = false;
    m_bEnd = false;
    m_bStop = false;
    m_startTime = 0.0;
    m_endTime = FLAGS_max_lift_time;
  }

  /**
   * @brief 设置开始状态
   *
   */
  void SetStart() TX_NOEXCEPT { m_bStart = true; }

  /**
   * @brief 设置结束状态
   *
   */
  void SetEnd() TX_NOEXCEPT { m_bEnd = true; }

  /**
   * @brief 设置暂停状态
   *
   */
  void SetStop() TX_NOEXCEPT { m_bStop = true; }

  /**
   * @brief 是否结束
   *
   * @return Base::txBool
   */
  Base::txBool IsEnd() const TX_NOEXCEPT { return m_bEnd; }

  /**
   * @brief 是否开始
   *
   * @return Base::txBool
   */
  Base::txBool IsStart() const TX_NOEXCEPT { return m_bStart; }

  /**
   * @brief 是否活跃
   *
   * @return Base::txBool
   */
  Base::txBool IsAlive() const TX_NOEXCEPT { return IsStart() && !IsEnd(); }

  /**
   * @brief 是否停止
   *
   * @return Base::txBool
   */
  Base::txBool IsStop() const TX_NOEXCEPT { return m_bStop; }

  /**
   * @brief 获取开始时间
   *
   * @return const Base::txFloat
   */
  const Base::txFloat StartTime() const TX_NOEXCEPT { return m_startTime; }
  Base::txFloat& StartTime() TX_NOEXCEPT { return m_startTime; }

  /**
   * @brief 获取结束时间
   *
   * @return const Base::txFloat
   */
  const Base::txFloat EndTime() const TX_NOEXCEPT { return m_endTime; }
  Base::txFloat& EndTime() TX_NOEXCEPT { return m_endTime; }

  /**
   * @brief 杀死对象的生命周期
   *
   * @return Base::txBool
   */
  Base::txBool Kill() TX_NOEXCEPT {
    m_bEnd = true;
    return (!IsAlive());
  }

 protected:
  Base::txBool m_bStart = false;
  Base::txBool m_bEnd = false;
  Base::txBool m_bStop = false;
  Base::txFloat m_startTime = 0.0;
  Base::txFloat m_endTime = FLAGS_max_lift_time;

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("bStart", m_bStart), _MAKE_NVP_("bEnd", m_bEnd), _MAKE_NVP_("bStop", m_bStop),
       _MAKE_NVP_("StartTime", m_startTime), _MAKE_NVP_("EndTime", m_endTime));
  }
};

class Pseudorandom : public txComponent {
 public:
  enum { _random_size_ = 64 /*a power of 2*/, _and_divisor_ = 63, _IntScalar_ = 1000 };

  /**
   * @brief 初始化函数
   *
   * @param elemId
   * @param randomSeed
   */
  virtual void Initialize(const txSysId elemId, const txInt randomSeed) TX_NOEXCEPT {
    m_random_seed = elemId + randomSeed;
    std::mt19937 rng(m_random_seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < _random_size_; i++) {
      m_array_random_0_1[i] = dis(rng);
    }
  }

  virtual txFloat GetRandomValue() TX_NOEXCEPT {
    m_cursor = (m_cursor + 1) & _and_divisor_;
    TX_MARK("Wrapping Counter");
    return m_array_random_0_1.at(m_cursor);
  }

  /**
   * @brief 获取 -1 to 1 随机值
   *
   * @return txFloat
   */
  virtual txFloat GetRandomValue_NegOne2PosOne() TX_NOEXCEPT {
    const txFloat FltValue = GetRandomValue();
    return ((FltValue * 2.0) - 1.0);
  }

  /**
   * @brief 获取随机整数
   *
   * @return txInt
   */
  virtual txInt GetRandomInt() TX_NOEXCEPT {
    const txFloat FltValue = GetRandomValue();
    return txInt(FltValue * _IntScalar_);
  }
  virtual txInt CurrentCursor() const TX_NOEXCEPT { return m_cursor; }
  virtual txFloat CurrentRndValue() const TX_NOEXCEPT { return m_array_random_0_1.at((m_cursor + 1) & _and_divisor_); }
  virtual txString CurrentStateStr(txString strMark) const TX_NOEXCEPT {
    return (strMark +
            txString("Cursor = " + std::to_string(CurrentCursor()) + "; Value = " + std::to_string(CurrentRndValue())));
  }

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("value_cache", m_array_random_0_1), _MAKE_NVP_("cache_cursor", m_cursor),
       _MAKE_NVP_("random_seed", m_random_seed));
  }

 protected:
  std::array<txFloat, _random_size_> m_array_random_0_1;
  txInt m_cursor = 0;
  std::mt19937::result_type m_random_seed = 0;
};

TX_NAMESPACE_CLOSE(Component)
TX_NAMESPACE_CLOSE(Base)
