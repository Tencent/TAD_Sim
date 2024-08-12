// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_road_network.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_hadmap_utils.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_RoadNetwork)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(HdMap)
txRoadNetwork::JointPointId2TreeNodePtr txRoadNetwork::s_map_jointpoint2treeNode;
std::vector<txRoadNetwork::JointPointTreeNodePtr> txRoadNetwork::s_vec_non_leaf_nodes;
txRoadNetwork::JointPointId2VertexPropertyType txRoadNetwork::s_JointPtId2VtxProperty;
txRoadNetwork::VtxPropertyId2VtxPropertyType txRoadNetwork::s_VtxId2VtxProperty;

std::tuple<Base::JointPointId_t, Base::JointPointId_t> CreateLaneLinkJointPoint(const hadmap::txLaneLinkPtr& refLinkPtr)
    TX_NOEXCEPT {
  std::tuple<Base::JointPointId_t, Base::JointPointId_t> retTupleValue;
  if (NonNull_Pointer(refLinkPtr)) {
    // 获取车道连接的ID
    const Base::txLaneLinkID linkId = refLinkPtr->getId();
    // 获取起始关节点
    Base::JointPointId_t& startJointPoint = std::get<0>(retTupleValue);
    // 获取结束关节点
    Base::JointPointId_t& endJointPoint = std::get<1>(retTupleValue);

    // 设置起始关节点的标志位为true
    startJointPoint.isStartPt = true;
    // 设置起始关节点的位置信息
    startJointPoint.locInfo.FromLaneLink(linkId, refLinkPtr->fromTxLaneId(), refLinkPtr->toTxLaneId());

    // 设置结束关节点的标志位为false
    endJointPoint.isStartPt = false;
    // 设置结束关节点的位置信息
    endJointPoint.locInfo.FromLaneLink(linkId, refLinkPtr->fromTxLaneId(), refLinkPtr->toTxLaneId());
  } else {
    LogWarn << "lane link ptr is nullptr.";
  }
  return retTupleValue;
}

// 创建车道的关节点
std::tuple<Base::JointPointId_t, Base::JointPointId_t> CreateLaneJointPoint(const hadmap::txLanePtr& refLanePtr)
    TX_NOEXCEPT {
  std::tuple<Base::JointPointId_t, Base::JointPointId_t> retTupleValue;
  if (NonNull_Pointer(refLanePtr)) {
    // 获取车道的ID
    const Base::txLaneUId& laneUid = refLanePtr->getTxLaneId();
    // 获取起始关节点
    Base::JointPointId_t& startJointPoint = std::get<0>(retTupleValue);
    // 获取结束关节点
    Base::JointPointId_t& endJointPoint = std::get<1>(retTupleValue);

    // 设置起始关节点的标志位为true
    startJointPoint.isStartPt = true;
    // 设置起始关节点的位置信息
    startJointPoint.locInfo.FromLane(laneUid);

    // 设置结束关节点的标志位为false
    endJointPoint.isStartPt = false;
    // 设置结束关节点的位置信息
    endJointPoint.locInfo.FromLane(laneUid);
  } else {
    LogWarn << "lane ptr is nullptr.";
  }
  return retTupleValue;
}

// 搜索根节点
Base::JointPointTreeNodeWeakPtr SearchRootNode(Base::JointPointTreeNodePtr currentPtr) TX_NOEXCEPT {
  Base::JointPointTreeNodeWeakPtr retPtr = currentPtr;
  // 当当前节点未过期且父节点未过期时，继续向上查找
  while (CallFail(retPtr.expired()) && CallFail((retPtr.lock()->parentPtr).expired())) {
    // 更新当前节点为父节点
    retPtr = retPtr.lock()->parentPtr;
  }
  return retPtr;
}

Base::txBool txRoadNetwork::MergeNode(Base::JointPointTreeNodePtr leftPtr,
                                      Base::JointPointTreeNodePtr rightPtr) TX_NOEXCEPT {
  // 检查两个节点指针是否有效
  if (NonNull_Pointer(leftPtr) && NonNull_Pointer(rightPtr)) {
    // 查找左节点的根节点
    Base::JointPointTreeNodeWeakPtr leftRootPtr = SearchRootNode(leftPtr);
    // 查找右节点的根节点
    Base::JointPointTreeNodeWeakPtr rightRootPtr = SearchRootNode(rightPtr);

    // 创建一个新的根节点，将左右节点的根节点作为子节点
    Base::JointPointTreeNodePtr newRootPtr = std::make_shared<Base::JointPointTreeNode>(leftRootPtr, rightRootPtr);
    // 将左节点的根节点的父节点设置为新的根节点
    leftRootPtr.lock()->parentPtr = newRootPtr;
    // 将右节点的根节点的父节点设置为新的根节点
    rightRootPtr.lock()->parentPtr = newRootPtr;
    // 将新的根节点添加到非叶子节点向量中
    s_vec_non_leaf_nodes.push_back(newRootPtr);
    return true;
  } else {
    LogWarn << "NonNull_Pointer(leftPtr) && NonNull_Pointer(rightPtr)";
    return false;
  }
}

hadmap::txPoint txRoadNetwork::ComputeJointPointLocation(
    const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& hashSetId) TX_NOEXCEPT {
  // 遍历关节点ID集合
  for (auto jointId : (hashSetId)) {
    // 如果关节点在车道连接上
    if (jointId.isOnLaneLink()) {
      // 获取车道连接指针
      hadmap::txLaneLinkPtr linkPtr =
          HadmapCacheConCurrent::GetTxLaneLinkPtr(jointId.onLinkFromLaneUid(), jointId.onLinkToLaneUid());
      // 检查车道连接是否有效
      if (Utils::IsLaneLinkValid(linkPtr)) {
        // 如果关节点是起始点
        if (jointId.isStartPt) {
          // 返回车道连接的起始位置
          return linkPtr->getGeometry()->getStart();
        } else {
          // 返回车道连接的结束位置
          return linkPtr->getGeometry()->getEnd();
        }
      }
      /*else {
              LOG(WARNING) << "ComputeJointPointLocation error. "  << TX_VARS(jointId)
                      << TX_COND(NonNull_Pointer(linkPtr)) << TX_COND(NonNull_Pointer(linkPtr->getGeometry()))
                      << TX_VARS(linkPtr->getGeometry()->getLength()) << TX_COND((!(linkPtr->getGeometry()->empty())));
      }*/
    } else {
      // 获取车道指针
      hadmap::txLanePtr lanePtr = HadmapCacheConCurrent::GetTxLanePtr(jointId.onLaneUid());
      // 检查车道是否有效
      if (Utils::IsLaneValid(lanePtr)) {
        // 如果关节点是起始点
        if (jointId.isStartPt) {
          // 返回车道的起始位置
          return lanePtr->getGeometry()->getStart();
        } else {
          // 返回车道的结束位置
          return lanePtr->getGeometry()->getEnd();
        }
      }
      /*else {
              LOG(WARNING) << "ComputeJointPointLocation error. " << TX_VARS(jointId)
                      << TX_COND(NonNull_Pointer(lanePtr)) << TX_COND(NonNull_Pointer(lanePtr->getGeometry()))
                      << TX_VARS(lanePtr->getGeometry()->getLength()) << TX_COND((!(lanePtr->getGeometry()->empty())));
      }*/
    }
  }
  LOG(WARNING) << "ComputeJointPointLocation error.";
  return hadmap::txPoint();
}

Base::txBool txRoadNetwork::CheckJointTree() TX_NOEXCEPT {
  // 创建一个ID到关节点树节点指针的映射
  std::map<Base::txSize, JointPointTreeNodePtr> mapId2Ptr;
  // 遍历关节点ID到关节点树节点指针的映射
  for (auto refPair_JointId2NodePtr : s_map_jointpoint2treeNode) {
    // 获取关节点树节点指针
    JointPointTreeNodePtr refNodePtr = refPair_JointId2NodePtr.second;
    // 查找关节点树节点的根节点
    refNodePtr = SearchRootNode(refNodePtr).lock();
    // 获取当前节点的ID
    const Base::txSize curNodeId = refNodePtr->mSysId;
    // 如果映射中不存在当前节点ID，将其添加到映射中
    if (mapId2Ptr.end() == mapId2Ptr.find(curNodeId)) {
      mapId2Ptr[curNodeId] = refNodePtr;
    }
  }

  // 创建一个关节点位置向量
  std::vector<hadmap::txPoint> vecJointPoints;
  // 遍历ID到关节点树节点指针的映射
  for (auto refPair : mapId2Ptr) {
    vecJointPoints.emplace_back(ComputeJointPointLocation(refPair.second->mHashSetId));
  }

  /*std::ofstream outfile("d:/joint_point.txt");
  outfile << "x,y" << std::endl;
  for (const auto pt : vecJointPoints) {
      outfile << _StreamFormat_ << pt.x << "," << pt.y << std::endl;
  }
  outfile.close();
  txPAUSE;*/
  return true;
}

Base::txBool txRoadNetwork::CreateLaneLinkJointPointVec(const hadmap::txLaneLinks& lanelinkPtrVec) TX_NOEXCEPT {
  // 车道连接起始关节点ID
  Base::JointPointId_t link_start_joint_point;
  // 车道连接结束关节点ID
  Base::JointPointId_t link_end_joint_point;
  // 遍历车道连接指针向量
  for (const auto& refLinkPtr : lanelinkPtrVec) {
    // 创建车道连接关节点
    std::tie(link_start_joint_point, link_end_joint_point) = CreateLaneLinkJointPoint(refLinkPtr);
    // 如果车道连接起始关节点ID不在关节点映射中
    if (_NotContain_(s_map_jointpoint2treeNode, link_start_joint_point)) {
      // 将车道连接起始关节点添加到关节点映射中
      s_map_jointpoint2treeNode.insert(
          std::make_pair(link_start_joint_point, std::make_shared<Base::JointPointTreeNode>(link_start_joint_point)));
      // s_map_jointpoint2treeNode[link_start_joint_point] =
      // std::make_shared<Base::JointPointTreeNode>(link_start_joint_point); LogWarn << "[Error_JointPoint_20230717]" <<
      // link_start_joint_point << ", insert.";
    } else {
      // LogWarn << "[Error_JointPoint_20230717]" << link_start_joint_point << ", has existing.";
    }

    // 如果车道连接结束关节点ID不在关节点映射中
    if (_NotContain_(s_map_jointpoint2treeNode, link_end_joint_point)) {
      // 将车道连接结束关节点添加到关节点映射中
      s_map_jointpoint2treeNode.insert(
          std::make_pair(link_end_joint_point, std::make_shared<Base::JointPointTreeNode>(link_end_joint_point)));
      // s_map_jointpoint2treeNode[link_end_joint_point] =
      // std::make_shared<Base::JointPointTreeNode>(link_end_joint_point); LogWarn << "[Error_JointPoint_20230717]" <<
      // link_end_joint_point << ", insert.";
    } else {
      // LogWarn << "[Error_JointPoint_20230717]" << link_end_joint_point << ", has existing.";
    }
  }
  return true;
}

Base::txBool txRoadNetwork::CreateLaneJointPointVec(const hadmap::txLanes& lanePtrVec) TX_NOEXCEPT {
  // 车道起始关节点ID
  Base::JointPointId_t lane_start_joint_point;
  // 车道结束关节点ID
  Base::JointPointId_t lane_end_joint_point;
  // 遍历车道指针向量
  for (const auto& refLanePtr : lanePtrVec) {
    // 创建车道关节点
    std::tie(lane_start_joint_point, lane_end_joint_point) = CreateLaneJointPoint(refLanePtr);
    // 如果车道起始关节点ID不在关节点映射中
    if (_NotContain_(s_map_jointpoint2treeNode, lane_start_joint_point)) {
      // 将车道起始关节点添加到关节点映射中
      s_map_jointpoint2treeNode.insert(
          std::make_pair(lane_start_joint_point, std::make_shared<Base::JointPointTreeNode>(lane_start_joint_point)));
      // s_map_jointpoint2treeNode[lane_start_joint_point] =
      // std::make_shared<Base::JointPointTreeNode>(lane_start_joint_point);
    } else {
      // 如果车道起始关节点ID已经存在，输出警告信息
      LOG(FATAL) << TX_VARS_NAME(new_id, lane_start_joint_point)
                 << TX_VARS_NAME(old_id, *(s_map_jointpoint2treeNode.at(lane_start_joint_point)->mHashSetId.begin()))
                 << ", has existing.";
    }

    // 如果车道结束关节点ID不在关节点映射中
    if (_NotContain_(s_map_jointpoint2treeNode, lane_end_joint_point)) {
      // 将车道结束关节点添加到关节点映射中
      s_map_jointpoint2treeNode.insert(
          std::make_pair(lane_end_joint_point, std::make_shared<Base::JointPointTreeNode>(lane_end_joint_point)));
      // s_map_jointpoint2treeNode[lane_end_joint_point] =
      // std::make_shared<Base::JointPointTreeNode>(lane_end_joint_point);
    } else {
      // 如果车道结束关节点ID已经存在，输出警告信息
      LOG(FATAL) << lane_end_joint_point << ", has existing.";
    }
  }
  return true;
}

Base::txBool txRoadNetwork::MergeJointWithLanelinkAndLane(const Base::txLaneLinkID linkId,
                                                          const Base::txLaneUId& fromLaneUid,
                                                          const Base::txLaneUId& toLaneUid,
                                                          const Base::txBool isLinkStartPt) TX_NOEXCEPT {
  Base::JointPointId_t link_joint_point;

  // 设置车道连接关节点的起始点标志
  link_joint_point.isStartPt = isLinkStartPt;
  // 从车道连接ID、起始车道ID和目标车道ID创建车道连接关节点的位置信息
  link_joint_point.locInfo.FromLaneLink(linkId, fromLaneUid, toLaneUid);
  // 在关节点映射中查找车道连接关节点
  JointPointId2TreeNodePtr::iterator find_link_result_itr = s_map_jointpoint2treeNode.find(link_joint_point);
  // 如果找到了车道连接关节点
  if (s_map_jointpoint2treeNode.end() != find_link_result_itr) {
    // 获取车道连接关节点指针
    JointPointTreeNodePtr linkNodePtr =
        (*find_link_result_itr).second; /*s_map_jointpoint2treeNode.at(link_joint_point);*/
    // 如果车道连接关节点指针非空
    if (NonNull_Pointer(linkNodePtr)) {
      // 获取与车道连接关联的车道ID集合
      std::set<Base::txLaneUId> laneUidSet;
      if (isLinkStartPt) {
        laneUidSet = HadmapCacheConCurrent::GetLaneLinkFromLaneUidSet(link_joint_point.locInfo);
      } else {
        laneUidSet = HadmapCacheConCurrent::GetLaneLinkToLaneUidSet(link_joint_point.locInfo);
      }

      // 遍历车道ID集合
      for (const auto curLaneUid : laneUidSet) {
        Base::JointPointId_t lane_joint_point;

        // 设置车道关节点的起始点标志
        lane_joint_point.isStartPt = !(link_joint_point.isStartPt);
        // 从车道ID创建车道关节点的位置信息
        lane_joint_point.locInfo.FromLane(curLaneUid);

        // 在关节点映射中查找车道关节点
        JointPointId2TreeNodePtr::iterator find_lane_result_itr = s_map_jointpoint2treeNode.find(lane_joint_point);
        // 如果找到了车道关节点
        if (s_map_jointpoint2treeNode.end() != find_lane_result_itr) {
          // 获取车道关节点指针
          JointPointTreeNodePtr laneNodePtr = (*find_lane_result_itr).second;
          // 如果车道关节点指针非空
          if (NonNull_Pointer(laneNodePtr)) {
            // 合并车道连接关节点和车道关节点
            if (CallSucc(MergeNode(linkNodePtr, laneNodePtr))) {
              // LogInfo << "MergeNode " << TXST_TRACE_VARIABLES(link_joint_point) <<
              // TXST_TRACE_VARIABLES(lane_joint_point) << ", success.";
            } else {
              LOG(WARNING) << "MergeNode " << TX_VARS(link_joint_point) << TX_VARS(lane_joint_point) << ", failure.";
            }
          } else {
            LOG(WARNING) << lane_joint_point << ", laneNodePtr is nullptr.";
          }
        } else {
          LOG(WARNING) << lane_joint_point << ", has not existing." << TX_VARS(fromLaneUid) << TX_VARS(toLaneUid);
        }
      }
    } else {
      LOG(WARNING) << link_joint_point << ", linkNodePtr is nullptr.";
    }
  } else {
    LOG(WARNING) << link_joint_point << ", has not existing.";
  }
  return true;
}

Base::txBool txRoadNetwork::MergeJointWithLaneAndLane(const Base::txLaneUId srcLaneUid,
                                                      const Base::txBool isLaneStartPt) TX_NOEXCEPT {
  // 创建车道关节点ID
  Base::JointPointId_t src_Lane_joint_point;

  // 设置车道关节点的起始点标志
  src_Lane_joint_point.isStartPt = isLaneStartPt;
  // 从车道ID创建车道关节点的位置信息
  src_Lane_joint_point.locInfo.FromLane(srcLaneUid);
  // 在关节点映射中查找车道关节点
  JointPointId2TreeNodePtr::iterator find_lane_result_itr = s_map_jointpoint2treeNode.find(src_Lane_joint_point);
  // 如果找到了车道关节点
  if (s_map_jointpoint2treeNode.end() != find_lane_result_itr) {
    // 获取车道关节点指针
    JointPointTreeNodePtr srcLaneNodePtr = (*find_lane_result_itr).second;
    // 如果车道关节点指针非空
    if (NonNull_Pointer(srcLaneNodePtr)) {
      std::set<Base::txLaneUId> laneUidSet;
      // 如果车道关节点的起始点标志为真
      if (isLaneStartPt) {
        // 获取与当前车道相邻的前向车道ID集合
        laneUidSet = HadmapCacheConCurrent::GetPreLaneSetByLaneUid(srcLaneUid);
        /*for (const auto& refuid : laneUidSet) {
                LOG(INFO) << "[20221019]" << TX_VARS_NAME(src_uid, Utils::ToString(srcLaneUid))
                        << TX_VARS_NAME(pre_uid, Utils::ToString(refuid));
        } */
      } else {
        // 获取与当前车道相邻的后向车道ID集合
        laneUidSet = HadmapCacheConCurrent::GetNextLaneSetByLaneUid(srcLaneUid);
        /*for (const auto& refuid : laneUidSet) {
                LOG(INFO) << "[20221019]" << TX_VARS_NAME(src_uid, Utils::ToString(srcLaneUid))
                        << TX_VARS_NAME(next_uid, Utils::ToString(refuid));
        }*/
      }

      // 遍历相邻车道ID集合
      for (const auto curLaneUid : laneUidSet) {
        Base::JointPointId_t lane_joint_point;

        // 设置相邻车道关节点的起始点标志
        lane_joint_point.isStartPt = !(src_Lane_joint_point.isStartPt);
        // 从相邻车道ID创建相邻车道关节点的位置信息
        lane_joint_point.locInfo.FromLane(curLaneUid);
        // 在关节点映射中查找相邻车道关节点
        JointPointId2TreeNodePtr::iterator find_other_lane_result_itr =
            s_map_jointpoint2treeNode.find(lane_joint_point);
        // 如果找到了相邻车道关节点
        if (s_map_jointpoint2treeNode.end() != find_other_lane_result_itr) {
          // 获取相邻车道关节点指针
          JointPointTreeNodePtr laneNodePtr = (*find_other_lane_result_itr).second;
          // 如果相邻车道关节点指针非空
          if (NonNull_Pointer(laneNodePtr)) {
            // 合并当前车道关节点和相邻车道关节点
            if (CallSucc(MergeNode(srcLaneNodePtr, laneNodePtr))) {
              // LogInfo << "MergeNode " << TXST_TRACE_VARIABLES(src_Lane_joint_point) <<
              // TXST_TRACE_VARIABLES(lane_joint_point) << ", success.";
            } else {
              LOG(FATAL) << "MergeNode " << TX_VARS(src_Lane_joint_point) << TX_VARS(lane_joint_point) << ", failure.";
            }
          } else {
            LOG(FATAL) << lane_joint_point << ", laneNodePtr is nullptr.";
          }
        } else {
          LOG(WARNING) << lane_joint_point << ", has not existing.";
        }
      }
    } else {
      LOG(FATAL) << src_Lane_joint_point << ", srcLaneNodePtr is nullptr.";
    }
  } else {
    LOG(FATAL) << src_Lane_joint_point << ", has not existing.";
  }
  return true;
}

Base::txBool txRoadNetwork::CreateGraphVertexList() TX_NOEXCEPT {
  // 清空关节点ID到顶点属性的映射和顶点ID到顶点属性的映射
  s_JointPtId2VtxProperty.clear();
  s_VtxId2VtxProperty.clear();
  // 遍历关节点映射
  for (auto refPair_JointId2NodePtr : s_map_jointpoint2treeNode) {
    // 查找关节点的根节点
    Base::JointPointTreeNodeWeakPtr rootNodePtr = SearchRootNode(refPair_JointId2NodePtr.second);
    // 设置关节点的祖先指针为根节点
    refPair_JointId2NodePtr.second->ancestorPtr = rootNodePtr;

    // 获取根节点的全局位置ID
    const Base::txSize JointPointId = rootNodePtr.lock()->mSysId;
    TX_MARK("global location id");
    // 计算关节点的经纬度坐标
    const hadmap::txPoint wsg84 = ComputeJointPointLocation(rootNodePtr.lock()->mHashSetId);
    // 创建顶点属性
    const VertexProperty vtx{JointPointId, wsg84, rootNodePtr.lock()->mHashSetId};
    // 将关节点ID和顶点属性插入到关节点ID到顶点属性的映射中
    JointPointId2VertexPropertyType::const_accessor ca;
    if (CallFail(s_JointPtId2VtxProperty.find(ca, refPair_JointId2NodePtr.first))) {
      JointPointId2VertexPropertyType::accessor a;
      s_JointPtId2VtxProperty.insert(a, refPair_JointId2NodePtr.first);
      a->second = vtx;
      a.release();
    }

    // 将顶点ID和顶点属性插入到顶点ID到顶点属性的映射中
    VtxPropertyId2VtxPropertyType::const_accessor ca_vtxId;
    if (CallFail(s_VtxId2VtxProperty.find(ca_vtxId, JointPointId))) {
      VtxPropertyId2VtxPropertyType::accessor a_vtxId;
      s_VtxId2VtxProperty.insert(a_vtxId, JointPointId);
      a_vtxId->second = vtx;
      a_vtxId.release();
    }

    // 输出关节点ID到顶点属性的映射和顶点ID到顶点属性的映射的大小
    LogInfo << TX_VARS(s_JointPtId2VtxProperty.size()) << TX_VARS(s_VtxId2VtxProperty.size());
  }
  return true;
}

void txRoadNetwork::Release() TX_NOEXCEPT {
  LOG(WARNING) << "[~] txRoadNetwork" << TX_VARS(s_map_jointpoint2treeNode.size())
               << TX_VARS(s_vec_non_leaf_nodes.size()) << TX_VARS(s_JointPtId2VtxProperty.size())
               << TX_VARS(s_VtxId2VtxProperty.size());
  s_map_jointpoint2treeNode.clear();
  s_vec_non_leaf_nodes.clear();
  s_JointPtId2VtxProperty.clear();
  s_VtxId2VtxProperty.clear();
}

// 初始化道路网络
Base::txBool txRoadNetwork::Initialize(const hadmap::txLanes& lanePtrVec,
                                       const hadmap::txLaneLinks& lanelinkPtrVec) TX_NOEXCEPT {
  // 如果地图缓存有效
  if (CallSucc(HadmapCacheConCurrent::IsValid())) {
    // 释放资源
    Release();
    // s_map_jointpoint2treeNode.clear();
    // 创建车道连接关节点集合
    if (CallFail(CreateLaneLinkJointPointVec(lanelinkPtrVec))) {
      LogWarn << "[Error] CreateLaneLinkJointPointVec failure.";
      return false;
    }

    // 创建车道关节点集合
    if (CallFail(CreateLaneJointPointVec(lanePtrVec))) {
      LogWarn << "[Error] CreateLaneJointPointVec failure.";
      return false;
    }

    // 输出关节点叶子节点的大小
    LogInfo << "joint point leaf size = " << s_map_jointpoint2treeNode.size();

    // 遍历车道连接指针集合
    for (const auto& refLinkPtr : lanelinkPtrVec) {
      // 如果车道连接指针非空
      if (NonNull_Pointer(refLinkPtr)) {
        // 获取车道连接ID
        const Base::txLaneLinkID linkId = refLinkPtr->getId();
        // 获取车道连接的起始车道ID
        const Base::txLaneUId fromLaneUid = refLinkPtr->fromTxLaneId();
        // 获取车道连接的终止车道ID
        const Base::txLaneUId toLaneUid = refLinkPtr->toTxLaneId();
        // 合并车道连接起始点关节点与车道关节点
        if (CallFail(MergeJointWithLanelinkAndLane(linkId, fromLaneUid, toLaneUid, true /*lanelink start pt*/))) {
          LogWarn << "[Error] MergeJointWithLanelinkAndLane(" << linkId << ", true) failure.";
          return false;
        }
        // 合并车道连接终止点关节点与车道关节点
        if (CallFail(MergeJointWithLanelinkAndLane(linkId, fromLaneUid, toLaneUid, false /*lanelink end pt*/))) {
          LogWarn << "[Error] MergeJointWithLanelinkAndLane(" << linkId << ", false) failure.";
          return false;
        }
      }
    }

    // 遍历车道指针集合
    for (const auto& refLanePtr : lanePtrVec) {
      // 如果车道指针非空
      if (NonNull_Pointer(refLanePtr)) {
        // 获取车道ID
        const Base::txLaneUId srcLaneUid = refLanePtr->getTxLaneId();
        // 合并车道起始点关节节点与车道关节节点
        if (CallFail(MergeJointWithLaneAndLane(srcLaneUid, true /*lane start pt*/))) {
          LogWarn << "[Error] MergeJointWithLaneAndLane(" << Utils::ToString(srcLaneUid) << ", true) failure.";
          return false;
        }
        // 合并车道终止点关节节点与车道关节节点
        if (CallFail(MergeJointWithLaneAndLane(srcLaneUid, false /*lane end pt*/))) {
          LogWarn << "[Error] MergeJointWithLaneAndLane(" << Utils::ToString(srcLaneUid) << ", false) failure.";
          return false;
        }
      }
    }
    // 创建图顶点列表
    CreateGraphVertexList();
    return true;
    TX_MARK("__stack_chk_fail");
  } else {
    LogWarn << "HadmapCacheConCurrent is not valid.";
    return false;
  }
}

txRoadNetwork::VertexProperty txRoadNetwork::GetVtxProperty(const JointPointId_t& jointPtId) TX_NOEXCEPT {
  // 在关节点ID到顶点属性的映射中查找关节点ID
  JointPointId2VertexPropertyType::const_accessor ca;
  if (CallSucc(s_JointPtId2VtxProperty.find(ca, jointPtId))) {
    // 如果找到了关节点ID，返回对应的顶点属性
    return (*ca).second;
  } else {
    // 如果没有找到关节点ID，输出错误信息并返回空顶点属性
    LOG(FATAL) << jointPtId << " do not existing.";
    return VertexProperty();
  }
}

// 获取顶点ID对应的顶点属性
txRoadNetwork::VertexProperty txRoadNetwork::GetVtxProperty(const Base::txSize& vtxId) TX_NOEXCEPT {
  // 在顶点ID到顶点属性的映射中查找顶点ID
  VtxPropertyId2VtxPropertyType::const_accessor ca;
  if (CallSucc(s_VtxId2VtxProperty.find(ca, vtxId))) {
    // 如果找到了顶点ID，返回对应的顶点属性
    return (*ca).second;
  } else {
    LOG(FATAL) << vtxId << " do not existing.";
    return VertexProperty();
  }
}

Base::txBool txRoadNetwork::CreateGraphEdgeList(const hadmap::txLanes& lanePtrVec,
                                                const hadmap::txLaneLinks& lanelinkPtrVec) TX_NOEXCEPT {
  return true;
}

TX_NAMESPACE_CLOSE(HdMap)

#undef LogWarn
#undef LogInfo
