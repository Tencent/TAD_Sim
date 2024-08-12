// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_centripetal_cat_mull.h"
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

HdMap::txLaneInfoInterface::controlPointVec CentripetalCatMull::to_controlPointVec(
    const control_path_node_vec& samplepoints) TX_NOEXCEPT {
  controlPointVec move_ret;
  for (const auto& gps : samplepoints) {
    Coord::txENU enu;
    enu.FromWGS84(gps.waypoint);
    move_ret.emplace_back(controlPointVec::value_type{enu.X(), enu.Y()});
  }
  return std::move(move_ret);
}

CentripetalCatMull::CentripetalCatMull(const control_path_node_vec& samplepoints)
    : txLaneProjectionUtils(to_controlPointVec(samplepoints)) {
  for (const auto& gps : samplepoints) {
    Coord::txENU enu;
    enu.FromWGS84(gps.waypoint);
    m_sample_points.emplace_back(Base::txVec2(enu.X(), enu.Y()));
    m_sample_locInfos.emplace_back(std::make_tuple(gps.locInfo, gps.dist_on_s));
  }
  m_real_lenth = 0.0;
  for (Base::txInt i = 1; i < m_sample_points.size(); ++i) {
    auto curSeg = std::make_tuple(m_real_lenth, 0.0);
    m_real_lenth += (m_sample_points[i] - m_sample_points[i - 1]).norm();
    std::get<1>(curSeg) = m_real_lenth;
    m_vec_segment_lookup.emplace_back(curSeg);
  }
  const Base::txFloat curveLength = GetLength();
  Base::txFloat s = 0.0;
  m_points4xysl.clear();
  while (s < curveLength) {
    const Base::txVec3 enu3d = GetLocalPos(s);
    m_points4xysl.emplace_back(controlPointVec::value_type{enu3d.x(), enu3d.y()});
    s += 1.0;
  }
  const Base::txVec3 enu3d = GetLocalPos(curveLength);
  m_points4xysl.emplace_back(controlPointVec::value_type{enu3d.x(), enu3d.y()});
#if 1
  m_segment_length_list.resize(m_points4xysl.size());
  m_segment_length_list[0] = 0.0;
  for (std::size_t i = 1; i < m_points4xysl.size(); ++i) {
    m_segment_length_list[i] =
        m_segment_length_list[i - 1] + std::sqrt(sqrlen(sub(m_points4xysl[i], m_points4xysl[i - 1])));
  }
#endif
}

Base::txBool CentripetalCatMull::GetLocInfo(const Base::txInt idx, Base::Info_Lane_t& locInfo,
                                            Base::txFloat& s) const TX_NOEXCEPT {
  if (idx < m_sample_locInfos.size()) {
    locInfo = std::get<0>(m_sample_locInfos[idx]);
    s = std::get<1>(m_sample_locInfos[idx]);
    return true;
  } else {
    return false;
  }
}

Base::txInt CentripetalCatMull::LocateIndexByDistance(const Base::txFloat dist) const TX_NOEXCEPT {
  if (dist >= GetLength()) {
    return m_sample_points.size() - 1;
  } else if (dist < 0.0) {
    return 0;
  } else {
    for (Base::txInt i = 0; i < m_vec_segment_lookup.size(); ++i) {
      if (std::get<0>(m_vec_segment_lookup[i]) <= dist && dist < std::get<1>(m_vec_segment_lookup[i])) {
        return i;
      }
    }
    return m_sample_points.size() - 1;
  }
}

std::tuple<Base::txVec2, Base::txVec2, Base::txVec2, Base::txVec2> CentripetalCatMull::MakeControlPoint(
    const Base::txInt p1_idx, const Base::txInt p2_idx) const TX_NOEXCEPT {
  const Base::txVec2 p1 = m_sample_points[p1_idx];
  const Base::txVec2 p2 = m_sample_points[p2_idx];
  Base::txVec2 p0, p3;
  if (0 == p1_idx) {
    p0 = (p1 - p2) + p1;
    TX_MARK("extrapolate first point");
  } else {
    p0 = m_sample_points[p1_idx - 1];
  }

  if ((m_sample_points.size() - 1) == p2_idx) {
    p3 = (p2 - p1) + p2;
    TX_MARK("extrapolate last point");
  } else {
    p3 = m_sample_points[p2_idx + 1];
  }

  return std::make_tuple(p0, p1, p2, p3);
}
Base::txVec3 CentripetalCatMull::GetLocalPos(const Base::txFloat move_dist) const TX_NOEXCEPT {
  if (move_dist >= GetLength()) {
    return GetEndPt();
  }
  const Base::txFloat alpha = FLAGS_CentripetalCatMull_Alpha;
  const Base::txFloat tension = FLAGS_CentripetalCatMull_Tension;
  const Base::txInt segmentIdx = LocateIndexByDistance(move_dist);
  // LOG(INFO) << TX_VARS(segmentIdx) << TX_VARS(move_dist);
  Base::txVec2 p0, p1, p2, p3;
  std::tie(p0, p1, p2, p3) = MakeControlPoint(segmentIdx, segmentIdx + 1);

  const Base::txFloat t0 = 0.0f;
  const Base::txFloat t1 = t0 + pow(Math::Distance(p0, p1), alpha);
  const Base::txFloat t2 = t1 + pow(Math::Distance(p1, p2), alpha);
  const Base::txFloat t3 = t2 + pow(Math::Distance(p2, p3), alpha);

  const Base::txVec2 m1 =
      (1.0 - tension) * (t2 - t1) * ((p1 - p0) / (t1 - t0) - (p2 - p0) / (t2 - t0) + (p2 - p1) / (t2 - t1));
  const Base::txVec2 m2 =
      (1.0f - tension) * (t2 - t1) * ((p2 - p1) / (t2 - t1) - (p3 - p1) / (t3 - t1) + (p3 - p2) / (t3 - t2));

  const Base::txVec2 segment_a = 2.0f * (p1 - p2) + m1 + m2;
  const Base::txVec2 segment_b = -3.0f * (p1 - p2) - m1 - m1 - m2;
  const Base::txVec2 segment_c = m1;
  const Base::txVec2 segment_d = p1;

  auto pa = m_vec_segment_lookup[segmentIdx];
  const Base::txFloat t = (move_dist - std::get<0>(pa)) / (std::get<1>(pa) - std::get<0>(pa));
  return Utils::Vec2_Vec3(segment_a * t * t * t + segment_b * t * t + segment_c * t + segment_d);
}

Base::txVec3 CentripetalCatMull::GetLaneDir(const Base::txFloat move_dist) const TX_NOEXCEPT {
  Base::txFloat s = move_dist;
  if (s < 0.0) {
    s = 0.0;
  }
  if (s > GetLength()) {
    s = GetLength();
  }

  if (move_dist + 0.05 < GetLength()) {
    Base::txVec3 dir = (GetLocalPos(move_dist + 0.05) - GetLocalPos(move_dist));
    dir.normalize();
    return dir;
  } else {
    Base::txVec3 dir = (GetLocalPos(GetLength()) - GetLocalPos(GetLength() - 0.05));
    dir.normalize();
    return dir;
  }
}

Base::txFloat CentripetalCatMull::ParameterAtPoint(const size_t idx) const TX_NOEXCEPT {
  return std::get<1>(m_vec_segment_lookup[idx - 1]);
}

HdMap::txLaneInfoInterfacePtr CentripetalCatMull::GenerateParallelSpline(
    const Base::Enums::VehicleLaneChangeType _dir, const Base::txFloat _offset) const TX_NOEXCEPT {
  return nullptr;
  /*if (_offset > 0) {
  } else {
          LogWarn << "GenerateParallelSpline failure. " << TX_VARS(_offset);
          return nullptr;
  }*/
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogWarn
