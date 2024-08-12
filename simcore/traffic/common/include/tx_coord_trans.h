// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_math.h"
#if __UseProj__
#include <proj_api.h>
#endif/*__UseProj__*/
#include <boost/format.hpp>
#include "structs/base_struct.h"

namespace Coord {
using txFloat = Base::txFloat;
#ifndef M_PI
constexpr txFloat M_PI = 3.141592653589;
#endif
constexpr txFloat M_PI_Inv = 1. / M_PI;
constexpr txFloat M_Half_PI = M_PI / 2.0;
constexpr txFloat roudu = 57.2957795130823210;
constexpr txFloat roudu_reciprocal = 1.0 / roudu;
constexpr txFloat roufen = 3437.74677078493917;
constexpr txFloat roumiao = 206264.8062470963551565;

constexpr txFloat WGS84_a = 6378137.0;
constexpr txFloat WGS84_e = 0.003352810664;
constexpr txFloat WGS84_b = WGS84_a - WGS84_a * WGS84_e;
constexpr txFloat WGS84_c = 6399593.6258;
constexpr txFloat WGS84_e2 = 0.0066943799013;
constexpr txFloat WGS84_epie2 = 0.00673949674227;

constexpr txFloat MERCATOR_LON_MAX = 180.0;
constexpr txFloat MERCATOR_LAT_MAX = 85.05112878;

constexpr txFloat METER_PER_DEGREE = WGS84_a * M_PI / 180.0;

/**
 * @brief 将两个3x3矩阵相乘，并将结果存储在Result中
 *
 * @param A 第一个3x3矩阵，用float数组表示
 * @param B 第二个3x3矩阵，用float数组表示
 * @param Result 存储乘积的3x3矩阵，用float数组表示
 */
inline void mult_matrix_333(const txFloat* A, const txFloat* B, txFloat* Result) TX_NOEXCEPT {
  Result[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
  Result[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
  Result[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];
  Result[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
  Result[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
  Result[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];
  Result[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
  Result[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
  Result[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];
}

/**
 * @brief 矩阵乘法，计算矩阵A与矩阵B的乘积，并将结果存储在矩阵Result中
 *
 * @param A 第一个3x3矩阵，用float数组表示
 * @param B 第二个3x3矩阵，用float数组表示
 * @param Result 存储乘积的3x3矩阵，用float数组表示
 */
inline void mult_matrix_331(const txFloat* A, const txFloat* B, txFloat* Result) TX_NOEXCEPT {
  Result[0] = A[0] * B[0] + A[1] * B[1] + A[2] * B[2];
  Result[1] = A[3] * B[0] + A[4] * B[1] + A[5] * B[2];
  Result[2] = A[6] * B[0] + A[7] * B[1] + A[8] * B[2];
}

/**
 * @brief 从坐标转换的矩阵 R2
 * 将传入的参数L, B, H作为用于zx坐标系到dx坐标系变换的增量值。
 * 计算出transformMatrix和s_reduce_matrix，并返回两个矩阵的乘积，以将zx坐标系转换为dx坐标系。
 *
 * @param L[in] 从zx坐标系到dx坐标系的旋转角度，精度为以度为单位的浮点数。
 * @param B[in] 从zx坐标系到dx坐标系的纵向位移距离，精度为以米为单位的浮点数。
 * @param H[in] 从zx坐标系到dx坐标系的规范化因子，精度为以米为单位的浮点数。
 * @param resultMatrix[out] 包含输出矩阵的二维float数组。
 */
inline void zx_to_dx_matrix(const txFloat L, const txFloat B, const txFloat H, txFloat* m) TX_NOEXCEPT {
  const txFloat newL = L * roudu_reciprocal;
  const txFloat half_pi_sub_newB = M_Half_PI - B * roudu_reciprocal;

  txFloat R1[9], R2[9];

  R1[0] = cos(half_pi_sub_newB);
  R1[1] = 0;
  R1[2] = sin(half_pi_sub_newB);
  R1[3] = 0;
  R1[4] = 1;
  R1[5] = 0;
  R1[6] = -R1[2];
  R1[7] = 0;
  R1[8] = R1[0];

  R2[0] = cos(newL);
  R2[1] = -sin(newL);
  R2[2] = 0;
  R2[3] = -R2[1];
  R2[4] = R2[0];
  R2[5] = 0;
  R2[6] = 0;
  R2[7] = 0;
  R2[8] = 1;

  mult_matrix_333(R2, R1, m);
}

/**
 * @brief dx_to_zx_matrix 计算从坐标转换的矩阵 R2
 * 根据输入的 L, B, H 参数，计算 zx 坐标系到 dx 坐标系的转换矩阵 R2。
 * 这个矩阵可以将 zx 坐标系的点坐标转换为 dx 坐标系的点坐标。
 *
 * @param L[in] 从 zx 坐标系到 dx 坐标系的旋转角度，精度为以度为单位的浮点数。
 * @param B[in] 从 zx 坐标系到 dx 坐标系的纵向位移距离，精度为以米为单位的浮点数。
 * @param H[in] 从 zx 坐标系到 dx 坐标系的规范化因子，精度为以米为单位的浮点数。
 * @param[out] m 输出一个二维浮点数组，用于存储矩阵 R2 的值。
 */
inline void dx_to_zx_matrix(txFloat L, txFloat B, txFloat H, txFloat* m) TX_NOEXCEPT {
  const txFloat newL = L * roudu_reciprocal;
  const txFloat half_pi_sub_newB = M_Half_PI - B * roudu_reciprocal;

  txFloat R1[9], R2[9];

  R2[0] = cos(newL);
  R2[1] = sin(newL);
  R2[2] = 0;
  R2[3] = -R2[1];
  R2[4] = R2[0];
  R2[5] = 0;
  R2[6] = 0;
  R2[7] = 0;
  R2[8] = 1;

  R1[0] = cos(half_pi_sub_newB);
  R1[1] = 0;
  R1[2] = -sin(half_pi_sub_newB);
  R1[3] = 0;
  R1[4] = 1;
  R1[5] = 0;
  R1[6] = -R1[2];
  R1[7] = 0;
  R1[8] = R1[0];

  mult_matrix_333(R1, R2, m);
}

/**
 * @brief 将经纬度坐标转换为地图坐标
 *
 * 根据给定的经纬度坐标（x, y, z）和高度参数（h），将经纬度坐标转换为地图坐标。
 *
 * @param x      经度坐标，单位为度
 * @param y      纬度坐标，单位为度
 * @param z      高度参数，单位为米
 * @param[out] y 地图坐标，单位为米
 */
inline void lonlat2global(txFloat& x, txFloat& y, txFloat& z) TX_NOEXCEPT {
  const txFloat lon = x;
  const txFloat lat = y;
  const txFloat h = z;
  const txFloat phi = lat * roudu_reciprocal;
  const txFloat lam = lon * roudu_reciprocal;
  const txFloat sphi = sin(phi);
  const txFloat cphi = fabs(lat) == 90 ? 0 : cos(phi);
  const txFloat n = WGS84_a / sqrt(1 - WGS84_e2 * sphi * sphi);
  const txFloat slam = lon == -180 ? 0 : sin(lam);
  const txFloat clam = fabs(lon) == 90 ? 0 : cos(lam);

  z = ((1 - WGS84_e2) * n + h) * sphi;
  x = (n + h) * cphi;
  y = x * slam;
  x *= clam;
}

/**
 * @brief global2lonlat
 *
 * 将输入的地球表面的坐标（x, y, z）转换为经纬度坐标（x, y）
 *
 * @param[in] x   坐标x值，已知位于地球表面
 * @param[in] y   坐标y值，已知位于地球表面
 * @param[in] z   坐标z值，已知位于地球表面，为高度值
 * @param[out] x  经度坐标，范围：[-180, 180]
 * @param[out] y  纬度坐标，范围：[-90, 90]
 */
inline void global2lonlat(txFloat& x, txFloat& y, txFloat& z) TX_NOEXCEPT {
  const txFloat sqrt_x2_y2 = sqrt(x * x + y * y);
  const txFloat L = atan2(y, x);

  txFloat B0 = atan2(z * (1 + WGS84_e2), sqrt_x2_y2);

  const txFloat Th = 1E-10;  // condition to finish the iteration
  txFloat dDiff = 1;
  const txFloat tmpTan = z / sqrt_x2_y2;
  while (dDiff > Th) {
    const txFloat tmp = WGS84_a * WGS84_e2 * sin(B0) / (z * sqrt(1 - WGS84_e2 * sin(B0) * sin(B0)));
    const txFloat B = atan(tmpTan * (1 + tmp));
    dDiff = fabs(B - B0);
    B0 = B;
  }
  const txFloat W = sqrt(1 - WGS84_e2 * sin(B0) * sin(B0));
  const txFloat N = WGS84_a / W;

  const txFloat dis = sqrt_x2_y2 / cos(B0);
  z = dis - N;
  x = L * 180. * M_PI_Inv;
  y = B0 * 180. * M_PI_Inv;
}

class Trans {
 public:
  using txFloat = Base::txFloat;
  Trans() TX_DEFAULT;
  virtual ~Trans() TX_DEFAULT;

 public:
  /**
   * @brief 初始化txCoordTrans类
   *
   * 使用经度、纬度和高度信息初始化类。请注意，这个函数只需要在构造函数时调用一次。
   *
   * @param L 经度
   * @param B 纬度
   * @param H 高度
   */
  static void initialize(const txFloat L, const txFloat B, const txFloat H) TX_NOEXCEPT;

  /**
   * @brief 获取经度
   *
   * @return const txFloat
   */
  static const txFloat Lon() TX_NOEXCEPT { return sLon; }

  /**
   * @brief 获取纬度
   *
   * @return const txFloat
   */
  static const txFloat Lat() TX_NOEXCEPT { return sLat; }

  /**
   * @brief 获取高度
   *
   * @return const txFloat
   */
  static const txFloat Alt() TX_NOEXCEPT { return sAlt; }

  /**
   * @brief 获取全局坐标到局部坐标的矩阵
   *
   * 返回一个指向全局坐标到局部坐标的转换矩阵的指针。该矩阵允许将坐标从全局坐标系转换为局部坐标系。
   *
   * @return 指向全局坐标到局部坐标的转换矩阵的指针
   */
  static const txFloat* Mat_R_global2local() TX_NOEXCEPT { return &R_global2local[0]; }

  /**
   * @brief 获取局部坐标到全局坐标的矩阵
   *
   * 返回一个指向局部坐标到全局坐标的转换矩阵的指针。该矩阵允许将坐标从局部坐标系转换为全局坐标系。
   *
   * @return 指向局部坐标到全局坐标的转换矩阵的指针
   */
  static const txFloat* Mat_R_local2global() TX_NOEXCEPT { return &R_local2global[0]; }

 protected:
  static txFloat sLon;
  static txFloat sLat;
  static txFloat sAlt;
  static txFloat R_global2local[9];
  static txFloat R_local2global[9];
};

/**
 * @brief 将全球坐标转换为局部坐标
 *
 * 根据传入的全球坐标 x, y, z，以及矩阵 Mat_R_global2local() 将坐标转换为相对于机器人坐标系的局部坐标 x', y', z'.
 *
 * @param x 全球坐标的 x 分量
 * @param y 全球坐标的 y 分量
 * @param z 全球坐标的 z 分量
 * @param R 用于转换的矩阵 Mat_R_global2local()
 * @return void
 *
 */
inline void global2local(txFloat& x, txFloat& y, txFloat& z, txFloat, txFloat, txFloat) TX_NOEXCEPT {
  const txFloat* R = Trans::Mat_R_global2local();

  txFloat X[3], rt[3];
  X[0] = x - Trans::Lon();
  X[1] = y - Trans::Lat();
  X[2] = z - Trans::Alt();

  mult_matrix_331(R, X, rt);

  x = rt[0];
  y = rt[1];
  z = rt[2];
}

/**
 * @brief 将局部坐标转换为全球坐标
 *
 * 根据传入的局部坐标 (x, y, z)，以及矩阵 Mat_R_local2global()，将坐标转换为全球坐标 (x', y', z')。
 *
 * @param x 局部坐标的 x 分量
 * @param y 局部坐标的 y 分量
 * @param z 局部坐标的 z 分量
 * @param R 用于转换的矩阵 Mat_R_local2global()
 * @return 无返回值
 */
inline void local2global(txFloat& x, txFloat& y, txFloat& z, txFloat, txFloat, txFloat) TX_NOEXCEPT {
  const txFloat* R = Trans::Mat_R_local2global();

  txFloat X[3], rt[3];

  X[0] = x;
  X[1] = y;
  X[2] = z;

  mult_matrix_331(R, X, rt);

  x = rt[0] + Trans::Lon();
  y = rt[1] + Trans::Lat();
  z = rt[2] + Trans::Alt();
}

/**
 * @brief 将经纬度坐标转换为相对于局部坐标系的坐标
 *
 * @param x 经度坐标
 * @param y 纬度坐标
 * @param z 高度坐标
 * @param refLon 参考经度坐标
 * @param refLat 参考纬度坐标
 * @param refAlt 参考高度坐标
 */
inline void lonlat2local(txFloat& x, txFloat& y, txFloat& z, txFloat refLon, txFloat refLat,
                         txFloat refAlt) TX_NOEXCEPT {
  lonlat2global(x, y, z);
  global2local(x, y, z, refLon, refLat, refAlt);
}

/**
 * @brief 将局部坐标转换为经纬度坐标
 *
 * @param lon 经度坐标
 * @param lat 纬度坐标
 * @param ele 高度坐标
 * @param refLon 参考经度坐标
 * @param refLat 参考纬度坐标
 * @param refAlt 参考高度坐标
 */
inline void local2lonlat(txFloat& lon, txFloat& lat, txFloat& ele, txFloat refLon, txFloat refLat,
                         txFloat refAlt) TX_NOEXCEPT {
  local2global(lon, lat, ele, refLon, refLat, refAlt);

  global2lonlat(lon, lat, ele);
}

/**
 * @brief 将给定的坐标从Seu坐标系（椭球坐标系）转换为ENU坐标系
 *
 * @param enuX 输出的ENU坐标系的X轴坐标
 * @param enuY 输出的ENU坐标系的Y轴坐标
 * @param enuZ 输出的ENU坐标系的Z轴坐标
 * @param seuX 输入的Seu坐标系的X轴坐标
 * @param seuY 输入的Seu坐标系的Y轴坐标
 * @param seuZ 输入的Seu坐标系的Z轴坐标
 */
inline void seu2enu(txFloat& enuX, txFloat& enuY, txFloat& enuZ, txFloat seuX, txFloat seuY, txFloat seuZ) TX_NOEXCEPT {
  enuX = seuY;
  enuY = -seuX;
  enuZ = seuZ;
}

/**
 * @brief 将给定的ENU坐标系坐标转换为Seu坐标系坐标
 *
 * @param[out] seuX 输出的Seu坐标系的X轴坐标
 * @param[out] seuY 输出的Seu坐标系的Y轴坐标
 * @param[out] seuZ 输出的Seu坐标系的Z轴坐标
 * @param[in] enuX 输入的ENU坐标系的X轴坐标
 * @param[in] enuY 输入的ENU坐标系的Y轴坐标
 * @param[in] enuZ 输入的ENU坐标系的Z轴坐标
 */
inline void enu2seu(txFloat& seuX, txFloat& seuY, txFloat& seuZ, txFloat enuX, txFloat enuY, txFloat enuZ) TX_NOEXCEPT {
  seuX = -enuY;
  seuY = enuX;
  seuZ = enuZ;
}

/**
 * @brief 将ENU坐标系的坐标转换为Longitude, Latitude坐标系的坐标
 *
 * @param[out] x 输出的经度坐标
 * @param[out] y 输出的纬度坐标
 * @param[in] x 输入的ENU坐标系的X轴坐标
 * @param[in] y 输入的ENU坐标系的Y轴坐标
 * @param[in] z 输入的ENU坐标系的Z轴坐标
 * @param[in] refLon 参考经度
 * @param[in] refLat 参考纬度
 * @param[in] refAlt 参考高度
 */
inline void enu2lonlat(txFloat& x, txFloat& y, txFloat& z, txFloat refLon, txFloat refLat, txFloat refAlt) TX_NOEXCEPT {
  enu2seu(x, y, z, x, y, z);

  local2lonlat(x, y, z, refLon, refLat, refAlt);
}

/**
 * @brief 将经纬度坐标系的坐标转换为ENU坐标系的坐标
 *
 * @param[out] x 输出的x坐标，表示东向量
 * @param[out] y 输出的y坐标，表示北向量
 * @param[out] z 输出的z坐标，表示上向量
 * @param[in] refLon 参考点的经度
 * @param[in] refLat 参考点的纬度
 * @param[in] refAlt 参考点的高度
 */
inline void lonlat2enu(txFloat& x, txFloat& y, txFloat& z, txFloat refLon, txFloat refLat, txFloat refAlt) TX_NOEXCEPT {
  lonlat2local(x, y, z, refLon, refLat, refAlt);

  seu2enu(x, y, z, x, y, z);
}

#if __UseProj__
struct UTM2WGS84Utility {
  const Base::txString m_wgs84 = "+proj=longlat +datum=WGS84 +no_defs";
  projPJ pj_utm;
  projPJ pj_wgs84;
  Base::txInt m_last_zone = 48;
  Base::txBool m_last_north = true;

  /**
   * @brief 创建UTM区域的坐标转换字符串
   * @param zone_num 区域编号
   * @param is_north 是否为北经
   * @return 转换字符串
   */
  Base::txString MakeUTMZone(const Base::txInt zone_num, const Base::txBool is_north) const TX_NOEXCEPT {
    if (is_north) {
      return (boost::format("+proj=utm +zone=%1% +datum=WGS84 +units=m +no_defs") % zone_num).str();
    } else {
      return (boost::format("+proj=utm +zone=%1% +datum=WGS84 +south +units=m +no_defs") % zone_num).str();
    }
  }

  /**
   * @brief 检查是否需要重置UTM坐标转换器
   * @param zone_num 区域编号
   * @param is_north 是否为北经
   * @return 如果需要重置则返回 true，否则返回 false
   */
  Base::txBool NeedResetUtm(const Base::txInt zone_num, const Base::txBool is_north) const TX_NOEXCEPT {
    return !((zone_num == m_last_zone) && (is_north == m_last_north));
  }

  /**
   * @brief 将经度值转换为UTM区域编号
   * @param lon 经度值，单位为度
   * @return UTM区域编号
   */
  Base::txInt Lon2Zone(const Base::txFloat lon) const TX_NOEXCEPT { return ((Base::txInt)((lon + 186.0) / 6.0)); }

  /**
   * @brief 检查给定经度是否在北方
   * @param lat 经度值，单位为度
   * @return 若经度在北方，则返回 true，否则返回 false
   */
  Base::txBool IsNorthEarth(const Base::txFloat lat) const TX_NOEXCEPT { return ((lat > 0) ? true : false); }

  Base::txBool Initialize(const Base::txInt utm_zone, const Base::txBool is_north) TX_NOEXCEPT;

  /**
   * @brief 将UTM坐标转换为经纬度坐标
   * @param x UTM坐标x
   * @param y UTM坐标y
   * @return 返回经纬度坐标，包含经度和纬度
   */
  hadmap::txPoint UTM2LonLat(const Base::txFloat x, const Base::txFloat y) TX_NOEXCEPT;

  /**
   * @brief 将经纬度坐标转换为UTM坐标
   * @param lon 经度值，单位为度
   * @param lat 纬度值，单位为度
   * @return 返回一个包含UTM坐标的三维向量
   */
  Base::txVec3 LonLat2UTM(const Base::txFloat lon, const Base::txFloat lat) TX_NOEXCEPT;
};
#endif/*__UseProj__*/

struct WGS84_GCJ02 {
  // /////
  const double x_pi = 3.14159265358979324 * 3000.0 / 180.0;
  // π
  const double pi = 3.1415926535897932384626;
  // 长半轴
  const double a = 6378245.0;
  // 扁率
  const double ee = 0.00669342162296594323;

  /**
   * 纬度转换
   *
   * @param lng
   * @param lat
   * @return
   */
  double transformlat(double lng, double lat) {
    double ret = -100.0 + 2.0 * lng + 3.0 * lat + 0.2 * lat * lat + 0.1 * lng * lat + 0.2 * sqrt(abs(lng));
    ret += (20.0 * sin(6.0 * lng * pi) + 20.0 * sin(2.0 * lng * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(lat * pi) + 40.0 * sin(lat / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(lat / 12.0 * pi) + 320 * sin(lat * pi / 30.0)) * 2.0 / 3.0;
    return ret;
  }

  /**
   * 经度转换
   *
   * @param lng
   * @param lat
   * @return
   */
  double transformlng(double lng, double lat) {
    double ret = 300.0 + lng + 2.0 * lat + 0.1 * lng * lng + 0.1 * lng * lat + 0.1 * sqrt(abs(lng));
    ret += (20.0 * sin(6.0 * lng * pi) + 20.0 * sin(2.0 * lng * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(lng * pi) + 40.0 * sin(lng / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(lng / 12.0 * pi) + 300.0 * sin(lng / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
  }

  /**
   * @brief 超出china经纬度范围
   *
   * @param lng
   * @param lat
   * @return
   */
  bool out_of_china(double lng, double lat) {
    if (lng < 72.004 || lng > 137.8347) {
      return true;
    } else if (lat < 0.8293 || lat > 55.8271) {
      return true;
    }
    return false;
  }

  /**
   * @brief WGS84坐标转换为GCJ02坐标
   * @param p 输入的WGS84坐标点
   * @return 返回转换后的GCJ02坐标点
   */
  bool WGS84ToGCJ02(hadmap::txPoint& p) {
    if (out_of_china(p.x, p.y)) {
      return false;
    }
    double dlat = transformlat(p.x - 105.0, p.y - 35.0);
    double dlng = transformlng(p.x - 105.0, p.y - 35.0);
    double radlat = p.y / 180.0 * pi;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * pi);
    dlng = (dlng * 180.0) / (a / sqrtmagic * cos(radlat) * pi);
    double mglat = p.y + dlat;
    double mglng = p.x + dlng;
    p.x = mglng;
    p.y = mglat;
    return true;
  }

  /**
   * @brief GCJ02坐标转换为WGS84坐标
   * @param p 输入的GCJ02坐标点
   * @return 返回转换后的WGS84坐标点
   */
  bool GCJ02ToWGS84(hadmap::txPoint& p) {
    if (out_of_china(p.x, p.y)) {
      return false;
    }
    double dlat = transformlat(p.x - 105.0, p.y - 35.0);
    double dlng = transformlng(p.x - 105.0, p.y - 35.0);
    double radlat = p.y / 180.0 * pi;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * pi);
    dlng = (dlng * 180.0) / (a / sqrtmagic * cos(radlat) * pi);
    double mglat = p.y + dlat;
    double mglng = p.x + dlng;
    p.x = p.x * 2 - mglng;
    p.y = p.y * 2 - mglat;
    return true;
  }
};

}  // namespace Coord
