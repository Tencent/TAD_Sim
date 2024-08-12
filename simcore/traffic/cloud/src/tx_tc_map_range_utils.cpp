// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_map_range_utils.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <deque>
#include "tx_enum_utils.h"
#include "tx_sim_point.h"
#include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(Utils)

MapRangeUtils::polygon_clockwise_close MapRangeUtils::bottomleft_topright_to_clockwise_close_polygon(
    const Base::txVec2& enu_bottomleft, const Base::txVec2& enu_topright) TX_NOEXCEPT {
  polygon_clockwise_close ret_clockwise_close_polygon;
  const Base::txVec2 enu_topleft(enu_bottomleft.x(), enu_topright.y());
  const Base::txVec2 enu_bottomright(enu_topright.x(), enu_bottomleft.y());

  bgi_point tmpPt;
  tmpPt.x(enu_bottomleft.x());
  tmpPt.y(enu_bottomleft.y());
  ret_clockwise_close_polygon.outer().emplace_back(tmpPt);

  tmpPt.x(enu_topleft.x());
  tmpPt.y(enu_topleft.y());
  ret_clockwise_close_polygon.outer().emplace_back(tmpPt);

  tmpPt.x(enu_topright.x());
  tmpPt.y(enu_topright.y());
  ret_clockwise_close_polygon.outer().emplace_back(tmpPt);

  tmpPt.x(enu_bottomright.x());
  tmpPt.y(enu_bottomright.y());
  ret_clockwise_close_polygon.outer().emplace_back(tmpPt);

  tmpPt.x(enu_bottomleft.x());
  tmpPt.y(enu_bottomleft.y());
  ret_clockwise_close_polygon.outer().emplace_back(tmpPt);
  return ret_clockwise_close_polygon;
}

Base::txBool MapRangeUtils::Init(const Scene::InitInfoWrap::map_range_info_t& selfMapRange,
                                 const Scene::SurroundingTrafficRangeMap& surrounding_map_ranges) TX_NOEXCEPT {
  LOG(INFO) << "Current Traffic Map Range : "
            << TX_VARS_NAME(bottom_left, Utils::ToString(selfMapRange.map_range.bottom_left))
            << TX_VARS_NAME(top_right, Utils::ToString(selfMapRange.map_range.top_right))
            << TX_VARS(selfMapRange.perception_radius);
  for (const auto& refMapRangePair : surrounding_map_ranges) {
    LOG(INFO) << "Surrounding Traffic Map Range : " << refMapRangePair.second;
  }

  if (selfMapRange.perception_radius > 0.0) {
    Coord::txWGS84 ws84_bottom_left(selfMapRange.map_range.bottom_left),
        ws84_top_right(selfMapRange.map_range.top_right);

    const Base::txVec2 enu_bottom_left = ws84_bottom_left.ToENU().ENU2D();
    const Base::txVec2 enu_top_right = ws84_top_right.ToENU().ENU2D();
    // const Base::txVec2 enu_extern_step = Base::txVec2(selfMapRange.perception_radius,
    // selfMapRange.perception_radius);

    Base::txVec2 enu_extern_dir_tag = enu_top_right - enu_bottom_left;
    enu_extern_dir_tag.x() = ((enu_extern_dir_tag.x() > 0.0) ? (1.0) : (-1.0));
    enu_extern_dir_tag.y() = ((enu_extern_dir_tag.y() > 0.0) ? (1.0) : (-1.0));

    const polygon_clockwise_close center_range_inner =
        bottomleft_topright_to_clockwise_close_polygon(enu_bottom_left, enu_top_right);
    const polygon_clockwise_close center_range_outter = bottomleft_topright_to_clockwise_close_polygon(
        enu_bottom_left + (-1.0 * selfMapRange.perception_radius * enu_extern_dir_tag),
        enu_top_right + (selfMapRange.perception_radius * enu_extern_dir_tag));

    for (const auto& refMapRangePair : surrounding_map_ranges) {
      refMapRangePair.second.surrounding_traffic_pos;
      const auto& otherMapRange = refMapRangePair.second.surrounding_traffic_map_range;
      Coord::txWGS84 ws84_other_bottom_left(otherMapRange.bottom_left), ws84_other_top_right(otherMapRange.top_right);
      const Base::txVec2 enu_other_bottom_left = ws84_other_bottom_left.ToENU().ENU2D();
      const Base::txVec2 enu_other_top_right = ws84_other_top_right.ToENU().ENU2D();

      Base::txVec2 enu_other_extern_dir_tag = enu_other_top_right - enu_other_bottom_left;
      enu_other_extern_dir_tag.x() = ((enu_other_extern_dir_tag.x() > 0.0) ? (1.0) : (-1.0));
      enu_other_extern_dir_tag.y() = ((enu_other_extern_dir_tag.y() > 0.0) ? (1.0) : (-1.0));

      const polygon_clockwise_close other_range_inner =
          bottomleft_topright_to_clockwise_close_polygon(enu_other_bottom_left, enu_other_top_right);
      const polygon_clockwise_close other_range_outter = bottomleft_topright_to_clockwise_close_polygon(
          enu_other_bottom_left + (-1.0 * selfMapRange.perception_radius * enu_other_extern_dir_tag),
          enu_other_top_right + (selfMapRange.perception_radius * enu_other_extern_dir_tag));

      std::deque<polygon_clockwise_close> deque_outter_intersection, deque_change_owner_region;
      boost::geometry::intersection(center_range_outter, other_range_inner, deque_change_owner_region);
      boost::geometry::intersection(center_range_outter, other_range_outter, deque_outter_intersection);

      if (1 == deque_change_owner_region.size() && 1 == deque_outter_intersection.size()) {
        const auto area_change_owner_region = boost::geometry::area(deque_change_owner_region[0]);
        const auto area_outter_intersection = boost::geometry::area(deque_outter_intersection[0]);

        LOG_IF(INFO, FLAGS_LogLevel_Cloud) << TX_VARS_NAME(pos, refMapRangePair.first)
                                           << TX_VARS(area_change_owner_region) << TX_VARS(area_outter_intersection);

        if (area_change_owner_region > 0.0 && area_outter_intersection > 0.0 &&
            area_outter_intersection > area_change_owner_region) {
          const polygon_clockwise_close& change_owner_region = deque_change_owner_region[0];
          std::vector<Base::txVec2> change_owner_region_vec;
          for (const auto bgiPt : change_owner_region.outer()) {
            TX_MARK("polygon size may not be 5.");
            change_owner_region_vec.emplace_back(Base::txVec2(bgiPt.x(), bgiPt.y()));
          }

          const polygon_clockwise_close& outter_intersection = deque_outter_intersection[0];
          std::vector<Base::txVec2> outter_intersection_vec;
          for (const auto bgiPt : outter_intersection.outer()) {
            outter_intersection_vec.emplace_back(Base::txVec2(bgiPt.x(), bgiPt.y()));
          }

          mRangeIntersectionRegionMap_clockwise_close[(refMapRangePair.first)] =
              std::make_tuple(outter_intersection_vec, change_owner_region_vec);
          LOG(INFO) << "Initialize Surrounding Success. " << TX_VARS_NAME(pos, refMapRangePair.first);
          mValid = true;
        }
      }
    }
  } else {
    LOG(WARNING) << "Error : " << TX_VARS(selfMapRange.perception_radius);
  }
  return IsValid();
}

Base::txBool MapRangeUtils::GetRange_LocalClockWiseClose_ByIndex(
    sim_msg::MapPosition pos, std::vector<Base::txVec2>& outter_intersection,
    std::vector<Base::txVec2>& change_owner_region) const TX_NOEXCEPT {
  if (IsValid()) {
    if (mRangeIntersectionRegionMap_clockwise_close.count(pos) > 0) {
      std::tie(outter_intersection, change_owner_region) = mRangeIntersectionRegionMap_clockwise_close.at(pos);
      txAssert(5 == outter_intersection.size() && 5 == change_owner_region.size());
      return true;
    } else {
      outter_intersection.clear();
      change_owner_region.clear();
      LOG(WARNING) << "Error, Unknown Surrounding Map Range " << TX_VARS(pos);
      return false;
    }
  } else {
    LOG(WARNING) << "Error, MapRangeUtils is invalid.";
    return false;
  }
}

Base::txInt MapRangeUtils::enum2int(const sim_msg::MapPosition enumValue) TX_NOEXCEPT {
  switch (enumValue) {
    case sim_msg::MapPosition::MAP_POSITION_TOP: {
      return 0;
    }
    case sim_msg::MapPosition::MAP_POSITION_TOP_LEFT: {
      return 1;
    }
    case sim_msg::MapPosition::MAP_POSITION_LEFT: {
      return 2;
    }
    case sim_msg::MapPosition::MAP_POSITION_BOTTOM_LEFT: {
      return 3;
    }
    case sim_msg::MapPosition::MAP_POSITION_BOTTOM: {
      return 4;
    }
    case sim_msg::MapPosition::MAP_POSITION_BOTTOM_RIGHT: {
      return 5;
    }
    case sim_msg::MapPosition::MAP_POSITION_RIGHT: {
      return 6;
    }
    case sim_msg::MapPosition::MAP_POSITION_TOP_RIGHT: {
      return 7;
    }
    default: {
      return 0;
    }
  }
}

TX_NAMESPACE_CLOSE(Utils)
