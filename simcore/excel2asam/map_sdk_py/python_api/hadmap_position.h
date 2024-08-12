// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct Position.

#pragma once

#include "hadmap_position_lane.h"
#include "hadmap_position_world.h"

namespace hadmap {
struct Position {
  PositionWorld world = PositionWorld();
  PositionLane lane = PositionLane();

  /*! \~Chinese
   * @brief 默认构造函数.
   *
   */
  Position();

  /*! \~Chinese
   * @brief 参数化构造函数.
   *
   * @param[in] k_world(Optional) 世界坐标
   * @param[in] k_lane(Optional) 车道坐标
   */
  explicit Position(PositionWorld k_world);
  explicit Position(PositionLane k_lane);
  Position(PositionWorld k_world, PositionLane k_lane);

  /*! \~Chinese
   * \brief 拷贝构造函数.
   *
   * @param[in] other another Position.
   */
  Position(const Position& other);

  /*! \~Chinese
   * @brief 赋值运算符重载, 允许将一个 Position 对象的值赋给另一个同类型对象的实例.
   *
   * @param[in] k another Position.
   * @return Position&
   */
  Position& operator=(const Position& k);

  /*! \~Chinese
   * @brief 等号运算符重载, 用于比较两个 Position 对象是否相等.
   *
   * @param[in] other another Position.
   * @return bool
   */
  bool operator==(const Position& other) const;

  /*! \~Chinese
   * @brief 逻辑非运算符重载, 用于判断一个 Position 对象是否为空.
   *
   * @return bool
   */
  bool operator!() const;

  /*! \~Chinese
   * @brief 加运算符重载, 用于将一个 Position 对象的值加上另一个同类型对象的实例.
   *
   * @param[in] position
   * @return Position&
   */
  const Position operator+(const Position& position) const;

  /*! \~Chinese
   * @brief 减运算符重载, 用于将一个 Position 对象的值减去另一个同类型对象的实例.
   *
   * @param[in] position
   * @return Position&
   */
  const Position operator-(const Position& position) const;

  /*! \~Chinese
   * @brief 乘运算符重载, 用于将一个 Position 对象的值乘以一个实数.
   *
   * @param[in] d 实数
   * @return Position&
   */
  const Position operator*(const double d) const;

  /*! \~Chinese
   * @brief 除运算符重载, 用于将一个 Position 对象的值除以一个实数.
   *
   * @param[in] d 实数
   * @return Position&
   */
  const Position operator/(const double d) const;
};

}  // namespace hadmap
