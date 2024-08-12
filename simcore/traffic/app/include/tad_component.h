// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <list>
#include "tx_component.h"
#include "tx_container_defs.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_route_path_manager.h"

TX_NAMESPACE_OPEN(TrafficFlow)
TX_NAMESPACE_OPEN(Component)

using txComponent = Base::Component::txComponent;

class Route : public txComponent {
 public:
  Route() TX_DEFAULT;
  ~Route() TX_DEFAULT;

  /**
   * @brief 判断是否为有效的组件
   *
   * @return Base::txBool 有效与否
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return m_isValid; }
  /**
   * @brief 初始化函数
   *
   * @param _startPt 起始点
   * @param _midpoints 中间点数组
   * @param _endPt 终点
   * @return 成功返回true，失败返回false
   */
  Base::txBool Initialize(const hadmap::txPoint& _startPt, const std::vector<hadmap::txPoint>& _midpoints,
                          const hadmap::txPoint& _endPt) TX_NOEXCEPT;
  /**
   * @brief 获取组件开始点
   *
   * @return const hadmap::txPoint& 组件开始点的引用
   */
  const hadmap::txPoint& StartPt() const TX_NOEXCEPT { return m_StartPt; }
  /**
   * @brief 获取组件结束点
   *
   * @return const hadmap::txPoint& 组件结束点的引用
   */
  const hadmap::txPoint& EndPt() const TX_NOEXCEPT { return m_EndPt; }
  /**
   * @brief 获取组件的终点坐标
   *
   * @return const Coord::txENU& 组件的终点坐标的引用
   */
  const Coord::txENU& GoalPt() const TX_NOEXCEPT { return m_EndPosition; }
  /**
   * @brief 获取中间点
   *
   * 获取到存储中间点的数组，用于访问或者修改中间点。
   *
   * @return const std::vector<hadmap::txPoint>& 返回存储中间点的数组引用
   */
  const std::vector<hadmap::txPoint>& MidPts() const TX_NOEXCEPT { return m_midpoints; }
  /**
   * @brief 获取路径信息数组
   *
   * 获取到存储路径信息的数组，用于访问或者修改道路信息。
   *
   * @return const TrafficFlow::RoutePathManager& 返回存储路径信息的数组引用
   */
  const TrafficFlow::RoutePathManager& roadInfoArray() const TX_NOEXCEPT { return m_roadInfoArray; }
  /**
   * @brief 获取路径信息数组
   *
   * 获取到存储路径信息的数组，用于访问或者修改道路信息。
   *
   * @return const TrafficFlow::RoutePathManager& 返回存储路径信息的数组引用
   */
  TrafficFlow::RoutePathManager& roadInfoArray() TX_NOEXCEPT { return m_roadInfoArray; }
  /**
   * @brief 判断当前位置是否在到达时间限制区域内
   *
   * 判断当前位置是否在预设的到达时间限制区域内
   *
   * @param elemLaneUid 当前元素所在的车道 ID
   * @param elementPt 当前元素的坐标
   * @return Base::txBool 当前位置在到达时间限制区域内
   */
  Base::txBool ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid,
                                    const Coord::txENU& elementPt) const TX_NOEXCEPT;
  /**
   * @brief 获取组件的文本信息
   *
   * @return const Base::txString& 组件的文本信息
   */
  Base::txString Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const Route& v) TX_NOEXCEPT {
    os << "{" << v.Str() << "}";
    return os;
  }

 protected:
  /**
   * @brief 设置路由路径信息
   * @param _startPt 起始点
   * @param _midpoints 中间点数组
   * @param _endPt 终点
   * @return void
   */
  void SetRoutingPathInfo(const hadmap::txPoint& _startPt, const std::vector<hadmap::txPoint>& _midpoints,
                          const hadmap::txPoint& _endPt) TX_NOEXCEPT;

 public:
  /**
   * @brief 序列化模板
   *
   * @tparam Archive 类型
   * @param ar 实例对象
   */
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("isValid", m_isValid), _MAKE_NVP_("StartPt", m_StartPt), _MAKE_NVP_("EndPt", m_EndPt),
       _MAKE_NVP_("EndPosition", m_EndPosition), _MAKE_NVP_("MidPoints", m_midpoints),
       _MAKE_NVP_("DeadLineArea", m_deadline), _MAKE_NVP_("RouteMgr", m_roadInfoArray));
  }

 protected:
  hadmap::txPoint m_StartPt;
  hadmap::txPoint m_EndPt;
  Coord::txENU m_EndPosition;
  std::vector<hadmap::txPoint> m_midpoints;
  Base::DeadLineAreaManager m_deadline;
  TrafficFlow::RoutePathManager m_roadInfoArray;
  std::unique_ptr<planner::RoutePlan> m_pathPlanPtr;
  Base::txBool m_isValid = false;
};

class RouteAI : public txComponent {
 public:
  enum class LaneChangeTrend : Base::txInt { eNone = 0, eTrend = 1 };

 public:
  RouteAI() TX_DEFAULT;
  ~RouteAI() TX_DEFAULT;
  /**
   * @brief 初始化路由信息
   * @param routeId 路由id
   * @param subRouteId 子路由id
   * @param vehicleId 车辆id
   * @param _startPt 起始点坐标
   * @param _midpoints 中间点坐标数组
   * @param _endPt 终点坐标
   * @param roadIds 参与路由的道路id数组，默认为空
   * @return 成功返回true，失败返回false
   */
  Base::txBool Initialize(const Base::txSysId routeId, const Base::txInt subRouteId, const Base::txSysId vehicleId,
                          const hadmap::txPoint& _startPt, const std::vector<hadmap::txPoint>& _midpoints,
                          const hadmap::txPoint& _endPt, const std::vector<int64_t>& roadIds = {}) TX_NOEXCEPT;
  /**
   * @brief 设置精确的结束点
   *
   * 设置一个标志，表明该路径的结束点在多段路径中是否需要完全匹配
   */
  void SetExactEndPoint() TX_NOEXCEPT { mExactEndPoint = true; }
  /**
   * @brief 计算路由
   *
   * 计算一条路由，在给定的伪随机数生成器的基础上启动此计算。
   *
   * @param _random 伪随机数生成器的实例
   * @return 路由计算是否成功，true 表示成功，false 表示失败
   */
  Base::txBool ComputeRoute(Base::Component::Pseudorandom& _random) TX_NOEXCEPT;
  /**
   * @brief 获取路由ID
   *
   * 获取当前路由实例的ID，ID用于区分不同的路由实例。
   *
   * @return 路由ID
   */
  Base::txSysId RouteId() const TX_NOEXCEPT { return m_RouteId; }
  /**
   * @brief 获取子路由ID
   *
   * @return Base::txInt
   */
  Base::txInt SubRouteId() const TX_NOEXCEPT { return m_SubRouteId; }
  /**
   * @brief 获取车辆ID
   *
   * @return Base::txSysId
   */
  Base::txSysId VehicleId() const TX_NOEXCEPT { return m_VehicleId; }
  /**
   * @brief 设置车辆ID
   *
   * @param _vId
   */
  void ResetVehicleId(const Base::txSysId _vId) TX_NOEXCEPT { m_VehicleId = _vId; }
  /**
   * @brief 获取起始点
   *
   * @return const hadmap::txPoint&
   */
  const hadmap::txPoint& StartPt() const TX_NOEXCEPT { return m_StartPt; }
  /**
   * @brief 获取终止点
   *
   * @return const hadmap::txPoint&
   */
  const hadmap::txPoint& EndPt() const TX_NOEXCEPT { return m_EndPt; }
  /**
   * @brief 获取中间点集合
   *
   * @return const std::vector< hadmap::txPoint >&
   */
  const std::vector<hadmap::txPoint>& MidPts() const TX_NOEXCEPT { return m_midpoints; }
  /**
   * @brief 当前路由是否有效
   *
   * @return Base::txBool
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return m_isValid; }
  /**
   * @brief 判断当前元素是否已经到达消亡区
   *
   * 当元素在消亡区域内时，返回 true。 在死线区域外时，返回 false。
   *
   * @param elemLaneUid 元素所在的道路ID
   * @param elementPt 元素当前的坐标
   * @return Base::txBool 当前元素是否已经到达消亡区
   */
  Base::txBool ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid,
                                    const Coord::txENU& elementPt) const TX_NOEXCEPT;
  /**
   * @brief 获取字符串形式的信息
   *
   * 获取当前组件的字符串形式的信息，可以在需要用户信息时提供给其他组件。
   *
   * @return 组件的字符串形式的信息
   */
  Base::txString Str() const TX_NOEXCEPT;

  /**
   * @brief 设置车道改变趋势为激活
   *
   * 当此函数被调用时，车道改变趋势被设置为激活状态，系统将试图触发车道变更。
   */
  void SetTrendOn() TX_NOEXCEPT { mTrend = LaneChangeTrend::eTrend; }
  /**
   * @brief 关闭车道改变趋势
   *
   * 当此函数被调用时，车道改变趋势被关闭，系统将停止触发车道变更。
   */
  void SetTrendOff() TX_NOEXCEPT { mTrend = LaneChangeTrend::eNone; }
  /**
   * @brief 获取车道改变趋势状态
   *
   * 返回当前车道改变趋势的状态。如果车道改变趋势被激活，返回 true；否则，返回 false。
   *
   * @return Base::txBool 车道改变趋势状态
   */
  Base::txBool HasTrend() const TX_NOEXCEPT { return LaneChangeTrend::eTrend == mTrend; }

  friend std::ostream& operator<<(std::ostream& os, const RouteAI& v) TX_NOEXCEPT {
    os << "{" << v.Str() << "}";
    return os;
  }

 protected:
  /**
   * @brief 重置边权重
   *
   * 重置所有边的权重，使之变为初始状态。如果重置成功，返回 true；否则，返回 false。
   *
   * @return Base::txBool 重置是否成功
   */
  Base::txBool ResetEdgeWeight() TX_NOEXCEPT;
  /**
   * @brief 设置路径信息
   *
   * 设置线路路径信息，接受一个整数向量roadIds。
   * 如果设置成功，返回 true；否则，返回 false。
   *
   * @param roadIds 线路ID列表
   * @return bool 设置是否成功
   */
  Base::txBool SetRoutingPathInfo(const std::vector<int64_t>& roadIds) TX_NOEXCEPT;

  Base::txBool CheckWaypointArea(const std::vector<WayPoints>& vec_waypoint_area) TX_NOEXCEPT;

  /**
   * @brief 将道路信息转换为关节点点列表
   *
   * 将给定的道路信息数组转换为关节点点列表。关节点点列表是由有效路径上的点组成的列表。
   *
   * @param wayPointList 有效路径的点列表
   * @param roadInfoArray 道路信息数组
   * @return bool 转换是否成功
   */
  Base::txBool ConvertRoadInfo2JointPointList(const std::vector<hadmap::txPoint>& wayPointList,
                                              const std::vector<int64_t>& roadInfoArray) TX_NOEXCEPT;
  /**
   * @brief 根据给定的点数组生成边
   *
   * 根据给定的点数组（curWayPoint）生成边，并存储在vector中。如果生成成功，返回 true；否则，返回 false。
   *
   * @param curWayPoint 当前路径上的点数组
   * @return bool 生成是否成功
   */
  Base::txBool GenerateEdge(const WayPoints& curWayPoint) TX_NOEXCEPT;
  /**
   * @brief 从起点到终点生成路线
   *
   * 根据给定的起点和终点，生成路线并返回true。如果生成失败，返回false。
   *
   * @param startWayPoint 起点的路线
   * @param endWayPoint 终点的路线
   * @return bool 生成路线是否成功
   */
  Base::txBool GenerateRouteStartEnd(const WayPoints& startWayPoint, const WayPoints& endWayPoint) TX_NOEXCEPT;

 public:
  using EdgePair = std::pair<Base::txSize, Base::txSize>;
  using EdgePairArray = std::vector<EdgePair>;
  using EdgeWeightType = Base::txFloat;

  struct RouteAI_EdgeData {
    using edge_dist_type = Base::txFloat;
    /**
     * @brief 获取权重
     *
     * 获取当前图的权重。
     *
     * @return edge_dist_type 当前权重值
     */
    edge_dist_type getWeight() { return weight; }
    /**
     * @brief 返回最大距离
     *
     * 这个函数返回一个最大距离值，通常情况下与图中的边长度有关。
     *
     * @return edge_dist_type 返回最大距离值
     */
    static edge_dist_type MaxDistance() TX_NOEXCEPT { return 1000.0; }
    /**
     * @brief 检查当前车辆是否需要保持直线
     *
     * 当前车辆是否需要保持直线，如果需要保持直线，则返回true，否则返回false
     * @return Base::txBool 当前车辆是否需要保持直线
     */
    Base::txBool NeedStraight() const TX_NOEXCEPT {
      return (_plus_(Base::Enums::VehicleMoveLaneState::eStraight) == m_lanechange_action);
    }
    /**
     * @brief 是否需要转向向左
     *
     * 当前车辆是否需要转向向左，如果需要，返回 true，否则返回 false
     * @return Base::txBool 是否需要转向向左
     */
    Base::txBool NeedTurnLeft() const TX_NOEXCEPT {
      return (_plus_(Base::Enums::VehicleMoveLaneState::eLeft) == m_lanechange_action);
    }
    /**
     * @brief 检查车辆是否需要转向向右
     *
     * 当前车辆是否需要转向向右，如果需要，返回 true，否则返回 false
     * @return Base::txBool 当前车辆是否需要转向向右
     */
    Base::txBool NeedTurnRight() const TX_NOEXCEPT {
      return (_plus_(Base::Enums::VehicleMoveLaneState::eRight) == m_lanechange_action);
    }

    friend std::ostream& operator<<(std::ostream& os, const RouteAI_EdgeData& v) TX_NOEXCEPT {
      os << "{" << TX_VARS_NAME(start_joint, v.m_fromJointId) << TX_VARS_NAME(end_joint, v.m_toJointId)
         << TX_VARS_NAME(action, __enum2lpsz__(VehicleMoveLaneState, v.m_lanechange_action))
         << TX_VARS_NAME(changeCnt, v.m_lanechange_count) << "}";
      return os;
    }

    edge_dist_type weight;
    EdgeWeightType m_distance = 0.0;
    Base::JointPointId_t m_fromJointId;
    Base::JointPointId_t m_toJointId;
    Base::Enums::VehicleMoveLaneState m_lanechange_action = Base::Enums::VehicleMoveLaneState::eStraight;
    TX_MARK("straight, left, right");
    Base::txUInt m_lanechange_count = 0;

    /**
     * @brief 对象序列化模板函数
     *
     * @tparam Archive 类型
     * @param archive 序列化对象
     */
    template <class Archive>
    void serialize(Archive& archive) {
      archive(_MAKE_NVP_("weight", weight));
      archive(_MAKE_NVP_("distance", m_distance));
      archive(_MAKE_NVP_("fromJointId", m_fromJointId));
      archive(_MAKE_NVP_("toJointId", m_toJointId));
      archive(_MAKE_NVP_("lanechange_action", m_lanechange_action));
      archive(_MAKE_NVP_("lanechange_count", m_lanechange_count));
    }
  };

  struct RouteAI_VertexData {
    using vtx_id_type = Base::txSize;
    vtx_id_type vtx_id;
    explicit RouteAI_VertexData(vtx_id_type _id = -1) : vtx_id(_id) {}
  };

  /**
   * @brief 检查当前路线是否可达
   *
   * @param _laneInfo 当前路线信息
   * @param routeHint 路径推荐信息
   * @return Base::txBool 当前路线是否可达
   */
  Base::txBool IsReachable(const Base::Info_Lane_t& _laneInfo, RouteAI_EdgeData& routeHint) const TX_NOEXCEPT;

  /**
   * @brief 检查当前路线是否可达
   *
   * @param _laneInfo 当前路线信息
   * @param routeHint 路径推荐信息
   * @return Base::txBool 当前路线是否可达
   */
  Base::txBool IsReachable(const Base::txLaneUId& _lane_uid, RouteAI_EdgeData& routeHint) const TX_NOEXCEPT;

  Base::txBool CheckReachableDirection(const Base::Info_Lane_t& _laneInfo,
                                       Base::txInt& dir /*-1: right,1:  left*/) const TX_NOEXCEPT;
  /**
   * @brief 判断当前路线是否可达
   *
   * @param[in] _laneInfo 当前路线信息
   * @return bool 当前路线是否可达
   */
  Base::txBool CheckReachable(const Base::Info_Lane_t& _laneInfo) const TX_NOEXCEPT;

  /**
   * @brief 判断当前路线是否可达
   *
   * @param[in] _laneInfo 当前路线信息
   * @return bool 当前路线是否可达
   */
  Base::txBool CheckReachable(const Base::txLaneUId& _lane_uid) const TX_NOEXCEPT;

  /**
   * @brief 判断当前道路是否可达
   *
   * @param _roadId 道路ID
   * @return Base::txBool 当前道路是否可达
   */
  Base::txBool CheckReachableRoad(const Base::txRoadID& _roadId) const TX_NOEXCEPT;
  /**
   * @brief 获取消亡管理器实例
   *
   * @return const Base::DeadLineAreaManager& 消亡管理器实例
   */
  const Base::DeadLineAreaManager& GetDeadLineMgr() const TX_NOEXCEPT { return m_deadline; }

 protected:
  Base::txBool m_isValid = false;
  Base::txSysId m_RouteId = -1;
  Base::txInt m_SubRouteId = 0;
  Base::txSysId m_VehicleId = -1;
  hadmap::txPoint m_StartPt;
  std::vector<hadmap::txPoint> m_midpoints;
  hadmap::txPoint m_EndPt;
  std::list<Coord::txENU> m_list_waypoints;
  Base::DeadLineAreaManager m_deadline;
  std::map<EdgePair, RouteAI_EdgeData TX_MARK("edge weight")> m_map_idpair_2_edgeLength;
  std::unordered_map<Base::txSize, HdMap::txRoadNetwork::VertexProperty> m_map_vtxId_2_vtxProp;
  std::set<Base::txSize> m_set_start_vtx_id;
  std::set<Base::txSize> m_set_end_vtx_id;
  std::unordered_map<Base::Info_Lane_t, RouteAI_EdgeData, Utils::Info_Lane_t_HashCompare> mMapConnectedLocInfo;
  LaneChangeTrend mTrend = LaneChangeTrend::eNone;
  std::set<Base::txRoadID> mSetWayPointRoadId;
  Base::txBool mExactEndPoint = false;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    archive(_MAKE_NVP_("isValid", m_isValid));
    archive(_MAKE_NVP_("RouteId", m_RouteId));
    archive(_MAKE_NVP_("SubRouteId", m_SubRouteId));
    archive(_MAKE_NVP_("VehicleId", m_VehicleId));
    archive(_MAKE_NVP_("StartPt", m_StartPt));
    archive(_MAKE_NVP_("midpoints", m_midpoints));
    archive(_MAKE_NVP_("EndPt", m_EndPt));
    archive(_MAKE_NVP_("deadline", m_deadline));

    archive(_MAKE_NVP_("mMapConnectedLocInfo", mMapConnectedLocInfo));

    archive(_MAKE_NVP_("SetWayPointRoadId", mSetWayPointRoadId));
    archive(_MAKE_NVP_("ExactEndPoint", mExactEndPoint));
  }
};

TX_NAMESPACE_CLOSE(Component)
TX_NAMESPACE_CLOSE(TrafficFlow)
