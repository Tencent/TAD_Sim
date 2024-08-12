// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_sim_point.h"
#include "tx_spatial_query.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class PolyLineTraj {
 public:
  PolyLineTraj() TX_DEFAULT;
  ~PolyLineTraj() TX_DEFAULT;

  /**
   * @brief 根据输入的WayPoints，初始化轨迹
   *
   * @param vecWayPoints 给定的轨迹点输入
   */
  void Init(const std::vector<Coord::txWGS84>& vecWayPoints) TX_NOEXCEPT {
    m_distanceOnTraj = 0.0;
    m_total_length = 0.0;
    m_vecWayPoints = vecWayPoints;
    m_vec_segment_line.clear();
    m_vec_segment_dir.clear();
    for (Base::txSize i = 1; i < m_vecWayPoints.size(); ++i) {
      const Base::txFloat segLen =
          Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(m_vecWayPoints[i - 1], m_vecWayPoints[i]);
      m_total_length += segLen;
      m_vec_segment_line.push_back(m_total_length);

      Base::txVec3 segDir = m_vecWayPoints[i].ToENU().ENU() - m_vecWayPoints[i - 1].ToENU().ENU();
      segDir.normalize();
      m_vec_segment_dir.push_back(segDir);
    }

    std::stringstream ss;
    std::copy(m_vec_segment_line.begin(), m_vec_segment_line.end(), std::ostream_iterator<Base::txFloat>(ss, ", "));
    LOG(WARNING) << TX_VARS(m_total_length) << TX_VARS_NAME(segment_line, ss.str());
  }

  /**
   * @brief Move 移动到指定位置
   * @param step 移动的步长
   * @param idx 返回当前的索引
   * @param vTargetDir 返回目标方向向量
   * @param isStop 是否到达目标位置
   * @return 当前位置坐标
   */
  Coord::txWGS84 Move(const Base::txFloat step, Base::txInt& idx, Base::txVec3& vTargetDir,
                      Base::txBool& isStop) TX_NOEXCEPT {
    isStop = false;
    m_distanceOnTraj += step;
    idx = -1;
    for (Base::txSize i = 0; i < m_vec_segment_line.size(); ++i) {
      if (m_vec_segment_line[i] > m_distanceOnTraj) {
        idx = i;
        vTargetDir = m_vec_segment_dir[idx];
        break;
      }
    }

    if (-1 == idx) {
      if (1 == m_vecWayPoints.size()) {
        double distanceCurve = 0.0;
        Base::Info_Lane_t locInfo;
        Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
            m_vecWayPoints[0], locInfo, distanceCurve);
        if (CallSucc(locInfo.IsOnLane())) {
          vTargetDir = HdMap::HadmapCacheConCurrent::GetLaneDir(locInfo.onLaneUid, distanceCurve);
        } else {
          vTargetDir = HdMap::HadmapCacheConCurrent::GetLaneLinkDir(locInfo, distanceCurve);
          /*distanceCurve = 1.0;
          hadmap::txLanes lanes;
          int32_t opCode = hadmap::getLanes(txMapHdr, hadmap::txPoint(m_vecWayPoints[0].Lon(), m_vecWayPoints[0].Lat(),
          FLAGS_default_altitude), FLAGS_MAP_NearLaneDist, lanes); hadmap::txLanePtr initLaneNear = ((lanes.size()) > 0
          ? lanes.front() : nullptr); if (initLaneNear) {
              vTargetDir = HdMap::HadmapCacheConCurrent::GetLaneDir(initLaneNear, distanceCurve);
          } else {
              LOG(WARNING) << ", GetLane Failure. Pos = " << (m_vecWayPoints[0]) << TX_VARS(opCode);
              vTargetDir = Base::txVec3(1, 0, 0);
          }*/
        }
        isStop = true;
        return m_vecWayPoints[idx + 1];
      } else {
        idx = m_vec_segment_line.size() - 1;
        vTargetDir = m_vec_segment_dir[idx];
        isStop = true;
        return m_vecWayPoints[idx + 1];
      }
    } else {
      Coord::txWGS84 seg_start = m_vecWayPoints[idx];
      Base::txFloat len_on_seg = m_distanceOnTraj;
      if (idx > 0) {
        len_on_seg = m_distanceOnTraj - m_vec_segment_line[idx - 1];
      }
      seg_start.TranslateLocalPos(len_on_seg * __East__(vTargetDir), len_on_seg * __North__(vTargetDir));
      return seg_start;
    }
  }

  /**
   * @brief Move 移动到指定位置
   * @param step 移动的步长
   * @param idx 返回当前的索引
   * @param vTargetDir 返回目标方向向量
   * @param isStop 是否到达目标位置
   * @return 当前位置坐标
   */
  std::vector<Coord::txWGS84> GetRawWayPoints() const TX_NOEXCEPT { return m_vecWayPoints; }

 protected:
  std::vector<Coord::txWGS84> m_vecWayPoints;
  std::vector<Base::txFloat> m_vec_segment_line;
  std::vector<Base::txVec3> m_vec_segment_dir;
  Base::txFloat m_distanceOnTraj = 0.0;
  Base::txFloat m_total_length = 0.0;
};
TX_NAMESPACE_CLOSE(TrafficFlow)
