// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_component.h"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/transform_value_property_map.hpp>
#include <iterator>
#include "HdMap/tx_road_network.h"
#include "tx_tadsim_flags.h"
#include "tx_timer_on_cpu.h"
#include "tx_way_points.h"
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)
TX_NAMESPACE_OPEN(Component)

#if __TX_Mark__("Route")
#  define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Routing)
void Route::SetRoutingPathInfo(const hadmap::txPoint& _startPt, const std::vector<hadmap::txPoint>& _midpoints,
                               const hadmap::txPoint& _endPt) TX_NOEXCEPT {
  // 如果 FLAGS_LogLevel_Routing 为 true
  if (FLAGS_LogLevel_Routing) {
    // 创建一个 std::ostringstream 对象
    std::ostringstream oss;
    for (const auto& refPair : _midpoints) {
      // 将 refPair 转换为字符串，并复制到 oss 中，以逗号分隔
      oss << Utils::ToString(refPair) << ", ";
    }
    // 输出日志，表示路由信息
    LogInfo << " [Routing]" << TX_VARS_NAME(StartPos, Utils::ToString(_startPt)) << TX_VARS_NAME(MidPos, oss.str())
            << TX_VARS_NAME(EndPos, Utils::ToString(_endPt));
  }

  // 清空 m_roadInfoArray
  m_roadInfoArray.Clear();
  // 创建一个 std::vector<hadmap::txPoint> 对象，用于存储路径点
  std::vector<hadmap::txPoint> pointList;
  // 将起始位置添加到 pointList
  pointList.push_back(__3d_Alt__(_startPt));
  // 遍历中间位置
  for (const auto& refPair : _midpoints) {
    // 检查中间位置的经纬度是否为零
    if (Math::isNotZero(__Lon__(refPair)) && Math::isNotZero(__Lat__(refPair))) {
      pointList.push_back(__3d_Alt__(refPair));
    } else {
      LOG(WARNING) << "Way Point is zero. " << Utils::ToString(refPair);
    }
  }
  // 将终点位置添加到 pointList
  pointList.push_back(__3d_Alt__(_endPt));

  // 使用 std::unique_ptr 创建一个 planner::RoutePlan 对象
  TX_MARK("do not use std::make_unique, gcc not support.");
  m_pathPlanPtr = std::unique_ptr<planner::RoutePlan>(new planner::RoutePlan(txMapHdr));

  // 创建一个 Base::TimingCPU 对象，用于计时
  Base::TimingCPU s_timer;
  // 输出日志，表示开始搜索路径
  LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread) << "searchPath start.";
  s_timer.StartCounter();
  // 调用 m_pathPlanPtr 的 searchPath 方法，搜索路径
  m_pathPlanPtr->searchPath(pointList, m_roadInfoArray.roadInfoArray);
  LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread)
      << "searchPath end." << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds());
  m_roadInfoArray.Reset();
  // 设置 m_roadInfoArray 的路径点为 pointList
  m_roadInfoArray.SetWayPoints(pointList);
}

// 初始化路由信息
Base::txBool Route::Initialize(const hadmap::txPoint& _startPt, const std::vector<hadmap::txPoint>& _midpoints,
                               const hadmap::txPoint& _endPt) TX_NOEXCEPT {
  // 存储传入的起点、中点和终点
  m_StartPt = _startPt;
  m_EndPt = _endPt;
  m_EndPosition.FromWGS84(_endPt);
  m_midpoints = _midpoints;

  // 判断终点的经纬度是否为非零值，如果是，则进行路径计算和路由信息设置
  if (Math::isNotZero(__Lon__(m_EndPt)) && Math::isNotZero(__Lat__(m_EndPt))) {
    SetRoutingPathInfo(StartPt(), MidPts(), EndPt());
    m_deadline.Initialize(static_cast<Coord::txWGS84>(m_EndPt));
    m_isValid = true;
  }
  return true;
}

Base::txBool Route::ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid,
                                         const Coord::txENU& elementPt) const TX_NOEXCEPT {
  return m_deadline.ArriveAtDeadlineArea(elemLaneUid, elementPt);
}
#  undef LogInfo

#endif /*__TX_Mark__("Route")*/

#if __TX_Mark__("RouteAI")
#  define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Routing)

Base::txBool RouteAI::Initialize(const Base::txSysId routeId, const Base::txInt subRouteId,
                                 const Base::txSysId vehicleId, const hadmap::txPoint& _startPt,
                                 const std::vector<hadmap::txPoint>& _midpoints, const hadmap::txPoint& _endPt,
                                 const std::vector<int64_t>& roadIds) TX_NOEXCEPT {
  // 存储传入的路由ID、子路由ID和车辆ID
  m_RouteId = routeId;
  m_SubRouteId = subRouteId;
  m_VehicleId = vehicleId;
  // 存储传入的起点、中点和终点
  m_StartPt = _startPt;
  m_midpoints = _midpoints;
  m_EndPt = _endPt;
  // 清空 mSetWayPointRoadId 和 m_list_waypoints
  mSetWayPointRoadId.clear();
  m_list_waypoints.clear();
  // 判断起点的经纬度是否为非零值，如果是，则将起点添加到 m_list_waypoints
  if (Utils::IsGPSValid(m_StartPt)) {
    m_list_waypoints.emplace_back(Coord::txWGS84(m_StartPt).ToENU());
  } else {
    m_isValid = false;
    LogWarn << "start gps point is invalid. " << Utils::ToString(m_StartPt);
    return false;
  }

  // 遍历中点，判断中点的经纬度是否为非零值，如果是，则将中点添加到 m_list_waypoints
  for (const auto& refMidPt : m_midpoints) {
    if (Utils::IsGPSValid(refMidPt)) {
      m_list_waypoints.emplace_back(Coord::txWGS84(refMidPt).ToENU());
    }
  }

  // 判断终点的经纬度是否为非零值，如果是，则将终点添加到 m_list_waypoints，并设置截止时间
  if (Utils::IsGPSValid(m_EndPt)) {
    m_list_waypoints.emplace_back(Coord::txWGS84(m_EndPt).ToENU());
    m_deadline.Initialize(m_list_waypoints.back().ToWGS84());
    // 设置路径信息
    m_isValid = SetRoutingPathInfo(roadIds);
  } else {
    m_isValid = false;
    LogInfo << "goal/end gps point is invalid. " << Utils::ToString(m_EndPt);
    return false;
  }

  // 创建一个 std::ostringstream 对象，用于存储路径点
  std::ostringstream oss;
  // 将 m_list_waypoints 中的路径点复制到 oss 中，以分号分隔
  std::copy(m_list_waypoints.begin(), m_list_waypoints.end(), std::ostream_iterator<Coord::txENU>(oss, ";"));
  LogInfo << TX_VARS(VehicleId()) << TX_VARS(RouteId()) << TX_VARS_NAME(way_point_size, m_list_waypoints.size())
          << TX_VARS_NAME(way_points, oss.str());
  return IsValid();
}

Base::txBool RouteAI::ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid,
                                           const Coord::txENU& elementPt) const TX_NOEXCEPT {
  // 如果路由信息有效且到达截止区域，则返回 true，否则返回 false
  return (m_isValid && m_deadline.ArriveAtDeadlineArea(elemLaneUid, elementPt));
}

Base::txString RouteAI::Str() const TX_NOEXCEPT {
  /*
  std::set< Base::txSize > m_set_vtxId;
  std::set< std::tuple< Base::txSize, Base::txSize> > m_set_edge;
  HdMap::txRoadNetwork::RouteGraph m_route_graph;
  */
  std::ostringstream oss, oss_route_joint, oss_all_way_points;

  // 将 m_list_waypoints 中的所有点的坐标转换为字符串，并存储到 oss_all_way_points 中
  std::copy(m_list_waypoints.begin(), m_list_waypoints.end(),
            std::ostream_iterator<Coord::txENU>(oss_all_way_points, ";"));

  // 遍历 m_map_idpair_2_edgeLength 并将每条边的两个顶点的属性转换为字符串，并存储到 oss_route_joint 中
  for (const auto vtxIdPair : m_map_idpair_2_edgeLength) {
    const auto& refStartVtxProp = m_map_vtxId_2_vtxProp.at(std::get<0>(vtxIdPair.first));
    const auto& refEndVtxProp = m_map_vtxId_2_vtxProp.at(std::get<1>(vtxIdPair.first));

    oss_route_joint << "{" << refStartVtxProp << ", " << refEndVtxProp << "},";
  }

  // 将 m_VehicleId, m_RouteId, m_isValid, m_all_way_points, m_deadline, m_route_graph 转换为字符串并存储到 oss 中
  oss << TX_VARS_NAME(VehicleId, m_VehicleId) << TX_VARS_NAME(RouteId, m_RouteId)
      << TX_VARS_NAME(Valid, bool2lpsz(m_isValid)) << TX_VARS_NAME(WayPointList, oss_all_way_points.str())
      << TX_VARS_NAME(DeadLine, m_deadline) << TX_VARS_NAME(RouteResult, oss_route_joint.str());
  return oss.str();
}

Base::txBool RouteAI::CheckWaypointArea(const std::vector<WayPoints>& vec_waypoint_area) TX_NOEXCEPT {
  if (vec_waypoint_area.size() > 1) {
    // 遍历 vec_waypoint_area
    for (const auto& wp : vec_waypoint_area) {
      // 如果当前元素不是有效的路径点
      if (CallFail(wp.IsValid())) {
        // 设置 m_isValid 为 false
        m_isValid = false;
        LogWarn << _AI_TRAFFIC_ << " Way Point is invalid." << TX_VARS(RouteId()) << TX_VARS(VehicleId());
        return false;
      }
    }

    // 如果 GenerateRouteStartEnd 函数返回失败
    if (CallFail(GenerateRouteStartEnd(vec_waypoint_area.front(), vec_waypoint_area.back()))) {
      // 设置 m_isValid 为 false
      m_isValid = false;
      LogWarn << _AI_TRAFFIC_ << " GenerateRouteStartEnd failure." << TX_VARS(RouteId()) << TX_VARS(VehicleId());
      return false;
    }

    // 清空 m_map_idpair_2_edgeLength 和 m_map_vtxId_2_vtxProp
    m_map_idpair_2_edgeLength.clear();
    m_map_vtxId_2_vtxProp.clear();
    // 遍历 vec_waypoint_area
    for (const WayPoints& refCurWayPoint : vec_waypoint_area) {
      // 调用 GenerateEdge 函数，传入当前元素
      if (CallFail(GenerateEdge(refCurWayPoint))) {
        // 如果 GenerateEdge 函数返回失败，则设置 m_isValid 为 false
        m_isValid = false;
        LogWarn << _AI_TRAFFIC_ << " GenerateEdge failure." << TX_VARS(RouteId()) << TX_VARS(VehicleId())
                << TX_VARS(refCurWayPoint);
        return false;
      }
    }

    if (FLAGS_LogLevel_Routing) {
      // 创建一个 std::ostringstream 对象，用于构建输出的字符串
      std::ostringstream oss;
      oss << "Route_Joint_Pair_Set : {";
      // 遍历 m_map_vtxId_2_vtxProp
      for (const auto& refVtxId : m_map_vtxId_2_vtxProp) {
        oss << Utils::ToString(refVtxId.second.getWgs84()) << ", ";
      }
      oss << "}";
      LogInfo << oss.str();
    }

    // 如果 m_map_idpair_2_edgeLength 不为空
    if (_NonEmpty_(m_map_idpair_2_edgeLength)) {
      // 如果 FLAGS_LogLevel_Routing 为 true
      if (FLAGS_LogLevel_Routing) {
        // 创建一个 std::ostringstream 对象，用于构建输出的字符串
        std::ostringstream oss;
        oss << TX_VARS(RouteId()) << TX_VARS(VehicleId()) << TX_VARS(SubRouteId()) << "Route_Edge_Info : {"
            << std::endl;
        // 遍历 m_map_idpair_2_edgeLength
        for (const auto& edgeInfo : m_map_idpair_2_edgeLength) {
          /*
          vtx_id_pair_set[std::make_pair(start_VtxProperty.getVertexID(), end_VtxProperty.getVertexID())] = tmpEdgeProp;
          map_vtxId_2_vtxProp[start_VtxProperty.getVertexID()] = start_VtxProperty;
          map_vtxId_2_vtxProp[end_VtxProperty.getVertexID()] = end_VtxProperty;
          */
          // 获取起始顶点 ID 和结束顶点 ID
          const auto start_vertex_id = std::get<0>(edgeInfo.first);
          const auto end_vertex_id = std::get<1>(edgeInfo.first);
          // 获取起始顶点属性和结束顶点属性
          const auto start_vtx_prop = m_map_vtxId_2_vtxProp.at(start_vertex_id);
          const auto end_vtx_prop = m_map_vtxId_2_vtxProp.at(end_vertex_id);

          oss << "{ " << std::endl;
          // 判断起始节点是否在车道链接上
          if (edgeInfo.second.m_fromJointId.isOnLaneLink()) {
            // 获取车道链接指针
            auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
                edgeInfo.second.m_fromJointId.onLinkFromLaneUid(), edgeInfo.second.m_fromJointId.onLinkToLaneUid());

            oss << "[from] joint is link : " << TX_VARS_NAME(fromLaneUid, linkPtr->fromTxLaneId())
                << TX_VARS_NAME(toLaneUid, linkPtr->toTxLaneId());
          } else {
            oss << "[from] joint is lane : " << TX_VARS_NAME(laneUid, edgeInfo.second.m_fromJointId.onLaneUid());
          }

          // 判断结束节点是否在车道链接上
          if (edgeInfo.second.m_toJointId.isOnLaneLink()) {
            auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
                edgeInfo.second.m_toJointId.onLinkFromLaneUid(), edgeInfo.second.m_toJointId.onLinkToLaneUid());
            oss << "[to] joint is link : " << TX_VARS_NAME(fromLaneUid, linkPtr->fromTxLaneId())
                << TX_VARS_NAME(toLaneUid, linkPtr->toTxLaneId());
          } else {
            oss << "[to] joint is lane : " << TX_VARS_NAME(laneUid, edgeInfo.second.m_toJointId.onLaneUid());
          }
          oss << "}, " << std::endl;
        }
        LogInfo << oss.str();
      }
      return true;
    } else {
      m_isValid = false;
      LogWarn << _AI_TRAFFIC_ << " m_map_idpair_2_edgeLength is empty." << TX_VARS(RouteId()) << TX_VARS(VehicleId());
      return false;
    }
  } else {
    m_isValid = false;
    LogWarn << _AI_TRAFFIC_ << TX_VARS(vec_waypoint_area.size()) << TX_COND(vec_waypoint_area.size() > 1);
    return false;
  }
}

Base::txBool RouteAI::ConvertRoadInfo2JointPointList(const std::vector<hadmap::txPoint>& wayPointList,
                                                     const std::vector<int64_t>& roadInfoArray) TX_NOEXCEPT {
  using txInt = Base::txInt;
  std::vector<WayPoints> vec_waypoint_area;
  if (wayPointList.size() > 1 && roadInfoArray.size() > 0) {
    vec_waypoint_area.clear();
    mSetWayPointRoadId.clear();
    LogInfo << "call SetWayPoints.";
    // 遍历路径点
    for (const auto refRoadInfo : roadInfoArray) {
      // 获取当前路径点的道路ID
      const auto curRoadId = refRoadInfo;
      // 判断道路ID是否有效
      if (Utils::IsRoadValid(curRoadId) TX_MARK("road_id may be zero.")) {
        TX_MARK("lane");
        // 将当前道路ID插入到 mSetWayPointRoadId 中
        mSetWayPointRoadId.insert(curRoadId);
        // 获取当前道路ID对应的道路指针
        const auto curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(curRoadId);
        // 判断道路指针是否非空
        if (NonNull_Pointer(curRoadPtr)) {
          const txInt nSectionSize = curRoadPtr->getSections().size();
          // 遍历当前道路的所有区段
          for (txInt s = 0; s < nSectionSize; ++s) {
            WayPoints wpMgr;
            wpMgr.Initialize(curRoadId, s);
            vec_waypoint_area.push_back(wpMgr);
          }
        } else {
          m_isValid = false;
          LogWarn << "unknown road id : " << TX_VARS(curRoadId);
          return false;
        }
      } else {
        TX_MARK("lane link");
        // 如果 vec_waypoint_area 非空，则初始化一个 WayPoints 对象，设置为车道链接类型，并添加到 vec_waypoint_area 中
        if (_NonEmpty_(vec_waypoint_area)) {
          WayPoints wpMgr;
          wpMgr.Initialize(-1, Base::txLaneUId(), Base::txLaneUId());
          vec_waypoint_area.push_back(wpMgr);
        } else {
          m_isValid = false;
          LogWarn << "un-support first route on lanelink. " << TX_COND(_NonEmpty_(vec_waypoint_area));
          return false;
        }
      }
    }

    // 创建一个临时的 WayPoints 向量，用于存储路径点
    std::vector<WayPoints> tmp_vec_waypoint_area;
    // 预分配 tmp_vec_waypoint_area 的大小，以提高性能
    tmp_vec_waypoint_area.reserve(wayPointList.size());
    // 遍历路径点列表
    for (const auto& refWayPoint : wayPointList) {
      WayPoints wayPointArea;
      // 将路径点转换为 WGS84 坐标
      Coord::txWGS84 wayPt(refWayPoint);
      // 初始化 WayPoints 对象，设置路径点坐标
      wayPointArea.Initialize(wayPt);
      // 将 WayPoints 对象添加到 tmp_vec_waypoint_area 中
      tmp_vec_waypoint_area.push_back(wayPointArea);
      LogInfo << TX_VARS_NAME(VehicleId, m_VehicleId) << TX_VARS_NAME(RouteId, m_RouteId) << "way_point "
              << Utils::ToString(wayPointArea.GetLocLaneUid());
    }

    {
      TX_MARK("erase prev start point road.section");
      // 获取临时 WayPoints 向量的第一个元素的道路区段信息
      const Base::txLaneUId& refWayPointStartPtUid = tmp_vec_waypoint_area[0].GetLocLaneUid();
      // 输出日志，表示车辆ID、路由ID和起点道路区段信息
      LogInfo << TX_VARS_NAME(VehicleId, m_VehicleId) << TX_VARS_NAME(RouteId, m_RouteId)
              << "[start_point_road_section] " << Utils::ToString(refWayPointStartPtUid);
      // 创建一个迭代器，指向 vec_waypoint_area 的开始位置
      auto itr = vec_waypoint_area.begin();
      for (; itr != vec_waypoint_area.end();) {
        // 获取当前元素的道路区段信息
        const Base::txLaneUId& refRouteLaneUid = (*itr).GetLocLaneUid();
        // 判断当前元素的道路区段信息是否与临时 WayPoints 向量的第一个元素的道路区段信息相同
        if (CallFail(refRouteLaneUid.roadId == refWayPointStartPtUid.roadId &&
                     refRouteLaneUid.sectionId == refWayPointStartPtUid.sectionId)) {
          // 如果相同，则删除当前元素
          itr = vec_waypoint_area.erase(itr);
        } else {
          TX_MARK("keep start point lane. ");
          break;
        }
      }
    }

    {
      TX_MARK("erase next end point road.section");
      // 获取临时 WayPoints 向量的最后一个元素的道路区段信息
      const Base::txLaneUId& refWayPointEndPtUid =
          tmp_vec_waypoint_area[tmp_vec_waypoint_area.size() - 1].GetLocLaneUid();
      LogInfo << TX_VARS_NAME(VehicleId, m_VehicleId) << TX_VARS_NAME(RouteId, m_RouteId) << "[end_point_road_section] "
              << Utils::ToString(refWayPointEndPtUid);
      // 创建一个迭代器，指向 vec_waypoint_area 的开始位置
      auto itr = vec_waypoint_area.begin();
      // 初始化一个标志变量，表示是否到达终点道路区段
      Base::txBool bEndFlag = false;
      for (; itr != vec_waypoint_area.end();) {
        // 如果已经到达终点道路区段，则删除当前元素
        if (bEndFlag) {
          itr = vec_waypoint_area.erase(itr);
        } else {
          // 获取当前元素的道路区段信息
          const Base::txLaneUId& refRouteLaneUid = (*itr).GetLocLaneUid();
          // 判断当前元素的道路区段信息是否与临时 WayPoints 向量的最后一个元素的道路区段信息相同
          if (CallSucc(refRouteLaneUid.roadId == refWayPointEndPtUid.roadId &&
                       refRouteLaneUid.sectionId == refWayPointEndPtUid.sectionId)) {
            TX_MARK("keep end point lane.");
            bEndFlag = true;
          }
          ++itr;
        }
      }
    }
#  if 1
    // 创建一个临时的 map，用于存储相同道路区段的路径点
    std::map<std::pair<Base::txRoadID, Base::txSectionID>, std::vector<WayPoints> > tmp_pointAreasInSameSection;
    // 遍历临时的 WayPoints 向量
    for (const auto& refWayPoint : tmp_vec_waypoint_area) {
      // 遍历 vec_waypoint_area
      for (const auto& refWayPointArea : vec_waypoint_area) { /*for (int i = 0; i < vec_waypoint_area.size(); ++i)*/
        // 如果当前元素不是车道链接区域
        if (CallFail(refWayPointArea.IsLaneLinkArea())) {
          // 获取当前元素的道路区段信息
          const Base::txLaneUId& refRouteLaneUid = refWayPointArea.GetLocLaneUid();
          const Base::txLaneUId& refWayPointLaneUid = refWayPoint.GetLocLaneUid();
          // 判断当前元素的道路区段信息是否与临时 WayPoints 向量中的路径点的道路区段信息相同
          if (refRouteLaneUid.roadId == refWayPointLaneUid.roadId &&
              refRouteLaneUid.sectionId == refWayPointLaneUid.sectionId) {
            // 如果相同，则将临时 WayPoints 向量中的路径点添加到临时 map 中
            tmp_pointAreasInSameSection[std::make_pair(refRouteLaneUid.roadId, refRouteLaneUid.sectionId)].push_back(
                refWayPoint);
          }
        }
      }
    }

    // 遍历临时的 map，用于存储相同道路区段的路径点
    for (const auto& refPointAreaVec : tmp_pointAreasInSameSection) {
      // 获取当前元素的道路区段信息
      Base::txRoadID _roadId = 0;
      Base::txSectionID _sectionId = 0;
      std::tie(_roadId, _sectionId) = refPointAreaVec.first;
      for (txInt i = 0; i < vec_waypoint_area.size(); ++i) {
        // 如果当前元素不是车道链接区域
        if (CallFail(vec_waypoint_area[i].IsLaneLinkArea())) {
          const Base::txLaneUId& refRouteLaneUid = vec_waypoint_area[i].GetLocLaneUid();
          // 判断当前元素的道路区段信息是否与临时 map 中的道路区段信息相同
          if (CallSucc(refRouteLaneUid.roadId == _roadId && refRouteLaneUid.sectionId == _sectionId)) {
            // 如果相同，则将临时 map 中的路径点插入到 vec_waypoint_area 中
            auto insert_itr = std::begin(vec_waypoint_area) + i;
            vec_waypoint_area.insert(insert_itr, std::begin(refPointAreaVec.second), std::end(refPointAreaVec.second));
            // 删除 vec_waypoint_area 中的当前元素
            auto delete_itr = std::begin(vec_waypoint_area) + i + refPointAreaVec.second.size();
            vec_waypoint_area.erase(delete_itr);
            break;
          }
        }
      }
    }
#  endif
    // 如果 vec_waypoint_area 不为空
    if (_NonEmpty_(vec_waypoint_area)) {
      // 如果 vec_waypoint_area 的第一个元素和最后一个元素都是路径点区域
      if (CallSucc(vec_waypoint_area.front().IsPointArea()) && CallSucc(vec_waypoint_area.back().IsPointArea())) {
        // 遍历 vec_waypoint_area
        for (txInt i = 0; i < vec_waypoint_area.size(); ++i) {
          // 如果当前元素是车道链接区域
          if (CallSucc(vec_waypoint_area[i].IsLaneLinkArea())) {
            // 获取当前元素的前一个元素的道路ID
            const Base::txRoadID fromRoadId = vec_waypoint_area[i - 1].GetLocLaneUid().roadId;
            const Base::txRoadID toRoadId = vec_waypoint_area[i + 1].GetLocLaneUid().roadId;
            const Base::LocInfoSet set_lanelink =
                HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(fromRoadId, toRoadId);
            // 如果车道链接信息不为空
            if (_NonEmpty_(set_lanelink)) {
              // 初始化当前元素的车道链接信息
              vec_waypoint_area[i].Initialize(set_lanelink);
            } else {
              m_isValid = false;
              LogWarn << TX_VARS(VehicleId()) << "can not find lanelink between : " << TX_VARS(fromRoadId)
                      << TX_VARS(toRoadId);
              return false;
            }
          }
        }
      } else {
        // 如果 vec_waypoint_area 的第一个元素和最后一个元素不是路径点区域，则设置 m_isValid 为 false，并输出日志
        m_isValid = false;
        LogWarn << TX_VARS(VehicleId()) << " vec_waypoint_area front back is not point area.";
        return false;
      }
    } else {
      m_isValid = false;
      LogWarn << TX_VARS(VehicleId()) << TX_COND(_NonEmpty_(vec_waypoint_area));
      return false;
    }

    std::ostringstream oss_log;
    oss_log << _AI_TRAFFIC_ << "[Routing_Result] " << TX_VARS(RouteId()) << TX_VARS(VehicleId());
    std::copy(vec_waypoint_area.begin(), vec_waypoint_area.end(), std::ostream_iterator<WayPoints>(oss_log, ","));
    oss_log << "[Routing_Result_End]";
    LogInfo << oss_log.str();
  } else {
    // 如果 vec_waypoint_area 为空，则设置 m_isValid 为 false，并输出日志
    m_isValid = false;
    LogWarn << _AI_TRAFFIC_ << TX_VARS(wayPointList.size()) << TX_VARS(roadInfoArray.size())
            << TX_COND(wayPointList.size() > 1) << TX_COND(roadInfoArray.size() > 0);
    return false;
  }

  return CheckWaypointArea(vec_waypoint_area);
}

#  if __TX_Mark__("GenerateEdge")
#    if 1
Base::txBool RouteAI::GenerateRouteStartEnd(const WayPoints& startWayPoint, const WayPoints& endWayPoint) TX_NOEXCEPT {
  BOOST_ASSERT(startWayPoint.IsPointArea() && endWayPoint.IsPointArea());
  {
    // 清空 m_set_start_vtx_id
    m_set_start_vtx_id.clear();
    const auto& curWayPoint = startWayPoint;
    // 获取当前 WayPoint 所在的车道 ID
    const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
    // 获取当前 WayPoint 所在的区段 ID
    Base::txSectionUId currSectionUid = std::make_pair(currLaneUid.roadId, currLaneUid.sectionId);
    // 获取当前区段下的所有车道
    const hadmap::txLanes currLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(currSectionUid);
    for (const auto& refCurrLanePtr : currLanes) {
      // 判断车道是否可行驶
      if (Utils::IsLaneValidDriving(refCurrLanePtr)) {
        // 获取当前车道 ID
        const Base::txLaneUId currLaneUid = refCurrLanePtr->getTxLaneId();
        // 获取当前车道起始节点的 JointPointId
        const Base::JointPointId_t curr_lane_start_jointId = Base::JointPointId_t::MakeJointStartPointId(currLaneUid);
        // 获取当前车道起始节点的顶点属性
        const HdMap::txRoadNetwork::VertexProperty start_VtxProperty =
            HdMap::txRoadNetwork::GetVtxProperty(curr_lane_start_jointId);
        m_set_start_vtx_id.insert(start_VtxProperty.getVertexID());
      }
    }
  }
  // 如果 mExactEndPoint 调用失败
  if (CallFail(mExactEndPoint)) {
    TX_MARK("multi end point.");
    // 清空 m_set_end_vtx_id
    m_set_end_vtx_id.clear();
    // 获取当前 WayPoint
    const auto& curWayPoint = endWayPoint;
    // 获取当前 WayPoint 所在的车道 ID
    const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
    // 获取当前 WayPoint 所在的区段 ID
    Base::txSectionUId currSectionUid = std::make_pair(currLaneUid.roadId, currLaneUid.sectionId);
    // 获取当前区段下的所有车道
    const hadmap::txLanes currLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(currSectionUid);
    for (const auto& refCurrLanePtr : currLanes) {
      // 判断车道是否可行驶
      if (Utils::IsLaneValidDriving(refCurrLanePtr)) {
        // 获取当前车道 ID
        const Base::txLaneUId currLaneUid = refCurrLanePtr->getTxLaneId();
        const Base::JointPointId_t curr_lane_end_jointId = Base::JointPointId_t::MakeJointEndPointId(currLaneUid);
        // 获取当前车道起始节点的顶点属性
        const HdMap::txRoadNetwork::VertexProperty end_VtxProperty =
            HdMap::txRoadNetwork::GetVtxProperty(curr_lane_end_jointId);
        // 将当前车道起始节点的顶点 ID 插入到 m_set_end_vtx_id 中
        m_set_end_vtx_id.insert(end_VtxProperty.getVertexID());
      }
    }
  } else {
    TX_MARK("exact end point, perfect planning.");
    m_set_end_vtx_id.clear();
    const auto& curWayPoint = endWayPoint;
    // 获取当前 WayPoint 所在的车道 ID
    const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
    // 获取当前车道指针
    const hadmap::txLanePtr refCurrLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(currLaneUid);

    // 判断车道是否可行驶
    if (Utils::IsLaneValidDriving(refCurrLanePtr)) {
      const Base::txLaneUId currLaneUid = refCurrLanePtr->getTxLaneId();
      // 获取当前车道起始节点的 JointPointId
      const Base::JointPointId_t curr_lane_end_jointId = Base::JointPointId_t::MakeJointEndPointId(currLaneUid);
      const HdMap::txRoadNetwork::VertexProperty end_VtxProperty =
          HdMap::txRoadNetwork::GetVtxProperty(curr_lane_end_jointId);
      // 将当前车道起始节点的顶点 ID 插入到 m_set_end_vtx_id 中
      m_set_end_vtx_id.insert(end_VtxProperty.getVertexID());
    }
  }
  return (_NonEmpty_(m_set_start_vtx_id)) && (_NonEmpty_(m_set_end_vtx_id));
}

// 函数生成Edge point
// 输入参数:
// curWayPoint: 当前道路点
// vtx_id_pair_set: 存储边属性的map
// map_vtxId_2_vtxProp: 存储顶点属性的map
// 返回值: 函数是否成功执行的标志
Base::txBool GenerateEdge_point(
    const WayPoints& curWayPoint, std::map<RouteAI::EdgePair, RouteAI::RouteAI_EdgeData>& vtx_id_pair_set,
    std::unordered_map<Base::txSize, HdMap::txRoadNetwork::VertexProperty>& map_vtxId_2_vtxProp) TX_NOEXCEPT {
  // 验证输入参数的合法性
  BOOST_ASSERT(curWayPoint.IsPointArea());
  // 获取当前道路点的车道
  const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
  Base::txSectionUId currSectionUid = std::make_pair(currLaneUid.roadId, currLaneUid.sectionId);
  const hadmap::txLanes currLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(currSectionUid);

  // 遍历所有车道
  const Base::txInt nLaneSize = currLanes.size();
  for (Base::txInt i = 0; i < nLaneSize; ++i) {
    const auto& refCurrLanePtr = currLanes[i];
    const auto lane_length = refCurrLanePtr->getLength();
    Base::txInt nMaxLaneChangeSize = 0;
    // 计算车道可变宽度的限制
    if (lane_length > FLAGS_Enable_LaneChange_Length) {
      nMaxLaneChangeSize = 1;
    }

    // 判断车道是否有效
    if (Utils::IsLaneValidDriving(refCurrLanePtr)) {
      const Base::txLaneUId& fromLaneUid = refCurrLanePtr->getTxLaneId();
      const Base::txFloat curLaneLength = refCurrLanePtr->getLength();
      RouteAI::RouteAI_EdgeData tmpEdgeProp;
      // 设置起始点和终点的JointPointId
      tmpEdgeProp.m_fromJointId = Base::JointPointId_t::MakeJointStartPointId(fromLaneUid);
      // 遍历所有车道，生成边和顶点属性
      for (Base::txInt j = 0; j < nLaneSize; ++j) {
        const auto& refDestLanePtr = currLanes[j];
        if (Utils::IsLaneValidDriving(refDestLanePtr)) {
          const Base::txLaneUId& toLaneUid = refDestLanePtr->getTxLaneId();
          tmpEdgeProp.m_toJointId = Base::JointPointId_t::MakeJointEndPointId(toLaneUid);
          // 计算车道变更次数
          const auto gapLaneId = fromLaneUid.laneId - toLaneUid.laneId;
          if (std::abs(gapLaneId) > nMaxLaneChangeSize) {
            continue;
          }
          // 计算车道变更类型和次数
          if (gapLaneId > 0) {
            TX_MARK("eg: -2 - (-3) = 1 > 0, need turn right.");
            tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eRight;
            tmpEdgeProp.m_lanechange_count = std::abs(gapLaneId);
            tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);
          } else if (gapLaneId < 0) {
            TX_MARK("eg: -2 - (-1) = -1 < 0, need turn left.");
            tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eLeft;
            tmpEdgeProp.m_lanechange_count = std::abs(gapLaneId);
            tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);
          } else {
            TX_MARK("eg: -2 - (-2) = 0 == 0, need go straight.");
            tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eStraight;
            tmpEdgeProp.m_lanechange_count = 0;
            tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);
          }

          // 生成边和顶点属性
          const Base::JointPointId_t& curr_lane_start_jointId = tmpEdgeProp.m_fromJointId;
          const Base::JointPointId_t& curr_lane_end_jointId = tmpEdgeProp.m_toJointId;
          const HdMap::txRoadNetwork::VertexProperty start_VtxProperty =
              HdMap::txRoadNetwork::GetVtxProperty(curr_lane_start_jointId);
          const HdMap::txRoadNetwork::VertexProperty end_VtxProperty =
              HdMap::txRoadNetwork::GetVtxProperty(curr_lane_end_jointId);
          vtx_id_pair_set[std::make_pair(start_VtxProperty.getVertexID(), end_VtxProperty.getVertexID())] = tmpEdgeProp;
          map_vtxId_2_vtxProp[start_VtxProperty.getVertexID()] = start_VtxProperty;
          map_vtxId_2_vtxProp[end_VtxProperty.getVertexID()] = end_VtxProperty;
        }
      }
    }
  }
  return true;
}

Base::txBool GenerateEdge_roadsection(
    const WayPoints& curWayPoint, std::map<RouteAI::EdgePair, RouteAI::RouteAI_EdgeData>& vtx_id_pair_set,
    std::unordered_map<Base::txSize, HdMap::txRoadNetwork::VertexProperty>& map_vtxId_2_vtxProp) TX_NOEXCEPT {
  BOOST_ASSERT(curWayPoint.IsRoadSectionArea());
  // 获取当前 WayPoint 所在的车道 ID
  const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
  Base::txSectionUId currSectionUid = std::make_pair(currLaneUid.roadId, currLaneUid.sectionId);
  const hadmap::txLanes currLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(currSectionUid);
  // 获取当前车道所在的道路指针
  const hadmap::txRoadPtr curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(currLaneUid.roadId);
  const auto nCurSectionCnt = curRoadPtr->getSections().size();
  // 判断当前区段是否为最后一个区段
  const Base::txBool isLastSection = ((nCurSectionCnt - 1) == currLaneUid.sectionId);
  const Base::txBool isSingleSectionRoad = (1 == nCurSectionCnt);
  // 获取当前区段下的车道数量
  const Base::txInt nLaneSize = currLanes.size();
  const Base::txFloat Enable_LaneChange_Length_X2 = FLAGS_Enable_LaneChange_Length * 2.0;
  // 遍历当前区段下的所有车道
  for (Base::txInt i = 0; i < nLaneSize; ++i) {
    const auto& refCurrLanePtr = currLanes[i];
    const auto lane_length = refCurrLanePtr->getLength();
    Base::txInt nMaxLaneChangeSize = 0;
    // 判断当前车道是否可以变道
    if (CallSucc(lane_length > FLAGS_Enable_LaneChange_Length) &&
        (CallFail(isLastSection) ||
         (CallSucc(isSingleSectionRoad) && CallSucc(lane_length > Enable_LaneChange_Length_X2)))) {
      nMaxLaneChangeSize = 1;
    } else {
      // 记录当前车道不能变道的信息
      LogInfo << TX_VARS_NAME(currLaneUid, Utils::ToString(currLaneUid)) << " do not lane change. "
              << TX_VARS(lane_length) << TX_VARS(FLAGS_Enable_LaneChange_Length)
              << TX_COND((lane_length > FLAGS_Enable_LaneChange_Length)) << TX_COND(CallFail(isLastSection))
              << TX_COND(CallSucc(isSingleSectionRoad));
    }
    // 判断当前车道是否可行驶
    if (Utils::IsLaneValidDriving(refCurrLanePtr)) {
      const Base::txLaneUId& fromLaneUid = refCurrLanePtr->getTxLaneId();
      const Base::txFloat curLaneLength = refCurrLanePtr->getLength();
      RouteAI::RouteAI_EdgeData tmpEdgeProp;
      tmpEdgeProp.m_fromJointId = Base::JointPointId_t::MakeJointStartPointId(fromLaneUid);
      // 遍历当前区段下的所有车道
      for (Base::txInt j = 0; j < nLaneSize; ++j) {
        const auto& refDestLanePtr = currLanes[j];
        // 判断目标车道是否可行驶
        if (Utils::IsLaneValidDriving(refDestLanePtr)) {
          const Base::txLaneUId& toLaneUid = refDestLanePtr->getTxLaneId();
          tmpEdgeProp.m_toJointId = Base::JointPointId_t::MakeJointEndPointId(toLaneUid);
          const auto gapLaneId = fromLaneUid.laneId - toLaneUid.laneId;
          // 判断车道变道的范围
          if (std::abs(gapLaneId) > nMaxLaneChangeSize) {
            continue;
          }
          // 判断车道变道的方向
          if (gapLaneId > 0) {
            TX_MARK("eg: -2 - (-3) = 1 > 0, need turn right.");
            tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eRight;
            tmpEdgeProp.m_lanechange_count = std::abs(gapLaneId);
            tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);
          } else if (gapLaneId < 0) {
            TX_MARK("eg: -2 - (-1) = -1 < 0, need turn left.");
            tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eLeft;
            tmpEdgeProp.m_lanechange_count = std::abs(gapLaneId);
            tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);
          } else {
            TX_MARK("eg: -2 - (-2) = 0 == 0, need go straight.");
            tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eStraight;
            tmpEdgeProp.m_lanechange_count = 0;
            tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);
          }

          // 获取当前车道起始节点和结束节点的 JointPointId
          const Base::JointPointId_t& curr_lane_start_jointId = tmpEdgeProp.m_fromJointId;
          const Base::JointPointId_t& curr_lane_end_jointId = tmpEdgeProp.m_toJointId;
          // 获取当前车道起始节点和结束节点的顶点属性
          const HdMap::txRoadNetwork::VertexProperty start_VtxProperty =
              HdMap::txRoadNetwork::GetVtxProperty(curr_lane_start_jointId);
          const HdMap::txRoadNetwork::VertexProperty end_VtxProperty =
              HdMap::txRoadNetwork::GetVtxProperty(curr_lane_end_jointId);
          vtx_id_pair_set[std::make_pair(start_VtxProperty.getVertexID(), end_VtxProperty.getVertexID())] = tmpEdgeProp;
          // 将当前车道起始节点和结束节点的顶点 ID 和顶点属性存储到 map_vtxId_2_vtxProp 中
          map_vtxId_2_vtxProp[start_VtxProperty.getVertexID()] = start_VtxProperty;
          map_vtxId_2_vtxProp[end_VtxProperty.getVertexID()] = end_VtxProperty;
        }
      }
    }
  }
  return true;
}

Base::txBool GenerateEdge_lanelink(
    const WayPoints& curWayPoint, std::map<RouteAI::EdgePair, RouteAI::RouteAI_EdgeData>& vtx_id_pair_set,
    std::unordered_map<Base::txSize, HdMap::txRoadNetwork::VertexProperty>& map_vtxId_2_vtxProp) TX_NOEXCEPT {
  // 确保 curWayPoint 是一个lanlink的区域
  BOOST_ASSERT(curWayPoint.IsLaneLinkArea());
  // 获取当前地点的所有link
  const Base::LocInfoSet& allLinkSet = curWayPoint.GetAllLinkSet();
  // 遍历所有连接link
  for (const auto locInfo_link : allLinkSet) {
    const Base::txFloat curLaneLength = 1.0;
    // 为当前link创建一个新的边缘信息对象
    RouteAI::RouteAI_EdgeData tmpEdgeProp;
    tmpEdgeProp.m_fromJointId = Base::JointPointId_t::MakeJointStartPointId(
        locInfo_link.onLinkId_without_equal, locInfo_link.onLinkFromLaneUid, locInfo_link.onLinkToLaneUid);
    tmpEdgeProp.m_toJointId = Base::JointPointId_t::MakeJointEndPointId(
        locInfo_link.onLinkId_without_equal, locInfo_link.onLinkFromLaneUid, locInfo_link.onLinkToLaneUid);
    tmpEdgeProp.m_lanechange_action = Base::Enums::VehicleMoveLaneState::eStraight;
    tmpEdgeProp.m_lanechange_count = 0;
    tmpEdgeProp.m_distance = curLaneLength * (tmpEdgeProp.m_lanechange_count + 1);

    // 获取当前道路的起始和结束点的属性
    const HdMap::txRoadNetwork::VertexProperty start_VtxProperty =
        HdMap::txRoadNetwork::GetVtxProperty(tmpEdgeProp.m_fromJointId);
    const HdMap::txRoadNetwork::VertexProperty end_VtxProperty =
        HdMap::txRoadNetwork::GetVtxProperty(tmpEdgeProp.m_toJointId);

    // 将新创建的边缘信息对象添加到映射中
    vtx_id_pair_set[std::make_pair(start_VtxProperty.getVertexID(), end_VtxProperty.getVertexID())] = tmpEdgeProp;
    // 将起始和结束点的属性添加到映射中
    map_vtxId_2_vtxProp[start_VtxProperty.getVertexID()] = start_VtxProperty;
    map_vtxId_2_vtxProp[end_VtxProperty.getVertexID()] = end_VtxProperty;
  }
  return true;
}

Base::txBool RouteAI::GenerateEdge(const WayPoints& curWayPoint) TX_NOEXCEPT {
  // 根据curWayPoint所在的区域类型生成对应的边缘信息
  if (curWayPoint.IsPointArea()) {
    return GenerateEdge_point(curWayPoint, m_map_idpair_2_edgeLength, m_map_vtxId_2_vtxProp);
  } else if (curWayPoint.IsRoadSectionArea()) {
    return GenerateEdge_roadsection(curWayPoint, m_map_idpair_2_edgeLength, m_map_vtxId_2_vtxProp);
  } else if (curWayPoint.IsLaneLinkArea()) {
    return GenerateEdge_lanelink(curWayPoint, m_map_idpair_2_edgeLength, m_map_vtxId_2_vtxProp);
  } else {
    LogWarn << "un support way point type. [curWayPoint]";
    return false;
  }
}

#    else
Base::txBool GenerateEdge_point2point(const WayPoints& prevWayPoint, const WayPoints& curWayPoint,
                                      std::list<std::tuple<Base::JointPointId_t, Base::JointPointId_t> >& edgeList)
    TX_NOEXCEPT {
  BOOST_ASSERT(prevWayPoint.IsPointArea() && curWayPoint.IsPointArea());
  const Base::JointPointId_t prev_lane_start_jointId =
      Base::JointPointId_t::MakeJointStartPointId(prevWayPoint.GetLocLaneUid());
  const Base::JointPointId_t curr_lane_start_jointId =
      Base::JointPointId_t::MakeJointStartPointId(curWayPoint.GetLocLaneUid());
  edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_lane_start_jointId));
  return true;
}

Base::txBool GenerateEdge_point2roadsection(const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
                                            std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsPointArea() && curWayPoint.IsRoadSectionArea());
  const JointPointId_t prev_lane_start_jointId = JointPointId_t::MakeJointStartPointId(prevWayPoint.GetLocLaneUid());
  const JointPointId_t prev_lane_end_jointId = JointPointId_t::MakeJointEndPointId(prevWayPoint.GetLocLaneUid());
  const HdMap::txRoadNetwork::VertexProperty vtxProperty = HdMap::txRoadNetwork::GetVtxProperty(prev_lane_end_jointId);
  const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refJointIdSet = vtxProperty.RelatJointPointSet();
  const Base::txLaneUId curLaneUid = curWayPoint.GetLocLaneUid();
  Base::txSectionUId curSectionUid = std::make_pair(curLaneUid.roadId, curLaneUid.sectionId);
  const hadmap::txLanes curLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(curSectionUid);
  const Base::txSize nLaneCntOnSection = curLanes.size();
  for (const auto& refLanePtr : curLanes) {
    if (NonNull_Pointer(refLanePtr)) {
      const Base::txLaneUId fuzzyLaneUid = refLanePtr->getTxLaneId();
      const JointPointId_t curr_lane_start_jointId = JointPointId_t::MakeJointStartPointId(fuzzyLaneUid);
      if (_Contain_(refJointIdSet, curr_lane_start_jointId)) {
        TXSTMARK("may be : one to multi");
        edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_lane_start_jointId));
        retV = true;
        if (Utils::HasLeftLane(fuzzyLaneUid)) {
          const Base::txLaneUId fuzzyLeftLaneUid = Utils::LeftLaneUid(fuzzyLaneUid);
          const JointPointId_t curr_left_lane_start_jointId = JointPointId_t::MakeJointStartPointId(fuzzyLeftLaneUid);
          edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_left_lane_start_jointId));
        }

        if (Utils::HasRightLane(fuzzyLaneUid, nLaneCntOnSection)) {
          const Base::txLaneUId fuzzyRightLaneUid = Utils::RightLaneUid(fuzzyLaneUid);
          const JointPointId_t curr_right_lane_start_jointId = JointPointId_t::MakeJointStartPointId(fuzzyRightLaneUid);
          edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_right_lane_start_jointId));
        }
      }
    }
  }
  return retV;
}

Base::txBool GenerateEdge_point2lanelink(const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
                                         std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  TXSTMARK("can be simplified, but, for the sake of preciseness");
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsPointArea() && curWayPoint.IsLaneLinkArea());
  const JointPointId_t prev_lane_start_jointId = JointPointId_t::MakeJointStartPointId(prevWayPoint.GetLocLaneUid());
  const JointPointId_t prev_lane_end_jointId = JointPointId_t::MakeJointEndPointId(prevWayPoint.GetLocLaneUid());
  const HdMap::txRoadNetwork::VertexProperty vtxProperty = HdMap::txRoadNetwork::GetVtxProperty(prev_lane_end_jointId);
  const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refJointIdSet = vtxProperty.RelatJointPointSet();

  const std::set<Base::txLaneLinkID> allLinkSet = curWayPoint.GetAllLinkSet();
  for (const auto linkId : allLinkSet) {
    const JointPointId_t curr_link_start_jointId = JointPointId_t::MakeJointStartPointId(linkId);
    if (_Contain_(refJointIdSet, curr_link_start_jointId)) {
      edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_link_start_jointId));
      retV = true;
    }
  }
  return retV;
}

Base::txBool GenerateEdge_roadsection2point(const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
                                            std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsRoadSectionArea() && curWayPoint.IsPointArea());
  const JointPointId_t curr_lane_start_jointId = JointPointId_t::MakeJointStartPointId(curWayPoint.GetLocLaneUid());
  const HdMap::txRoadNetwork::VertexProperty vtxProperty =
      HdMap::txRoadNetwork::GetVtxProperty(curr_lane_start_jointId);
  const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refJointIdSet = vtxProperty.RelatJointPointSet();

  const Base::txLaneUId prevLaneUid = prevWayPoint.GetLocLaneUid();
  Base::txSectionUId prevSectionUid = std::make_pair(prevLaneUid.roadId, prevLaneUid.sectionId);
  const hadmap::txLanes prevLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(prevSectionUid);
  const Base::txSize nLaneCntOnSection = prevLanes.size();

  for (const auto& refPrevLanePtr : prevLanes) {
    if (NonNull_Pointer(refPrevLanePtr)) {
      const Base::txLaneUId prevLaneUid = refPrevLanePtr->getTxLaneId();
      const JointPointId_t prev_lane_start_jointId = JointPointId_t::MakeJointStartPointId(prevLaneUid);
      const JointPointId_t prev_lane_end_jointId = JointPointId_t::MakeJointEndPointId(prevLaneUid);
      if (_Contain_(refJointIdSet, prev_lane_end_jointId)) {
        retV = true;
        edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_lane_start_jointId));
        if (Utils::HasLeftLane(prevLaneUid)) {
          const Base::txLaneUId fuzzyLeftLaneUid = Utils::LeftLaneUid(prevLaneUid);
          const JointPointId_t prev_left_lane_start_jointId = JointPointId_t::MakeJointStartPointId(fuzzyLeftLaneUid);
          edgeList.push_back(std::make_tuple(prev_left_lane_start_jointId, curr_lane_start_jointId));
        }
        if (Utils::HasRightLane(prevLaneUid, nLaneCntOnSection)) {
          const Base::txLaneUId fuzzyRightLaneUid = Utils::LeftLaneUid(prevLaneUid);
          const JointPointId_t prev_right_lane_start_jointId = JointPointId_t::MakeJointStartPointId(fuzzyRightLaneUid);
          edgeList.push_back(std::make_tuple(prev_right_lane_start_jointId, curr_lane_start_jointId));
        }
      }
    }
  }
  return retV;
}

Base::txBool GenerateEdge_roadsection2roadsection(
    const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
    std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsRoadSectionArea() && curWayPoint.IsRoadSectionArea());

  const Base::txLaneUId prevLaneUid = prevWayPoint.GetLocLaneUid();
  Base::txSectionUId prevSectionUid = std::make_pair(prevLaneUid.roadId, prevLaneUid.sectionId);
  const hadmap::txLanes prevLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(prevSectionUid);
  const Base::txSize nPrevLaneCntOnSection = prevLanes.size();

  const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
  Base::txSectionUId currSectionUid = std::make_pair(currLaneUid.roadId, currLaneUid.sectionId);
  const hadmap::txLanes currLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(currSectionUid);
  const Base::txSize nCurrLaneCntOnSection = currLanes.size();

  for (const auto& refPrevLanePtr : prevLanes) {
    const Base::txLaneUId prevLaneUid = refPrevLanePtr->getTxLaneId();
    const JointPointId_t prev_lane_start_jointId = JointPointId_t::MakeJointStartPointId(prevLaneUid);
    const JointPointId_t prev_lane_end_jointId = JointPointId_t::MakeJointEndPointId(prevLaneUid);
    const HdMap::txRoadNetwork::VertexProperty vtxProperty =
        HdMap::txRoadNetwork::GetVtxProperty(prev_lane_end_jointId);
    const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refPrevJointIdSet =
        vtxProperty.RelatJointPointSet();

    for (const auto& refCurrLanePtr : currLanes) {
      const Base::txLaneUId currLaneUid = refCurrLanePtr->getTxLaneId();
      const JointPointId_t curr_lane_start_jointId = JointPointId_t::MakeJointStartPointId(currLaneUid);
      if (_Contain_(refPrevJointIdSet, curr_lane_start_jointId)) {
        retV = true;
        edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_lane_start_jointId));
        if (Utils::HasLeftLane(currLaneUid)) {
          const Base::txLaneUId fuzzyCurrLeftLaneUid = Utils::LeftLaneUid(currLaneUid);
          const JointPointId_t curr_left_lane_start_jointId =
              JointPointId_t::MakeJointStartPointId(fuzzyCurrLeftLaneUid);
          edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_left_lane_start_jointId));
        }
        if (Utils::HasRightLane(currLaneUid, nCurrLaneCntOnSection)) {
          const Base::txLaneUId fuzzyCurrRightLaneUid = Utils::LeftLaneUid(currLaneUid);
          const JointPointId_t curr_right_lane_start_jointId =
              JointPointId_t::MakeJointStartPointId(fuzzyCurrRightLaneUid);
          edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_right_lane_start_jointId));
        }
      }
    }
  }
  return retV;
}

Base::txBool GenerateEdge_roadsection2lanelink(
    const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
    std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsRoadSectionArea() && curWayPoint.IsLaneLinkArea());

  const Base::txLaneUId prevLaneUid = prevWayPoint.GetLocLaneUid();
  Base::txSectionUId prevSectionUid = std::make_pair(prevLaneUid.roadId, prevLaneUid.sectionId);
  const hadmap::txLanes prevLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(prevSectionUid);

  const std::set<Base::txLaneLinkID> currAllLinkIdSet = curWayPoint.GetAllLinkSet();

  for (const auto& refPrevLanePtr : prevLanes) {
    const Base::txLaneUId prevLaneUid = refPrevLanePtr->getTxLaneId();
    const JointPointId_t prev_lane_start_jointId = JointPointId_t::MakeJointStartPointId(prevLaneUid);
    const JointPointId_t prev_lane_end_jointId = JointPointId_t::MakeJointEndPointId(prevLaneUid);
    const HdMap::txRoadNetwork::VertexProperty vtxProperty =
        HdMap::txRoadNetwork::GetVtxProperty(prev_lane_end_jointId);
    const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refPrevJointIdSet =
        vtxProperty.RelatJointPointSet();

    for (const auto refCurrLaneLinkId : currAllLinkIdSet) {
      const JointPointId_t curr_lanelink_start_jointId = JointPointId_t::MakeJointStartPointId(refCurrLaneLinkId);
      if (_Contain_(refPrevJointIdSet, curr_lanelink_start_jointId)) {
        retV = true;
        edgeList.push_back(std::make_tuple(prev_lane_start_jointId, curr_lanelink_start_jointId));
      }
    }
  }

  return retV;
}

Base::txBool GenerateEdge_lanelink2roadsection(
    const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
    std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsLaneLinkArea() && curWayPoint.IsRoadSectionArea());
  const std::set<Base::txLaneLinkID> prevAllLinkIdSet = prevWayPoint.GetAllLinkSet();

  const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
  Base::txSectionUId currSectionUid = std::make_pair(currLaneUid.roadId, currLaneUid.sectionId);
  const hadmap::txLanes currLanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(currSectionUid);

  for (const auto refPrevLaneLinkId : prevAllLinkIdSet) {
    const JointPointId_t prev_lanelink_start_jointId = JointPointId_t::MakeJointStartPointId(refPrevLaneLinkId);
    const JointPointId_t prev_lanelink_end_jointId = JointPointId_t::MakeJointEndPointId(refPrevLaneLinkId);
    const HdMap::txRoadNetwork::VertexProperty vtxProperty =
        HdMap::txRoadNetwork::GetVtxProperty(prev_lanelink_end_jointId);
    const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refPrevJointIdSet =
        vtxProperty.RelatJointPointSet();

    for (const auto& currLanePtr : currLanes) {
      const Base::txLaneUId currFuzzyLaneUid = currLanePtr->getTxLaneId();
      const JointPointId_t curr_lane_start_jointId = JointPointId_t::MakeJointStartPointId(currFuzzyLaneUid);
      if (_Contain_(refPrevJointIdSet, curr_lane_start_jointId)) {
        retV = true;
        edgeList.push_back(std::make_tuple(prev_lanelink_start_jointId, curr_lane_start_jointId));
      }
    }
  }
  return retV;
}

Base::txBool GenerateEdge_lanelink2point(const WayPointsManager& prevWayPoint, const WayPointsManager& curWayPoint,
                                         std::list<std::tuple<JointPointId_t, JointPointId_t> >& edgeList) NOEXCEPT {
  Base::txBool retV = false;
  BOOST_ASSERT(prevWayPoint.IsLaneLinkArea() && curWayPoint.IsPointArea());
  const std::set<Base::txLaneLinkID> prevAllLinkIdSet = prevWayPoint.GetAllLinkSet();
  const Base::txLaneUId currLaneUid = curWayPoint.GetLocLaneUid();
  const JointPointId_t curr_lane_start_jointId = JointPointId_t::MakeJointStartPointId(currLaneUid);

  for (const auto& refPrevLaneLinkId : prevAllLinkIdSet) {
    const JointPointId_t prev_lanelink_start_jointId = JointPointId_t::MakeJointStartPointId(refPrevLaneLinkId);
    const JointPointId_t prev_lanelink_end_jointId = JointPointId_t::MakeJointEndPointId(refPrevLaneLinkId);
    const HdMap::txRoadNetwork::VertexProperty vtxProperty =
        HdMap::txRoadNetwork::GetVtxProperty(prev_lanelink_end_jointId);
    const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& refPrevJointIdSet =
        vtxProperty.RelatJointPointSet();

    if (_Contain_(refPrevJointIdSet, curr_lane_start_jointId)) {
      retV = true;
      edgeList.push_back(std::make_tuple(prev_lanelink_start_jointId, curr_lane_start_jointId));
    }
  }

  return retV;
}

Base::txBool RouteAI::GenerateEdge(const WayPoints& prevWayPoint, const WayPoints& curWayPoint) TX_NOEXCEPT {
  if (prevWayPoint.IsPointArea()) {
    if (curWayPoint.IsPointArea()) {
      return GenerateEdge_point2point(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else if (curWayPoint.IsRoadSectionArea()) {
      return GenerateEdge_point2roadsection(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else if (curWayPoint.IsLaneLinkArea()) {
      return GenerateEdge_point2lanelink(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else {
      LogWarn << "un support way point type. [curWayPoint]";
      return false;
    }
  } else if (prevWayPoint.IsRoadSectionArea()) {
    if (curWayPoint.IsPointArea()) {
      return GenerateEdge_roadsection2point(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else if (curWayPoint.IsRoadSectionArea()) {
      return GenerateEdge_roadsection2roadsection(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else if (curWayPoint.IsLaneLinkArea()) {
      return GenerateEdge_roadsection2lanelink(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else {
      LogWarn << "un support way point type. [curWayPoint]";
      return false;
    }
  } else if (prevWayPoint.IsLaneLinkArea()) {
    if (curWayPoint.IsPointArea()) {
      return GenerateEdge_lanelink2point(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else if (curWayPoint.IsRoadSectionArea()) {
      return GenerateEdge_lanelink2roadsection(prevWayPoint, curWayPoint, m_list_JointPointTuple);
    } else if (curWayPoint.IsLaneLinkArea()) {
      LogWarn << "impossible lanelink to lanelink.";
      return false;
    } else {
      LogWarn << "un support way point type. [curWayPoint]";
      return false;
    }
  } else {
    LogWarn << "un support way point type. [prevWayPoint]";
    return false;
  }
}
#    endif

#  endif /*__TX_Mark__("GenerateEdge")*/

Base::txBool RouteAI::SetRoutingPathInfo(const std::vector<int64_t>& customRoadInfoArray) TX_NOEXCEPT {
  // 配置日志输出
  if (FLAGS_LogLevel_Routing) {
    std::ostringstream oss_all_way_points;
    std::copy(m_list_waypoints.begin(), m_list_waypoints.end(),
              std::ostream_iterator<Coord::txENU>(oss_all_way_points, ";"));
    LogInfo << _AI_TRAFFIC_ << " [Routing]" << TX_VARS_NAME(RouteId, m_RouteId)
            << TX_VARS_NAME(WayPoints, oss_all_way_points.str());
  }

  // 转换坐标系
  std::vector<hadmap::txPoint> pointList;
  std::transform(m_list_waypoints.begin(), m_list_waypoints.end(), std::back_inserter(pointList),
                 [](Coord::txENU& value) { return value.ToWGS84().WGS84(); });

  // 判断是否使用自定义路线信息
  if (_NonEmpty_(customRoadInfoArray)) {
    // 转换路线信息
    m_isValid = ConvertRoadInfo2JointPointList(pointList, customRoadInfoArray);
  } else {
    // 使用路径规划器计算路线信息
    TX_MARK("do not use std::make_unique, gcc not support.");
    auto pathPlanPtr = std::unique_ptr<planner::RoutePlan>(new planner::RoutePlan(txMapHdr));

    // 计算路线信息的时间开始
    Base::TimingCPU t_timer;
    LogInfo << "searchPath start." << TX_VARS(RouteId());
    t_timer.StartCounter();
    // 使用路径规划器计算路线信息
    planner::RoadInfoArray roadInfoArray;
    pathPlanPtr->searchPath(pointList, roadInfoArray);
    // 计算路线信息的时间结束
    LogInfo << "searchPath end." << TX_VARS(RouteId()) << TX_VARS_NAME(timer, t_timer.GetElapsedMicroseconds());

    // 转换路线信息
    std::vector<int64_t> recommendRoadInfoArray;
    std::transform(roadInfoArray.begin(), roadInfoArray.end(), std::back_inserter(recommendRoadInfoArray),
                   [](planner::RoadInfo& value) { return value.roadId; });
    if (FLAGS_LogLevel_Routing) {
      std::ostringstream oss;
      std::copy(recommendRoadInfoArray.begin(), recommendRoadInfoArray.end(), std::ostream_iterator<int64_t>(oss, " "));
      LogInfo << "recommendRoadInfoArray : " << TX_VARS(RouteId()) << TX_VARS(oss.str());
    }
    // 转换路径信息到 point list 中
    m_isValid = ConvertRoadInfo2JointPointList(pointList, recommendRoadInfoArray);
  }
  return IsValid();
}

Base::txBool RouteAI::ResetEdgeWeight() TX_NOEXCEPT {
  for (auto& refEdgeWeightPair : m_map_idpair_2_edgeLength) {
    TX_MARK("hz8010 1"); /*refEdgeWeightPair.second.weight = refEdgeWeightPair.second.m_distance;*/
    refEdgeWeightPair.second.weight = 1.0;
  }
  return true;
}

Base::txBool RouteAI::ComputeRoute(Base::Component::Pseudorandom& _random_per_vehicle) TX_NOEXCEPT {
  // 检查当前路线是否有效，如果有效则继续执行
  if (IsValid()) {
    // 重置所有边的权重为 1.0，表示初始化时所有边的权重都为 1.0
    ResetEdgeWeight();
    TX_MARK("edge weight reset 1.0");

    // 遍历所有边的权重，并将其乘以一个随机值
    for (auto& refEdgeWeightPair : m_map_idpair_2_edgeLength) {
      refEdgeWeightPair.second.weight *= _random_per_vehicle.GetRandomValue();
      /*LogInfo << "[distance2weight] " << TX_VARS(RouteId()) << TX_VARS(SubRouteId())
          << TX_VARS_NAME(distance, refEdgeWeightPair.first.first)
          << TX_VARS_NAME(to_vtx_id, refEdgeWeightPair.first.second)
          << TX_VARS_NAME(from, refEdgeWeightPair.second.m_fromJointId)
          << TX_VARS_NAME(to, refEdgeWeightPair.second.m_toJointId);*/
    }
    using RouteAIGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, RouteAI_VertexData, RouteAI_EdgeData>;
    using vertex_descriptor = boost::graph_traits<RouteAIGraph>::vertex_descriptor;
    using edge_descriptor = boost::graph_traits<RouteAIGraph>::edge_descriptor;
    // 创建一个新的路由图谱
    RouteAIGraph m_route_graph;
    m_route_graph.clear();

    // 获取地图中的顶点数量
    const Base::txSize num_nodes = m_map_vtxId_2_vtxProp.size();
    // 初始化一个映射，用于存储顶点 ID 到顶点描述符的映射
    std::map<Base::txSize, vertex_descriptor> vtxId2vertex_desc;
    // 创建一个字符串流，用于存储日志信息
    std::ostringstream oss;
    // 遍历地图中的边长度
    for (auto& refEdgeWeightPair : m_map_idpair_2_edgeLength) {
      // 获取边的起始顶点和终止顶点
      const auto start_vtx = std::get<0>(refEdgeWeightPair.first);
      const auto end_vtx = std::get<1>(refEdgeWeightPair.first);

      // 如果映射中不存在起始顶点，则添加顶点并将其插入映射中
      if (_NotContain_(vtxId2vertex_desc, start_vtx)) {
        vertex_descriptor vd = boost::add_vertex(RouteAI_VertexData(start_vtx), m_route_graph);
        vtxId2vertex_desc[start_vtx] = vd;
      }

      // 如果映射中不存在终止顶点，则添加顶点并将其插入映射中
      if (_NotContain_(vtxId2vertex_desc, end_vtx)) {
        vertex_descriptor vd = boost::add_vertex(RouteAI_VertexData{end_vtx}, m_route_graph);
        vtxId2vertex_desc[end_vtx] = vd;
      }

      // 获取起始顶点和终止顶点在映射中的描述符
      auto& start_vtx_desc = vtxId2vertex_desc.at(start_vtx);
      auto& end_vtx_desc = vtxId2vertex_desc.at(end_vtx);

      // 在路由图中添加边，并将其插入映射中
      auto edge_desc =
          boost::add_edge(end_vtx_desc, TX_MARK("reverse") start_vtx_desc, refEdgeWeightPair.second, m_route_graph)
              .first;
      oss << TX_VARS_NAME(start, refEdgeWeightPair.second.m_toJointId) << TX_VARS(end_vtx)
          << TX_VARS_NAME(end, refEdgeWeightPair.second.m_fromJointId) << TX_VARS(start_vtx)
          << TX_VARS_NAME(weight, refEdgeWeightPair.second.getWeight())
          << TX_VARS_NAME(distance, refEdgeWeightPair.second.m_distance) << std::endl;
    }

    LogInfo << "[ReverseRouteList]" << TX_VARS(VehicleId()) << TX_VARS(RouteId()) << TX_VARS(SubRouteId()) << oss.str();
    mMapConnectedLocInfo.clear();

    TX_MARK("hz8010 1, dest random"); /*for (const auto vtxId : m_set_end_vtx_id)*/
    if (_NonEmpty_(m_set_end_vtx_id)) {
      // 随机选择一个终止点
      const Base::txInt nRndIdx = (_random_per_vehicle.GetRandomInt() % m_set_end_vtx_id.size());
      auto itr = m_set_end_vtx_id.begin();
      std::advance(itr, nRndIdx);
      const Base::txSize vtxId = *(itr);
      // 计算从终止点到其他各个顶点的最短路径
      std::vector<vertex_descriptor> parent(num_vertices(m_route_graph));
      std::vector<RouteAI_EdgeData::edge_dist_type> distance(num_vertices(m_route_graph));

      vertex_descriptor s = vtxId2vertex_desc.at(vtxId);
      auto v_index = boost::get(boost::vertex_index, m_route_graph);
      auto weight = boost::make_transform_value_property_map(std::mem_fn(&RouteAI_EdgeData::getWeight),
                                                             get(boost::edge_bundle, m_route_graph));

      boost::dijkstra_shortest_paths(m_route_graph, s,
                                     boost::predecessor_map(make_iterator_property_map(parent.begin(), v_index))
                                         .distance_map(make_iterator_property_map(distance.begin(), v_index))
                                         .weight_map(weight));

      // 遍历所有顶点
      boost::graph_traits<RouteAIGraph>::vertex_iterator vi, vend;
      for (boost::tie(vi, vend) = vertices(m_route_graph); vi != vend; ++vi) {
        // 获取到达终止点的距离
        const RouteAI_EdgeData::edge_dist_type dist_from_endVtx = distance[*vi];
        const Base::txSize vtxId = m_route_graph[(*vi)].vtx_id;
        const Base::txSize parentVtxId = m_route_graph[parent[*vi]].vtx_id;
        // 如果距离小于最大值且存在连接信息
        if (dist_from_endVtx < RouteAI_EdgeData::MaxDistance() &&
            _Contain_(m_map_idpair_2_edgeLength, std::make_pair(vtxId, parentVtxId))) {
          const RouteAI_EdgeData& refEdgeData = m_map_idpair_2_edgeLength.at(std::make_pair(vtxId, parentVtxId));
          if (refEdgeData.m_fromJointId.isStartPt) {
            // 将连接信息添加到已连接信息中
            mMapConnectedLocInfo[refEdgeData.m_fromJointId.locInfo] = refEdgeData;
          }
        } else {
          TX_MARK("do not connection.");
          /*LogWarn << "do not connection. "  << TX_VARS(dist_from_endVtx)
              << TX_VARS_NAME(from_vtx_id, parentVtxId)
              << TX_VARS_NAME(to_vtx_id, vtxId);*/
        }
      } /*for each grpth vertex*/
    }   /*for each end vtx*/
#  if 1
    oss.str("");
    for (const auto locInfo_data : mMapConnectedLocInfo) {
      oss << locInfo_data.second << ", " << std::endl;
    }

    LogInfo << "[Connected Path List]" << TX_VARS(VehicleId()) << TX_VARS(RouteId()) << TX_VARS(SubRouteId())
            << TX_COND(IsValid()) << oss.str();
#  endif
    return true;
  } else {
    return false;
  }
}

Base::txBool RouteAI::IsReachable(const Base::Info_Lane_t& _laneInfo, RouteAI_EdgeData& routeHint) const TX_NOEXCEPT {
  if (IsValid()) {
    if (_Contain_(mMapConnectedLocInfo, _laneInfo)) {
      routeHint = mMapConnectedLocInfo.at(_laneInfo);
      return true;
    } else {
      return false;
    }
  } else {
    routeHint = RouteAI_EdgeData();
    return true;
  }
}

Base::txBool RouteAI::IsReachable(const Base::txLaneUId& _lane_uid, RouteAI_EdgeData& routeHint) const TX_NOEXCEPT {
  if (IsValid()) {
    const Base::Info_Lane_t _lane_loc_info(_lane_uid);
    if (_Contain_(mMapConnectedLocInfo, _lane_loc_info)) {
      routeHint = mMapConnectedLocInfo.at(_lane_loc_info);
      return true;
    } else {
      return false;
    }
  } else {
    routeHint = RouteAI_EdgeData();
    return true;
  }
}

Base::txBool RouteAI::CheckReachable(const Base::Info_Lane_t& _laneInfo) const TX_NOEXCEPT {
  RouteAI_EdgeData tmp;
  if (IsReachable(_laneInfo, tmp) && tmp.NeedStraight()) {
    return true;
  } else {
    return false;
  }
}

Base::txBool RouteAI::CheckReachable(const Base::txLaneUId& _lane_uid) const TX_NOEXCEPT {
  RouteAI_EdgeData tmp;
  if (IsReachable(_lane_uid, tmp) && tmp.NeedStraight()) {
    return true;
  } else {
    return false;
  }
}

Base::txBool RouteAI::CheckReachableDirection(const Base::Info_Lane_t& _laneInfo, Base::txInt& dir) const TX_NOEXCEPT {
  RouteAI_EdgeData tmp;
  if (IsReachable(_laneInfo, tmp)) {
    if (tmp.NeedStraight()) {
      dir = 0;
      return true;
    } else {
      if (tmp.NeedTurnLeft()) dir = 1;
      if (tmp.NeedTurnRight()) dir = -1;
      return false;
    }
  } else {
    return false;
  }
  /*if (IsValid()) {
          return _Contain_(mMapConnectedLocInfo, _laneInfo);
  } else {
          return true;
  }*/
}

Base::txBool RouteAI::CheckReachableRoad(const Base::txRoadID& _roadId) const TX_NOEXCEPT {
  if (IsValid()) {
    return _Contain_(mSetWayPointRoadId, _roadId);
  } else {
    return true;
  }
}

#  undef LogInfo
#endif /*__TX_Mark__("RouteAI")*/

TX_NAMESPACE_CLOSE(Component)
TX_NAMESPACE_CLOSE(TrafficFlow)
