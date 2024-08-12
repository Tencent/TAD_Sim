// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_planning.h"
#include "HdMap/tx_hashed_lane_info.h"
#define VehicleTrajInfo LOG_IF(INFO, FLAGS_LogLevel_Vehicle_Traj)

TX_NAMESPACE_OPEN(TrafficFlow)

// 获取驾驶模式对应的泛型类型
TAD_PlanningModule::Planning_Activate TAD_PlanningModule::pb2enum(const sim_msg::Activate& pb) const TX_NOEXCEPT {
  switch (pb.automode()) {
    case sim_msg::Activate_Automode_AUTOMODE_AUTOPILOT:
      return _plus_(Planning_Activate::eAutopilot);
    case sim_msg::Activate_Automode_AUTOMODE_LATERAL:
      return _plus_(Planning_Activate::eLateral);
    case sim_msg::Activate_Automode_AUTOMODE_LONGITUDINAL:
      return _plus_(Planning_Activate::eLongitudinal);
    case sim_msg::Activate_Automode_AUTOMODE_OFF:
      return _plus_(Planning_Activate::eOff);
    default:
      return _plus_(Planning_Activate::eAutopilot);
      break;
  }
}

Base::txBool TAD_PlanningModule::SearchOrthogonalListFrontByLevel(
    const Base::txBool check_reachable, const Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr& center_node_ptr,
    const Base::txFloat front_search_distance, const Base::txInt level_idx,
    std::list<Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr>& results_by_level) TX_NOEXCEPT {
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using AdjoinContainerType = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::AdjoinContainerType;
  // 如果具有可达性
  if (check_reachable) {
    // 如果center_node_ptr不为空且前方搜索距离大于0
    if (NonNull_Pointer(center_node_ptr) && front_search_distance > 0.0) {
      // 获取center_node_ptr的HashedLaneInfo信息
      const HashedLaneInfo& center_node_hashed_lane_info = center_node_ptr->GetSelfHashedInfo();
      // 获取当前范围的长度
      const Base::txFloat cur_scope_len = center_node_hashed_lane_info.RealLength();
      // 如果当前范围长度大于等于前方搜索距离
      if (cur_scope_len >= front_search_distance) {
        // 检查是否可达
        if (CallSucc(CheckReachable(center_node_hashed_lane_info.LaneInfo()))) {
          // 将center_node_ptr添加到结果列表中
          results_by_level.push_front(center_node_ptr);
          // 输出日志信息
          VehicleTrajInfo << "[trajectory][route_reach_success]" << TX_VARS_NAME(idx, results_by_level.size())
                          << TX_VARS(center_node_hashed_lane_info.LaneInfo());
          return true;
        } else {
          VehicleTrajInfo << "[trajectory][route_do_no_reach]"
                          << TX_VARS_NAME(, center_node_hashed_lane_info.LaneInfo());
          return false;
        }
      } else {
        // 获取前方节点的映射
        const AdjoinContainerType& map_front_nodes = center_node_ptr->GetFrontNodes();
        // 遍历前方节点的映射
        for (const auto& map_pair : map_front_nodes) {
          // 递归调用SearchOrthogonalListFrontByLevel函数，检查是否可达
          if (SearchOrthogonalListFrontByLevel(true, map_pair.second, front_search_distance - cur_scope_len,
                                               level_idx + 1, results_by_level)) {
            // 将center_node_ptr添加到结果列表中
            results_by_level.push_front(center_node_ptr);
            VehicleTrajInfo << "[trajectory][route_reach_success]" << TX_VARS_NAME(idx, results_by_level.size())
                            << TX_VARS(center_node_hashed_lane_info.LaneInfo());
            return true;
          }
        }
        // 如果遍历完所有前方节点都不可达，返回false
        return false;
      }
    } else {
      return false;
    }
  } else {
    TX_MARK("random select");
    // 如果center_node_ptr不为空且前方搜索距离大于0
    if (NonNull_Pointer(center_node_ptr) && front_search_distance > 0.0) {
      // 将center_node_ptr添加到结果列表中
      results_by_level.push_back(center_node_ptr);
      /*results_by_level[level_idx].push_back(center_node_ptr);*/
      // 获取当前范围的长度
      const Base::txFloat cur_scope_len = center_node_ptr->GetSelfHashedInfo().RealLength();
      // 如果当前范围长度大于等于前方搜索距离
      if (cur_scope_len >= front_search_distance) {
        return true;
      } else {
        const AdjoinContainerType& map_front_nodes = center_node_ptr->GetFrontNodes();
        // 遍历前方节点的映射
        for (const auto& map_pair : map_front_nodes) {
          // 递归调用SearchOrthogonalListFrontByLevel函数，检查是否可达
          SearchOrthogonalListFrontByLevel(false, map_pair.second, front_search_distance - cur_scope_len, level_idx + 1,
                                           results_by_level);
          break;
        }
        return true;
      }
    } else {
      return false;
    }
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
