// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_geometry_element_base.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Element)

class txLine : public txGeomElementBase {
 public:
  using ParentClass = txGeomElementBase;
  txLine() TX_DEFAULT;
  virtual ~txLine() TX_DEFAULT;
  /**
   * @brief 获取当前对象的类型
   *
   * 该函数用于获取当前对象的类型，并返回该类型的枚举值。
   *
   * @return GeomElemType 当前对象的类型枚举值
   */
  virtual GeomElemType ElemType() const TX_NOEXCEPT TX_OVERRIDE { return txGeomElementBase::GeomElemType::eLine; }

  /**
   * @brief 获取当前对象的类型字符串
   *
   * 该函数用于获取当前对象的类型字符串，并返回该字符串。
   *
   * @return txString 当前对象的类型字符串
   */
  virtual txString ElemTypeStr() const TX_NOEXCEPT TX_OVERRIDE { return txString("eLine"); }

  /**
   * @brief 返回当前对象的字符串表示形式
   *
   * 该函数返回当前对象的字符串表示形式，例如："10"，"Hello" 等。
   *
   * @return txString 当前对象的字符串表示形式
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回多边形几何体的起始顶点
   *
   * 该函数返回多边形几何体的起始顶点，即第一个顶点。
   *
   * @return txVertex 多边形几何体的起始顶点
   */
  virtual txVertex StartVtx() const TX_NOEXCEPT TX_OVERRIDE { return mVtxStart; }

  /**
   * @brief 获取多边形的最后一个顶点
   *
   * 该函数返回多边形的最后一个顶点，即最后一个赋值的顶点。
   *
   * @return txVertex 多边形的最后一个顶点
   */
  virtual txVertex EndVtx() const TX_NOEXCEPT TX_OVERRIDE { return mVtxEnd; }

  /**
   * @brief 检查两个几何元素是否相交
   *
   * 该函数检查两个几何元素是否相交，如果相交，则返回 true，否则返回 false。
   * 如果相交，将交点位置存储在 intersectPt 中，距离起点的长度存储在 distOnTraj 中。
   *
   * @param obj_ptr 另一个要检查的几何元素
   * @param intersectPt 交点的位置
   * @param distOnTraj 距离起点的长度
   * @return txBool 如果两个几何元素相交，则返回 true，否则返回 false
   */
  virtual txBool CheckIntersection(txGeomElementBasePtr obj_ptr, txVertex& intersectPt, txFloat& distOnTraj,
                                   txFloat& distOnObjTraj) const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 初始化线段对象
   *
   * 初始化线段对象，将线段的起点和终点设置为指定的坐标。
   * 函数返回 true 表示线段初始化成功，false 表示线段初始化失败。
   *
   * @param _start_vtx 线段的起点坐标
   * @param _end_vtx 线段的终点坐标
   * @return 线段初始化成功返回 true，否则返回 false
   */
  virtual txBool Initialize(const txVec3& _start_vtx, const txVec3& _end_vtx) TX_NOEXCEPT;

 protected:
  txVertex mVtxStart, mVtxEnd;
};
using txLinePtr = std::shared_ptr<txLine>;

class txPolyline : public txGeomElementBase {
 public:
  using ParentClass = txGeomElementBase;
  txPolyline() TX_DEFAULT;
  virtual ~txPolyline() TX_DEFAULT;

  /**
   * @brief 判断当前对象是否有效
   *
   * 判断当前对象是否有效，如果有效则返回 true，否则返回 false。
   *
   * @return txBool 当前对象是否有效
   */
  virtual txBool IsValid() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前对象的类型
   *
   * 该函数用于获取当前对象的类型，并返回该类型的枚举值。
   *
   * @return GeomElemType 当前对象的类型枚举值
   */
  virtual GeomElemType ElemType() const TX_NOEXCEPT TX_OVERRIDE { return txGeomElementBase::GeomElemType::ePolyline; }

  /**
   * @brief 获取当前对象的类型字符串
   *
   * 该函数用于获取当前对象的类型字符串，并返回该字符串。
   *
   * @return txString 当前对象的类型字符串
   */
  virtual txString ElemTypeStr() const TX_NOEXCEPT TX_OVERRIDE { return txString("ePolyline"); }

  /**
   * @brief 返回当前对象的字符串表示形式
   *
   * 该函数返回当前对象的字符串表示形式，例如："10"，"Hello" 等。
   *
   * @return txString 当前对象的字符串表示形式
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回多边形几何体的起始顶点
   *
   * 该函数返回多边形几何体的起始顶点，即第一个顶点。
   *
   * @return txVertex 多边形几何体的起始顶点
   */
  virtual txVertex StartVtx() const TX_NOEXCEPT TX_OVERRIDE { return SamplingPt(0); }

  /**
   * @brief 获取多边形的最后一个顶点
   *
   * 该函数返回多边形的最后一个顶点，即最后一个赋值的顶点。
   *
   * @return txVertex 多边形的最后一个顶点
   */
  virtual txVertex EndVtx() const TX_NOEXCEPT TX_OVERRIDE { return SamplingPt(SamplingPtSize() - 1); }

  /**
   * @brief 当前对象释放
   *
   */
  virtual void Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查两个几何元素是否相交
   *
   * 该函数检查两个几何元素是否相交，如果相交，则返回 true，否则返回 false。
   * 如果相交，将交点位置存储在 intersectPt 中，距离起点的长度存储在 distOnTraj 中。
   *
   * @param obj_ptr 另一个要检查的几何元素
   * @param intersectPt 交点的位置
   * @param distOnTraj 距离起点的长度
   * @return txBool 如果两个几何元素相交，则返回 true，否则返回 false
   */
  virtual txBool CheckIntersection(txGeomElementBasePtr obj_ptr, txVertex& intersectPt, txFloat& distOnTraj,
                                   txFloat& distOnObjTraj) const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 初始化线段数组对象
   *
   * 根据指定的顶点数组初始化线段数组对象，并将其中的线段顶点存储在一个内部的 vector 中。
   * 此函数返回值表示线段数组对象是否初始化成功。
   *
   * @param _vtx_array 包含顶点的 vector，其中 _vtx_array[i] 代表第 i 个线段的起点，_vtx_array[i+1] 代表第 i
   * 个线段的终点。
   * @return 如果线段数组对象初始化成功，返回 true，否则返回 false。
   */
  virtual txBool Initialize(const std::vector<txVec3>& _vtx_array) TX_NOEXCEPT;

  /**
   * @brief 从线段数组中提取指定索引的线段
   *
   * 此函数返回指定索引处的线段对象，该线段对象表示线段的起点和终点。
   *
   * @param idx 线段的索引
   * @return 返回指定索引处的线段对象
   */
  virtual txLinePtr SubSegmentLine(const txSize idx) const TX_NOEXCEPT;

  /**
   * @brief 计算线段的抽样点数量
   *
   * 该函数返回线段的抽样点数量，线段的抽样点数量可能因为与采样间隔、弯线等设置的不同而有所不同。
   *
   * @return txSize 返回线段的抽样点数量
   */
  virtual txSize SamplingPtSize() const TX_NOEXCEPT { return mPts.size(); }

  /**
   * @brief 计算线段的个数
   *
   * 该函数计算线段的个数，返回值为线段数量减一。由于每个线段都只能有一个起点和终点，因此线段的个数是相邻两个点的数量。
   *
   * @return txSize 返回线段的个数
   */
  virtual txSize SegmentLineSize() const TX_NOEXCEPT { return SamplingPtSize() - 1; }

  /**
   * @brief 计算线段的起始点
   *
   * 根据线段的索引值，获取线段的起始点。
   * 由于每个线段都只有一个起点和一个终点，因此线段的起始点为其相邻点的第一个。
   *
   * @param idx 线段的索引值
   * @return txVertex 线段的起始点
   */
  virtual txVertex SamplingPt(const txSize idx) const TX_NOEXCEPT { return mPts.at(idx); }

 protected:
  std::vector<txVertex> mPts;
};
using txPolylinePtr = std::shared_ptr<txPolyline>;

TX_NAMESPACE_CLOSE(Element)
TX_NAMESPACE_CLOSE(Geometry)
