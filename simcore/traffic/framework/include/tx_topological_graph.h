// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <iostream>
#include <limits>
#include <set>
#include <vector>
#include "tx_header.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Topology)

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                    boost::property<boost::vertex_color_t, boost::default_color_type> >;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
using OrderList = std::list<Vertex>;
using GraphPtr = std::shared_ptr<Graph>;

std::ostream& operator<<(std::ostream& os, const txEdge& e) TX_NOEXCEPT;

struct txCycleDetector : public boost::dfs_visitor<> {
  txCycleDetector(bool& has_cycle, std::string& error) : _has_cycle(has_cycle), _error(error) {}

  /**
   * @brief txTopologicalGraph::back_edge
   * @param _edge 反向边
   * @param graph 图
   * 在检测到循环边时调用此函数
   * @return void
   */
  template <class Edge, class Graph>
  void back_edge(Edge _edge, Graph&) {
    _has_cycle = true;
    _error = Base::txString("[") + std::to_string(_edge.m_source) + Base::txString(" -> ") +
             std::to_string(_edge.m_target) + Base::txString("]");
  }

 protected:
  Base::txBool& _has_cycle;
  Base::txString& _error;
};

class txTopologicalSort {
 public:
  using VertexType = txEdge::first_type;

 public:
  txTopologicalSort() TX_DEFAULT;

  /**
   * @brief txTopologicalGraph::Initialize 初始化拓扑图
   * @param edges 拓扑图的边集合
   * @param nSize 节点数量
   * @return 初始化是否成功
   * 使用给定的边集合和节点数量初始化拓扑图，并返回初始化是否成功。
   */
  Base::txBool Initialize(const txEdgeVec& edges, const Base::txInt nSize) TX_NOEXCEPT;

  /**
   * @brief HasCycle 判断图中是否存在环路
   * @param errInfo 如果存在错误，将填充错误信息
   * @return true 存在环路；false 不存在环路
   */
  Base::txBool HasCycle(Base::txString& errInfo) TX_NOEXCEPT;

  /**
   * @brief txTopologicalGraph::Sort
   * @param ValidVertexVec 有效的顶点向量
   * @param InvalidVertexVec 无效的顶点向量
   * @return 返回排序后的顶点向量
   */
  std::vector<VertexType> Sort(std::vector<VertexType>& ValidVertexVec,
                               std::vector<VertexType>& InvalidVertexVec) TX_NOEXCEPT;

  /**
   * @brief 资源释放
   *
   */
  void Release() TX_NOEXCEPT;

 protected:
  GraphPtr m_topologyGraph;
  std::set<VertexType> m_vertexSet;
};

TX_NAMESPACE_CLOSE(Topology)
TX_NAMESPACE_CLOSE(Geometry)
