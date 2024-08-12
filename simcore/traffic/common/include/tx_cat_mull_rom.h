// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#if __TX_Mark__("boost::catmull_rom required c++17")
#  include <algorithm>
#  include <boost/config.hpp>
#  include <cmath>
#  include <iterator>
#  include <vector>
#  include "tx_frame_utils.h"

TX_NAMESPACE_OPEN(Geometry)

/**
 * @brief alpha_distance 根据给定的 alpha 值计算点之间的反弦变换距离
 *
 * @tparam Point 点类型，应支持获取其维数和元素值的操作
 * @param p1 第一个点
 * @param p2 第二个点
 * @param alpha 反弦变换的幂次
 * @return 第一个点到第二个点的反弦变换距离
 */
template <class Point>
typename Point::value_type alpha_distance(Point const& p1, Point const& p2, typename Point::value_type alpha) {
  using std::pow;
  typename Point::value_type dsq = 0;
  for (size_t i = 0; i < p1.size(); ++i) {
    typename Point::value_type dx = p1[i] - p2[i];
    dsq += dx * dx;
  }
  return pow(dsq, alpha / 2);
}

template <class Point, class RandomAccessContainer = std::vector<Point> >
class catmull_rom {
  typedef typename Point::value_type value_type;

 public:
  catmull_rom() TX_DEFAULT;

  /**
   * @brief txCatMullRom 通过给定的点集生成catmull-rom曲线
   *
   * @tparam Point 需要的是一个支持随机访问迭代器的容器
   * @param points 输入的点集，确保点集中的每个点都是一个三维向量
   * @param closed 是否将结果视为闭合曲线
   * @param alpha 控制曲线的样式
   * @return 返回生成的catmull-rom曲线
   */
  explicit catmull_rom(RandomAccessContainer&& points, bool closed = false,
                       value_type alpha = (value_type)1 / (value_type)2);

  catmull_rom(std::initializer_list<Point> l, bool closed = false, value_type alpha = (value_type)1 / (value_type)2)
      : catmull_rom<Point, RandomAccessContainer>(RandomAccessContainer(l), closed, alpha) {}

  /**
   * @brief max_parameter 获取参数 s 的最大值
   * @return 返回 s 的最大值
   */
  value_type max_parameter() const { return m_max_s; }

  /**
   * @brief parameter_at_point 获取指定点 i 对应的参数 s
   * @param i 点的索引
   * @return 返回参数 s
   */
  value_type parameter_at_point(size_t i) const { return m_s[i + 1]; }

  Point operator()(const value_type s) const;

  /**
   * @brief prime 计算 catmull-rom 曲线在参数 s 处的导数
   * @param s 参数 s
   * @return 返回导数
   */
  Point prime(value_type s) const;

  /**
   * @brief get_points 获取 catmull-rom 曲线的点集
   * @return 返回点集
   */
  RandomAccessContainer&& get_points() { return std::move(m_pnts); }

  /**
   * @brief get_points_value 获取 catmull-rom 曲线的点集
   * @return 返回点集
   */
  const RandomAccessContainer& get_points_value() const { return m_pnts; }

 private:
  RandomAccessContainer m_pnts;
  std::vector<value_type> m_s;
  value_type m_max_s;
};

/**
 * @brief txCatMullRom 类的构造函数
 * 构建一个 Catmull-Rom 曲线对象，根据给定的点集合，以及关闭标志和参数 alpha。
 * @param points 包含类型 Point 的临时容器，存储曲线的点集合。
 * @param closed 表示曲线是否是闭合的，若为 true，则闭合；否则打开。
 * @param alpha 控制参数，表示曲线的插值方式，范围 [0,1]。
 * @throw std::domain_error 若输入的点集合中的点不足 4 个，或者 alpha 超出范围，抛出异常。
 */
template <class Point, class RandomAccessContainer>
catmull_rom<Point, RandomAccessContainer>::catmull_rom(RandomAccessContainer&& points, bool closed,
                                                       typename Point::value_type alpha)
    : m_pnts(std::move(points)) {
  std::size_t num_pnts = m_pnts.size();
  // std::cout << "Number of points = " << num_pnts << "\n";
  if (num_pnts < 4) {
    throw std::domain_error("The Catmull-Rom curve requires at least 4 points.");
  }
  if (alpha < 0 || alpha > 1) {
    throw std::domain_error("The parametrization alpha must be in the range [0,1].");
  }

  using std::abs;
  m_s.resize(num_pnts + 3);
  m_pnts.resize(num_pnts + 3);
  // std::cout << "Number of points now = " << m_pnts.size() << "\n";

  m_pnts[num_pnts + 1] = m_pnts[0];
  m_pnts[num_pnts + 2] = m_pnts[1];

  auto tmp = m_pnts[num_pnts - 1];
  for (std::ptrdiff_t i = num_pnts - 1; i >= 0; --i) {
    m_pnts[i + 1] = m_pnts[i];
  }
  m_pnts[0] = tmp;

  m_s[0] = -alpha_distance<Point>(m_pnts[0], m_pnts[1], alpha);
  if (fabs(m_s[0]) < std::numeric_limits<typename Point::value_type>::epsilon()) {
    throw std::domain_error("The first and last point should not be the same.\n");
  }
  m_s[1] = 0;
  for (size_t i = 2; i < m_s.size(); ++i) {
    typename Point::value_type d = alpha_distance<Point>(m_pnts[i], m_pnts[i - 1], alpha);
    if (fabs(d) < std::numeric_limits<typename Point::value_type>::epsilon()) {
      throw std::domain_error(
          "The control points of the Catmull-Rom curve are too close together; this will lead to ill-conditioning.\n");
    }
    m_s[i] = m_s[i - 1] + d;
  }
  if (closed) {
    m_max_s = m_s[num_pnts + 1];
  } else {
    m_max_s = m_s[num_pnts];
  }
}

/**
 * @brief 操作符重写
 *
 * @tparam Point 类型
 * @tparam RandomAccessContainer 类型
 * @param s Point类型值
 * @return Point
 */
template <class Point, class RandomAccessContainer>
Point catmull_rom<Point, RandomAccessContainer>::operator()(const typename Point::value_type s) const {
  if (s < 0 || s > m_max_s) {
    std::ostringstream oss;
    oss << "Parameter outside bounds." << TX_VARS(s) << TX_VARS(m_max_s);
    throw std::domain_error(oss.str());
  }
  auto it = std::upper_bound(m_s.begin(), m_s.end(), s);
  // Now *it >= s. We want the index such that m_s[i] <= s < m_s[i+1]:
  size_t i = std::distance(m_s.begin(), it - 1);

  // Only denom21 is used twice:
  typename Point::value_type denom21 = 1 / (m_s[i + 1] - m_s[i]);
  typename Point::value_type s0s = m_s[i - 1] - s;
  typename Point::value_type s1s = m_s[i] - s;
  typename Point::value_type s2s = m_s[i + 1] - s;
  typename Point::value_type s3s = m_s[i + 2] - s;

  Point A1_or_A3;
  typename Point::value_type denom = 1 / (m_s[i] - m_s[i - 1]);
  const size_t nSize = m_pnts[0].size();
  for (size_t j = 0; j < nSize; ++j) {
    A1_or_A3[j] = denom * (s1s * m_pnts[i - 1][j] - s0s * m_pnts[i][j]);
  }

  Point A2_or_B2;
  for (size_t j = 0; j < nSize; ++j) {
    A2_or_B2[j] = denom21 * (s2s * m_pnts[i][j] - s1s * m_pnts[i + 1][j]);
  }

  Point B1_or_C;
  denom = 1 / (m_s[i + 1] - m_s[i - 1]);
  for (size_t j = 0; j < nSize; ++j) {
    B1_or_C[j] = denom * (s2s * A1_or_A3[j] - s0s * A2_or_B2[j]);
  }

  denom = 1 / (m_s[i + 2] - m_s[i + 1]);
  for (size_t j = 0; j < nSize; ++j) {
    A1_or_A3[j] = denom * (s3s * m_pnts[i + 1][j] - s2s * m_pnts[i + 2][j]);
  }

  Point B2;
  denom = 1 / (m_s[i + 2] - m_s[i]);
  for (size_t j = 0; j < nSize; ++j) {
    B2[j] = denom * (s3s * A2_or_B2[j] - s1s * A1_or_A3[j]);
  }

  for (size_t j = 0; j < nSize; ++j) {
    B1_or_C[j] = denom21 * (s2s * B1_or_C[j] - s1s * B2[j]);
  }

  return B1_or_C;
}

template <class Point, class RandomAccessContainer>
Point catmull_rom<Point, RandomAccessContainer>::prime(typename Point::value_type s) const {
  /*if (s < 0 || s > m_max_s) {
      throw std::domain_error("Parameter outside bounds.\n");
  }*/
  if (s < 0) {
    s = 0;
  }
  if (s > m_max_s) {
    s = m_max_s - 0.1;
  }
  auto it = std::upper_bound(m_s.begin(), m_s.end(), s);
  // Now *it >= s. We want the index such that m_s[i] <= s < m_s[i+1]:
  size_t i = std::distance(m_s.begin(), it - 1);
  Point A1;
  typename Point::value_type denom = 1 / (m_s[i] - m_s[i - 1]);
  typename Point::value_type k1 = (m_s[i] - s) * denom;
  typename Point::value_type k2 = (s - m_s[i - 1]) * denom;
  const size_t nSize = m_pnts[0].size();
  for (size_t j = 0; j < nSize; ++j) {
    A1[j] = k1 * m_pnts[i - 1][j] + k2 * m_pnts[i][j];
  }

  Point A1p;
  for (size_t j = 0; j < nSize; ++j) {
    A1p[j] = denom * (m_pnts[i][j] - m_pnts[i - 1][j]);
  }

  Point A2;
  denom = 1 / (m_s[i + 1] - m_s[i]);
  k1 = (m_s[i + 1] - s) * denom;
  k2 = (s - m_s[i]) * denom;
  for (size_t j = 0; j < nSize; ++j) {
    A2[j] = k1 * m_pnts[i][j] + k2 * m_pnts[i + 1][j];
  }

  Point A2p;
  for (size_t j = 0; j < nSize; ++j) {
    A2p[j] = denom * (m_pnts[i + 1][j] - m_pnts[i][j]);
  }

  Point B1;
  for (size_t j = 0; j < nSize; ++j) {
    B1[j] = k1 * A1[j] + k2 * A2[j];
  }

  Point A3;
  denom = 1 / (m_s[i + 2] - m_s[i + 1]);
  k1 = (m_s[i + 2] - s) * denom;
  k2 = (s - m_s[i + 1]) * denom;
  for (size_t j = 0; j < nSize; ++j) {
    A3[j] = k1 * m_pnts[i + 1][j] + k2 * m_pnts[i + 2][j];
  }

  Point A3p;
  for (size_t j = 0; j < nSize; ++j) {
    A3p[j] = denom * (m_pnts[i + 2][j] - m_pnts[i + 1][j]);
  }

  Point B2;
  denom = 1 / (m_s[i + 2] - m_s[i]);
  k1 = (m_s[i + 2] - s) * denom;
  k2 = (s - m_s[i]) * denom;
  for (size_t j = 0; j < nSize; ++j) {
    B2[j] = k1 * A2[j] + k2 * A3[j];
  }

  Point B1p;
  denom = 1 / (m_s[i + 1] - m_s[i - 1]);
  for (size_t j = 0; j < nSize; ++j) {
    B1p[j] = denom * (A2[j] - A1[j] + (m_s[i + 1] - s) * A1p[j] + (s - m_s[i - 1]) * A2p[j]);
  }

  Point B2p;
  denom = 1 / (m_s[i + 2] - m_s[i]);
  for (size_t j = 0; j < nSize; ++j) {
    B2p[j] = denom * (A3[j] - A2[j] + (m_s[i + 2] - s) * A2p[j] + (s - m_s[i]) * A3p[j]);
  }

  Point Cp;
  denom = 1 / (m_s[i + 1] - m_s[i]);
  for (size_t j = 0; j < nSize; ++j) {
    Cp[j] = denom * (B2[j] - B1[j] + (m_s[i + 1] - s) * B1p[j] + (s - m_s[i]) * B2p[j]);
  }
  return Cp;
}

TX_NAMESPACE_CLOSE(Geometry)
#endif /*__TX_Mark__("boost::catmull_rom required c++17")*/
