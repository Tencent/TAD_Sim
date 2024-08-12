// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_way_points.h"
#include "tx_spatial_query.h"
TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool WayPoints::Initialize(Coord::txWGS84& endPt) TX_NOEXCEPT {
  _type = WayPointType::Point;
  Base::txFloat distanceCurve = 0.0;
  // 通过空间查询模块获取离输入坐标较近的道路线
  // 并将结果存储到 _EndPtLanePtr 变量中
  _EndPtLanePtr = Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().GetLaneNearBy(
      endPt.Lon(), endPt.Lat(), distanceCurve);
  if (NonNull_Pointer(_EndPtLanePtr)) {
    _valid = true;
    _deadline_LaneUid = _EndPtLanePtr->getTxLaneId();
    // 将输入坐标转换为 ENU 坐标系
    _EndPos = endPt.ToENU();
  }
  return _valid;
}

Base::txString WayPoints::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS(IsValid());

  // 判断 WayPoints 对象的类型，并根据不同的类型生成不同的字符串
  if (IsPointArea()) {
    oss << "[PointArea]" << TX_VARS_NAME(Position, _EndPos)
        << TX_VARS_NAME(laneUid, Utils::ToString(_deadline_LaneUid));
  } else if (IsRoadSectionArea()) {
    oss << "[RoadSectionArea]" << TX_VARS_NAME(roadId, _deadline_LaneUid.roadId)
        << TX_VARS_NAME(sectionId, _deadline_LaneUid.sectionId);
  } else if (IsLaneLinkArea()) {
    std::ostringstream oss_links;
    std::copy(_all_link_set.begin(), _all_link_set.end(), std::ostream_iterator<Base::Info_Lane_t>(oss_links, ","));
    oss << "[LaneLinkArea]" << oss_links.str();
  } else {
    oss << "unknown WayPointType";
  }
  // 将最终生成的字符串返回
  return oss.str();
}

TX_NAMESPACE_CLOSE(TrafficFlow)
