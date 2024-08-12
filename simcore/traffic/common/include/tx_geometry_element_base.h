// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/optional.hpp>
#include <list>
#include <vector>
#include "tx_header.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Element)

class txGeomElementBase;
using txGeomElementBasePtr = std::shared_ptr<txGeomElementBase>;

class txGeomElementBase {
 public:
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txFloat = Base::txFloat;
  using txString = Base::txString;
  using txSize = Base::txSize;
  using txVec2 = Base::txVec2;
  using txVec3 = Base::txVec3;
  using txVertex = txVec3;
  enum class GeomElemType : txInt { eUndef = 0, eLine = 1, ePolyline = 2 };
  virtual ~txGeomElementBase() TX_DEFAULT;

  /**
   * @brief 判断当前对象是否有效
   *
   * 判断当前对象是否有效，如果有效则返回 true，否则返回 false。
   *
   * @return txBool 当前对象是否有效
   */
  virtual txBool IsValid() const TX_NOEXCEPT { return mIsValid; }

  /**
   * @brief 当前对象释放
   *
   */
  virtual void Release() TX_NOEXCEPT {
    mIsValid = false;
    m_PolylineLength = 0.0;
    m_RealLength = 0.0;
  }

  /**
   * @brief 获取当前对象的类型
   *
   * 该函数用于获取当前对象的类型，并返回该类型的枚举值。
   *
   * @return GeomElemType 当前对象的类型枚举值
   */
  virtual GeomElemType ElemType() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前对象的类型字符串
   *
   * 该函数用于获取当前对象的类型字符串，并返回该字符串。
   *
   * @return txString 当前对象的类型字符串
   */
  virtual txString ElemTypeStr() const TX_NOEXCEPT = 0;

  /**
   * @brief 返回当前对象的字符串表示形式
   *
   * 该函数返回当前对象的字符串表示形式，例如："10"，"Hello" 等。
   *
   * @return txString 当前对象的字符串表示形式
   */
  virtual txString Str() const TX_NOEXCEPT;

  /**
   * @brief 返回多边形几何体的长度
   *
   * 该函数返回多边形几何体的总长度，即所有边的总长度。
   *
   * @return txFloat 多边形几何体的长度
   */
  txFloat PolylineLength() const TX_NOEXCEPT { return m_PolylineLength; }

  /**
   * @brief 返回多边形的真实长度
   *
   * 该函数返回多边形的真实长度，即经过标准化处理后的长度，该长度为尽可能接近实际情况下的长度。
   *
   * @return txFloat 多边形的真实长度
   */
  txFloat RealLength() const TX_NOEXCEPT { return m_RealLength; }

  /**
   * @brief 返回多边形几何体的起始顶点
   *
   * 该函数返回多边形几何体的起始顶点，即第一个顶点。
   *
   * @return txVertex 多边形几何体的起始顶点
   */
  virtual txVertex StartVtx() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取多边形的最后一个顶点
   *
   * 该函数返回多边形的最后一个顶点，即最后一个赋值的顶点。
   *
   * @return txVertex 多边形的最后一个顶点
   */
  virtual txVertex EndVtx() const TX_NOEXCEPT = 0;

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
                                   txFloat& distOnObjTraj) const TX_NOEXCEPT = 0;
  friend std::ostream& operator<<(std::ostream& os, const txGeomElementBase& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

 protected:
  txBool mIsValid = false;
  txFloat m_PolylineLength = 0.0;
  txFloat m_RealLength = 0.0;
};

using txGeomElementBasePtr = std::shared_ptr<txGeomElementBase>;
using txGeomElementBasePtrArray = std::vector<txGeomElementBasePtr>;
using txGeomElementBasePtrList = std::list<txGeomElementBasePtr>;

TX_NAMESPACE_CLOSE(Element)
TX_NAMESPACE_CLOSE(Geometry)
