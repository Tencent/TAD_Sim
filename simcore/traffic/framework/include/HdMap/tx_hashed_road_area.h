// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#if USE_HashedRoadNetwork
#  include "HdMap/hashed_lane_info_orthogonal_list.h"
#  include "HdMap/tx_hashed_lane_info.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

class txHashedRoadArea {
 public:
  using VehicleContainer = HashedLaneInfoOrthogonalList::VehicleContainer;

 public:
  /**
   * @brief 初始化一个具有特定终点和覆盖车道的txHashedRoadArea对象
   *
   * @param _id 要创建的txHashedRoadArea对象的唯一标识符
   * @param endPt 指定的终点经纬度坐标
   * @return 如果初始化成功，则返回true，否则返回false
   */
  virtual Base::txBool Initialize(const Base::txSysId _id, Coord::txWGS84 endPt) TX_NOEXCEPT;

  /**
   * @brief 初始化一个指定终点和覆盖车道的txHashedRoadArea对象
   * @param _id 对象的唯一ID
   * @param endPt 指定终点的经纬度坐标
   * @param cover 指定覆盖的车道ID集合
   * @return 如果初始化成功，则返回true，否则返回false
   */
  virtual Base::txBool Initialize(const Base::txSysId _id, Coord::txWGS84 endPt,
                                  std::set<Base::txLaneID> cover) TX_NOEXCEPT;

  /**
   * @brief 查询车辆
   *
   * 根据当前交叉点的哈希值，查询车辆列表，将满足查询条件的车辆添加到传入的参数中。
   *
   * @param refVehicleContainer 车辆列表容器，用于存储查询到的车辆
   * @return txBool 如果查询成功，则返回true；否则，返回false。
   */
  virtual Base::txBool QueryVehicles(VehicleContainer& refVehicleContainer) TX_NOEXCEPT;

  /**
   * @brief 当前车辆是否到达指定哈希道路区域
   *
   * 根据传入的哈希道路区域对象，判断当前车辆是否已经到达其所属的道路区域。
   *
   * @param objHashedNode 传入的哈希道路区域对象
   * @return txBool 如果当前车辆到达目标区域，则返回true；否则，返回false。
   */
  virtual Base::txBool ArriveAtArea(const HashedLaneInfo& objHashedNode) const TX_NOEXCEPT;

  /**
   * @brief 获取当前道路区域的ID
   * @return 返回当前道路区域的ID
   */
  Base::txSysId AreaId() const TX_NOEXCEPT { return _area_id; }

  /**
   * @brief 将当前实例设置为无效状态
   *
   */
  void SetInValid() TX_NOEXCEPT { _valid = false; }

  /**
   * @brief 获取当前实例的有效状态
   * @return 当前实例的有效状态，如果为 true，则实例有效，否则实例无效
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return _valid; }

  /**
   * @brief 获取结束位置
   * @return 返回结束位置的ENU坐标
   */
  const Coord::txENU& GetEndPos() const TX_NOEXCEPT { return _EndPos; }

  /**
   * @brief 返回一个字符串表示的txHashedRoadArea
   * @return 返回一个字符串表示的txHashedRoadArea
   */
  Base::txString Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const txHashedRoadArea& v) TX_NOEXCEPT {
    os << "{" << v.Str() << "}";
    return os;
  }

 protected:
  std::vector<HashedLaneInfoOrthogonalListPtr> _vec_OrthogonalListPtr;
  Base::txBool _valid = false;
  Coord::txENU _EndPos;
  Base::txSysId _area_id = -1;
};

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

#endif /*USE_HashedRoadNetwork*/
