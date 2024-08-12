// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <algorithm>
#include <boost/algorithm/clamp.hpp>
#include <boost/math/constants/constants.hpp>
#include <iostream>
#include <limits>
#include <random>
#include "fastrange.h"
#include "tx_header.h"
TX_NAMESPACE_OPEN(Math)

TX_MARK("Common constants.")

#if UseDouble
static constexpr Base::txFloat EPSILON = 1e-6;
#else
static constexpr Base::txFloat EPSILON = 1e-3;
#endif
static constexpr Base::txFloat PI = boost::math::constants::pi<Base::txFloat>();
static constexpr Base::txFloat TWO_PI = 2.0 * PI;
static constexpr Base::txFloat HALF_PI = 0.5 * PI;
static constexpr Base::txFloat QUARTER_PI = 0.25 * PI;
static constexpr Base::txFloat OneEighth_PI = 0.125 * PI;
static constexpr Base::txFloat INV_PI = 1.0 / PI;
static constexpr Base::txFloat INV_TWO_PI = 1.0 / TWO_PI;
static constexpr Base::txFloat DEG_TO_RAD = PI / 180.0;
static constexpr Base::txFloat RAD_TO_DEG = 180.0 / PI;

/**
 * @brief 计算给定数值的指数
 *
 * 根据以下公式计算给定数值的指数：
 * value^count
 *
 * @param value 要计算指数的基数值
 * @param count 要计算的指数次数
 * @return txFloat 返回计算结果
 */
Base::txFloat Power(const Base::txFloat _value, const Base::txInt count) TX_NOEXCEPT;

/**
 * @brief 将角度转换为弧度的函数
 *
 * 根据已知的公式将角度值转换为弧度值。
 * 公式：角度 * π / 180
 *
 * @param angleDegrees 输入角度（以度为单位）
 * @return Base::txFloat 返回弧度值
 */
inline Base::txFloat Degrees2Radians(Base::txFloat const angleDegrees) TX_NOEXCEPT {
  return (angleDegrees * PI / 180.0);
}

/**
 * @brief 将弧度转换为角度的函数
 *
 * 根据已知的公式将弧度值转换为角度值。
 * 公式：弧度 * 180/ π
 *
 * @param angleDegrees 输入角度（以弧度为单位）
 * @return Base::txFloat 返回角度值
 */
inline Base::txFloat Radians2Degrees(Base::txFloat const angleRadians) TX_NOEXCEPT {
  return (angleRadians * 180.0 / PI);
}

template <typename T>
T Clamp(const T& n, const T& lower, const T& upper) {
  return boost::algorithm::clamp(n, lower, upper);
}

/**
 * @brief ==0判断
 *
 * @param val 值
 * @param _eps 精度
 * @return Base::txBool
 */
Base::txBool isZero(const Base::txFloat val, const Base::txFloat _eps = EPSILON) TX_NOEXCEPT;

/**
 * @brief 非0判断
 *
 * @param val 值
 * @return Base::txBool
 */
Base::txBool isNotZero(const Base::txFloat val) TX_NOEXCEPT;

/**
 * @brief 浮点数等于判断
 *
 * @param lval 左值
 * @param rval 右值
 * @return Base::txBool
 */
inline Base::txBool isEqual(Base::txFloat lval, Base::txFloat rval) TX_NOEXCEPT { return isZero(lval - rval); }

/**
 * @brief 浮点数不等于判断
 *
 * @param lval
 * @param rval
 * @return Base::txBool
 */
inline Base::txBool isNotEqual(Base::txFloat lval, Base::txFloat rval) TX_NOEXCEPT { return !isEqual(lval, rval); }

/**
 * @brief 对给定值归一化到给定区间
 *
 * 将指定的值归一化到指定区间内，输出值占区间宽度的比例占比。例如，如果区间为 [0, 10]，输入值为 5.5，则输出值为 0.5。
 *
 * @param value 要归一化的值
 * @param start 归一化区间的起始值
 * @param end 归一化区间的结束值
 * @return Base::txFloat 归一化后的值
 */
inline Base::txFloat IntervalNormalise(Base::txFloat const value, Base::txFloat const start,
                                       Base::txFloat const end) TX_NOEXCEPT {
  Base::txFloat const width = end - start;
  Base::txFloat const offsetValue = value - start;
  return (offsetValue - (std::floor(offsetValue / width) * width)) + start;
}

/**
 * @brief 计算两个向量的夹角，以弧度为单位
 *
 * 计算两个三维向量(v0, v1)之间的夹角，假设向量已经归一化，返回值在范围[0, PI]之间。
 *
 * @param v0 向量v0
 * @param v1 向量v1
 * @return Base::txFloat 返回夹角，范围为[0, PI]，单位为弧度
 */
inline Base::txFloat VectorRadian3D(Base::txVec3 v0, Base::txVec3 v1) TX_NOEXCEPT {
  v0.normalize();
  v1.normalize();
  Base::txFloat const dot = Clamp(v0.dot(v1), -1.0, 1.0);

  return std::acos(static_cast<float>(dot));
  TX_MARK("the range [0 , PI], is returned");
}

/**
 * @brief 计算三维向量间的角度，并以度为单位返回结果。
 *
 * 该函数计算的是由v0指向v1的角度。
 * 输入向量必须是已经归一化的向量，否则结果将不可预测。
 *
 * @param v0 第一个三维向量
 * @param v1 第二个三维向量
 * @return Base::txFloat 返回角度值，范围在[0, 180]之间，单位为度。
 */
inline Base::txFloat VectorDegree3D(Base::txVec3 v0, Base::txVec3 v1) TX_NOEXCEPT {
  return Radians2Degrees(VectorRadian3D(v0, v1));
}

/**
 * @brief 计算两个向量的夹角 (以弧度表示)
 *
 * @param v0 第一个向量
 * @param v1 第二个向量
 *
 * @return 返回两个向量之间的夹角，范围为 [0, PI]
 *
 * @todo 请在您的代码中找到一个适当的位置调用此函数，并实现 clamp 函数
 */
inline Base::txFloat VectorRadian2D(Base::txVec2 v0, Base::txVec2 v1) TX_NOEXCEPT {
  v0.normalize();
  v1.normalize();
  const Base::txFloat dot = Clamp(v0.dot(v1), -1.0, 1.0);

  return std::acos(static_cast<float>(dot));
  TX_MARK("the range [0 , PI], is returned");
}

/**
 * @brief 计算两个向量的夹角 (以度为单位)
 *
 * @param v0 第一个向量
 * @param v1 第二个向量
 *
 * @return 返回两个向量之间的夹角，范围为 [0, 180]
 */
inline Base::txFloat VectorDegree2D(Base::txVec2 v0, Base::txVec2 v1) TX_NOEXCEPT {
  return Radians2Degrees(VectorRadian2D(v0, v1));
}

/**
 * @brief 固定角度，确保它在0~360度之间
 *
 * @param _angle 要固定的角度
 *
 * @return 固定后的角度
 */
inline Base::txFloat FixAngle(const Base::txFloat _angle) TX_NOEXCEPT {
  Base::txFloat angle = _angle;
  if (angle > 360) {
    angle -= 360;
  } else if (angle < 0) {
    angle += 360;
  }
  return angle;
}

/**
 * @brief 函数用于对传入的值进行范围限制
 *
 * @param lower 下限值
 * @param upper 上限值
 * @param val 传入的要进行范围限制的值
 * @return Base::txFloat 返回限制后的值
 */
inline Base::txFloat ScopeContraint(const Base::txFloat lower, const Base::txFloat upper,
                                    const Base::txFloat val) TX_NOEXCEPT {
  return ((val < lower) ? (lower) : (((val > upper) ? (upper) : (val))));
}

/*inline Base::txFloat AngleBetweenVectors(Base::txVec3& const Vector1, Base::txVec3& const  Vector2) {
    return Math::VectorAngle3D(Vector1, Vector2);
}*/

/**
 * @brief 函数用于将一个角度值映射到指定范围内
 *
 * @param _srcDegree 要映射的角度值
 * @param _start 指定的范围起始值, 默认为0.0
 * @param _end 指定的范围结束值, 默认为360.0
 * @return Base::txFloat 返回映射后的角度值
 */
inline Base::txFloat DegreeNormalise(Base::txFloat const _srcDegree, Base::txFloat const _start = 0.0,
                                     Base::txFloat const _end = 360.0) TX_NOEXCEPT {
  TX_MARK("Mapping Degree in Range(0.0, 360.0)");
  return IntervalNormalise(_srcDegree, _start, _end);
}

/**
 * @brief 将一个角度值映射到指定范围内
 *
 * @param _objRadian 要映射的角度值，以弧度为单位
 * @param _start 指定的范围起始值，默认为0.0，以弧度为单位
 * @param _end 指定的范围结束值，默认为2*PI，以弧度为单位
 * @return Base::txFloat 返回映射后的角度值，以弧度为单位
 */
inline Base::txFloat RadianNormalise(Base::txFloat const _objRadian, Base::txFloat const _start = 0.0,
                                     Base::txFloat const _end = TWO_PI) TX_NOEXCEPT {
  TX_MARK("Mapping Radian in Range(0.0, 2*PI)");
  return IntervalNormalise(_objRadian, _start, _end);
}

/**
 * @brief 检查两个向量是否具有相同方向
 *
 * @param v1 向量1
 * @param v2 向量2
 * @return txBool 如果两个向量具有相同方向则返回 true, 否则返回 false
 */
inline Base::txBool IsSynchronicity(const Base::txVec2& v1, const Base::txVec2& v2) TX_NOEXCEPT {
  return (v1.dot(v2) >= 0.0);
}

Base::txFloat ProjectPoint2Line(const Base::txVec2& p, const Base::txVec2& start, const Base::txVec2& end,
                                Base::txVec2& projectP) TX_NOEXCEPT;

/**
 * @brief 检查浮点数是否为整数
 *
 * @param _f 要检查的浮点数
 * @return Base::txBool 如果浮点数是整数则返回 true，否则返回 false
 */
inline Base::txBool IsInteger(const Base::txFloat _f) TX_NOEXCEPT { return isEqual(static_cast<int>(_f), _f); }

/**
 * @brief 返回输入浮点数的反正切值。
 *
 * @param _y 输入的浮点数 _y
 * @param _x 输入的浮点数 _x
 * @return 返回一个浮点数，表示 _y 和 _x 的反正切值
 *
 * 根据输入的浮点数 _y 和 _x，计算反正切值，并将结果作为一个浮点数返回。
 */
inline Base::txFloat Atan2(const Base::txFloat _y, const Base::txFloat _x) TX_NOEXCEPT {
  if (0.0 == _x) {
    if (_y > 0.0) {
      return HALF_PI;
    } else if (0.0 == _y) {
      return 0.0;
    } else {
      return -1.0 * HALF_PI;
    }
  }

  Base::txFloat atan = 0.0;
  Base::txFloat _z = _y / _x;
  if (std::fabs(_z) < 1.0) {
    atan = _z / (1.0 + 0.28 * _z * _z);
    if (_x < 0.0) {
      if (_y < 0.0) {
        return atan - PI;
      } else {
        return atan + PI;
      }
    }
  } else {
    atan = HALF_PI - _z / (_z * _z + 0.28);
    if (_y < 0.0) {
      return atan - PI;
    }
  }
  return atan;
}

/**
 * @brief 计算两条线段是否相交，如果相交则返回相交点。
 *
 * @param A 线段 1 的起点
 * @param B 线段 1 的终点
 * @param C 线段 2 的起点
 * @param D 线段 2 的终点
 * @param intersectionPt 用于存储相交点的坐标
 * @return true 如果两条线段相交，false 如果两条线段不相交
 *
 * 该函数用于计算两条线段是否相交，如果相交则返回相交点的坐标。
 */
inline Base::txBool intersection_between_2d_line(const Base::txVec2& A, const Base::txVec2& B, const Base::txVec2& C,
                                                 const Base::txVec2& D, Base::txVec2& intersectionPt) TX_NOEXCEPT {
#if 1
  Base::txFloat p0_x = A.x();
  Base::txFloat p0_y = A.y();

  Base::txFloat p1_x = B.x();
  Base::txFloat p1_y = B.y();

  Base::txFloat p2_x = C.x();
  Base::txFloat p2_y = C.y();

  Base::txFloat p3_x = D.x();
  Base::txFloat p3_y = D.y();

  Base::txFloat s1_x, s1_y, s2_x, s2_y;
  s1_x = p1_x - p0_x;
  s1_y = p1_y - p0_y;
  s2_x = p3_x - p2_x;
  s2_y = p3_y - p2_y;

  Base::txFloat s, t;
  s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
  t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

  if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
    intersectionPt.x() = p0_x + (t * s1_x);
    intersectionPt.y() = p0_y + (t * s1_y);
    return true;
  }

  return false;  // No collision
#endif
}

/**
 * @brief 在给定区间内生成一个随机浮点数。
 *
 * @param left_close 随机数的下限（包含）
 * @param right_open 随机数的上限（不包含）
 * @return 返回一个在[left_close, right_open)范围内的随机浮点数
 *
 * 使用均匀分布生成一个随机浮点数，其值落在[left_close, right_open)范围内。
 */
inline Base::txFloat RandomFloat(const Base::txFloat left_close, const Base::txFloat right_open) TX_NOEXCEPT {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  std::uniform_real_distribution<Base::txFloat> dist(left_close, right_open);
  return dist(mt);
}

/**
 * @brief 获取一个32位无符号整数某个范围内的值
 *
 * @param word 输入的32位无符号整数
 * @param p 范围的大小（步长）
 * @return 返回word在[0,p)范围内的值
 *
 * 此函数用于获取word的取模值，使其落在[0, p)范围内。
 */
static inline uint32_t RangeInt32(uint32_t word, uint32_t p) TX_NOEXCEPT {
  return (word % p);
  return fastrange32(word, p);
}

/**
 * @brief 返回两个浮点数中较大值
 *
 * @param lv 第一个浮点数
 * @param rv 第二个浮点数
 * @return Base::txFloat 返回两个浮点数中较大值
 *
 * 此函数用于计算两个浮点数中较大值，返回较大值。
 */
inline Base::txFloat Max(const Base::txFloat lv, const Base::txFloat rv) TX_NOEXCEPT {
  return ((lv < rv) ? (rv) : (lv));
}

/**
 * @brief 计算两个浮点数中较小的值
 *
 * @param lv 第一个浮点数
 * @param rv 第二个浮点数
 * @return Base::txFloat 返回两个浮点数中较小的值
 *
 * 此函数用于计算两个浮点数中较小的值，返回较小值。
 */
inline Base::txFloat Min(const Base::txFloat lv, const Base::txFloat rv) TX_NOEXCEPT {
  return ((lv < rv) ? (lv) : (rv));
}

/**
 * @brief 检查当前时间是否在给定时间范围内
 *
 * @param lTime   范围起始时间
 * @param rTime   范围结束时间
 * @param _curSceneTime  当前场景时间
 * @return Base::txBool 当前时间是否在给定时间范围内
 *
 * 此函数用于检查当前时间是否在给定时间范围内，如果在范围内则返回 true，否则返回 false。
 */
inline Base::txBool isInScope(const Base::txFloat lTime, const Base::txFloat rTime, const Base::txFloat _curSceneTime) {
  return (lTime <= _curSceneTime) && (_curSceneTime < rTime);
}

/**
 * @brief 计算两个向量之间的距离
 *
 * @param p 向量p
 * @param q 向量q
 * @return Base::txFloat 返回两个向量之间的距离
 *
 * 此函数用于计算两个向量之间的距离，返回值为浮点数。
 */
inline Base::txFloat Distance(const Base::txVec2& p, const Base::txVec2& q) TX_NOEXCEPT { return (p - q).norm(); }

/**
 * @brief 四舍五入值
 *
 * @param value 要进行四舍五入的值
 * @param prec 舍入精度
 * @return Base::txFloat 四舍五入后的值
 *
 * 这个函数用于将一个浮点数进行四舍五入操作，根据舍入精度参数进行舍入。
 */
inline Base::txFloat RoundOff(const Base::txFloat value, const Base::txFloat prec) TX_NOEXCEPT {
  return std::round(value * prec) / prec;
}

/**
 * @brief 返回一个数值的符号
 *
 * @param _v 输入的数值
 * @return Base::txFloat 如果 _v 大于等于 0，返回 1.0，否则返回 -1.0
 *
 * 此函数用于获取一个数值的符号，如果大于等于 0，则返回 1.0，否则返回 -1.0。
 */
inline Base::txFloat Sign(const Base::txFloat _v) TX_NOEXCEPT { return (_v >= 0.0) ? (1.0) : (-1.0); }

/**
 * @brief 对两个范围的线性插值运算
 *
 * @param t1 第一个范围的起始时间
 * @param t2 第一个范围的结束时间
 * @param step1 第一个范围对应的步长
 * @param step2 第二个范围对应的步长
 * @param t 当前时间
 * @return Base::txFloat 返回当前时间对应的步长值
 *
 * 此函数用于对两个范围内的线性插值运算，返回当前时间对应的步长值。
 */
inline Base::txFloat linear_interp(const Base::txFloat t1, const Base::txFloat t2, const Base::txFloat step1,
                                   const Base::txFloat step2, const Base::txFloat t) TX_NOEXCEPT {
  return step1 + (step2 - step1) * (t - t1) / (t2 - t1);
}
TX_NAMESPACE_CLOSE(Math)
