// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct PositionWorld.

#pragma once

namespace hadmap {
struct PositionWorld {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double h = 0.0;
  double p = 0.0;
  double r = 0.0;

  /*! \~Chinese
   * @brief 默认构造函数.
   *
   */
  PositionWorld();

  /*! \~Chinese
   * @brief 参数化构造函数.
   *
   * @param[in] kx 坐标值 X.
   * @param[in] ky 坐标值 Y.
   * @param[in] kz 坐标值 Z.
   * @param[in] kh(Optional) 航向角 heading angle. (Unit: rad)
   * @param[in] kp(Optional) 俯仰角 pitch angle. (Unit: rad)
   * @param[in] kr(Optional) 旋转角 roll angle. (Unit: rad)
   */
  PositionWorld(double kx, double ky, double kz);
  PositionWorld(double kx, double ky, double kz, double kh, double kp, double kr);

  /*! \~Chinese
   * \brief 拷贝构造函数
   *
   * @param[in] other another PositionWorld.
   */
  PositionWorld(const PositionWorld& k);

  /*! \~Chinese
   * @brief 赋值运算符重载, 允许将一个 PositionWorld 对象的值赋给另一个同类型对象的实例
   *
   * @param[in] k another PositionWorld.
   * @return PositionWorld&
   */
  PositionWorld& operator=(const PositionWorld& k);

  /*! \~Chinese
   * @brief 等号运算符重载, 用于比较两个 PositionWorld 对象是否相等.
   *
   * @param[in] other another PositionWorld.
   * @return bool
   */
  bool operator==(const PositionWorld& other) const;

  /*! \~Chinese
   * @brief 逻辑非运算符重载, 用于判断一个 PositionWorld 对象是否为空.
   *
   * @return bool
   */
  bool operator!() const;

  /*! \~Chinese
   * @brief 加运算符重载, 用于将一个 PositionWorld 对象的值加上另一个同类型对象的实例.
   *
   * @param[in] point
   * @return PositionWorld&
   */
  const PositionWorld operator+(const PositionWorld& point) const;

  /*! \~Chinese
   * @brief 减运算符重载, 用于将一个 PositionWorld 对象的值减去另一个同类型对象的实例.
   *
   * @param[in] point
   * @return PositionWorld&
   */
  const PositionWorld operator-(const PositionWorld& point) const;

  /*! \~Chinese
   * @brief 乘运算符重载, 用于将一个 PositionWorld 对象的值乘以一个实数.
   *
   * @param[in] d 实数
   * @return PositionWorld&
   */
  const PositionWorld operator*(const double d) const;

  /*! \~Chinese
   * @brief 除运算符重载, 用于将一个 PositionWorld 对象的值除以一个实数.
   *
   * @param[in] d 实数
   * @return PositionWorld&
   */
  const PositionWorld operator/(const double d) const;
};

}  // namespace hadmap
