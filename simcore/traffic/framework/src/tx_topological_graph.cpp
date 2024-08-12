// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_topological_graph.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Topology)

Base::txBool txTopologicalSort::Initialize(const txEdgeVec& edges, const Base::txInt nSize) TX_NOEXCEPT {
  // 创建拓扑图
  m_topologyGraph = std::make_shared<Graph>(edges.begin(), edges.end(), nSize);
  if (m_topologyGraph) {
    m_vertexSet.clear();
    // 遍历边，将顶点添加到顶点集合中
    for (const auto& e : edges) {
      m_vertexSet.insert(e.first);
      m_vertexSet.insert(e.second);
    }
    return true;
  } else {
    return false;
  }
}

Base::txBool txTopologicalSort::HasCycle(Base::txString& errInfo) TX_NOEXCEPT {
  // 如果拓扑图已初始化
  if (m_topologyGraph) {
    Base::txBool hasCycle = false;

    txCycleDetector checkCycle(hasCycle, errInfo);
    // 使用深度优先搜索遍历拓扑图，并使用环检测器访问每个顶点
    boost::depth_first_search((*m_topologyGraph), boost::visitor(checkCycle));
    // 如果存在环，则返回true
    if (hasCycle) {
      return true;
    } else {
      // 如果不存在环，清空错误信息并返回false
      errInfo.clear();
      return false;
    }
  } else {
    errInfo.clear();
    return false;
  }
}

std::vector<txTopologicalSort::VertexType> txTopologicalSort::Sort(
    std::vector<VertexType>& ValidVertexVec, std::vector<VertexType>& InvalidVertexVec) TX_NOEXCEPT {
  // 清空有效顶点和无效顶点向量
  ValidVertexVec.clear();
  InvalidVertexVec.clear();
  // 将顶点集合复制到有效顶点向量中
  std::copy(m_vertexSet.begin(), m_vertexSet.end(), std::back_inserter(ValidVertexVec));
  std::vector<VertexType> retVec;
  // 创建错误信息字符串
  Base::txString errInfo;
  // 如果拓扑图已初始化且不存在环
  if (m_topologyGraph && !HasCycle(errInfo)) {
    OrderList make_order;
    // 使用Boost库的topological_sort函数对拓扑图进行排序
    boost::topological_sort((*m_topologyGraph), std::front_inserter(make_order));

    // 遍历排序后的顶点
    for (const auto orderVtx : make_order) {
      // 如果顶点在顶点集合中，则将其添加到返回向量中
      if (0 != m_vertexSet.count(orderVtx)) {
        /*valid vertex*/
        retVec.emplace_back(orderVtx);
      } else {
        // 如果顶点不在顶点集合中，则将其添加到无效顶点向量中
        InvalidVertexVec.emplace_back(orderVtx);
      }
    }
  }
  return retVec;
}

void txTopologicalSort::Release() TX_NOEXCEPT { m_topologyGraph = nullptr; }

std::ostream& operator<<(std::ostream& os, const txEdge& e) TX_NOEXCEPT {
  os << "(" << TX_VARS_NAME(from, e.first) << TX_VARS_NAME(to, e.second) << ")";
  return os;
}

TX_NAMESPACE_CLOSE(Topology)
TX_NAMESPACE_CLOSE(Geometry)
