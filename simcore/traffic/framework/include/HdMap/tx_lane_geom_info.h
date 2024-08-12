// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <array>
#include "tx_class_counter.h"
#include "tx_frame_utils.h"
#include "tx_header.h"
#include "tx_locate_info.h"
TX_NAMESPACE_OPEN(HdMap)

#define _ct_x_(pt) (pt[0])
#define _ct_y_(pt) (pt[1])

struct txLaneProjectionUtils {
  using txFloat = Base::txFloat;
  using controlPoint = std::array<txFloat, 2>;
  using controlPointVec = std::vector<controlPoint>;

  /**
   * @brief txLaneProjectionUtils 构造函数
   *
   * @param inputPts 传入的控制点向量
   */
  explicit txLaneProjectionUtils(const controlPointVec& inputPts) : m_points4xysl(inputPts) {
    m_segment_length_list.resize(m_points4xysl.size());
    m_segment_length_list[0] = 0.0;
    for (std::size_t i = 1; i < m_points4xysl.size(); ++i) {
      m_segment_length_list[i] =
          m_segment_length_list[i - 1] + std::sqrt(sqrlen(sub(m_points4xysl[i], m_points4xysl[i - 1])));
    }
  }
  virtual ~txLaneProjectionUtils() TX_DEFAULT;
  struct txProjResult {
   public:
    txProjResult() TX_DEFAULT;
    txProjResult(const txFloat _t, const txFloat _dist) : m_t(_t), m_distance(_dist) {}

    /**
     * @brief Get the t value
     * @return txFloat 返回t值
     */
    txFloat t() const TX_NOEXCEPT { return m_t; }

    /**
     * @brief Get the distance between the lane.
     * @return txFloat 获取该车道之间的距离
     */
    txFloat dist() const TX_NOEXCEPT { return m_distance; }

   protected:
    txFloat m_t = 0.0; /*[0.0, 1.0]*/
    txFloat m_distance = 99999.0;
  };

  /**
   * @brief Subtract two control points
   * @param p_a The first control point to subtract
   * @param p_b The second control point to subtract
   * @return The result of subtracting p_b from p_a
   * @note The return value is a controlPoint struct containing the coordinates of the subtraction
   * @details This function subtracts two control points, returning the resulting control point
   */
  static controlPoint sub(const controlPoint& p_a, const controlPoint& p_b) TX_NOEXCEPT {
    return {p_a[0] - p_b[0], p_a[1] - p_b[1]};
  }

  /**
   * @brief 求平方
   *
   * @param a 输入值
   * @return txFloat 值的平方
   */
  static txFloat sqr(const txFloat a) TX_NOEXCEPT { return a * a; }

  /**
   * @brief 计算一个点到原点的距离的平方
   * @param a 控制点
   * @return 平方距离
   */
  static txFloat sqrlen(const controlPoint& a) { return sqr(_ct_x_(a)) + sqr(_ct_y_(a)); }

  /**
   * @brief 计算一个点到线段的最近点
   *
   * @param p_a 线段的起点
   * @param p_b 线段的终点
   * @param q 输入的点
   * @return 返回一个包含两个值的结构体，分别是最近点与线段起点的距离和最近点与输入点的距离
   */
  static txProjResult Point2Segment(const controlPoint& p_a, const controlPoint& p_b,
                                    const controlPoint& q) TX_NOEXCEPT {
    txFloat t = -1.0, current_dist = 999.0;
    controlPoint b = p_a;
    controlPoint dbq = sub(b, q);
    txFloat dist = sqrlen(dbq);

    {
      const controlPoint a = b;
      const controlPoint daq = dbq;
      b = p_b;
      dbq = sub(b, q);
      const controlPoint dab = sub(a, b);
      const txFloat inv_sqrlen = 1. / sqrlen(dab);

      t = (_ct_x_(dab) * _ct_x_(daq) + _ct_y_(dab) * _ct_y_(daq)) * inv_sqrlen;

      if (t < 0.) {
        t = 0.0;
        current_dist = dist;
      } else if (t <= 1.0) {
        current_dist = sqr(_ct_x_(dab) * _ct_y_(dbq) - _ct_y_(dab) * _ct_x_(dbq)) * inv_sqrlen;
      } else {
        t = 1.0;
        current_dist = sqrlen(dbq);
      }
    }
    return txProjResult(t, current_dist);
  }

  /**
   * @brief 将给定的二维坐标点映射到线段的投影点上，并计算最近点与线段的距离。
   *
   * @param enu2d 输入的二维坐标点。
   * @param s 输出，最近点与线段起点之间的距离。
   * @param l 输出，最近点与线段终点之间的距离。
   * @return 返回true表示计算成功，返回false表示计算失败。
   */
  virtual Base::txBool xy2sl_func(const Base::txVec2& enu2d, Base::txFloat& s, Base::txFloat& l) const TX_NOEXCEPT {
    txProjResult optRes;
    std::size_t idx = 0;
    const controlPoint q{enu2d.x(), enu2d.y()};
    for (std::size_t i = 1; i < m_points4xysl.size(); ++i) {
      const txProjResult res = Point2Segment(m_points4xysl[i - 1], m_points4xysl[i], q);
      if (res.dist() < optRes.dist()) {
        optRes = res;
        idx = i - 1;
      }
    }

    const Base::txVec2 startPt(_ct_x_(m_points4xysl[idx]), _ct_y_(m_points4xysl[idx]));
    const Base::txVec2 endPt(_ct_x_(m_points4xysl[idx + 1]), _ct_y_(m_points4xysl[idx + 1]));

    Base::txVec2 targent = (endPt - startPt);
    const Base::txFloat length = targent.norm();
    targent.normalize();
    const Utils::PtVecRelation2D sign = Utils::PointOnVector(startPt, endPt, enu2d);
    Base::txVec2 normal(-1.0 * targent.y(), targent.x());

    const Base::txVec2 projPt = startPt + optRes.t() * length * targent;

    const Base::txVec2 offsetVec = (enu2d - projPt);

    s = m_segment_length_list[idx] + optRes.t() * length;
    l = offsetVec.norm() * ((Utils::PtVecRelation2D::Left == sign) ? (1.0) : (-1.0));
    return true;
  }

 protected:
  controlPointVec m_points4xysl;
  std::vector<txFloat> m_segment_length_list;
};

#undef _ct_x_
#undef _ct_y_

class txLaneInfoInterface {
 public:
  using txFloat = Base::txFloat;
  using controlPoint = std::array<txFloat, 2>;
  using controlPointVec = std::vector<controlPoint>;

  enum class CurveType : Base::txInt { tLane = 0, tLink = 1, tUndef = 2, tTraj = 3 };
  enum class LaneShape : Base::txInt { sLine = 0, sArc90_L = 1, sArc90_R = 2, sArc180 = 3 };

  friend std::ostream& operator<<(std::ostream& os, const LaneShape& v) TX_NOEXCEPT {
    switch (v) {
      case LaneShape::sLine: {
        os << "Straight";
        break;
      }
      case LaneShape::sArc90_L: {
        os << "Left";
        break;
      }
      case LaneShape::sArc90_R: {
        os << "Right";
        break;
      }
      case LaneShape::sArc180: {
        os << "UTurn";
        break;
      }
      default: {
        os << "unknown";
      }
    }
    return os;
  }

  explicit txLaneInfoInterface(CurveType _t = CurveType::tUndef) : m_curveType(_t) {}
  virtual ~txLaneInfoInterface() TX_DEFAULT;

  /**
   * @brief 获取指定位置的车道方向向量
   *
   * @param[in] pos 指定位置的车道长度比例，范围为[0,1]
   * @return 返回指定位置的车道方向向量
   */
  virtual Base::txVec3 GetLaneDir(const Base::txFloat) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取指定位置的局部坐标
   *
   * @param[in] pos 指定位置的距离比例，范围为[0, 1]
   * @return 返回指定位置的局部坐标
   */
  virtual Base::txVec3 GetLocalPos(const Base::txFloat) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前车道的形状
   *
   * @param[in] none
   * @return 返回当前车道的形状
   */
  virtual LaneShape GetShape() const TX_NOEXCEPT = 0;

  /**
   * @brief 检查当前对象是否为车道
   *
   * @param none
   * @return 如果当前对象为车道，则返回true，否则返回false
   */
  virtual Base::txBool IsLane() const TX_NOEXCEPT { return CurveType::tLane == m_curveType; }

  /**
   * @brief 判断当前对象是否为链路
   *
   * @return 如果当前对象为链路，则返回true，否则返回false
   */
  virtual Base::txBool IsLink() const TX_NOEXCEPT { return CurveType::tLink == m_curveType; }

  /**
   * @brief 获取当前链路的起始点坐标
   *
   * @return 返回当前链路的起始点坐标
   */
  virtual Base::txVec3 GetStartPt() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前链路的结束点坐标
   *
   * @return 返回当前链路的结束点坐标
   */
  virtual Base::txVec3 GetEndPt() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前链路的长度
   *
   * @return 返回当前链路的长度，单位：米
   */
  virtual Base::txFloat GetLength() const TX_NOEXCEPT = 0;

  /**
   * @brief 将二维空间坐标转换为ENU(East-North-Up)系统下的车辆坐标
   *
   * @param[in]  enu2d 二维空间坐标点(x,y)
   * @param[out] s     车辆向前方的距离，单位为米
   * @param[out] l     车辆向左侧的距离，单位为米
   * @return true 转换成功
   * @return false 转换失败
   */
  virtual Base::txBool xy2sl(const Base::txVec2& enu2d, Base::txFloat& s, Base::txFloat& l) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前车道的位置信息
   *
   * @return Base::Info_Lane_t 当前车道的位置信息
   */
  virtual Base::Info_Lane_t LaneLocInfo() const TX_NOEXCEPT { return m_lane_loc_info; }

  /**
   * @brief 获取当前车道的位置信息
   *
   * @return Base::Info_Road_t 当前车道的位置信息
   */
  virtual Base::Info_Road_t RoadLocInfo() const TX_NOEXCEPT {
    LOG(FATAL) << "un-support class type. lane info.";
    return m_road_loc_info;
  }

 protected:
  const CurveType m_curveType;
  Base::Info_Lane_t m_lane_loc_info;
  Base::Info_Road_t m_road_loc_info;
  Utils::Counter<txLaneInfoInterface> _c;
};

using txLaneInfoInterfacePtr = std::shared_ptr<txLaneInfoInterface>;

class txLaneSplineInfo : public txLaneInfoInterface {
 public:
  enum { CubicSpline_ControlPointSize = 3 };

 public:
  txLaneSplineInfo(const Base::txLaneLinkID lanelinkUid, const Base::txLaneUId& fromLaneUid,
                   const Base::txLaneUId& toLaneUid, const controlPointVec& inputControlPoints, const txFloat nLength);
  virtual ~txLaneSplineInfo() TX_DEFAULT;

  /**
   * @brief 获取指定位置的车道方向向量
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的车道方向
   */
  virtual Base::txVec3 GetLaneDir(const Base::txFloat _s) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前位置在车道中的坐标
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的坐标
   */
  virtual Base::txVec3 GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的形状信息
   *
   * @return LaneShape 当前车道的形状信息
   */
  virtual LaneShape GetShape() const TX_NOEXCEPT;

  /**
   * @brief 获取当前车道的起始点坐标
   *
   * @return txVec3 当前车道的起始点坐标
   */
  virtual Base::txVec3 GetStartPt() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的终点坐标
   *
   * @return txVec3 当前车道的终点坐标
   */
  virtual Base::txVec3 GetEndPt() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的长度
   *
   * @return txFloat 当前车道的长度
   */
  virtual Base::txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 将二维空间坐标转换为ENU(East-North-Up)系统下的车辆坐标
   *
   * @param[in]  enu2d 二维空间坐标点(x,y)
   * @param[out] s     车辆向前方的距离，单位为米
   * @param[out] l     车辆向左侧的距离，单位为米
   * @return true 转换成功
   * @return false 转换失败
   */
  virtual Base::txBool xy2sl(const Base::txVec2& enu2d, Base::txFloat& s,
                             Base::txFloat& l) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  struct txLaneSplineInfoImpl;
  std::shared_ptr<txLaneSplineInfoImpl> pImpl;
};
using txLaneSplineInfoPtr = std::shared_ptr<txLaneSplineInfo>;

class txLaneInfo : public txLaneInfoInterface {
 public:
  enum { CatmullRom_ControlPointSize = 4 };

 public:
  txLaneInfo(const Base::txLaneUId laneUid, controlPointVec inputControlPoints);
  virtual ~txLaneInfo() TX_DEFAULT;

  /**
   * @brief 获取指定位置的车道方向向量
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的车道方向
   */
  virtual Base::txVec3 GetLaneDir(const Base::txFloat s) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前位置在车道中的坐标
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的坐标
   */
  virtual Base::txVec3 GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的形状信息
   *
   * @return LaneShape 当前车道的形状信息
   */
  virtual LaneShape GetShape() const TX_NOEXCEPT;

  /**
   * @brief 获取当前车道的起始点坐标
   *
   * @return txVec3 当前车道的起始点坐标
   */
  virtual Base::txVec3 GetStartPt() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的终点坐标
   *
   * @return txVec3 当前车道的终点坐标
   */
  virtual Base::txVec3 GetEndPt() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的长度
   *
   * @return txFloat 当前车道的长度
   */
  virtual Base::txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据给定的idx返回对应点的参数
   *
   * @param idx 要查询的点在路线上的索引值
   * @return Base::txFloat 返回对应点的参数值
   */
  virtual Base::txFloat ParameterAtPoint(const size_t idx) const TX_NOEXCEPT;

  /**
   * @brief 将二维空间坐标转换为ENU(East-North-Up)系统下的车辆坐标
   *
   * @param[in]  enu2d 二维空间坐标点(x,y)
   * @param[out] s     车辆向前方的距离，单位为米
   * @param[out] l     车辆向左侧的距离，单位为米
   * @return true 转换成功
   * @return false 转换失败
   */
  virtual Base::txBool xy2sl(const Base::txVec2& enu2d, Base::txFloat& s,
                             Base::txFloat& l) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  struct txLaneInfoImpl;
  std::shared_ptr<txLaneInfoImpl> pImpl;
};

using txLaneInfoPtr = std::shared_ptr<txLaneInfo>;

class txLaneShortInfo : public txLaneInfoInterface {
 public:
  txLaneShortInfo(const Base::txLaneUId laneUid, const controlPoint& startVertex, const controlPoint& endVertex);
  txLaneShortInfo(const Base::txLaneLinkID lanelinkUid, const Base::txLaneUId& fromLaneUid,
                  const Base::txLaneUId& toLaneUid, const controlPoint& startVertex, const controlPoint& endVertex);
  virtual ~txLaneShortInfo() TX_DEFAULT;

  /**
   * @brief 获取指定位置的车道方向向量
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的车道方向
   */
  virtual Base::txVec3 GetLaneDir(const Base::txFloat) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前位置在车道中的坐标
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的坐标
   */
  virtual Base::txVec3 GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的形状信息
   *
   * @return LaneShape 当前车道的形状信息
   */
  virtual LaneShape GetShape() const TX_NOEXCEPT { return LaneShape::sLine; }

  /**
   * @brief 获取当前车道的起始点坐标
   *
   * @return txVec3 当前车道的起始点坐标
   */
  virtual Base::txVec3 GetStartPt() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的终点坐标
   *
   * @return txVec3 当前车道的终点坐标
   */
  virtual Base::txVec3 GetEndPt() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的长度
   *
   * @return txFloat 当前车道的长度
   */
  virtual Base::txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 将二维空间坐标转换为ENU(East-North-Up)系统下的车辆坐标
   *
   * @param[in]  enu2d 二维空间坐标点(x,y)
   * @param[out] s     车辆向前方的距离，单位为米
   * @param[out] l     车辆向左侧的距离，单位为米
   * @return true 转换成功
   * @return false 转换失败
   */
  virtual Base::txBool xy2sl(const Base::txVec2& enu2d, Base::txFloat& s,
                             Base::txFloat& l) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  struct txLaneShortInfoImpl;
  std::shared_ptr<txLaneShortInfoImpl> pImpl;
};

using txLaneShortInfoPtr = std::shared_ptr<txLaneShortInfo>;

class txLaneSinglePositionInfo : public txLaneInfoInterface {
 public:
  txLaneSinglePositionInfo(const Base::txLaneUId laneUid, const controlPoint& startVertex, const Base::txVec3 _laneDir)
      : mStartPt{startVertex[0], startVertex[1]}, mLaneDir(_laneDir) {}

  virtual ~txLaneSinglePositionInfo() TX_DEFAULT;

  /**
   * @brief 获取指定位置的车道方向向量
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的车道方向
   */
  virtual Base::txVec3 GetLaneDir(const Base::txFloat) const TX_NOEXCEPT TX_OVERRIDE { return mLaneDir; }

  /**
   * @brief 获取当前位置在车道中的坐标
   *
   * @param[in] _s 车道上的位置值
   * @return Base::txVec3 返回一个三维向量，表示当前位置的坐标
   */
  virtual Base::txVec3 GetLocalPos(const Base::txFloat) const TX_NOEXCEPT TX_OVERRIDE {
    return Base::txVec3(mStartPt[0], mStartPt[1], FLAGS_default_altitude);
  }

  /**
   * @brief 获取当前车道的形状信息
   *
   * @return LaneShape 当前车道的形状信息
   */
  virtual LaneShape GetShape() const TX_NOEXCEPT { return LaneShape::sLine; }

  /**
   * @brief 获取当前车道的起始点坐标
   *
   * @return txVec3 当前车道的起始点坐标
   */
  virtual Base::txVec3 GetStartPt() const TX_NOEXCEPT TX_OVERRIDE { return GetLocalPos(0.0); }

  /**
   * @brief 获取当前车道的终点坐标
   *
   * @return txVec3 当前车道的终点坐标
   */
  virtual Base::txVec3 GetEndPt() const TX_NOEXCEPT TX_OVERRIDE { return GetLocalPos(0.0); }

  /**
   * @brief 获取当前车道的长度
   *
   * @return txFloat 当前车道的长度
   */
  virtual Base::txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

 protected:
  /**
   * @brief 将二维空间坐标转换为ENU(East-North-Up)系统下的车辆坐标
   *
   * @param[in]  enu2d 二维空间坐标点(x,y)
   * @param[out] s     车辆向前方的距离，单位为米
   * @param[out] l     车辆向左侧的距离，单位为米
   * @return true 转换成功
   * @return false 转换失败
   */
  virtual Base::txBool xy2sl(const Base::txVec2& enu2d, Base::txFloat& s,
                             Base::txFloat& l) const TX_NOEXCEPT TX_OVERRIDE {
    return false;
  }

 protected:
  Base::txVec3 mLaneDir;
  controlPoint mStartPt;
};
using txLaneSinglePositionInfoPtr = std::shared_ptr<txLaneSinglePositionInfo>;

class txRoadSplineInfo : public txLaneSplineInfo {
 public:
  txRoadSplineInfo(const Base::Info_Road_t& linkRoadId, const controlPointVec& inputControlPoints,
                   const txFloat nLength)
      : txLaneSplineInfo(0, Base::txLaneUId(linkRoadId.fromRoadId, 0, 0), Base::txLaneUId(linkRoadId.toRoadId, 0, 0),
                         inputControlPoints, nLength) {
    m_road_loc_info = linkRoadId;
  }
  virtual ~txRoadSplineInfo() TX_DEFAULT;

  /**
   * @brief 获取当前车道的信息
   *
   * @return Base::Info_Lane_t 返回当前车道的信息，包括起点距离、终点距离等
   *
   */
  virtual Base::Info_Lane_t LaneLocInfo() const TX_NOEXCEPT TX_OVERRIDE {
    LOG(FATAL) << "un-support class type. road info.";
    return m_lane_loc_info;
  }

  /**
   * @brief 获取当前道路的信息
   *
   * @return Base::Info_Road_t 返回当前道路的信息，包括起点距离、终点距离等
   *
   */
  virtual Base::Info_Road_t RoadLocInfo() const TX_NOEXCEPT { return m_road_loc_info; }
};
using txRoadSplineInfoPtr = std::shared_ptr<txRoadSplineInfo>;

class txRoadInfo : public txLaneInfo {
 public:
  txRoadInfo(const Base::Info_Road_t& linkRoadId, controlPointVec inputControlPoints)
      : txLaneInfo(Base::txLaneUId(linkRoadId.fromRoadId, 0, 0), inputControlPoints) {
    m_road_loc_info = linkRoadId;
  }
  virtual ~txRoadInfo() TX_DEFAULT;

  /**
   * @brief 获取当前车道的信息
   *
   * @return Base::Info_Lane_t 返回当前车道的信息，包括起点距离、终点距离等
   *
   */
  virtual Base::Info_Lane_t LaneLocInfo() const TX_NOEXCEPT TX_OVERRIDE {
    LOG(FATAL) << "un-support class type. road info.";
    return m_lane_loc_info;
  }

  /**
   * @brief 获取当前道路的信息
   *
   * @return Base::Info_Road_t 返回当前道路的信息，包括起点距离、终点距离等
   *
   */
  virtual Base::Info_Road_t RoadLocInfo() const TX_NOEXCEPT { return m_road_loc_info; }
};
using txRoadInfoPtr = std::shared_ptr<txRoadInfo>;

class txRoadShortInfo : public txLaneShortInfo {
 public:
  txRoadShortInfo(const Base::Info_Road_t& linkRoadId, const controlPoint& startVertex, const controlPoint& endVertex)
      : txLaneShortInfo(Base::txLaneUId(linkRoadId.fromRoadId, 0, 0), startVertex, endVertex) {
    m_road_loc_info = linkRoadId;
  }
  virtual ~txRoadShortInfo() TX_DEFAULT;

  /**
   * @brief 获取当前车道的信息
   *
   * @return Base::Info_Lane_t 返回当前车道的信息，包括起点距离、终点距离等
   *
   */
  virtual Base::Info_Lane_t LaneLocInfo() const TX_NOEXCEPT TX_OVERRIDE {
    LOG(FATAL) << "un-support class type. road info.";
    return m_lane_loc_info;
  }

  /**
   * @brief 获取当前道路的信息
   *
   * @return Base::Info_Road_t 返回当前道路的信息，包括起点距离、终点距离等
   *
   */
  virtual Base::Info_Road_t RoadLocInfo() const TX_NOEXCEPT { return m_road_loc_info; }
};
using txRoadShortInfoPtr = std::shared_ptr<txRoadShortInfo>;

TX_NAMESPACE_CLOSE(HdMap)
