// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <Eigen/Geometry>
#include "tx_math.h"
#include "tx_sim_point.h"
#include "tx_units.h"
TX_NAMESPACE_OPEN(Geometry)

enum class CoordAxisType : Base::txInt { _X_, _Y_, _Z_ };

/**
 * @brief 返回指定轴的单位向量
 * @param _axis 坐标轴类型
 * @return 指定轴的单位向量
 */
inline Base::txVec3 Axis(const CoordAxisType _axis) TX_NOEXCEPT {
  switch (_axis) {
    case CoordAxisType::_X_:
      return Base::txAxis::UnitX();
    case CoordAxisType::_Y_:
      return Base::txAxis::UnitY();
    case CoordAxisType::_Z_:
      return Base::txAxis::UnitZ();
    default:
      break;
  }
  return Base::txAxis::UnitZ();
}

/**
 * @brief 根据给定的角度创建一个旋转矩阵
 * @param deg_x 绕 x 轴旋转的角度
 * @param deg_y 绕 y 轴旋转的角度
 * @param deg_z 绕 z 轴旋转的角度
 * @return 创建的旋转矩阵
 */
inline Base::txMat3 CreateRotationMatrix(const Unit::txDegree& deg_x, const Unit::txDegree& deg_y,
                                         const Unit::txDegree& deg_z) TX_NOEXCEPT {
  const Base::txAngleAxis rollAngle(deg_x.GetRadian(), Base::txAxis::UnitX());
  const Base::txAngleAxis pitchAngle(deg_y.GetRadian(), Base::txAxis::UnitY());
  const Base::txAngleAxis yawAngle(deg_z.GetRadian(), Base::txAxis::UnitZ());
  return (yawAngle * pitchAngle * rollAngle).matrix();
}

/**
 * @brief 根据给定的旋转角度和轴类型，创建一个旋转矩阵。
 * @param deg 旋转角度
 * @param _axis 旋转轴类型
 * @return 创建的旋转矩阵
 */
inline Base::txMat3 CreateRotationMatrix(const Unit::txDegree deg, const CoordAxisType _axis) TX_NOEXCEPT {
  return Base::txAngleAxis(deg.GetRadian(), Axis(_axis)).matrix();
  // return Base::txAffine(Base::txAngleAxis(deg.GetRadian(), Axis(_axis)));
}

/**
 * @brief 执行矩阵变换
 *
 * @param srcPt 待变换的点
 * @param deg 旋转角度
 * @param _t 位移量
 * @return 变换后的点
 */
inline Coord::txENU RigidTransformation(const Coord::txENU& srcPt, const Unit::txDegree& deg,
                                        const Base::txVec3& _t) TX_NOEXCEPT {
  return Coord::txENU(CreateRotationMatrix(deg, CoordAxisType::_Z_) * srcPt.ENU() + _t);
}

TX_NAMESPACE_CLOSE(Geometry)
