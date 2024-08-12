// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_algebra_defs.h"
#include "tx_marco.h"
#include "tx_obb.h"
#include "tx_type_def.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(CD2D)

using txFloat = Base::txFloat;
using txSize = Base::txSize;
using cdVec2 = Base::txVec2;
using txBool = Base::txBool;
using txPolygon2D = Geometry::OBB2D::PolygonArray;

/**
 * @brief 计算两线段的最小距离以及线段之间的交点
 *
 * 该函数计算两条线段之间的最小距离，并求出它们的交点。函数以两个线段的两个端点和一个交点作为输入，返回最小距离，并将交点存储在输入的交点变量中。
 *
 * @param v 第一条线段的起点
 * @param w 第一条线段的终点
 * @param p 第二条线段的起点
 * @param ptInSeg 第二条线段与第一条线段相交的点
 * @return 返回两线段之间的最小距离
 */
txFloat minimum_distance(const cdVec2& v, const cdVec2& w, const cdVec2& p, cdVec2& ptInSeg) TX_NOEXCEPT;

/**
 * @brief 计算两条线段之间的最短距离及交点
 *
 * 该函数计算两条线段之间的最短距离，并求出它们的交点。函数以一对cdVec2类型的向量作为输入，分别表示两条线段的端点。
 *
 * @param S1_P0 第一条线段的起点
 * @param S1_P1 第一条线段的终点
 * @param S2_P0 第二条线段的起点
 * @param S2_P1 第二条线段的终点
 * @param ptInS1 第一条线段上与第二条线段相交的点
 * @param ptInS2 第二条线段上与第一条线段相交的点
 * @return 返回两线段之间的最短距离
 */
txFloat dist3D_Segment_to_Segment(const cdVec2& S1_P0, const cdVec2& S1_P1, const cdVec2& S2_P0, const cdVec2& S2_P1,
                                  cdVec2& ptInS1, cdVec2& ptInS2) TX_NOEXCEPT;

/**
 * @brief 计算两车辆之间的最小相对位移
 *
 * 该函数计算两车辆之间的最小相对位移，同时返回两车辆的相对位置向量（ego-obs）及其在两车辆的相对轨迹上的交点。
 *
 * @param ego 第一车辆的轮廓
 * @param obs 第二车辆的轮廓
 * @param finalPtInEgo 在egov车辆轨迹上的最近点
 * @param finalPtInObs 在obs车辆轨迹上的最近点
 * @return 两车辆之间的最小相对位移
 */
txFloat smallest_dist_between_ego_and_obsVehicle(const txPolygon2D& ego, const txPolygon2D& obs, cdVec2& finalPtInEgo,
                                                 cdVec2& finalPtInObs) TX_NOEXCEPT;

/**
 * @brief 计算一个点对一个平面的左右关系
 *
 * 这个函数计算一个点相对于由两个点确定的平面的左右关系。如果点的半平面投影在
 * 第三个点的左侧，则返回正数；如果投影在右侧，则返回负数；如果投影在平面上，则返回零。
 *
 * @param p1 第一个点，平面由p1和p2确定
 * @param p2 第二个点，平面由p1和p2确定
 * @param p3 要计算关系的点
 * @return 如果点在平面的左侧，则返回正数；如果在右侧，则返回负数；如果在平面上，则返回零。
 */
txFloat half_plane_sign_2d(const cdVec2& p1, const cdVec2& p2, const cdVec2& p3) TX_NOEXCEPT;

/**
 * @brief 判断一个点是否在一个三角形内部
 *
 * 该函数用于判断一个点是否位于由三个顶点 v1, v2, v3 定义的三角形内部。
 *
 * @param pt 待判断的点
 * @param v1 三角形的第一个顶点
 * @param v2 三角形的第二个顶点
 * @param v3 三角形的第三个顶点
 * @return 如果点在三角形内部，返回 true；否则，返回 false。
 */
txBool PointInTriangle2d(const cdVec2& pt, const cdVec2& v1, const cdVec2& v2, const cdVec2& v3) TX_NOEXCEPT;

TX_NAMESPACE_OPEN(GJK)

/**
 * @brief 检查两个多边形是否发生碰撞
 *
 * 此函数用于检查两个多边形是否发生碰撞，如果碰撞发生则返回 true，否则返回 false。
 *
 * @param polygon1 第一个多边形
 * @param polygon2 第二个多边形
 * @return 如果两个多边形发生碰撞，返回 true；否则，返回 false。
 */
txBool HitPolygons(const txPolygon2D& polygon1, const txPolygon2D& polygon2) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(GJK)
TX_NAMESPACE_CLOSE(CD2D)
TX_NAMESPACE_CLOSE(Geometry)
