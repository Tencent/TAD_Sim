// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the class Map.

#pragma once

#include "hadmap_position.h"
#include "hadmap_waypoint.h"
#include "mapengine/engine_defs.h"
#include "mapengine/hadmap_engine.h"

namespace hadmap {
class Map : public std::enable_shared_from_this<Map> {
 public:
  friend class Waypoint;

 public:
  Map();
  ~Map();

  /*! \~Chinese
   * @brief 等号运算符重载, 用于比较两个 Map 对象是否相等.
   *
   * @param[in] other another Map.
   * @return bool
   */
  bool operator==(const Map& other) const;

  /*! \~Chinese
   * @brief 从文件中加载地图.
   *
   * @param[in] fname 文件名.
   * @return bool
   */
  bool Load(const std::string& fname);

  /*! \~Chinese
   * @brief 释放地图资源.
   *
   * @return void
   */
  void Release();

  /*! \~Chinese
   * @brief 获取地图中的所有 Junctions.
   *
   * @return txJunctions
   */
  txJunctions GetAllJunctions();

  /*! \~Chinese
   * @brief 获取一个waypoint.
   *
   * @param[in] road_id 位置信息中的 road_id.
   * @param[in] lane_id 位置信息中的 lane_id.
   * @param[in] offset 与车道中心线的偏移量.
   * @param[in] s 从道路的起点沿道路参考线获取的 s 坐标.
   * @return std::shared_ptr<Waypoint>
   */
  std::shared_ptr<Waypoint> GetWaypoint(uint64_t road_id, int64_t lane_id, double offset, double s);

 private:
  txMapHandle* _maphandler = nullptr;

 private:
  /*! \~Chinese
   * @brief 支持的地图类型.
   *
   */
  static std::map<std::string, MAP_DATA_TYPE> _kHadmapTypeDict;

  /*! \~Chinese
   * @brief 找到 world 位置点最近的 Lane
   *
   * @param[in] world
   * @return txLanePtr
   */
  txLanePtr _GetNearestLane(const PositionWorld& world);

  /*! \~Chinese
   * @brief 将 lane 坐标转换为 world 坐标.
   *
   * @param[in] world
   * @return PositionLane
   */
  PositionWorld _ConvertLane2World(const PositionLane& lane);

  /*! \~Chinese
   * @brief 将 world 坐标转换为 lane 坐标.
   *
   * @param[in] world
   * @return PositionLane
   */
  PositionLane _ConvertWorld2Lane(const PositionWorld& world);

 protected:
  /*! \~Chinese
   * @brief Position 转换函数, 实现 world 和 lane 相互转换.
   *
   * @param[in] position
   * @return Position
   */
  Position ConvertPosition(const Position& position);

  /*! \~Chinese
   * @brief 计算并返回下一个位置点, world 坐标系.
   *
   * @param[in] world
   * @param[in] distance 下一个点与当前点的沿车道中心线的距离（Unit: m).
   * @param[in] direction 下一个点的朝向.
   * @return PositionWorld
   */
  PositionWorld Next(const PositionWorld& world, double distance, std::string direction);
};

}  // namespace hadmap
