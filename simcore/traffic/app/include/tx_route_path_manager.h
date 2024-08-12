// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "routeplan/route_plan.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_sim_point.h"
#include "tx_way_points.h"
TX_NAMESPACE_OPEN(TrafficFlow)

struct RoutePathManager {
  RoutePathManager();

  /**
   * @brief 重置路径管理器
   *
   * 将路径管理器的状态重置为初始状态。
   *
   * @note 此函数不抛出任何异常。
   */
  void Reset() TX_NOEXCEPT;

  /**
   * @brief 清除路径管理器
   *
   * 此函数将清除路径管理器中的所有路径，使其恢复到初始状态。
   *
   * @note 此函数不抛出任何异常。
   */
  void Clear() TX_NOEXCEPT;

  /**
   * @brief 获取路径管理器中的路径数量
   *
   * 返回路径管理器中保存的路径数量。
   *
   * @return Base::txSize 路径数量
   */
  Base::txSize Size() const TX_NOEXCEPT;

  /**
   * @brief 获取路径管理器中的路径索引
   *
   * 返回路径管理器中第一个路径的索引，如果路径管理器中没有路径，则返回 `-1`。
   *
   * @return Base::txDiff_t 路径索引
   */
  Base::txDiff_t Index() const TX_NOEXCEPT;
  Base::txDiff_t Relax() const TX_NOEXCEPT;

  /**
   * @brief 判断路径管理器中是否为空
   *
   * 检查路径管理器是否已经包含了任何路径，如果路径管理器为空，则返回true，否则返回false。
   *
   * @return 如果路径管理器为空，则返回true，否则返回false。
   */
  Base::txBool IsEmpty() const TX_NOEXCEPT;

  /**
   * @brief 判断路径管理器是否有效
   *
   * 判断当前路径管理器是否包含有效路径，如果存在有效路径则返回 true，否则返回 false。
   *
   * @return 如果路径管理器有效则返回 true，否则返回 false。
   */
  Base::txBool IsValid() const TX_NOEXCEPT;

  /**
   * @brief 判断是否还有下一条路
   *
   * 检查是否还有下一条路径。如果存在下一条路径，则返回 true，否则返回 false。
   *
   * @param idx 路线的索引，用于指定要检查的路线，默认为 1
   * @return true 表示还有下一条路，false 表示已经到达最后一条路
   */
  Base::txBool HasNextRoad(Base::txDiff_t const idx = 1) TX_NOEXCEPT;

  /**
   * @brief 获取当前道路的ID
   *
   * 获取当前道路的ID，并返回其签名版本的ID。
   *
   * @return 当前道路的签名版本ID
   */
  Base::txSignedRoadID CurRoadId() const TX_NOEXCEPT;

  /**
   * @brief 获取下一个路段的签名ID
   *
   * 获取指定索引位置的下一个路段的签名ID。
   *
   * @param idx 路段索引，默认为1。
   * @return 返回指定路段的签名ID。
   */
  Base::txSignedRoadID TX_MARK("+") NextRoadId(Base::txDiff_t const idx = 1) TX_NOEXCEPT;

  /**
   * @brief 获取下一个路段的迭代器
   *
   * 根据给定的索引，获取下一个路段的迭代器。
   *
   * @param idx 指定路段索引，默认为1。
   * @return 返回指定路段的迭代器。
   */
  planner::RoadInfoArray::iterator Next(Base::txDiff_t const idx = 1) TX_NOEXCEPT;

  /**
   * @brief 转换为指定的格式的路径信息字符串
   *
   * 根据指定的格式，将一个 roadInfo 转换为一个字符串
   *
   * @param refRoadInfo 待转换的 roadInfo
   * @return txString 转换后的字符串
   */
  Base::txString RoadInfoStr(const planner::RoadInfo& refRoadInfo) const TX_NOEXCEPT;

  /**
   * @brief 转换为指定格式的路径信息字符串
   *
   * 将当前的路径信息（即当前路径节点，依次指向的父节点）转换为一个可读的字符串，以指定格式输出。
   *
   * @param refRoadInfo 路径信息的引用
   * @return txString 转换后的字符串
   */
  Base::txString Str() const TX_NOEXCEPT;

  /**
   * @brief 判断是否有目标车道
   *
   * 当 m_iGoalLaneId 不为 0 时，表示存在目标车道。
   *
   * @return true 表示存在目标车道，false 表示不存在目标车道。
   */
  Base::txBool HasGoalLane() const TX_NOEXCEPT { return 0 != m_iGoalLaneId; }

  /**
   * @brief 获取当前目标车道ID
   *
   * 如果 m_iGoalLaneId 不为 0，表示存在目标车道。
   *
   * @return 当前目标车道ID，如果不存在则返回0
   */
  Base::txLaneID CurGoalLaneId() const TX_NOEXCEPT { return m_iGoalLaneId; }

  /**
   * @brief 计算目标车道索引
   *
   * @param[in] _goalLaneId 目标车道ID
   * @return 目标车道索引，如果不存在则返回-1
   */
  static Base::txLaneID ComputeGoalLaneIndex(Base::txLaneID _goalLaneId) TX_NOEXCEPT {
    TX_MARK("m_iGoalLaneIndex = (initLane->getId() + 1) * -1;");
    return (_goalLaneId + 1) * -1;
  }

  /**
   * @brief 计算当前目标车道索引
   * @return 当前目标车道索引，如果不存在则返回-1
   */
  Base::txLaneID CurGoalLaneIndex() const TX_NOEXCEPT { return ComputeGoalLaneIndex(m_iGoalLaneId); }

  /**
   * @brief 设置路线中的目标点列表
   * @param pointList 目标点列表，每个点的坐标对应txPoint结构体
   * @return 无返回值，直接在传入的pointList中设置路线中的目标点列表
   */
  void SetWayPoints(const std::vector<hadmap::txPoint>& pointList) TX_NOEXCEPT;

  /**
   * @brief 检查当前路径上的车道路标
   * @param curPos 当前车辆的WGS84坐标
   * @param laneInfo 当前道路的车道信息
   *
   * 检查当前车辆所在道路上的车道路标，并根据车道信息更新路标列表。
   */
  void CheckWayPoint(Coord::txWGS84 curPos, const Base::Info_Lane_t& laneInfo) TX_NOEXCEPT;

  /**
   * @brief 获取下一个目标车道ID
   *
   * @param nextStartIndex 起始索引
   * @return Base::txLaneID 下一个目标车道ID
   *
   * 在当前道路上找到下一个目标车道ID，并根据给定的起始索引更新下一个目标车道ID。
   */
  Base::txLaneID FindNextTargentLaneId(const Base::txSize nextStartIndex) const TX_NOEXCEPT;

  /**
   * @brief 获取下一个目标车道ID
   *
   * @param nextStartIndex 起始索引
   * @return Base::txLaneID 下一个目标车道ID
   *
   * 在当前道路上找到下一个目标车道ID，并根据给定的起始索引更新下一个目标车道ID。
   */
  WayPoints GetNextWayPoint() const { return m_vec_waypoint_area[_nextWayPointAreaIndex]; }
  /*Base::txBool IsValid() const TX_NOEXCEPT { return _isValid; }*/

  /**
   * @brief 判断是否有下一个路线点
   *
   * 通过该函数，您可以判断是否还有路线点需要驾驶，避免您的驾驶器在
   * 最后一个路线点时突然停车，可能导致车辆故障。
   *
   * @return 如果还有下一个路线点则返回 true，否则返回 false
   */
  Base::txBool HasNextWayPoint() const TX_NOEXCEPT {
    return (IsValid()) && (!m_vec_waypoint_area.empty()) &&
           (_nextWayPointAreaIndex < m_vec_waypoint_area.size() && _nextWayPointAreaIndex >= 0);
  }

  /**
   * @brief 获取下一个道路点的 laneLink 指针
   *
   * @param _roadId 道路 ID
   * @param _sectionId 路段 ID
   * @return const hadmap::txLaneLinkPtr& 下一个道路点的 laneLink 指针
   *
   * 该函数用于根据给定的道路 ID 和路段 ID 获取下一个道路点的 laneLink 指针。
   */
  hadmap::txLaneLinkPtr GetNextLaneLinkPtr(const Base::txRoadID _roadId,
                                           const Base::txSectionID _sectionId) const TX_NOEXCEPT;

  planner::RoadInfoArray roadInfoArray;
  planner::RoadInfoArray::iterator m_curPathPlanIndex;
  Base::txLaneID TX_MARK("Lane Id, -1, -2, -3") m_iGoalLaneId = 0;
  std::vector<WayPoints> m_vec_waypoint_area;
  Base::txBool _isValid = true;
  Base::txInt _nextWayPointAreaIndex = 0;
  friend std::ostream& operator<<(std::ostream& os, const RoutePathManager& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    txMsg("un-impl");
  }
};
TX_NAMESPACE_CLOSE(TrafficFlow)
