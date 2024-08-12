// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <algorithm>
#include <deque>
#include <iostream>
#include <set>

#include "routeplan/lane_routing.h"
#include "structs/hadmap_curve.h"

#define LANE_CHANGE_COST 80

namespace LaneRoute {

struct LaneNode : public LaneInfo {
  double cost = std::numeric_limits<double>::infinity();
  double start_s = 0;
  LaneNode* parent = nullptr;
  LaneNode* left_node = nullptr;
  LaneNode* right_node = nullptr;
  std::vector<LaneNode*> next_nodes;
};

struct Sec {
  LaneNode::Type type;
  std::vector<LaneNode> lanes;

  LaneNode* findLaneNode(hadmap::txLaneId lane_id) {
    if (type != LaneNode::Type::Lane) {
      return nullptr;
    }
    for (auto& node : lanes) {
      if (node.lane_id == lane_id) {
        return &node;
      }
    }
    return nullptr;
  }

  LaneNode* findLaneNode(hadmap::lanelinkpkid link_id) {
    if (type != LaneNode::Type::Link) {
      return nullptr;
    }
    for (auto& node : lanes) {
      if (node.link_id == link_id) {
        return &node;
      }
    }
    return nullptr;
  }
};

class LaneRouteGen {
 public:
  LaneRouteGen(hadmap::txMapHandle* map_handle, const hadmap::txPoint& start, const hadmap::txPoint& goal)
      : map_handle_(map_handle), start_(start), goal_(goal) {}

  virtual ~LaneRouteGen() {}

  bool generate(std::vector<LaneInfo>& lane_route);

 private:
  void packRouteSection(const hadmap::txRoute& road_route);
  bool init();
  void searchSecs(std::vector<LaneInfo>& lane_route);
  void connectSecs();

  void connectCurrentSecs(Sec& cur);
  void connectNextSecs(Sec& cur, Sec& next);
  void expendNode(LaneNode* node);

  void insertNodeToOpenList(LaneNode* node);
  void getLaneNodePoints(LaneNode* lane_node, hadmap::PointVec& points);

  hadmap::txMapHandle* map_handle_;
  hadmap::txPoint start_;
  hadmap::txPoint goal_;

  hadmap::txLanePtr start_lane_ptr;
  hadmap::txLanePtr goal_lane_ptr;

  std::vector<Sec> route_secs_;

  std::deque<LaneNode*> open_list_;
};

void LaneRouteGen::insertNodeToOpenList(LaneNode* node) {
  auto it = open_list_.begin();
  while (it != open_list_.end()) {
    if ((*it)->cost > node->cost) {
      it = open_list_.insert(it, node);
      return;
    }
    it++;
  }
  open_list_.push_back(node);
}

void LaneRouteGen::expendNode(LaneNode* node) {
  double cost;
  if (node->left_node) {
    cost = node->cost + LANE_CHANGE_COST;
    if (node->left_node->cost > cost) {
      node->left_node->cost = cost;
      node->left_node->parent = node;
      insertNodeToOpenList(node->left_node);
    }
  }

  if (node->right_node) {
    cost = node->cost + LANE_CHANGE_COST;
    if (node->right_node->cost > cost) {
      node->right_node->cost = cost;
      node->right_node->parent = node;
      insertNodeToOpenList(node->right_node);
    }
  }

  for (auto next_node : node->next_nodes) {
    cost = node->cost + node->length;
    if (next_node->cost > cost) {
      next_node->cost = cost;
      next_node->parent = node;
      insertNodeToOpenList(next_node);
    }
  }
}

bool findLaneRoute(hadmap::txMapHandle* map_handle, const hadmap::txPoint& start, const hadmap::txPoint& goal,
                   std::vector<LaneInfo>& lane_route) {
  if (!map_handle) {
    return false;
  }

  LaneRouteGen lane_route_gen(map_handle, start, goal);
  return lane_route_gen.generate(lane_route);
}

void LaneRouteGen::connectCurrentSecs(Sec& cur) {
  if (cur.type == LaneNode::Type::Link) return;
  for (auto& lane_node : cur.lanes) {
    hadmap::txLanePtr lane, left, right;
    hadmap::getLane(map_handle_, lane_node.lane_id, lane);
    if (lane) {
      hadmap::getLeftLane(map_handle_, lane, left);
      if (left) {
        lane_node.left_node = cur.findLaneNode(left->getTxLaneId());
      } else {
        lane_node.left_node = nullptr;
      }

      hadmap::getRightLane(map_handle_, lane, right);
      if (right) {
        lane_node.right_node = cur.findLaneNode(right->getTxLaneId());
      } else {
        lane_node.right_node = nullptr;
      }
    }
  }
}

void LaneRouteGen::connectNextSecs(Sec& cur, Sec& next) {
  for (LaneNode& lane_node : cur.lanes) {
    if (cur.type == LaneNode::Type::Lane) {
      if (next.type == LaneNode::Type::Lane) {
        hadmap::txLanePtr lane_ptr;
        hadmap::getLane(map_handle_, lane_node.lane_id, lane_ptr);
        if (!lane_ptr) continue;

        hadmap::txLanes lanes;
        hadmap::getNextLanes(map_handle_, lane_ptr, lanes);
        for (hadmap::txLanePtr next_lane_ptr : lanes) {
          LaneNode* node = next.findLaneNode(next_lane_ptr->getTxLaneId());
          if (node) {
            lane_node.next_nodes.push_back(node);
          }
        }
      } else {
        hadmap::txLaneLinks links;
        hadmap::getNextLaneLinks(map_handle_, lane_node.lane_id, links);
        for (hadmap::txLaneLinkPtr next_link_ptr : links) {
          LaneNode* node = next.findLaneNode(next_link_ptr->getId());
          if (node) {
            lane_node.next_nodes.push_back(node);
          }
        }
      }
    } else {
      hadmap::txLaneLinkPtr link_ptr;
      hadmap::getLaneLink(map_handle_, lane_node.link_id, link_ptr);
      if (!link_ptr) continue;

      hadmap::txLanePtr lane_ptr;
      hadmap::getLane(map_handle_, link_ptr->toTxLaneId(), lane_ptr);
      if (lane_ptr) {
        LaneNode* next_node = next.findLaneNode(lane_ptr->getTxLaneId());
        if (next_node) {
          lane_node.next_nodes.push_back(next_node);
        }
      }
    }
  }
}

void LaneRouteGen::connectSecs() {
  for (size_t i = 0; i < route_secs_.size(); ++i) {
    connectCurrentSecs(route_secs_[i]);
    if (i + 1 < route_secs_.size()) {
      connectNextSecs(route_secs_[i], route_secs_[i + 1]);
    }
  }
}

void LaneRouteGen::getLaneNodePoints(LaneNode* lane_node, hadmap::PointVec& points) {
  const hadmap::txCurve* curve;
  if (lane_node->type == LaneNode::Type::Lane) {
    hadmap::txLanePtr lane;
    hadmap::getLane(map_handle_, lane_node->lane_id, lane);
    curve = lane->getGeometry();

    auto line_curve = dynamic_cast<const hadmap::txLineCurve*>(curve);
    if (lane_node->length <= curve->getLength() - 1e-3) {
      double end_s = lane_node->start_s + lane_node->length;
      line_curve->cut(lane_node->start_s, end_s, points);
    } else {
      line_curve->getPoints(points);
    }
  } else {
    hadmap::txLaneLinkPtr link;
    hadmap::getLaneLink(map_handle_, lane_node->link_id, link);
    curve = link->getGeometry();
    auto line_curve = dynamic_cast<const hadmap::txLineCurve*>(curve);
    line_curve->getPoints(points);
  }
}

void LaneRouteGen::searchSecs(std::vector<LaneInfo>& lane_route) {
  LaneNode* start_node = nullptr;
  if (!route_secs_.empty()) {
    for (auto& lane_node : route_secs_.front().lanes) {
      if (lane_node.lane_id == start_lane_ptr->getTxLaneId()) {
        start_node = &lane_node;
      }
    }
  }

  if (start_node == nullptr) {
    return;
  } else {
    start_node->cost = 0;
  }

  open_list_.push_back(start_node);
  while (!open_list_.empty()) {
    auto node = open_list_.front();
    open_list_.pop_front();
    expendNode(node);
  }

  LaneNode* end_node = nullptr;
  if (!route_secs_.empty()) {
    for (auto& lane_node : route_secs_.back().lanes) {
      if (lane_node.lane_id == goal_lane_ptr->getTxLaneId()) {
        end_node = &lane_node;
      }
    }
  }

  if (end_node) {
    LaneNode* lane_node = end_node;
    while (lane_node) {
      lane_route.push_back(LaneInfo());
      LaneInfo& lane_info = lane_route.back();

      lane_info.type = lane_node->type;
      lane_info.lane_id = lane_node->lane_id;
      lane_info.link_id = lane_node->link_id;
      lane_info.length = lane_node->length;

      getLaneNodePoints(lane_node, lane_info.points);

      lane_node = lane_node->parent;
    }
  }

  std::reverse(lane_route.begin(), lane_route.end());
}

bool LaneRouteGen::generate(std::vector<LaneInfo>& lane_route) {
  if (!init()) {
    return false;
  }
  lane_route.clear();

  searchSecs(lane_route);

  return !lane_route.empty();
}

bool LaneRouteGen::init() {
  hadmap::PointVec way_pts;
  way_pts.push_back(start_);
  way_pts.push_back(goal_);

  hadmap::txRoute road_route;

  auto route_plan_ptr = std::unique_ptr<planner::RoutePlan>(new planner::RoutePlan(map_handle_));
  if (route_plan_ptr->searchPath(way_pts, road_route) == false) {
    return false;
  }

  if (road_route.empty()) {
    return false;
  }

  hadmap::getLane(map_handle_, start_, start_lane_ptr);
  if (!start_lane_ptr) return false;

  hadmap::getLane(map_handle_, goal_, goal_lane_ptr);
  if (!goal_lane_ptr) return false;

  if (road_route.front().getId() != start_lane_ptr->getRoadId() ||
      road_route.back().getId() != goal_lane_ptr->getRoadId()) {
    return false;
  }

  packRouteSection(road_route);
  connectSecs();

  return true;
}

void LaneRouteGen::packRouteSection(const hadmap::txRoute& road_route) {
  for (size_t i = 0; i < road_route.size(); ++i) {
    const hadmap::txRouteNode& node = road_route[i];

    if (node.getRouteType() == hadmap::txRouteNode::JUNCTION_ROUTE) {
      // for junction lanes
      hadmap::txLaneLinks links;
      hadmap::getLaneLinks(map_handle_, node.getPreId(), node.getNextId(), links);

      Sec sec_data;
      sec_data.type = LaneNode::Type::Link;

      for (size_t k = 0; k < links.size(); ++k) {
        LaneNode lane_data;

        lane_data.link_id = links[k]->getId();
        lane_data.length = links[k]->getGeometry()->getLength();
        lane_data.type = LaneNode::Type::Link;

        sec_data.lanes.emplace_back(lane_data);
      }

      route_secs_.emplace_back(sec_data);
    } else {
      // for road lanes
      hadmap::txRoadPtr road;
      hadmap::getRoad(map_handle_, node.getId(), true, road);
      for (size_t k = 0; k < road->getSections().size(); ++k) {
        hadmap::txSectionPtr sec = road->getSections()[k];

        // filter sections before and after start and goal points
        if (start_lane_ptr->getRoadId() == sec->getRoadId() && start_lane_ptr->getSectionId() > sec->getId()) {
          continue;
        }

        if (goal_lane_ptr->getRoadId() == sec->getRoadId() && goal_lane_ptr->getSectionId() < sec->getId()) {
          continue;
        }

        Sec sec_data;
        sec_data.type = LaneNode::Type::Lane;

        for (size_t m = 0; m < sec->getLanes().size(); ++m) {
          hadmap::txLanePtr lane_ptr = sec->getLanes()[m];
          LaneNode lane_data;
          lane_data.lane_id = lane_ptr->getTxLaneId();

          // get length of the first and last section
          double s, l, yaw;

          lane_data.start_s = 0;
          lane_data.length = lane_ptr->getLength();

          if (start_lane_ptr->getRoadId() == sec->getRoadId() && start_lane_ptr->getSectionId() == sec->getId()) {
            lane_ptr->getGeometry()->xy2sl(start_.x, start_.y, s, l, yaw);
            lane_data.start_s = s;
            lane_data.length -= s;
          }

          if (goal_lane_ptr->getRoadId() == sec->getRoadId() && goal_lane_ptr->getSectionId() == sec->getId()) {
            lane_ptr->getGeometry()->xy2sl(goal_.x, goal_.y, s, l, yaw);
            lane_data.length = s - lane_data.start_s;
          }

          lane_data.type = LaneNode::Type::Lane;
          sec_data.lanes.emplace_back(lane_data);
        }

        route_secs_.emplace_back(sec_data);
      }
    }
  }
}
}  // namespace LaneRoute
