// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the class Map.

#include "hadmap_map.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include "common/coord_trans.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "routingmap/routing_map.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_predef.h"
#include "types/map_defs.h"
#include "utils.h"

namespace hadmap {
Map::Map() {}
Map::~Map() { Release(); }

bool Map::operator==(const Map& other) const { return _maphandler == other._maphandler; }

bool Map::Load(const std::string& fname) {
  auto pos = fname.find_last_of(".");
  if (pos == std::string::npos) {
    return false;
  }
  std::string ext = fname.substr(pos);
  if (_kHadmapTypeDict.count(ext) == 0) {
    return false;
  }

  MAP_DATA_TYPE mdp = _kHadmapTypeDict.at(ext);

  return hadmapConnect(fname.c_str(), mdp, &_maphandler) == TX_HADMAP_HANDLE_OK;
}

void Map::Release() {
  hadmapClose(&_maphandler);
  _maphandler = nullptr;
}

txJunctions Map::GetAllJunctions() {
  txJunctions junctions;
  getJunctions(_maphandler, junctions);

  return junctions;
}

std::shared_ptr<Waypoint> Map::GetWaypoint(uint64_t road_id, int64_t lane_id, double offset, double s) {
  return std::make_shared<Waypoint>(shared_from_this(), Position(PositionLane(road_id, lane_id, offset, s)));
}

std::map<std::string, MAP_DATA_TYPE> Map::_kHadmapTypeDict = {
    {".sqlite", MAP_DATA_TYPE::SQLITE},
    {".xodr", MAP_DATA_TYPE::OPENDRIVE},
};

txLanePtr Map::_GetNearestLane(const PositionWorld& world) {
  txLanePtr _lane;
  getLane(_maphandler, hadmap::txPoint(world.x, world.y, world.z), _lane);
  if (_lane) {
    return _lane;
  }
  return nullptr;
}

PositionWorld Map::_ConvertLane2World(const PositionLane& lane) {
  // Empty result
  PositionWorld emptyResult;

  // Check for valid input
  if (&lane == nullptr) {
    std::cerr << "Warning: Input lane pointer is null.\n";
    return emptyResult;
  }

  // Avoid lane being modified
  roadpkid road_id = lane.road_id;
  lanepkid lane_id = lane.lane_id;
  double s = lane.s;
  double offset = lane.offset;

  // Initialize
  sectionpkid section_id;
  txLanes pLanesAll;

  if (lane.lane_id > 0) {
    road_id += 1000000000;
    lane_id = -lane_id;
  }

  // Get all lanes
  txLaneId tempid(road_id, SECT_PKID_INVALID, lane_id);
  if (!getLanes(_maphandler, tempid, pLanesAll) || pLanesAll.empty()) {
    std::cerr << "Warning: Unable to retrieve lanes.\n";
    return emptyResult;
  }

  // Compute the section id
  double accumulated_length = 0;
  bool found_section_id = false;
  for (const auto& singleLane : pLanesAll) {
    accumulated_length += singleLane->getLength();
    if (std::abs(accumulated_length - s) >= 0.000001) {
      section_id = singleLane->getSectionId();
      found_section_id = true;
      break;
    }
  }

  if (!found_section_id) {
    std::cerr << "Warning: No matching section id found.\n";
    return emptyResult;
  }

  // Get lane
  txLanePtr outLanePtr;
  if (!getLane(_maphandler, txLaneId(road_id, section_id, lane_id), outLanePtr) || outLanePtr == nullptr) {
    std::cerr << "Warning: Failed to retrieve specific lane.\n";
    return emptyResult;
  }

  // Get x, y, yaw and make PositionWorld
  double x = 0, y = 0, yaw = 0;
  if (outLanePtr->sl2xy(s, offset, x, y, yaw)) {
    auto p = outLanePtr->getGeometry()->getPoint(s);
    // std::cout << "The x is " << x << std::endl;
    // std::cout << "The y is " << y << std::endl;
    // std::cout << "The z is " << p.z << std::endl;
    // std::cout << "The yaw is " << yaw << std::endl;
    return PositionWorld(x, y, p.z, yaw, 0.0, 0.0);
  } else {
    std::cerr << "Warning: Failed to convert lane to world coordinates.\n";
    return emptyResult;
  }
}

PositionLane Map::_ConvertWorld2Lane(const PositionWorld& world) {
  PositionLane lane;
  txLanePtr _nearest_lane = _GetNearestLane(world);

  if (_nearest_lane != nullptr) {
    // 计算 s, offset, h
    double s, offset, h;
    _nearest_lane->xy2sl(world.x, world.y, s, offset, h);

    // lane.s = Round(s);
    // lane.offset = Round(offset);
    // lane.orientation.h = Round(h);
    lane.s = s;
    lane.offset = offset;
    lane.orientation.h = h;
    lane.road_id = _nearest_lane->getRoadId();
    lane.lane_id = _nearest_lane->getId();
  }

  return lane;
}

Position Map::ConvertPosition(const Position& position) {
  // 判断 position 是否 world 和 lane 都存在
  if (!position.world && !position.lane) {
    throw std::runtime_error("Neither world nor lane is in the position, must have one");
  }

  // if (!_maphandler) {
  //   throw std::runtime_error("Map is not loaded");
  //   return position;
  // }

  // 创建一个新的 Position 对象
  Position new_position = position;

  // if (!new_position.lane) {
  if (new_position.lane == PositionLane()) {
    // 仅 world 存在，则转换为 lane
    // std::cout << "仅 world 存在，则转换为 lane" << std::endl;
    new_position.lane = _ConvertWorld2Lane(new_position.world);
    // } else if (new_position.world) {
  } else if (new_position.world == PositionWorld()) {
    // 仅 lane 存在，则转换为 world
    // std::cout << "仅 lane 存在，则转换为 world" << std::endl;
    new_position.world = _ConvertLane2World(position.lane);
  }

  return new_position;
}

PositionWorld Map::Next(const PositionWorld& world, double distance, std::string direction) {
  // 定义变量
  std::vector<std::pair<double, hadmap::PointVec>> points;
  double traveled_distance;
  hadmap::txPoint lastpoint;
  PositionWorld outloc;

  std::vector<std::vector<hadmap::txLaneId>> laneids;
  hadmap::txLaneId lastlaneid;

  hadmap::txLanePtr lane;
  hadmap::txLaneLinkPtr lanelink;
  hadmap::txLaneLinks lanelinks;

  std::vector<std::string> out_directions;

  // 构建当前点的数据格式
  hadmap::txPoint txp(world.x, world.y, world.z);

  // 计算从当前点至前方指定距离的每条路线,及每条路线的信息
  // 返回 std::pair(路线长度, 路线所有点串)
  // 返回 所有ID
  hadmap::getForwardPoints(_maphandler, txp, distance, points, 0.0, &laneids);

  // 对每条路线找寻最后一个点的信息, 构建输出格式
  for (int i = 0; i < points.size(); i++) {
    // 当前路线的已经行驶距离, 最后一个点的 point 和 LaneId
    traveled_distance = points[i].first;
    lastpoint = points[i].second.back();
    lastlaneid = laneids[i].back();

    // 获得当前位置点 txp 前/后 distance 距离点的经纬度
    // 判断最后一个点是否在 lanelink 上
    // 如果在 lanelink 上, 则找到当前 lanelink 前方链接的 Lane, 计算 Lane 的起点经纬度
    // 否则则直接使用当前点的经纬度
    // 处理 laneLink
    if (lastlaneid.sectionId == SECT_PKID_INVALID || lastlaneid.laneId == LANE_PKID_INVALID) {
      // 找到当前点所在的 lanelink
      hadmap::getLaneLink(_maphandler, lastpoint, lanelink);
      // 计算方向
      out_directions.push_back(CalDirection(lanelink->getDeltaDierction()));
      // 找到当前 lanelink 前方链接的 Lane, 计算 Lane 的起点经纬度
      hadmap::getLane(_maphandler, lanelink->toTxLaneId(), lane);
      double x, y, yaw;
      double s, l = 0.0;
      if (lane->sl2xy(s, l, x, y, yaw)) {
        outloc.x = x;
        outloc.y = y;
        outloc.z = 0.0;
        outloc.h = yaw;
      } else {
        outloc.x = 0.0;
        outloc.y = 0.0;
        outloc.z = 0.0;
        outloc.h = 0.0;
      }
    } else {
      // 找到当前点前序的 lanelink
      hadmap::getPrevLaneLinks(_maphandler, lastlaneid, lanelinks);

      if (lanelinks.size() == 0) {
        out_directions.push_back(CalDirection(0));
      } else {
        for (hadmap::txLaneLinkPtr lanelink : lanelinks) {
          out_directions.push_back(CalDirection(lanelink->getDeltaDierction()));
        }
      }

      outloc.x = lastpoint.x;
      outloc.y = lastpoint.y;
      outloc.z = lastpoint.z;
      outloc.h = lastpoint.h;
    }

    // 构建最终输出的方向
    // 如果 out_directions  全部为 Straight, 则最终为 Straight
    // 如果 out_directions 为 Straight和 Left, 则最终为 Left
    // 如果 out_directions 为 Straight和 Right, 则最终为 Right
    // 如果 out_directions 存在 Left和 Right, 则以最后一位为最终结果
    std::string final_direction;
    bool has_left = false, has_right = false;

    for (const std::string& out_direction : out_directions) {
      if (out_direction == "L" || out_direction == "L0") {
        has_left = true;
      } else if (out_direction == "R" || out_direction == "R0") {
        has_right = true;
      }
    }

    if (has_left && has_right) {
      final_direction = out_directions.back();  // 如果存在 "Left" 和 "Right", 则以最后一位为最终结果
    } else if (has_left) {
      final_direction = "L";
    } else if (has_right) {
      final_direction = "R";
    } else {
      final_direction = "T";
    }

    // 构建最终输出
    if ((final_direction == direction) && (traveled_distance <= (std::fabs(distance) + 5))) {
      return outloc;
    } else {
      continue;
    }
  }

  // 默认返回
  return PositionWorld();
}

}  // namespace hadmap
