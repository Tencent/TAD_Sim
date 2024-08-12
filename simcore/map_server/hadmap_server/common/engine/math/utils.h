/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
namespace txMath {
const float const_pi = 3.1415926;
const float const_pi_to_angle = 180;
}  // namespace txMath

class CUtils {
 public:
  // 经纬度和高度转直角坐标XYZ
  // 输入参数：待转换的经度、纬度和高度
  // 输出参数：x、y和z
  static void ll_to_meter(double& x, double& y, double& z);

  // by roger 2013-6-20
  // input WGS84 xyz -> LBH
  static void meter_to_ll(double& x, double& y, double& z);

  static bool is_zero(float f);

  static std::string getTime();

  static std::string doubleToStringDot3(double d);
};
