// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <array>
#include "tx_algebra_defs.h"
#include "tx_header.h"
#include "tx_math.h"
TX_NAMESPACE_OPEN(Geometry)

class OBB2D {
 public:
  enum { PolygonSize = 4 };
  using PolygonArray = std::array<Base::txVec2, PolygonSize>;
  OBB2D() TX_DEFAULT;

  /**
   * @brief OBB2D构造函数
   *
   * 根据OBB椭圆体的中心点、正前向轴和正右侧轴，计算正前向轴和正右侧轴的半径长度，
   * 构造一个2D椭圆体OBB。
   *
   * @param _c                   OBB的中心点
   * @param _FrontAxis           正前向轴
   * @param _RightAxis            正右侧轴
   * @param _FrontHalfSize        正前向轴的半径长度
   * @param _RightHalfSize        正右侧轴的半径长度
   */
  OBB2D(const Base::txVec2& _c, const Base::txVec2& _FrontAxis, const Base::txVec2& _RightAxis,
        const Base::txFloat _FrontHalfSize, const Base::txFloat _RightHalfSize);

  /**
   * 获取由一个多边形数组表示的多边形，形成的多边形几何体。
   * 这些多边形组成的多边形几何体表示一个 OBB (对称轴投影椭圆)，它的轴对称。
   * @param {Polygon[]} polygonArray - 多边形数组
   * @return {OBB} 由多边形组成的多边形几何体
   */
  PolygonArray GetPolygon() const TX_NOEXCEPT;

  /**
   * 获取OBB的字符串表示形式
   * @return 由多个多边形组成的多边形几何体的字符串表示形式
   */
  Base::txString Str() const TX_NOEXCEPT;

 protected:
  Base::txVec2 center;
  Base::txVec2 Axis[2];                  /*0: front; 1: right*/
  std::array<Base::txFloat, 2> halfSize; /*0: front; 1: right*/
};

TX_NAMESPACE_CLOSE(Geometry)
