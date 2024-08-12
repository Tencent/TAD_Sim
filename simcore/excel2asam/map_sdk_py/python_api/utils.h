// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines some utils.

#pragma once

#include <cmath>
#include <string>

/*! \~Chinese
 * @brief 判断绝对值是否小于一个常量`EPSILON`.
 *
 * @param[in] a
 * @return bool
 */
bool AlmostEqual(double a);

/*! \~Chinese
 * @brief 判断两个浮点数是否相等.
 *
 * @param[in] a
 * @param[in] b
 * @return bool
 */
bool AlmostEqual(double a, double b);

/*! \~Chinese
 * @brief 计算行驶方向.
 *
 * @param[in] delta
 * @return T/L/L0/R/R0
 */
std::string CalDirection(double delta);

/*! \~Chinese
 * @brief 给定浮点值保留指定小数位数.
 *
 * @param[in] value 浮点数值
 * @param[in] precision 小数点位数. (Default: 5)
 * @return 浮点数的四舍五入值
 */
double Round(double value, int precision = 5);
