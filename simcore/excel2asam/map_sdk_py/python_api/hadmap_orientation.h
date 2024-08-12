// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct Orientation.

#pragma once

namespace hadmap {
struct Orientation {
  double h = 0.0;
  double p = 0.0;
  double r = 0.0;

  /*! \~Chinese
   * @brief 默认构造函数.
   *
   */
  Orientation();

  /*! \~Chinese
   * @brief 参数化构造函数.
   *
   * @param[in] kh 航向角 heading angle. (Unit: rad)
   * @param[in] kp 俯仰角 pitch angle. (Unit: rad)
   * @param[in] kr 旋转角 roll angle. (Unit: rad)
   * @return Orientation
   */
  Orientation(double kh, double kp, double kr);

  /*! \~Chinese
   * \brief 拷贝构造函数.
   *
   * @param[in] other another Orientation.
   */
  Orientation(const Orientation& k);

  /*! \~Chinese
   * @brief 赋值运算符重载, 允许将一个 Orientation 对象的值赋给另一个同类型对象的实例.
   *
   * @param[in] k another Orientation.
   * @return Orientation&
   */
  Orientation& operator=(const Orientation& k);

  /*! \~Chinese
   * @brief 等号运算符重载, 用于比较两个 Orientation 对象是否相等.
   *
   * @param[in] other another Orientation.
   * @return bool
   */
  bool operator==(const Orientation& other) const;

  /*! \~Chinese
   * @brief 逻辑非运算符重载, 用于判断一个 Orientation 对象是否为空.
   *
   * @return bool
   */
  bool operator!() const;

  /*! \~Chinese
   * @brief 加运算符重载, 用于将一个 Orientation 对象的值加上另一个同类型对象的实例.
   *
   * @param[in] point
   * @return Orientation&
   */
  const Orientation operator+(const Orientation& point) const;

  /*! \~Chinese
   * @brief 减运算符重载, 用于将一个 Orientation 对象的值减去另一个同类型对象的实例.
   *
   * @param[in] point
   * @return Orientation&
   */
  const Orientation operator-(const Orientation& point) const;

  /*! \~Chinese
   * @brief 乘运算符重载, 用于将一个 Orientation 对象的值乘以一个实数.
   *
   * @param[in] d 实数
   * @return Orientation&
   */
  const Orientation operator*(const double d) const;

  /*! \~Chinese
   * @brief 除运算符重载, 用于将一个 Orientation 对象的值除以一个实数.
   *
   * @param[in] d 实数
   * @return Orientation&
   */
  const Orientation operator/(const double d) const;
};

}  // namespace hadmap
