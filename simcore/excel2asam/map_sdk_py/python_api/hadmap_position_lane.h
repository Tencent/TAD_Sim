// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct PositionLane.

#pragma once

#include <cstdint>
#include "hadmap_orientation.h"

namespace hadmap {
struct PositionLane {
  uint64_t road_id = 0;
  int64_t lane_id = 0;
  double offset = 0.0;
  double s = 0.0;
  Orientation orientation = Orientation();

  /*! \~Chinese
   * @brief 默认构造函数
   *
   */
  PositionLane();

  /*! \~Chinese
   * @brief 参数化构造函数.
   * @param k_road_id 位置信息中的 road_id.
   * @param k_lane_id 位置信息中的 lane_id.
   * @param k_offset 与车道中心线的偏移量.
   * @param k_s 从道路的起点沿道路参考线获取的 s 坐标.
   *
   */
  PositionLane(uint64_t k_road_id, int64_t k_lane_id, double k_offset, double k_s);
  PositionLane(uint64_t k_road_id, int64_t k_lane_id, double k_offset, double k_s, Orientation k_orientation);

  /*! \~Chinese
   * \brief 拷贝构造函数.
   *
   * @param[in] other another PositionLane.
   */
  PositionLane(const PositionLane& other);

  /*! \~Chinese
   * @brief 赋值运算符
   *
   * @param[in] k another PositionLane.
   * @return PositionLane&
   */
  PositionLane& operator=(const PositionLane& k);

  /*! \~Chinese
   * @brief 比较运算符
   *
   * @param[in] k another PositionLane.
   * @return bool
   */
  bool operator==(const PositionLane& other) const;

  /*! \~Chinese
   * @brief 逻辑非运算符重载, 用于判断一个 PositionLane 对象是否为空.
   *
   * @return bool
   */
  bool operator!() const;

  /*! \~Chinese
   * @brief 加运算符重载, 用于将一个 PositionLane 对象的值加上另一个同类型对象的实例.
   *
   * @param[in] point
   * @return PositionLane&
   */
  const PositionLane operator+(const PositionLane& point) const;

  /*! \~Chinese
   * @brief 减运算符重载, 用于将一个 PositionLane 对象的值减去另一个同类型对象的实例.
   *
   * @param[in] point
   * @return PositionLane&
   */
  const PositionLane operator-(const PositionLane& point) const;

  /*! \~Chinese
   * @brief 乘运算符重载, 用于将一个 PositionLane 对象的值乘以一个实数.
   *
   * @param[in] d 实数
   * @return PositionLane&
   */
  const PositionLane operator*(const double d) const;

  /*! \~Chinese
   * @brief 除运算符重载, 用于将一个 PositionLane 对象的值除以一个实数.
   *
   * @param[in] d 实数
   * @return PositionLane&
   */
  const PositionLane operator/(const double d) const;
};

}  // namespace hadmap
