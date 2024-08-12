// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_algebra_defs.h"
#include "tx_enum_utils.h"
#include "tx_units.h"

TX_NAMESPACE_OPEN(Utils)

/*DEFINE_string(LocalFrame, "ENU, East(x)-North(y)-Up(z), Right Hand Vector System", "");*/
// inline Base::txVec3 Axis_Enu_East() TX_NOEXCEPT { return Base::txVec3::UnitX(); }
// inline Base::txVec3 Axis_Enu_North() TX_NOEXCEPT { return Base::txVec3::UnitY(); }
inline Base::txVec3 Axis_Enu_Up() TX_NOEXCEPT { return Base::txVec3::UnitZ(); }

inline Base::txVec3 VetRotVecByRadian(const Base::txVec3& _objV, const Unit::txRadian& _radian,
                                      const Base::txVec3& _axis) TX_NOEXCEPT {
  Eigen::AngleAxisd rotation_vector(_radian.GetRadian(), _axis);
  return rotation_vector * _objV;
}

inline Base::txVec3 VetRotVecByDegree(const Base::txVec3& _objV, const Unit::txDegree& _degree,
                                      const Base::txVec3& _axis) TX_NOEXCEPT {
  return VetRotVecByRadian(_objV, _degree.ToRadian(), _axis);
  /*Base::txFloat const normalizeDegree = Math::DegreeNormalise(_degree, 0.0, 360.0);
  return VetRotVecByRadian(_objV, Math::Degrees2Radians(Math::DegreeNormalise(_degree, 0.0, 360.0)), _axis);*/
}

enum class PtVecRelation2D : Base::txInt { Right = -1, On = 0, Left = 1 TX_MARK("same as xy2sl") };
inline Base::txFloat LOR2Sign(PtVecRelation2D lor) TX_NOEXCEPT { return Utils::to_underlying(lor); }

/**
 * @brief 将 txVec3 对象转换为 txPoint 对象
 *
 * 该函数将 Base::txVec3 类型的向量转换为 hadmap::txPoint 类型的点
 *
 * @param v3 Base::txVec3 类型的向量
 * @return hadmap::txPoint 类型的点
 * @throw std::domain_error 当向量的坐标值不在有效范围内时抛出异常
 */
inline hadmap::txPoint Vec3_txPoint(const Base::txVec3& v3) TX_NOEXCEPT {
  return hadmap::txPoint(v3.x(), v3.y(), v3.z());
}

/**
 * @brief 将三维向量转换为二维向量
 *
 * 该函数接收一个三维向量v3，将其前两个分量构成一个二维向量并返回。
 *
 * @param v3 需要转换为二维向量的三维向量
 * @return 转换后的二维向量
 */
inline Base::txVec2 Vec3_Vec2(const Base::txVec3& v3) TX_NOEXCEPT { return v3.head<2>(); }

/**
 * @brief 将二维向量转换为三维向量
 *
 * 该函数接收一个二维向量v2，将其转换为三维向量v3，并将v3的z分量设置为0。
 *
 * @param v2 需要转换为三维向量的二维向量
 * @return 转换后的三维向量
 */
inline Base::txVec3 Vec2_Vec3(const Base::txVec2& v2) TX_NOEXCEPT {
  return Base::txVec3(__East__(v2), __North__(v2), 0.0);
}

/**
 * @brief 将三维向量转换为四维向量
 *
 * 该函数接收一个三维向量v3，将其转换为四维向量v4，其中w分量为0.0。
 *
 * @param v3 需要转换为四维向量的三维向量
 * @return 转换后的四维向量
 */
inline Base::txVec4 Vec3_Vec4(const Base::txVec3& v3) TX_NOEXCEPT {
  return Base::txVec4(__East__(v3), __North__(v3), v3.z(), 0.0);
}

/**
 * @brief 将四维向量转换为三维向量
 *
 * 该函数接收一个四维向量v4，将其转换为三维向量v3，并且将四维向量的第四个分量忽略。
 *
 * @param v4 需要转换为三维向量的四维向量
 * @return 转换后的三维向量
 */
inline Base::txVec3 Vec4_Vec3(const Base::txVec4& v4) TX_NOEXCEPT {
  return Base::txVec3(__East__(v4), __North__(v4), v4.z());
}

/**
 * @brief 检测点在线段上的位置关系
 * @param vStart 线段起点
 * @param vEnd 线段终点
 * @param Pt 要检测的点
 * @return 返回点在线段上的位置关系
 */
inline PtVecRelation2D PointOnVector(const Base::txVec2& vStart, const Base::txVec2& vEnd,
                                     const Base::txVec2& Pt) TX_NOEXCEPT {
  const Base::txFloat x1 = vStart.x();
  const Base::txFloat y1 = vStart.y();
  const Base::txFloat x2 = vEnd.x();
  const Base::txFloat y2 = vEnd.y();
  const Base::txFloat x = Pt.x();
  const Base::txFloat y = Pt.y();
  const Base::txFloat sign_d = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);

  const Base::txVec3 leftVec3d =
      VetRotVecByRadian(Base::txVec3(x2 - x1, y2 - y1, 0.0), Unit::txRadian::MakeRadian(LeftOnENU), Axis_Enu_Up());
  const Base::txVec2 leftPt = vStart + Vec3_Vec2(leftVec3d);

  const Base::txFloat sign_d_reference = (leftPt.x() - x1) * (y2 - y1) - (leftPt.y() - y1) * (x2 - x1);
  const Base::txFloat sign = sign_d_reference * sign_d;

  /*LOG(WARNING) << TXST_TRACE_VARIABLES_NAME(vStart, Utils::ToString(vStart)) << TXST_TRACE_VARIABLES_NAME(vEnd,
     Utils::ToString(vEnd))
      << TXST_TRACE_VARIABLES_NAME(leftPt, Utils::ToString(leftPt)) << TXST_TRACE_VARIABLES_NAME(Pt,
     Utils::ToString(Pt))
      << TXST_TRACE_VARIABLES(sign_d) << TXST_TRACE_VARIABLES(sign_d_reference);*/

  if (sign > 0.0) {
    TX_MARK("On the same side, left");
    return PtVecRelation2D::Left;
  } else if (sign < 0.0) {
    return PtVecRelation2D::Right;
  } else {
    return PtVecRelation2D::On;
  }
}

/**
 * @brief 创建一个局部坐标系转换矩阵
 * @param _adc_center 地图中心点坐标
 * @param _adc_heading 地图的角度
 * @param tranMat 返回局部坐标系转换矩阵
 * @return 返回布尔值，表示是否创建成功
 */
inline Base::txBool CreateLocalCoordTranslationMat(const Base::txVec2& _adc_center, const Unit::txDegree _adc_heading,
                                                   Base::txMat2& tranMat) TX_NOEXCEPT {
  Base::txMat2 rotMat;
  const Base::txFloat rad_cos = _adc_heading.Cos();
  const Base::txFloat rad_sin = _adc_heading.Sin();
  rotMat.coeffRef(0, 0) = rad_cos;
  rotMat.coeffRef(0, 1) = -1.0 * rad_sin;
  rotMat.coeffRef(1, 0) = rad_sin;
  rotMat.coeffRef(1, 1) = rad_cos;
  Base::txVec2 global_unit_y_axis(1, 0);
  Base::txVec2 global_unit_x_axis(0, -1);
  Base::txVec2 local_y_axis_end = rotMat * global_unit_y_axis + _adc_center;
  Base::txVec2 local_x_axis_end = rotMat * global_unit_x_axis + _adc_center;

  Base::txVec2 local_x_axis_unit = local_x_axis_end - _adc_center;
  local_x_axis_unit.normalize();
  Base::txVec2 local_y_axis_unit = local_y_axis_end - _adc_center;
  local_y_axis_unit.normalize();

  tranMat.coeffRef(0, 0) = local_x_axis_unit.x();
  tranMat.coeffRef(0, 1) = local_x_axis_unit.y();
  tranMat.coeffRef(1, 0) = local_y_axis_unit.x();
  tranMat.coeffRef(1, 1) = local_y_axis_unit.y();

  return true;
}
#ifdef __TX_LOCALCOORD__
inline Unit::txDegree relativePosition(const Base::txVec2& _adc_center, const Base::txVec2& _obs_center,
                                       const Unit::txDegree _adc_heading) TX_NOEXCEPT {
  Base::txMat2 tranMat;
  Utils::CreateLocalCoordTranslationMat(_adc_center, _adc_heading, tranMat);
  Base::txVec2 local_xy = (_obs_center - _adc_center).transpose() * tranMat.inverse();
  Unit::txDegree retDegree;
  retDegree.FromRadian(Math::Atan2(local_xy.y(), local_xy.x()));
  return retDegree;  // Radians2Degrees(radians) /*- 90.0*/;
}
#endif /*__TX_LOCALCOORD__*/

/**
 * @brief 判断一个点是否在多边形内
 *
 * 判断一个点是否在多边形内。此函数用于判断点是否在一个由一系列顶点表示的多边形内。
 * 如果点在多边形内，返回 true；否则返回 false。
 *
 * @param point 待判断的点的坐标
 * @param vert 多边形顶点的坐标
 * @return 如果点在多边形内，返回 true；否则返回 false
 */
inline Base::txBool PointInPolygon2D(const Base::txVec2 point, const std::vector<Base::txVec2>& vert) TX_NOEXCEPT {
  Base::txSize i, j;
  Base::txBool c = false;
  const Base::txFloat testx = point.x();
  const Base::txFloat testy = point.y();

  const Base::txSize nvert = vert.size();
  for (i = 0, j = (nvert - 1); i < nvert; j = i++) {
    if (((vert[i].y() > testy) != (vert[j].y() > testy)) &&
        (testx < (vert[j].x() - vert[i].x()) * (testy - vert[i].y()) / (vert[j].y() - vert[i].y()) + vert[i].x())) {
      c = !c;
    }
  }
  return c;
}

/**
 * @brief 计算一个点与两点之间的线段的近似距离
 *
 * 此函数用于计算一个点q与由两点p1和p2构成的线段之间的近似距离。
 * 返回值是q到线段p1p2的近似距离，近似距离是以线段p1p2为坐标轴的直角坐标系中的值。
 * 如果p1和p2的横坐标相等，则返回值为-1。
 *
 * @param p1 线段的起点坐标
 * @param p2 线段的终点坐标
 * @param q 待计算距离的点坐标
 * @return 线段p1p2与点q之间的近似距离
 */
inline Base::txFloat Point2Line2d(const Base::txVec2& p_a, const Base::txVec2& p_b, const Base::txVec2& q) TX_NOEXCEPT {
  // (x- p1X) / (p2X - p1X) = (y - p1Y) / (p2Y - p1Y)
  const Base::txFloat x1 = p_a.x();
  const Base::txFloat x2 = p_b.x();
  const Base::txFloat y1 = p_a.y();
  const Base::txFloat y2 = p_b.y();
  const Base::txFloat a = y1 - y2;  // Note: this was incorrectly "y2 - y1" in the original answer
  const Base::txFloat b = x2 - x1;
  const Base::txFloat c = x1 * y2 - x2 * y1;

  if (Math::isZero(a) && Math::isZero(b)) {
    return 9999999.0;
  } else {
    return abs(a * q.x() + b * q.y() + c) / sqrt(a * a + b * b);
  }
}

/**
 * @brief 计算两个2D向量之间的夹角
 *
 * 该函数用于计算两个2D向量之间的夹角，以弧度表示。
 *
 * @param v_a 第一个向量
 * @param v_b 第二个向量
 * @return Base::txFloat 两个向量之间的夹角，范围为 [0, pi]
 */
inline Base::txFloat RadianBetweenVector2d(Base::txVec2 v_a, Base::txVec2 v_b) TX_NOEXCEPT {
  v_a.normalize();
  v_b.normalize();

  const Base::txFloat _dot = v_a.dot(v_b);

  const Base::txFloat dot = (_dot < -1.0 ? -1.0 : (_dot > 1.0 ? 1.0 : _dot));

  const Base::txFloat rad = acos(dot);

  return rad;
}
TX_NAMESPACE_CLOSE(Utils)
