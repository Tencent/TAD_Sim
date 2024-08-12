// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
TX_NAMESPACE_OPEN(TrafficFlow)

struct od_route_vector_in {
 public:
  using txInt = Base::txInt;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;

  using txLaneID = Base::txLaneID;
  using txSectionID = Base::txSectionID;
  using txRoadID = Base::txRoadID;
  using txLaneLinkID = Base::txLaneLinkID;
  using txString = Base::txString;

 public:
  struct route_node_in {
    txString route_id;
    Base::txInt dest_id;
    txString origin_desc;
    txString dest_desc;
    std::vector<txString> road_id;
    Base::txSize route_cnt;
    Base::txInt origin_id;
    Base::txFloat length;
  };
  std::vector<route_node_in> route;
  Base::txSize vehicle_cnt;
};

struct od_route_vector_out {
 public:
  using txInt = Base::txInt;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;

  using txLaneID = Base::txLaneID;
  using txSectionID = Base::txSectionID;
  using txRoadID = Base::txRoadID;
  using txLaneLinkID = Base::txLaneLinkID;
  using txString = Base::txString;

 public:
  struct route_node_out {
    txString route_id;
    Base::txInt dest_id;
    txString origin_desc;
    txString dest_desc;
    std::vector<txString> road_id;
    Base::txSize route_cnt;
    Base::txInt origin_id;
    Base::txFloat length;
    Base::txFloat dbAverageVelocityFromMicroSim = 0.0;
    Base::txFloat dbCongestionIndexFromMicroSim = 0.0;
    Base::txSize nVehicleCountFromMicroSim = 0;

    route_node_out() TX_DEFAULT;

    /**
     * @brief 结构体的构造函数
     *
     * @param inNode 用于初始化结构体内容
     */
    explicit route_node_out(const od_route_vector_in::route_node_in& inNode) {
      route_id = inNode.route_id;
      dest_id = inNode.dest_id;
      origin_desc = inNode.origin_desc;
      dest_desc = inNode.dest_desc;
      road_id = inNode.road_id;
      route_cnt = inNode.route_cnt;
      origin_id = inNode.origin_id;
      length = inNode.length;
    }

    /**
     * @brief 赋值运算符的重写
     *
     * @param inNode 用于初始化结构体内容
     * @return route_node_out
     */
    route_node_out operator=(const od_route_vector_in::route_node_in& inNode) TX_NOEXCEPT {
      route_id = inNode.route_id;
      dest_id = inNode.dest_id;
      origin_desc = inNode.origin_desc;
      dest_desc = inNode.dest_desc;
      road_id = inNode.road_id;
      route_cnt = inNode.route_cnt;
      origin_id = inNode.origin_id;
      length = inNode.length;
      return (*this);
    }
  };
  std::vector<route_node_out> route;
  Base::txSize vehicle_cnt;

 public:
  od_route_vector_out operator=(const od_route_vector_in& inRoute) TX_NOEXCEPT {
    route.resize(inRoute.route.size());
    for (auto idx = 0; idx < inRoute.route.size(); ++idx) {
      route[idx] = inRoute.route[idx];
    }
    vehicle_cnt = inRoute.vehicle_cnt;
    return (*this);
  }
};

class od_info {
 public:
  /**
   * @brief 导入JSON格式字符串
   * @param inJsonStr JSON格式字符串
   * @return 导入成功返回 true，否则返回 false
   */
  Base::txBool Import(const Base::txString& inJsonStr) TX_NOEXCEPT;

  /**
   * @brief 将数据导出为 JSON 格式字符串
   * @param outJsonStr 用于存储导出的 JSON 格式字符串的引用
   * @return 导出成功返回 true，否则返回 false
   */
  Base::txBool Export(Base::txString& outJsonStr) TX_NOEXCEPT;

  /**
   * @brief 获取出站 OD 路线向量
   * @return 出站 OD 路线向量的引用
   */
  od_route_vector_out& GetOdRouteVector() TX_NOEXCEPT { return outOdRouteVector; }

 protected:
  od_route_vector_out outOdRouteVector;
};

using od_info_ptr = std::shared_ptr<od_info>;

TX_NAMESPACE_CLOSE(TrafficFlow)
