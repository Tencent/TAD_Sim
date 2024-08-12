// #include "stdafx.h"
// #include <algorithm>
// #include <math.h>
// #include "coordinate_transform.h"
// #include "matrix_operation.h"

// pi
// #ifndef M_PI
import matrix_operation from './matrix_operation.cpp.js'

var M_PI = 3.141592653589
// #endif
var roudu = 57.2957795130823210
var roufen = 3437.74677078493917
var roumiao = 206264.8062470963551565

// WGS84�������
var WGS84_a = 6378137.0
var WGS84_e = 0.003352810664
var WGS84_b = WGS84_a - WGS84_a * WGS84_e
var WGS84_c = 6399593.6258
var WGS84_e2 = 0.0066943799013
var WGS84_epie2 = 0.00673949674227

// ��/��
var meter_per_degree = WGS84_a * M_PI / 180.0

// MERCATORͶӰ��󾭡�γ��
var MERCATOR_LON_MAX = 180.0
var MERCATOR_LAT_MAX = 85.05112878

function coordinate_transform ()// CPP2JS
{
}

//
// coordinate_transform::~coordinate_transform(void)
// {
// }

// 计算中间过程值X和N
coordinate_transform.prototype.CalculateXN = function (B, sinB, cosB, cosB2, cosB4, cosB6, X, N)// CPP2JS
{
  var epie4 = WGS84_epie2 * WGS84_epie2
  var epie6 = WGS84_epie2 * epie4
  var epie8 = WGS84_epie2 * epie6

  var beta0, beta2, beta4, beta6, beta8
  beta0 = 1 - 3.0 / 4.0 * WGS84_epie2 + 45.0 / 64.0 * epie4 - 175.0 / 256.0 * epie6 + 11025.0 / 16384.0 * epie8
  beta2 = beta0 - 1.0
  beta4 = 15.0 / 32.0 * epie4 - 175.0 / 384.0 * epie6 + 3675.0 / 8192.0 * epie8
  beta6 = -35.0 / 96.0 * epie6 + 735.0 / 2048.0 * epie8
  beta8 = 315.0 / 1024.0 * epie8

  X = WGS84_c *
    (beta0 * B / roudu + (beta2 * cosB + beta4 * cosB2 * cosB + beta6 * cosB4 * cosB + beta8 * cosB6 * cosB) * sinB)

  var n0pie, n2pie, n4pie, n6pie, n8pie

  n0pie = WGS84_c
  n2pie = -1.0 / 2.0 * WGS84_epie2 * n0pie
  n4pie = -3.0 / 4.0 * WGS84_epie2 * n2pie
  n6pie = -5.0 / 6.0 * WGS84_epie2 * n4pie
  n8pie = -7.0 / 8.0 * WGS84_epie2 * n6pie

  N = n0pie + n2pie * cosB2 + n4pie * cosB4 + n6pie * cosB6 + n8pie * cosB2 * cosB6
  return [X, N]// js
}

// 经纬度转XY（高斯投影）
// 输入参数：指定投影带的中央经线，待转换的经度和纬度
// 输出参数：x和y
coordinate_transform.prototype.ll_to_xy = function (L0, lon, lat)// CPP2JS
{
  // 计算纬度相关值
  var Bdeg, cosB, sinB, cosB2, cosB4, cosB6
  Bdeg = lat
  sinB = Math.sin(Bdeg / roudu)
  cosB = Math.cos(Bdeg / roudu)
  cosB2 = cosB * cosB
  cosB4 = cosB2 * cosB2
  cosB6 = cosB2 * cosB4

  L0 = L0 * 3600.0

  var X, N
  this.CalculateXN(Bdeg, sinB, cosB, cosB2, cosB4, cosB6, X, N)
  var lsec, lrad, lrad2, lrad4, lrad6

  lsec = lon * 3600.0
  lrad = lsec / roumiao
  lrad2 = lrad * lrad
  lrad4 = lrad2 * lrad2
  lrad6 = lrad2 * lrad4

  var A2, A3, A4, A5, A6
  A2 = 1.0 / 2.0 * N * sinB * cosB * lrad2
  A4 = 1.0 / 12.0 * A2 *
    (-1.0 + 6.0 * cosB2 + 9.0 * WGS84_epie2 * cosB4 + 4 * WGS84_epie2 * WGS84_epie2 * cosB6) * lrad2
  A6 = 1.0 / 360.0 * A2 * (1.0 - 60.0 * cosB2 + 120.0 * cosB4) * lrad4
  A3 = (-1.0 / 6.0 + cosB2 / 3.0 + WGS84_epie2 * cosB4 / 6.0)
  A5 = (1.0 / 120.0 - cosB2 / 6.0 + (12.0 - 29.0 * WGS84_epie2) / 60.0 * cosB4 + 3.0 / 5.0 * WGS84_epie2 * cosB6)

  // lon和lat分别作为x和y值返回
  lon = X + A2 + A4 + A6
  lat = N * cosB * lrad * (1.0 + (A3 + A5 * lrad2) * lrad2)
  return [lon, lat]// js
}

// 范围函数
// template <typename T>
var clamp = function (x, low, high)// CPP2JS
{
  return Math.min(Math.max(x, low), high)
}

// 经纬度转直角坐标(墨卡托投影)
// 输入参数：待转换的经度和纬度
// 输出参数：x和y
coordinate_transform.prototype.ll_to_mec = function (x, y)// CPP2JS
{
  // latlon-->mercator coord
  // 两极地区变形太大，需特殊处理
  y = clamp(y, -MERCATOR_LAT_MAX, MERCATOR_LAT_MAX)
  x = clamp(x, -MERCATOR_LON_MAX, MERCATOR_LON_MAX)

  x = x * meter_per_degree
  y = Math.log(Math.tan((90.0 + y) * M_PI / 360.0)) * WGS84_a
  return [x, y]// js
}

// 直角坐标(墨卡托投影)转经纬度
// 输入参数：x和y
// 输出参数：待转换的经度和纬度
coordinate_transform.prototype.mec_to_ll = function (x, y)// CPP2JS
{
  x = x / meter_per_degree
  y = Math.atan(exp(y / WGS84_a)) * 360.0 / M_PI - 90.0
  return [x, y]// js
}

// 经纬度和高度转直角坐标XYZ
// 输入参数：待转换的经度、纬度和高度
// 输出参数：x、y和z
coordinate_transform.prototype.ll_to_meter = function (x, y, z)// CPP2JS
{
  var lon = x
  var lat = y
  var h = z
  var phi = lat / roudu
  var lam = lon / roudu
  var sphi = Math.sin(phi)
  var cphi = Math.abs(lat) == 90 ? 0 : Math.cos(phi)
  var n = WGS84_a / Math.sqrt(1 - WGS84_e2 * sphi * sphi)
  var slam = lon == -180 ? 0 : Math.sin(lam)
  var clam = Math.abs(lon) == 90 ? 0 : Math.cos(lam)

  z = ((1 - WGS84_e2) * n + h) * sphi
  x = (n + h) * cphi
  y = x * slam
  x *= clam
  return [x, y, z]// js
}

// by roger 2013-6-20
// input WGS84 xyz -> LBH
coordinate_transform.prototype.meter_to_ll = function (x, y, z)// CPP2JS
{
  var L = Math.atan2(y, x)

  var B0 = Math.atan2(z * (1 + WGS84_e2), Math.sqrt(x * x + y * y))

  var Th = 1E-10 // condition to finish the iteration
  var dDiff = 1
  var tmpTan = z / Math.sqrt(x * x + y * y)
  while (dDiff > Th) {
    var tmp = WGS84_a * WGS84_e2 * Math.sin(B0) / (z * Math.sqrt(1 - WGS84_e2 * Math.sin(B0) * Math.sin(B0)))
    var B = Math.atan(tmpTan * (1 + tmp))
    dDiff = Math.abs(B - B0)
    B0 = B
  }
  var W = Math.sqrt(1 - WGS84_e2 * Math.sin(B0) * Math.sin(B0))
  var N = WGS84_a / W

  var dis = Math.sqrt(x * x + y * y) / Math.cos(B0)
  z = dis - N
  x = L * 180.0 / M_PI
  y = B0 * 180.0 / M_PI
  return [x, y, z]// js
}

// 站心直角坐标转地心直角坐标
// 输入参数：待转换的站心直角坐标x、y、z和站心原点的经度、纬度和高度
// 输出参数：地心直角坐标x、y和z
coordinate_transform.prototype.zx_to_dx = function (x, y, z, L, B, H)// CPP2JS
{
  // 计算旋转矩阵
  var R = new Array(9)
  this.zx_to_dx_matrix(L, B, H, R)
  // 计算平移参数(经纬度和高度转直角坐标)
  var _r = // js
    this.ll_to_meter(L, B, H)
  L = _r[0]
  B = _r[1]
  H = _r[2]// js
  // 计算旋转后的坐标
  var X = new Array(3)
  var rt = new Array(3)
  // 先将X轴反向
  X[0] = -x
  X[1] = y
  X[2] = z
  var mat = new matrix_operation()
  mat.mult_matrix_331(R, X, rt)
  // 加上平移参数
  x = rt[0] + L
  y = rt[1] + B
  z = rt[2] + H
  return [x, y, z]// js
}
coordinate_transform.prototype.zx_to_dx_matrix = function (L, B, H, m)// CPP2JS
{
  // 将角度转弧度
  var newL, newB
  newL = L / roudu
  newB = B / roudu
  // 将站心直角坐标系旋转到地心直角坐标系的位置
  var R1 = new Array(9)
  var R2 = new Array(9)
  // 先绕y轴顺时针旋转90°-B
  R1[0] = Math.cos(M_PI / 2 - newB)
  R1[1] = 0
  R1[2] = Math.sin(M_PI / 2 - newB)
  R1[3] = 0
  R1[4] = 1
  R1[5] = 0
  R1[6] = -R1[2]
  R1[7] = 0
  R1[8] = R1[0]
  // 再绕z轴顺时针旋转L
  R2[0] = Math.cos(newL)
  R2[1] = -Math.sin(newL)
  R2[2] = 0
  R2[3] = -R2[1]
  R2[4] = R2[0]
  R2[5] = 0
  R2[6] = 0
  R2[7] = 0
  R2[8] = 1
  // R2*R1得R
  var mat = new matrix_operation()
  mat.mult_matrix_333(R2, R1, m)
}

// 地心直角坐标转站心直角坐标
// 输入参数：待转换的地心直角坐标x、y、z和站心原点的经度、纬度和高度
// 输出参数：站心直角坐标x、y和z
coordinate_transform.prototype.dx_to_zx = function (x, y, z, L, B, H)// CPP2JS
{
  // 计算旋转矩阵
  var R = new Array(9)
  this.dx_to_zx_matrix(L, B, H, R)
  // 计算平移参数(经纬度和高度转直角坐标)
  var _r = // js
    this.ll_to_meter(L, B, H)
  L = _r[0]
  B = _r[1]
  H = _r[2]// js
  // 先减去平移参数
  var X = new Array(3)
  var rt = new Array(3)
  X[0] = x - L
  X[1] = y - B
  X[2] = z - H
  var mat = new matrix_operation()
  mat.mult_matrix_331(R, X, rt)
  // 赋值
  x = rt[0]
  y = rt[1]
  z = rt[2]
  return [x, y, z]// js
}
coordinate_transform.prototype.dx_to_zx_matrix = function (L, B, H, m)// CPP2JS
{
  // 将角度转弧度
  var newL, newB
  newL = L / roudu
  newB = B / roudu
  // 将地心直角坐标系旋转到站心直角坐标系的位置
  var R1 = new Array(9)
  var R2 = new Array(9)
  // 先绕z轴逆时针旋转L
  R2[0] = Math.cos(newL)
  R2[1] = Math.sin(newL)
  R2[2] = 0
  R2[3] = -R2[1]
  R2[4] = R2[0]
  R2[5] = 0
  R2[6] = 0
  R2[7] = 0
  R2[8] = 1
  // 再绕y轴逆时针旋转90°-B（注意x轴已经反向过了）
  R1[0] = Math.cos(M_PI / 2 - newB)
  R1[1] = 0
  R1[2] = -Math.sin(M_PI / 2 - newB)
  R1[3] = 0
  R1[4] = 1
  R1[5] = 0
  R1[6] = -R1[2]
  R1[7] = 0
  R1[8] = R1[0]
  // R1*R2得R(即先绕z轴旋转L，再绕y轴旋转90°-B)
  var mat = new matrix_operation()
  mat.mult_matrix_333(R1, R2, m)
}

coordinate_transform.prototype.zx_to_ll = function (x, y, z, L, B, H)// CPP2JS
{
  // zx to dx
  var _r = // js
    this.zx_to_dx(x, y, z, L, B, H)
  x = _r[0]
  y = _r[1]
  z = _r[2]// js

  // dx to ll
  return// js
  this.meter_to_ll(x, y, z)
}

coordinate_transform.prototype.ll_to_zx = function (x, y, z, L, B, H)// CPP2JS
{
  // ll to meters
  var _r = // js
    this.ll_to_meter(x, y, z)
  x = _r[0]
  y = _r[1]
  z = _r[2]// js

  return// js
  this.dx_to_zx(x, y, z, L, B, H)
}

// 计算经过旋转和平移后的坐标
// 输入参数：旋转角数组in_angle（方位角、横滚角、俯仰角）、平移向量t和原始坐标origin_xyz
// 输出参数：变换后坐标out_xyz
coordinate_transform.prototype.transform_rt = function (in_angle, t, origin_xyz, out_xyz)// CPP2JS
{
  var angle, roll, pitch
  angle = in_angle[0] * M_PI / 180
  roll = in_angle[1] * M_PI / 180
  pitch = in_angle[2] * M_PI / 180

  var R1 = new Array(9)
  var R2 = new Array(9)
  var R3 = new Array(9)
  var RT = new Array(9)
  var R = new Array(9)
  // 三个角元素在记录时，angle角顺时针方向，
  // roll角和pitch角均为逆时针方向
  // 先绕x轴旋转roll角（逆时针方向）
  R1[0] = 1
  R1[1] = 0
  R1[2] = 0
  R1[3] = 0
  R1[4] = Math.cos(roll)
  R1[5] = Math.sin(roll)
  R1[6] = 0
  R1[7] = -Math.sin(roll)
  R1[8] = Math.cos(roll)

  // 再绕y轴旋转pitch角（逆时针方向）
  R2[0] = Math.cos(pitch)
  R2[1] = 0
  R2[2] = -Math.sin(pitch)
  R2[3] = 0
  R2[4] = 1
  R2[5] = 0
  R2[6] = Math.sin(pitch)
  R2[7] = 0
  R2[8] = Math.cos(pitch)

  // 最后绕z轴旋转angle角（逆时针方向）
  R3[0] = Math.cos(angle)
  R3[1] = Math.sin(angle)
  R3[2] = 0
  R3[3] = -Math.sin(angle)
  R3[4] = Math.cos(angle)
  R3[5] = 0
  R3[6] = 0
  R3[7] = 0
  R3[8] = 1

  // 注意旋转矩阵的相乘顺序是R3*R2*R1
  var mat = new matrix_operation()
  mat.mult_matrix_333(R3, R2, RT)
  mat.mult_matrix_333(RT, R1, R)

  // R*X
  mat.mult_matrix_331(R, origin_xyz, out_xyz)

  // 加上平移参数
  out_xyz[0] += t[0]
  out_xyz[1] += t[1]
  out_xyz[2] += t[2]
}

// 将相机的姿态转到IMU坐标系
// IMU坐标系为Y轴朝前，X轴向右，Z轴向上
// 输入参数：相机俯仰和航偏角py，单位为度
// 输出参数：IMU坐标系的旋转矩阵R
coordinate_transform.prototype.cmr_to_imu = function (py, R)// CPP2JS
{
  // 相机坐标系转IMU坐标系
  var pitch, angle
  pitch = py[0] / roudu
  angle = py[1] / roudu

  var R1 = new Array(9)
  var R2 = new Array(9)
  // 先绕x轴旋转pitch角（顺时针方向）
  R1[0] = 1
  R1[1] = 0
  R1[2] = 0
  R1[3] = 0
  R1[4] = Math.cos(pitch)
  R1[5] = -Math.sin(pitch)
  R1[6] = 0
  R1[7] = -R1[5]
  R1[8] = R1[4]

  // 再绕z轴旋转angle角（顺时针方向）
  R2[0] = Math.cos(angle)
  R2[1] = -Math.sin(angle)
  R2[2] = 0
  R2[3] = -R2[1]
  R2[4] = R2[0]
  R2[5] = 0
  R2[6] = 0
  R2[7] = 0
  R2[8] = 1

  // 注意旋转矩阵的相乘顺序是R2*R1=R
  var mat = new matrix_operation()
  mat.mult_matrix_333(R2, R1, R)
}

// 将姿态从IMU坐标系转到WGS84坐标系
// 原始IMU坐标系为Y轴朝前，X轴向右，Z轴向上
// 输入参数：当前IMU坐标系的旋转矩阵R，IMU三姿态rpy和经纬度L,B，单位为度
// 输出参数：地心坐标系旋转矩阵R
coordinate_transform.prototype.imu_to_dx = function (R, rpy, L, B)// CPP2JS
{
  // IMU坐标系转站心坐标系
  var roll, pitch, angle
  roll = rpy[0] / roudu
  pitch = rpy[1] / roudu
  angle = rpy[2] / roudu

  var R1 = new Array(9)
  var R2 = new Array(9)
  var R3 = new Array(9)
  var RT = new Array(9)
  var RT2 = new Array(9)
  var RR = new Array(9)
  // 三个角元素在记录时，angle角顺时针方向
  // roll角和pitch角均为逆时针方向
  // 先绕y轴旋转roll角（顺时针方向）
  R1[0] = Math.cos(roll)
  R1[1] = 0
  R1[2] = Math.sin(roll)
  R1[3] = 0
  R1[4] = 1
  R1[5] = 0
  R1[6] = -R1[2]
  R1[7] = 0
  R1[8] = R1[0]

  // 再绕x轴旋转pitch角（顺时针方向）
  R2[0] = 1
  R2[1] = 0
  R2[2] = 0
  R2[3] = 0
  R2[4] = Math.cos(pitch)
  R2[5] = -Math.sin(pitch)
  R2[6] = 0
  R2[7] = -R2[5]
  R2[8] = R2[4]

  // 最后绕z轴旋转angle角（逆时针方向）
  R3[0] = Math.cos(angle)
  R3[1] = Math.sin(angle)
  R3[2] = 0
  R3[3] = -R3[1]
  R3[4] = R3[0]
  R3[5] = 0
  R3[6] = 0
  R3[7] = 0
  R3[8] = 1

  // 注意旋转矩阵的相乘顺序是R3*R2*R1
  var mat = new matrix_operation()
  mat.mult_matrix_333(R3, R2, RT)
  mat.mult_matrix_333(RT, R1, RR)
  // RR*R=RT2
  mat.mult_matrix_333(RR, R, RT2)

  // 将站心直角坐标系旋转到地心直角坐标系的位置
  // 将角度转弧度
  var newL, newB
  newL = L / roudu
  newB = B / roudu
  // 先绕z轴顺时针旋转90°
  R1[0] = Math.cos(M_PI / 2)
  R1[1] = -Math.sin(M_PI / 2)
  R1[2] = 0
  R1[3] = -R1[1]
  R1[4] = R1[0]
  R1[5] = 0
  R1[6] = 0
  R1[7] = 0
  R1[8] = 1
  // 再绕y轴顺时针旋转90°-B
  R2[0] = Math.cos(M_PI / 2 - newB)
  R2[1] = 0
  R2[2] = Math.sin(M_PI / 2 - newB)
  R2[3] = 0
  R2[4] = 1
  R2[5] = 0
  R2[6] = -R2[2]
  R2[7] = 0
  R2[8] = R2[0]
  // 再绕z轴顺时针旋转L
  R3[0] = Math.cos(newL)
  R3[1] = -Math.sin(newL)
  R3[2] = 0
  R3[3] = -R3[1]
  R3[4] = R3[0]
  R3[5] = 0
  R3[6] = 0
  R3[7] = 0
  R3[8] = 1
  // R3*R2*R1得RR
  mat.mult_matrix_333(R3, R2, RT)
  mat.mult_matrix_333(RT, R1, RR)
  // RR*RT2=R
  mat.mult_matrix_333(RR, RT2, R)
}

// 将相机的姿态转到地心坐标系
// IMU坐标系为Y轴朝前，X轴向右，Z轴向上
// 输入参数：相机俯仰和航偏角py，当前IMU坐标系的旋转矩阵R，
// 输入参数：IMU三姿态rpy和经纬度L,B，单位为度
// 输出参数：IMU坐标系的旋转矩阵R
coordinate_transform.prototype.cmr_to_dx = function (cmr_py, imu_rpy, L, B, R)// CPP2JS
{
  // 相机转IMU
  this.cmr_to_imu(cmr_py, R)
  // IMU转地心
  this.imu_to_dx(R, imu_rpy, L, B)
}

// 计算三维坐标对应的纹理坐标
// 输入参数：三维坐标pos、初始方位角angle、初始俯仰角pitch
// 输出参数：纹理坐标u、v
coordinate_transform.prototype.get_texture_coord = function (pos, angle0, pitch0, u, v)// CPP2JS
{
  var len = Math.sqrt(pos[0] * pos[0] + pos[1] * pos[1])
  var yaw = Math.atan2(pos[1], pos[0]) * 180 / M_PI
  var pitch = Math.atan2(pos[2], len) * 180 / M_PI

  yaw = 180 - yaw - angle0 + 180
  pitch = pitch - pitch0 + 90

  if (yaw < 0) {
    yaw += 360
  } else if (yaw >= 360) {
    yaw -= 360
  }
  if (pitch < 0) {
    pitch += 360
  } else if (pitch >= 360) {
    pitch -= 360
  }

  u = yaw / 360
  v = 1 - pitch / 180
  // if (*v < 0)
  //    printf("get_texture_coord pitch0=%f, pitch=%f\n", pitch0, pitch);
  return [u, v]// js
}

export default coordinate_transform

/* /JS_SKIP_BEGIN///

///JS_SKIP_END/ */
