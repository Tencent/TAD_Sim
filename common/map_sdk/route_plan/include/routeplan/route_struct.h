// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/03/29
// @brief path plan data structure

#pragma once
#include <cfloat>
#include <climits>
#include <list>
#include <memory>
#include <vector>

#include "mapengine/hadmap_engine.h"

namespace planner {
enum ROAD_LINK_TYPE {
  DEFAULT = 0,
  STRAIGHT = 1 << 0,
  TURN_LEFT = 1 << 1,
  TURN_RIGHT = 1 << 2,
  U_TURN = 1 << 3,
  WITH_TRAFFIC_LIGHT = 1 << 4,
  LEFT_WITH_TRAFFIC_LIGHT = TURN_LEFT | WITH_TRAFFIC_LIGHT,
  STRAIGHT_WITH_TRAFFIC_LIGHT = STRAIGHT | WITH_TRAFFIC_LIGHT,
  RIGHT_WITH_TRAFFIC_LIGHT = TURN_RIGHT | WITH_TRAFFIC_LIGHT,
  U_TURN_WITH_TRAFFIC_LIGHT = U_TURN | WITH_TRAFFIC_LIGHT
};

struct NodeId {
  hadmap::roadpkid roadId;
  hadmap::lanepkid laneId;

  NodeId() : roadId(0), laneId(-1) {}

  explicit NodeId(hadmap::roadpkid rId, hadmap::lanepkid lId = -1) : roadId(rId), laneId(lId) {}

  void clear() {
    roadId = 0;
    laneId = -1;
  }

  bool operator==(const NodeId& id) const { return this->roadId == id.roadId && this->laneId * id.laneId > 0; }

  bool operator!=(const NodeId& id) const { return this->roadId != id.roadId || this->laneId * id.laneId < 0; }

  bool operator<(const NodeId& id) const {
    if (this->roadId == id.roadId) {
      return this->laneId < id.laneId;
    } else {
      return this->roadId < id.roadId;
    }
  }
};

struct Node;
typedef std::shared_ptr<Node> NodePtr;
typedef std::list<NodePtr> NodePtrList;

struct Node {
  NodeId nodeId;
  double roadLen;
  double gCost;
  double hCost;
  std::weak_ptr<Node> parrent;
  NodePtrList adjNodes;

  hadmap::txPoint startPt;
  hadmap::txPoint endPt;

  Node() : roadLen(0.0), gCost(DBL_MAX), hCost(DBL_MAX) {}

  bool operator==(const Node& node) const { return this->nodeId == node.nodeId; }

  bool operator!=(const Node& node) const { return this->nodeId != node.nodeId; }

  // priority queue
  // desc
  bool operator>(const Node& node) const {
    if (fabs(this->gCost + this->hCost - node.gCost - node.hCost) < 1.0) {
      return this->gCost > node.gCost;
    } else {
      return (this->gCost + this->hCost) > (node.gCost + node.hCost);
    }
  }

  void clear() {
    nodeId.roadId = 0;
    nodeId.laneId = 0;
    roadLen = 0.0;
    gCost = 0.0;
    hCost = 0.0;
    adjNodes.clear();
  }
};

struct Greater {
  bool operator()(const NodePtr& a, const NodePtr& b) { return (*a) > (*b); }
};

typedef std::vector<NodePtr> PathList;

struct QueryParam {
  NodePtr srcNode;
  NodePtr dstNode;
  NodePtr bakDstNode;

  QueryParam() : srcNode(new Node), dstNode(new Node), bakDstNode(new Node) {}
};
}  // namespace planner
