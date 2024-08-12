// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/hashed_lane_info_orthogonal_list.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

#define _DistIdx_ (0)
#define _ElementIdx_ (1)

Base::txBool HashedLaneInfoOrthogonalList::SearchOrthogonalListFront(const Base::txSysId elemSysId,
                                                                     const Base::txSysId elemId,
                                                                     const Base::txFloat elemLength,
                                                                     const OrthogonalListSearchCmd& cmd,
                                                                     txSurroundVehiclefo& res_gap_vehPtr) TX_NOEXCEPT {
  // 获取最小距离和最近车辆的引用
  Base::txFloat& minDist = std::get<_DistIdx_>(res_gap_vehPtr);
  Base::WEAK_SimulationConsistencyPtr& front_nearest_veh_weak_ptr = std::get<_ElementIdx_>(res_gap_vehPtr);
  // 获取当前节点注册的车辆
  VehicleContainer register_vehicles;
  cmd.curNodePtr->GetRegisterVehicles(register_vehicles);
  // 遍历当前节点注册的车辆
  for (const auto& pair_id_vehPtr : register_vehicles) {
    const auto& obj_veh_ptr = pair_id_vehPtr.second;
    // 如果车辆的系统ID与目标车辆的系统ID相同，跳过当前车辆
    if (elemSysId == obj_veh_ptr->SysId()) {
      continue;
    }
    // 获取车辆的稳定HashedLaneInfo
    const auto& refHashNode = obj_veh_ptr->StableHashedLaneInfo();
    // 计算车辆在节点内的s值
    const Base::txFloat dist_s = refHashNode.S_in_Node() - cmd.original_s_in_node;
    // 计算实际距离
    const Base::txFloat real_dist =
        /*FLAGS_DETECT_OBJECT_DISTANCE*/ cmd.raw_original_search_dist - (cmd.valid_dist_along_original - dist_s);
    // 计算车辆之间的间距
    const Base::txFloat curGap = (obj_veh_ptr->GetLength() + elemLength) * 0.5;
    // 计算考虑间距后的距离
    const Base::txFloat dist_with_gap = real_dist - curGap;
    /*LOG(INFO) << TX_VARS(elemId) << TX_VARS(obj_veh_ptr->Id()) << TX_VARS(refHashNode.DistanceOnCurve())
        << TX_VARS(refHashNode.LaneInfo()) << TX_VARS(dist_s) << TX_VARS(real_dist)
        << TX_VARS(cmd.valid_dist_along_original);*/
    // 如果车辆在节点内的s值大于等于0，且小于有效距离，且考虑间距后的距离小于最小距离
    if (dist_s >= 0.0 && dist_s < cmd.valid_dist_along_original && dist_with_gap < minDist) {
      // 更新最小距离和最近车辆
      minDist = dist_with_gap;
      /*LOG(INFO) << TX_VARS(elemId) << TX_VARS(real_dist) << TX_VARS((obj_veh_ptr->GetLength() + elemLength) * 0.5) <<
       * TX_VARS(minDist);*/
      front_nearest_veh_weak_ptr = obj_veh_ptr;
    }
  }
  // 返回最近车辆是否有效
  return WeakIsValid(front_nearest_veh_weak_ptr); /*NonNull_Pointer(front_nearest_veh_ptr);*/
}

// 在HashedLaneInfoOrthogonalList中搜索前方车辆
void HashedLaneInfoOrthogonalList::SearchNearestFrontElement(const Base::txSysId elemSysId, const Base::txSysId elemId,
                                                             const Base::txFloat elemLength,
                                                             const HashedLaneInfo& selfStableHashedLaneInfo,
                                                             const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                                             txSurroundVehiclefo& resQueryInfo,
                                                             const Base::txFloat search_distance) TX_NOEXCEPT {
  // 如果中心节点指针有效
  if (NonNull_Pointer(center_node_ptr)) {
    std::queue<OrthogonalListSearchCmd> qCmd;
    /*1. front*/
    // 初始化搜索命令队列
    qCmd.push(OrthogonalListSearchCmd());
    qCmd.back()
        .SetOrthogonalListPtr(center_node_ptr)
        .SetOriginalS(selfStableHashedLaneInfo.S_in_Node())
        .SetNodeLength(selfStableHashedLaneInfo.S_Inv_in_Node())
        .SetValidDistanceBaseNode(search_distance)
        .SetRawSearchDist(search_distance);
    // 当搜索命令队列不为空且结果查询信息中的车辆指针无效时，继续搜索
    while (CallFail(qCmd.empty()) && WeakIsNull(std::get<_ElementIdx_>(resQueryInfo))
           /*Null_Pointer(std::get<_ElementIdx_>(resQueryInfo))*/) {
      TX_MARK("per-level");
      txSurroundVehiclefo cur_level_follow_front;
      std::get<_DistIdx_>(cur_level_follow_front) = 999.0;
      std::queue<OrthogonalListSearchCmd> qCmd_next_level;
      // 遍历当前层级的搜索命令
      while (CallFail(qCmd.empty())) {
        const auto cmd = qCmd.front();
        // LOG_IF(INFO, 54 == elemId) << TX_VARS(elemId) << TX_VARS(cmd.Str());
        qCmd.pop();
        // 在当前节点的正交列表中搜索前方车辆
        SearchOrthogonalListFront(elemSysId, elemId, elemLength, cmd, cur_level_follow_front);

        const AdjoinContainerType& refFrontNodes = cmd.curNodePtr->GetFrontNodes();
        const Base::txFloat last_step_pass_dist = cmd.length_in_node;
        const Base::txFloat next_step_valid_dist = cmd.valid_dist_along_original - last_step_pass_dist;
        // 如果剩余有效距离大于0
        if (next_step_valid_dist > 0.0) {
          // 遍历当前节点的前方节点
          for (const auto& refFrontNode : refFrontNodes) {
            OrthogonalListSearchCmd nextCmd;
            nextCmd.SetOrthogonalListPtr(refFrontNode.second)
                .SetOriginalS(0.0)
                .SetNodeLength(refFrontNode.first.RealLength())
                .SetValidDistanceBaseNode(next_step_valid_dist)
                .SetRawSearchDist(cmd.raw_original_search_dist);
            // 将下一层级的搜索命令加入队列
            qCmd_next_level.emplace(nextCmd);
          }
        }
      }

      // 如果当前层级的搜索结果有效，则更新结果查询信息
      if (WeakIsValid(std::get<_ElementIdx_>(cur_level_follow_front))
          /*NonNull_Pointer(std::get<_ElementIdx_>(cur_level_follow_front))*/) {
        resQueryInfo = cur_level_follow_front;
      } else {
        // 否则，将下一层级的搜索命令队列赋值给当前层级的搜索命令队列
        qCmd = qCmd_next_level;
      }
    }
  } else {
    LOG(WARNING) << "[Query_OrthogonalList][rear_element][null] " << TX_VARS(elemId)
                 << TX_VARS(selfStableHashedLaneInfo);
  }
}

// 在HashedLaneInfoOrthogonalList中搜索后方车辆
Base::txBool HashedLaneInfoOrthogonalList::SearchOrthogonalListRear(const Base::txSysId elemSysId,
                                                                    const Base::txSysId elemId,
                                                                    const Base::txFloat elemLength,
                                                                    const OrthogonalListSearchCmd& cmd,
                                                                    txSurroundVehiclefo& res_gap_vehPtr) TX_NOEXCEPT {
  // 获取最小距离和最近车辆的引用
  Base::txFloat& minDist = std::get<_DistIdx_>(res_gap_vehPtr);
  Base::WEAK_SimulationConsistencyPtr& rear_nearest_veh_weak_ptr = std::get<_ElementIdx_>(res_gap_vehPtr);
  // 获取当前节点注册的车辆
  VehicleContainer register_vehicles;
  cmd.curNodePtr->GetRegisterVehicles(register_vehicles);
  // 遍历当前节点注册的车辆
  for (const auto& pair_id_vehPtr : register_vehicles) {
    const auto& obj_veh_ptr = pair_id_vehPtr.second;
    // 如果车辆的系统ID与目标车辆的系统ID相同，跳过当前车辆
    if (elemSysId == obj_veh_ptr->SysId()) {
      continue;
    }
    // 获取车辆的稳定HashedLaneInfo
    const auto& refHashNode = obj_veh_ptr->StableHashedLaneInfo();
    // 计算车辆在节点内的s值
    const Base::txFloat dist_s = TX_MARK("REAR") cmd.original_s_in_node - refHashNode.S_in_Node();
    // 计算实际距离
    const Base::txFloat real_dist =
        /*FLAGS_DETECT_OBJECT_DISTANCE*/ cmd.raw_original_search_dist - (cmd.valid_dist_along_original - dist_s);
    // 计算车辆之间的间距
    const Base::txFloat curGap = (obj_veh_ptr->GetLength() + elemLength) * 0.5;
    const Base::txFloat dist_with_gap = real_dist - curGap;
    /*LOG(INFO) << TX_VARS(elemId) << TX_VARS(obj_veh_ptr->Id()) << TX_VARS(refHashNode.DistanceOnCurve())
        << TX_VARS(refHashNode.LaneInfo()) << TX_VARS(dist_s) << TX_VARS(real_dist)
        << TX_VARS(cmd.valid_dist_along_original);*/
    // 如果车辆在节点内的s值大于等于0，且小于有效距离，且考虑间距后的距离小于最小距离
    if (dist_s >= 0.0 && dist_s < cmd.valid_dist_along_original && dist_with_gap < minDist) {
      // 更新最小距离和最近车辆
      minDist = dist_with_gap;
      /*LOG(INFO) << TX_VARS(elemId) << TX_VARS(real_dist) << TX_VARS((obj_veh_ptr->GetLength() + elemLength) * 0.5) <<
       * TX_VARS(minDist);*/
      rear_nearest_veh_weak_ptr = obj_veh_ptr;
    }
  }
  // 返回最近车辆是否有效
  return WeakIsValid(rear_nearest_veh_weak_ptr); /*NonNull_Pointer(front_nearest_veh_ptr);*/
}

// 在HashedLaneInfoOrthogonalList中搜索后方车辆
void HashedLaneInfoOrthogonalList::SearchNearestRearElement(const Base::txSysId elemSysId, const Base::txSysId elemId,
                                                            const Base::txFloat elemLength,
                                                            const HashedLaneInfo& selfStableHashedLaneInfo,
                                                            const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                                            txSurroundVehiclefo& resQueryInfo,
                                                            const Base::txFloat search_distance) TX_NOEXCEPT {
  // 如果中心节点指针有效
  if (NonNull_Pointer(center_node_ptr)) {
    std::queue<OrthogonalListSearchCmd> qCmd;
    /*1. rear*/
    // 初始化搜索命令队列
    qCmd.push(OrthogonalListSearchCmd());
    qCmd.back()
        .SetOrthogonalListPtr(center_node_ptr)
        .SetOriginalS(selfStableHashedLaneInfo.S_in_Node())
        .SetNodeLength(selfStableHashedLaneInfo.S_in_Node(/*mark*/))
        .SetValidDistanceBaseNode(search_distance)
        .SetRawSearchDist(search_distance);
    // 当搜索命令队列不为空且结果查询信息中的车辆指针无效时，继续搜索
    while (CallFail(qCmd.empty()) && WeakIsNull(std::get<_ElementIdx_>(resQueryInfo))) {
      TX_MARK("per-level");
      txSurroundVehiclefo cur_level_follow_rear;
      std::get<_DistIdx_>(cur_level_follow_rear) = 999.0;
      std::queue<OrthogonalListSearchCmd> qCmd_next_level;
      // 遍历当前层级的搜索命令
      while (CallFail(qCmd.empty())) {
        const auto cmd = qCmd.front();
        qCmd.pop();
        // 在当前节点的正交列表中搜索后方车辆
        SearchOrthogonalListRear(elemSysId, elemId, elemLength, cmd, cur_level_follow_rear);

        const AdjoinContainerType& refBackNodes = cmd.curNodePtr->GetBackNodes();
        const Base::txFloat last_step_pass_dist = cmd.length_in_node;
        const Base::txFloat next_step_valid_dist = cmd.valid_dist_along_original - last_step_pass_dist;
        // 如果剩余有效距离大于0
        if (next_step_valid_dist > 0.0) {
          // 遍历当前节点的后方节点
          for (const auto& refBackNode : refBackNodes) {
            OrthogonalListSearchCmd nextCmd;
            nextCmd.SetOrthogonalListPtr(refBackNode.second)
                .SetOriginalS(refBackNode.first.RealLength(/*mark*/))
                .SetNodeLength(refBackNode.first.RealLength())
                .SetValidDistanceBaseNode(next_step_valid_dist)
                .SetRawSearchDist(cmd.raw_original_search_dist);
            // 将下一层级的搜索命令加入队列
            qCmd_next_level.emplace(nextCmd);
          }
        }
      } /*while*/

      // 如果当前层级的搜索结果有效，则更新结果查询信息
      if (WeakIsValid(std::get<_ElementIdx_>(cur_level_follow_rear))
          /*NonNull_Pointer(std::get<_ElementIdx_>(cur_level_follow_front))*/) {
        resQueryInfo = cur_level_follow_rear;
      } else {
        // 否则，将下一层级的搜索命令队列赋值给当前层级的搜索命令队列
        qCmd = qCmd_next_level;
      }
    }
  } else {
    LOG(WARNING) << "[Query_OrthogonalList][front_element][null] " << TX_VARS(elemId)
                 << TX_VARS(selfStableHashedLaneInfo);
  }
}

// 在HashedLaneInfoOrthogonalList中搜索后续节点
Base::txBool HashedLaneInfoOrthogonalList::SearchOrthogonalListPost(
    const HashedLaneInfoOrthogonalListPtr& center_node_ptr, const Base::txFloat post_search_distance,
    AdjoinContainerType& results, std::function<Base::txBool(const Base::txRoadID)> checkOp) TX_NOEXCEPT {
  // 如果中心节点指针有效，且搜索距离大于0，且满足检查条件
  if (NonNull_Pointer(center_node_ptr) && post_search_distance > 0.0 &&
      (center_node_ptr->GetSelfHashedInfo().LaneInfo().IsOnLaneLink() ||
       CallSucc(checkOp(center_node_ptr->GetSelfHashedInfo().LaneInfo().onLaneUid.roadId)))) {
    // 将中心节点添加到结果集中
    results[center_node_ptr->GetSelfHashedInfo()] = center_node_ptr;
    // 计算新的搜索距离
    const Base::txFloat new_post_search_distance =
        post_search_distance - center_node_ptr->GetSelfHashedInfo().RealLength();
    // 如果新的搜索距离大于0
    if (new_post_search_distance > 0.0) {
      // 获取后续节点
      const AdjoinContainerType& map_post_nodes = center_node_ptr->GetBackNodes();
      // 遍历后续节点
      for (const auto& pair_node_ptr : map_post_nodes) {
        // 递归搜索后续节点
        SearchOrthogonalListPost(pair_node_ptr.second, new_post_search_distance, results, checkOp);
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}
Base::txBool HashedLaneInfoOrthogonalList::SearchOrthogonalListPost(
    const HashedLaneInfoOrthogonalListPtr& center_node_ptr, const Base::txFloat post_search_distance,
    AdjoinContainerType& results) TX_NOEXCEPT {
  std::function<Base::txBool(const Base::txRoadID)> func_return_true = [](const Base::txRoadID) { return true; };
  // 调用另一个重载的SearchOrthogonalListPost函数，传入始终返回true的函数
  return SearchOrthogonalListPost(center_node_ptr, post_search_distance, results, func_return_true);
#if 0
    if (NonNull_Pointer(center_node_ptr) &&
        post_search_distance > 0.0) {
        results[center_node_ptr->GetSelfHashedInfo()] = center_node_ptr;
        const Base::txFloat new_post_search_distance =
            post_search_distance - center_node_ptr->GetSelfHashedInfo().RealLength();
        if (new_post_search_distance > 0.0) {
            const AdjoinContainerType& map_post_nodes = center_node_ptr->GetBackNodes();
            for (const auto& pair_node_ptr : map_post_nodes) {
                SearchOrthogonalListPost(pair_node_ptr.second, new_post_search_distance, results);
            }
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
#endif
}

Base::txBool HashedLaneInfoOrthogonalList::SearchOrthogonalListFront(
    const HashedLaneInfoOrthogonalListPtr& center_node_ptr, const Base::txFloat front_search_distance,
    AdjoinContainerType& results) TX_NOEXCEPT {
  // 如果中心节点指针有效，且搜索距离大于0
  if (NonNull_Pointer(center_node_ptr) && front_search_distance > 0.0) {
    // 将中心节点添加到结果集中
    results[center_node_ptr->GetSelfHashedInfo()] = center_node_ptr;
    // 计算新的搜索距离
    const Base::txFloat new_front_search_distance =
        front_search_distance - center_node_ptr->GetSelfHashedInfo().RealLength();
    // 如果新的搜索距离大于0
    if (new_front_search_distance > 0.0) {
      const AdjoinContainerType& map_front_nodes = center_node_ptr->GetFrontNodes();
      // 遍历前方节点
      for (const auto& pair_node_ptr : map_front_nodes) {
        // 递归搜索前方节点
        SearchOrthogonalListFront(pair_node_ptr.second, new_front_search_distance, results);
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// 在HashedLaneInfoOrthogonalList中搜索前方节点，按层级搜索
Base::txBool HashedLaneInfoOrthogonalList::SearchOrthogonalListFrontByLevel(
    const HashedLaneInfoOrthogonalListPtr& center_node_ptr, const Base::txFloat front_search_distance,
    const Base::txInt level_idx, std::vector<HashedLaneInfoOrthogonalListPtr>& results_by_level) TX_NOEXCEPT {
  // 如果中心节点指针有效，且搜索距离大于0
  if (NonNull_Pointer(center_node_ptr) && front_search_distance > 0.0) {
    results_by_level.push_back(center_node_ptr);
    /*results_by_level[level_idx].push_back(center_node_ptr);*/
    const Base::txFloat cur_scope_len = center_node_ptr->GetSelfHashedInfo().RealLength();
    // 如果当前节点的长度大于等于搜索距离，返回true
    if (cur_scope_len >= front_search_distance) {
      return true;
    } else {
      // 获取前方节点
      const AdjoinContainerType& map_front_nodes = center_node_ptr->GetFrontNodes();
      // 遍历前方节点
      for (const auto& map_pair : map_front_nodes) {
        // 递归搜索前方节点，层级加1
        SearchOrthogonalListFrontByLevel(map_pair.second, front_search_distance - cur_scope_len, level_idx + 1,
                                         results_by_level);
        break;
      }
      return true;
    }
  } else {
    return false;
  }
}

HashedLaneInfoOrthogonalListPtr HashedLaneInfoOrthogonalList::GetLeftNode() const TX_NOEXCEPT {
  return Weak2SharedPtr(m_left_node_list);
}

HashedLaneInfoOrthogonalListPtr HashedLaneInfoOrthogonalList::GetRightNode() const TX_NOEXCEPT {
  return Weak2SharedPtr(m_right_node_list);
}

// 设置前方节点
void HashedLaneInfoOrthogonalList::SetFrontNodes(AdjoinContainerType sharedPtrMap) TX_NOEXCEPT {
  // 清空前方节点列表
  m_front_node_list.clear();
  // 遍历传入的前方节点列表
  for (auto pa : sharedPtrMap) {
    // 将传入的前方节点添加到当前节点的前方节点列表中
    m_front_node_list[pa.first] = pa.second;
  }
}

void HashedLaneInfoOrthogonalList::SetBackNodes(AdjoinContainerType sharedPtrMap) TX_NOEXCEPT {
  // 清空后方节点列表
  m_back_node_list.clear();
  // 遍历传入的后方节点列表
  for (auto pa : sharedPtrMap) {
    // 将传入的后方节点添加到当前节点的后方节点列表中
    m_back_node_list[pa.first] = pa.second;
  }
}

HashedLaneInfoOrthogonalList::AdjoinContainerType HashedLaneInfoOrthogonalList::GetFrontNodes() const TX_NOEXCEPT {
  AdjoinContainerType res;
  // 遍历前方节点列表
  for (auto pa_weak : m_front_node_list) {
    // 如果前方节点未过期
    if (CallFail(pa_weak.second.expired())) {
      // 将前方节点添加到结果集中
      res[pa_weak.first] = pa_weak.second.lock();
    }
  }
  return res;
}

HashedLaneInfoOrthogonalList::AdjoinContainerType HashedLaneInfoOrthogonalList::GetBackNodes() const TX_NOEXCEPT {
  AdjoinContainerType res;
  // 遍历后方节点列表
  for (auto pa_weak : m_back_node_list) {
    // 如果后方节点未过期
    if (CallFail(pa_weak.second.expired())) {
      // 将后方节点添加到结果集中
      res[pa_weak.first] = pa_weak.second.lock();
    }
  }
  return res;
}

#undef _DistIdx_
#undef _ElementIdx_
TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)
