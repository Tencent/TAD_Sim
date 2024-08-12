// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <array>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include "tx_header.h"
#include "tx_tc_init_params.h"

TX_NAMESPACE_OPEN(Utils)

class MapRangeUtils {
 public:
  using bgi_point = boost::geometry::model::d2::point_xy<Base::txFloat>;
  using polygon_clockwise_close = boost::geometry::model::polygon<bgi_point>;

 public:
  MapRangeUtils() TX_DEFAULT;

  /**
   * @brief 初始化对象的自身地图范围与周围交通范围的关联关系
   *
   * 初始化函数使用传入的自身地图范围和周围交通范围信息，初始化对象的关联关系，
   * 并返回操作是否成功。
   *
   * @param selfMapRange 自身地图范围的信息
   * @param surrounding_map_ranges 周围交通范围的映射
   * @return 返回操作是否成功，如果成功则为true，否则为false
   */
  Base::txBool Init(const Scene::InitInfoWrap::map_range_info_t& selfMapRange,
                    const Scene::SurroundingTrafficRangeMap& surrounding_map_ranges) TX_NOEXCEPT;

  /**
   * @brief 根据给定的地图位置获取LocalClockWise闭合范围
   *
   * 根据给定的地图位置，计算LocalClockWise闭合范围的起始和结束点，然后将这些点存储在outter_intersection和change_owner_region向量中。
   *
   * @param pos 地图位置
   * @param outter_intersection 存储LocalClockWise闭合范围的起始和结束点的向量
   * @param change_owner_region 存储区域边界发生变化的区域的向量
   * @return 返回true，如果找到LocalClockWise闭合范围；否则返回false
   */
  Base::txBool GetRange_LocalClockWiseClose_ByIndex(sim_msg::MapPosition pos,
                                                    std::vector<Base::txVec2>& outter_intersection,
                                                    std::vector<Base::txVec2>& change_owner_region) const TX_NOEXCEPT;

  /**
   * @brief 判断当前区域是否有效
   * @return true 如果当前区域有效
   * @return false 如果当前区域无效
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

 protected:
  /**
   * @brief 将给定的枚举值转换为相应的整数值
   *
   * 此函数接受一个枚举值作为参数，返回该枚举值对应的整数值。
   *
   * @param enumValue 要转换的枚举值
   * @return 返回转换后的整数值
   */
  static Base::txInt enum2int(const sim_msg::MapPosition enumValue) TX_NOEXCEPT;

  /**
   * @brief 将给定的底部左侧点和顶部右侧点转换为顺时针闭合多边形
   *
   * 该函数将给定的底部左侧点和顶部右侧点转换为顺时针闭合多边形。
   *
   * @param enu_bottomleft 底部左侧点的ENU坐标（横纵坐标）
   * @param enu_topright 顶部右侧点的ENU坐标（横纵坐标）
   * @return polygon_clockwise_close 返回转换后的顺时针闭合多边形
   */
  static polygon_clockwise_close bottomleft_topright_to_clockwise_close_polygon(
      const Base::txVec2& enu_bottomleft, const Base::txVec2& enu_topright) TX_NOEXCEPT;

 protected:
  std::unordered_map<
      Base::txInt,
      std::tuple<std::vector<Base::txVec2> TX_MARK("self-outter && other-outter"), std::vector<Base::txVec2> > TX_MARK(
          "self-outter && other-inner")>
      mRangeIntersectionRegionMap_clockwise_close;
  Base::txBool mValid = false;
};

TX_NAMESPACE_CLOSE(Utils)
