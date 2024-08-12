// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_lane_geom_info.h"
#include <Eigen/Core>
#include <unsupported/Eigen/Splines>
#include "tx_algebra_defs.h"
#include "tx_cat_mull_rom.h"
#include "tx_flags.h"
#include "tx_hadmap_utils.h"
#include "tx_math.h"
#include "tx_type_def.h"
#include "tx_units.h"

#define GeomShapeInfo LOG_IF(INFO, FLAGS_LogLevel_GeomShape)

TX_NAMESPACE_OPEN(HdMap)

// 计算车道类型
txLaneInfoInterface::LaneShape ComputeLaneType(const Base::Info_Lane_t& laneLocInfo, const Base::txFloat _len,
                                               const Base::txVec3& startDir, const Base::txVec3& endDir) TX_NOEXCEPT {
  // 如果车道长度大于0
  if (_len > 0) {
    // 计算车道起始方向的弧度值
    const Base::txFloat _zeroRadian = Utils::GetLaneAngleFromVectorOnENU(startDir).GetRadian();
    // 创建一个圆形弧度对象
    Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::PI(), _zeroRadian + Unit::CircleRadian::PI(),
                               _zeroRadian);

    // 计算车道结束方向的弧度值
    const Base::txFloat _endRadian = Utils::GetLaneAngleFromVectorOnENU(endDir).GetRadian();
    // 计算包装后的弧度值
    const Base::txFloat wrap_headingRadian = circRad.Wrap(_endRadian);

    // 计算弧度差值
    const Base::txFloat diff = (wrap_headingRadian - _zeroRadian);
    // 计算弧度差值的绝对值
    const Base::txFloat diff_abs = std::fabs(diff);

    // 计算直线和弧线90度的阈值
    const Base::txFloat line2Arc90 = Math::PI * FLAGS_GeomShapeFactor1;
    // 计算弧线90度和转向的阈值
    const Base::txFloat Arc2Turn = Math::PI * FLAGS_GeomShapeFactor2;
    GeomShapeInfo << laneLocInfo << TX_VARS_NAME(beginDeg, Math::Radians2Degrees(_zeroRadian))
                  << TX_VARS_NAME(endDeg, Math::Radians2Degrees(_endRadian))
                  << TX_VARS_NAME(wrapDeg, Math::Radians2Degrees(wrap_headingRadian)) << TX_VARS(diff);
    // 根据弧度差值判断车道类型
    if (diff_abs < line2Arc90) {
      return txLaneInfoInterface::LaneShape::sLine;
    } else if (diff_abs < Arc2Turn) {
      if (diff > 0.0) {
        return txLaneInfoInterface::LaneShape::sArc90_L;
      } else {
        return txLaneInfoInterface::LaneShape::sArc90_R;
      }
    } else {
      return txLaneInfoInterface::LaneShape::sArc180;
    }
  } else {  // 如果车道长度小于等于0，返回直线类型
    return txLaneInfoInterface::LaneShape::sLine;
  }
}

struct txLaneSplineInfo::txLaneSplineInfoImpl : public txLaneProjectionUtils {
  enum { n_dim = 2 };
  using txInt = Base::txInt;
  using Spline2d = Eigen::Spline<txFloat, 2>;
  using PointType = Spline2d::PointType;
  using KnotVectorType = Spline2d::KnotVectorType;
  using ControlPointVectorType = Spline2d::ControlPointVectorType;
  using Spline2dFitting = Eigen::SplineFitting<Spline2d>;

 public:
  txLaneSplineInfoImpl(const Base::Info_Lane_t laneLocInfo, const controlPointVec& inputControlPoints,
                       const txFloat nLength)
      : txLaneProjectionUtils(inputControlPoints), m_nLength(nLength) {
    // 获取控制点数量
    const txInt nControlPtSize = inputControlPoints.size();
    // 创建一个ControlPointVectorType对象
    ControlPointVectorType points;  // = ControlPointVectorType::Random(2, 100);
    points.resize(n_dim, nControlPtSize);

    // 将输入的控制点转换为points对象
    for (txInt i = 0; i < nControlPtSize; ++i) {
      points.coeffRef(0, i) = inputControlPoints[i][0];
      points.coeffRef(1, i) = inputControlPoints[i][1];
    }

    // 使用控制点拟合一个三次样条曲线
    m_cubicSpline2d = Spline2dFitting::Interpolate(points, 3);
    // 计算控制点之间的弦长
    Eigen::ChordLengths(points, chord_lengths);

    // 遍历控制点，计算每个控制点在三次样条曲线上的对应点
    for (Eigen::DenseIndex i = 0; i < points.cols(); ++i) {
      PointType pt = m_cubicSpline2d(chord_lengths(i));
      PointType ref = points.col(i);
    }

    // 计算车道起始方向
    const Base::txVec3 startDir = GetLaneDir(0.1);
    // 计算车道结束方向
    const Base::txVec3 endDir = GetLaneDir(m_nLength - 0.1);
    m_laneShape = ComputeLaneType(laneLocInfo, m_nLength, startDir, endDir);
  }

  virtual ~txLaneSplineInfoImpl() TX_DEFAULT;

  Base::txVec3 GetLaneDir(const Base::txFloat _s) const TX_NOEXCEPT {
    // 定义一个步长，用于计算车道方向
#define dir_step (0.01)
    // 初始化一个因子变量
    txFloat factor = 0.0;
    // 初始化一个因子变量
    txFloat factor_1 = 0.0;
    txFloat sign = 1.0;
    // 如果输入的_s大于等于车道长度，将因子设置为1.0
    if (_s >= m_nLength) {
      factor = 1.0;
    } else {
      factor = _s / m_nLength;
    }

    // 如果因子小于1.0
    if (factor < 1.0) {
      factor_1 = factor + dir_step;
      // 如果因子_1大于等于1.0，将因子_1设置为1.0
      if (factor_1 >= 1.0) {
        factor_1 = 1.0;
      }
    } else {
      // 否则，将因子_1设置为因子减去步长，并将符号设置为-1.0
      factor_1 = factor - dir_step;
      sign = -1.0;
    }
#undef dir_step

    // 计算因子对应的点
    PointType pt = m_cubicSpline2d(factor);
    // 计算因子_1对应的点
    PointType pt_1 = m_cubicSpline2d(factor_1);
    // 创建两个txVec3对象，分别表示因子对应的点和因子_1对应的点
    Base::txVec3 vtx_0(pt[0], pt[1], FLAGS_default_altitude);
    Base::txVec3 vtx_1(pt_1[0], pt_1[1], FLAGS_default_altitude);
    // 计算车道方向
    Base::txVec3 LaneLinkDir(vtx_1 - vtx_0);
    LaneLinkDir.normalize();
    return LaneLinkDir * sign;
  }

  // 计算车道方向
  Base::txVec3 GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT {
    // 初始化一个因子变量
    txFloat factor = 0.0;
    // 如果输入的_s大于等于车道长度，将因子设置为1.0
    if (_s >= m_nLength) {
      factor = 1.0;
    } else {
      factor = _s / m_nLength;
    }
    // 计算因子对应的点
    PointType pt = m_cubicSpline2d(factor);
    return Base::txVec3(pt[0], pt[1], FLAGS_default_altitude);
  }

  Base::txVec3 GetStartPt() const TX_NOEXCEPT { return GetLocalPos(0.0); }

  Base::txVec3 GetEndPt() const TX_NOEXCEPT { return GetLocalPos(m_nLength); }

  Base::txFloat GetLength() const TX_NOEXCEPT { return m_nLength; }

  txLaneSplineInfo::LaneShape GetShape() const TX_NOEXCEPT { return m_laneShape; }

 protected:
  Spline2d m_cubicSpline2d;
  KnotVectorType chord_lengths;
  TX_MARK("knot parameters");
  const txFloat m_nLength;
  txLaneInfoInterface::LaneShape m_laneShape;
};

txLaneSplineInfo::txLaneSplineInfo(const Base::txLaneLinkID lanelinkUid, const Base::txLaneUId& fromLaneUid,
                                   const Base::txLaneUId& toLaneUid, const controlPointVec& inputControlPoints,
                                   const txFloat nLength)
    : txLaneInfoInterface(CurveType::tLink),
      pImpl(std::make_shared<txLaneSplineInfo::txLaneSplineInfoImpl>(
          Base::Info_Lane_t(lanelinkUid, fromLaneUid, toLaneUid), inputControlPoints, nLength)) {
  m_lane_loc_info.FromLaneLink(lanelinkUid, fromLaneUid, toLaneUid);
}

Base::txVec3 txLaneSplineInfo::GetLaneDir(const Base::txFloat _s) const TX_NOEXCEPT { return pImpl->GetLaneDir(_s); }

Base::txVec3 txLaneSplineInfo::GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT { return pImpl->GetLocalPos(_s); }

txLaneSplineInfo::LaneShape txLaneSplineInfo::GetShape() const TX_NOEXCEPT { return pImpl->GetShape(); }

Base::txVec3 txLaneSplineInfo::GetStartPt() const TX_NOEXCEPT { return pImpl->GetStartPt(); }

Base::txVec3 txLaneSplineInfo::GetEndPt() const TX_NOEXCEPT { return pImpl->GetEndPt(); }

Base::txFloat txLaneSplineInfo::GetLength() const TX_NOEXCEPT { return pImpl->GetLength(); }

Base::txBool txLaneSplineInfo::xy2sl(const Base::txVec2& enu2d, Base::txFloat& s, Base::txFloat& l) const TX_NOEXCEPT {
  return pImpl->xy2sl_func(enu2d, s, l);
}

struct txLaneInfo::txLaneInfoImpl : public txLaneProjectionUtils {
 public:
  using catmullRom2d = Geometry::catmull_rom<controlPoint>;

 public:
  txLaneInfoImpl(const Base::Info_Lane_t laneLocInfo, controlPointVec inputControlPoints)
      : txLaneProjectionUtils(inputControlPoints), m_catmullRom2d(std::move(inputControlPoints), false, 1.0) {
    // 获取车道长度
    const auto nLength = m_catmullRom2d.max_parameter();

    // 计算车道起始方向
    const Base::txVec3 startDir = GetLaneDir(0.1);
    // 计算车道结束方向
    const Base::txVec3 endDir = GetLaneDir(nLength - 0.1);
    m_laneShape = ComputeLaneType(laneLocInfo, nLength, startDir, endDir);
  }
  virtual ~txLaneInfoImpl() TX_DEFAULT;
  Base::txVec3 GetLaneDir(const Base::txFloat s) const TX_NOEXCEPT {
    // 初始化一个切线向量
    controlPoint tangent;
    try {
      // 如果输入的s与车道长度的差值小于1.0，将s设置为车道长度减去0.5
      if (std::fabs(m_catmullRom2d.max_parameter() - s) < 1.0) {
        tangent = m_catmullRom2d.prime(m_catmullRom2d.max_parameter() - 0.5);
      } else {
        tangent = m_catmullRom2d.prime(s);
      }
    } catch (const std::exception& e) {
      // 如果计算过程中出现异常，将切线向量设置为(0, 1)，并输出异常信息
      tangent[0] = 0.0;
      tangent[1] = 1.0;
      LOG(WARNING) << (e.what());
    }

    // 创建一个txVec3对象，表示车道方向
    Base::txVec3 LaneDir(tangent[0], tangent[1], FLAGS_default_altitude);
    // 归一化车道方向
    LaneDir.normalize();
    return LaneDir;
  }

  // 获取局部坐标
  Base::txVec3 GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT {
    controlPoint tangent;
    try {
      // 如果输入的_s大于车道长度，将切线向量设置为车道长度对应的点
      if (_s > m_catmullRom2d.max_parameter()) {
        tangent = m_catmullRom2d(m_catmullRom2d.max_parameter());
      } else {
        // 否则，计算_s对应的切线向量
        tangent = m_catmullRom2d(_s);
      }
    } catch (const std::exception& e) {
      // 如果计算过程中出现异常，将切线向量设置为(0, 0)，并输出异常信息
      tangent[0] = 0.0;
      tangent[1] = 0.0;
      LOG(WARNING) << (e.what());
    }
    // 创建一个txVec3对象，表示局部坐标
    return Base::txVec3(tangent[0], tangent[1], FLAGS_default_altitude);
  }

  txLaneSplineInfo::LaneShape GetShape() const TX_NOEXCEPT { return m_laneShape; }

  Base::txVec3 GetStartPt() const TX_NOEXCEPT { return GetLocalPos(0.0); }

  Base::txVec3 GetEndPt() const TX_NOEXCEPT { return GetLocalPos(GetLength()); }

  Base::txFloat GetLength() const TX_NOEXCEPT { return m_catmullRom2d.max_parameter(); }

  Base::txFloat ParameterAtPoint(const size_t idx) const TX_NOEXCEPT { return m_catmullRom2d.parameter_at_point(idx); }

 protected:
  const catmullRom2d m_catmullRom2d;
  txLaneInfoInterface::LaneShape m_laneShape;
  Base::txFloat m_Length;
};

txLaneInfo::txLaneInfo(const Base::txLaneUId laneUid, controlPointVec inputControlPoints)
    : txLaneInfoInterface(CurveType::tLane),
      pImpl(std::make_shared<txLaneInfo::txLaneInfoImpl>(static_cast<Base::Info_Lane_t>(laneUid), inputControlPoints)) {
  m_lane_loc_info.FromLane(laneUid);
}

Base::txVec3 txLaneInfo::GetLaneDir(const Base::txFloat _s) const TX_NOEXCEPT { return pImpl->GetLaneDir(_s); }
Base::txVec3 txLaneInfo::GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT { return pImpl->GetLocalPos(_s); }

txLaneInfo::LaneShape txLaneInfo::GetShape() const TX_NOEXCEPT { return pImpl->GetShape(); }

Base::txVec3 txLaneInfo::GetStartPt() const TX_NOEXCEPT { return pImpl->GetStartPt(); }

Base::txVec3 txLaneInfo::GetEndPt() const TX_NOEXCEPT { return pImpl->GetEndPt(); }

Base::txFloat txLaneInfo::GetLength() const TX_NOEXCEPT { return pImpl->GetLength(); }

Base::txFloat txLaneInfo::ParameterAtPoint(const size_t idx) const TX_NOEXCEPT { return pImpl->ParameterAtPoint(idx); }

Base::txBool txLaneInfo::xy2sl(const Base::txVec2& enu2d, Base::txFloat& s, Base::txFloat& l) const TX_NOEXCEPT {
  return pImpl->xy2sl_func(enu2d, s, l);
}

struct txLaneShortInfo::txLaneShortInfoImpl : public txLaneProjectionUtils {
 public:
  txLaneShortInfoImpl(const Base::txLaneUId laneUid, const controlPoint& startVertex, const controlPoint& endVertex)
      : txLaneProjectionUtils({startVertex, endVertex}), m_LaneUid(laneUid), m_LaneLinkUid(0) {
    // 设置车道起始点
    m_StartPt = Base::txVec3(startVertex[0], startVertex[1], FLAGS_default_altitude);
    // 设置车道结束点
    m_EndPt = Base::txVec3(endVertex[0], endVertex[1], FLAGS_default_altitude);
    // 计算车道方向
    m_LaneDir = (m_EndPt - m_StartPt);
    // 计算车道长度
    m_nLength = (m_EndPt - m_StartPt).norm();
    m_LaneDir.normalize();
  }
  txLaneShortInfoImpl(const Base::txLaneLinkID lanelinkUid, const controlPoint& startVertex,
                      const controlPoint& endVertex)
      : txLaneProjectionUtils({startVertex, endVertex}), m_LaneLinkUid(lanelinkUid), m_LaneUid(0, 0, 0) {
    // 设置车道起始点
    m_StartPt = Base::txVec3(startVertex[0], startVertex[1], FLAGS_default_altitude);
    // 设置车道结束点
    m_EndPt = Base::txVec3(endVertex[0], endVertex[1], FLAGS_default_altitude);
    // 计算车道方向
    m_LaneDir = (m_EndPt - m_StartPt);
    // 计算车道长度
    m_nLength = (m_EndPt - m_StartPt).norm();
    m_LaneDir.normalize();
  }
  virtual ~txLaneShortInfoImpl() TX_DEFAULT;
  Base::txVec3 GetLaneDir(const Base::txFloat) const TX_NOEXCEPT { return m_LaneDir; }

  Base::txVec3 GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT {
    // 如果输入的_s大于等于车道长度或车道长度为0，返回车道结束点
    if (_s >= m_nLength || Math::isZero(m_nLength)) {
      return m_EndPt;
    } else {
      return m_StartPt + (m_EndPt - m_StartPt) * (_s / m_nLength);
    }
  }

  Base::txVec3 GetStartPt() const TX_NOEXCEPT { return m_StartPt; }

  Base::txVec3 GetEndPt() const TX_NOEXCEPT { return m_EndPt; }

  Base::txFloat GetLength() const TX_NOEXCEPT { return m_nLength; }

 protected:
  Base::txVec3 m_LaneDir;
  Base::txVec3 m_StartPt;
  Base::txVec3 m_EndPt;
  Base::txFloat m_nLength;
  const Base::txLaneUId m_LaneUid;
  const Base::txLaneLinkID m_LaneLinkUid;
};

txLaneShortInfo::txLaneShortInfo(const Base::txLaneUId laneUid, const controlPoint& startVertex,
                                 const controlPoint& endVertex)
    : txLaneInfoInterface(CurveType::tLane),
      pImpl(std::make_shared<txLaneShortInfo::txLaneShortInfoImpl>(laneUid, startVertex, endVertex)) {
  m_lane_loc_info.FromLane(laneUid);
}

txLaneShortInfo::txLaneShortInfo(const Base::txLaneLinkID lanelinkUid, const Base::txLaneUId& fromLaneUid,
                                 const Base::txLaneUId& toLaneUid, const controlPoint& startVertex,
                                 const controlPoint& endVertex)
    : txLaneInfoInterface(CurveType::tLink),
      pImpl(std::make_shared<txLaneShortInfo::txLaneShortInfoImpl>(lanelinkUid, startVertex, endVertex)) {
  m_lane_loc_info.FromLaneLink(lanelinkUid, fromLaneUid, toLaneUid);
}

Base::txVec3 txLaneShortInfo::GetLaneDir(const Base::txFloat _unuse) const TX_NOEXCEPT {
  return pImpl->GetLaneDir(_unuse);
}

Base::txVec3 txLaneShortInfo::GetLocalPos(const Base::txFloat _s) const TX_NOEXCEPT { return pImpl->GetLocalPos(_s); }

Base::txVec3 txLaneShortInfo::GetStartPt() const TX_NOEXCEPT { return pImpl->GetStartPt(); }

Base::txVec3 txLaneShortInfo::GetEndPt() const TX_NOEXCEPT { return pImpl->GetEndPt(); }

Base::txFloat txLaneShortInfo::GetLength() const TX_NOEXCEPT { return pImpl->GetLength(); }

Base::txBool txLaneShortInfo::xy2sl(const Base::txVec2& enu2d, Base::txFloat& s, Base::txFloat& l) const TX_NOEXCEPT {
  return pImpl->xy2sl_func(enu2d, s, l);
}

TX_NAMESPACE_CLOSE(HdMap)

#undef _ct_x_
#undef _ct_y_
