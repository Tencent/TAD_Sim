// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_sim_point.h"
TX_NAMESPACE_OPEN(Base)
class TrajectorySamplingNode {
 public:
  TrajectorySamplingNode() TX_DEFAULT;
  TrajectorySamplingNode(const Base::Info_Lane_t _loc, const Base::txFloat _s, const Base::txFloat _offset,
                         const Base::txBool fistSegPt = false)
      : mLaneInfo(_loc), mS(_s), mlOffset(_offset), mSegmentStartPt(fistSegPt) {}

  /**
   * @brief ComputeSamplingPoint 生成车道上的采样点
   *
   * 根据给定的车道 ID 和车道上的位置，在当前车道上计算一个合适的采样点。
   *
   * @param mLaneInfo 车道 ID
   * @param mS 在车道上的位置
   * @param mlOffset 车道偏移量
   * @param mPt 生成的采样点
   * @param mIsValid 采样点是否有效
   * @return 采样点是否有效
   */
  Base::txBool ComputeSamplingPoint() TX_NOEXCEPT {
    auto geomPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(mLaneInfo);
    if (NonNull_Pointer(geomPtr)) {
      Base::txVec3 pos = geomPtr->GetLocalPos(mS);
      Coord::txWGS84 pt;
      pt.FromENU(pos);
      Base::txVec3 tmpLaneDir = geomPtr->GetLaneDir(mS);
      mPt = Coord::ComputeLaneOffset(pt, tmpLaneDir, mlOffset).ToENU();
      mIsValid = true;
    }
    return IsValid();
  }

  /**
   * @brief txTrajectorySamplingNode::SamplingPt 获取抽样点的位置
   *
   * 获取在抽样过程中所得到的抽样点的位置坐标。
   *
   * @return Coord::txENU 返回抽样点的坐标
   */
  Coord::txENU SamplingPt() const TX_NOEXCEPT { return mPt; }

  /**
   * @brief IsValid 判断当前轨迹节点是否合法
   *
   * 判断当前轨迹节点是否具有有效性。通常情况下，如果轨迹节点无法成功构造，则其合法性被认为是无效的。
   *
   * @return txBool 如果当前轨迹节点合法，则返回 txTrue，否则返回 txFalse。
   */
  const Base::txBool IsValid() const TX_NOEXCEPT { return mIsValid; }

  /**
   * @brief txTrajectorySamplingNode 获取当前轨迹节点的字符串表示
   *
   * 这个函数将返回当前轨迹节点的一个字符串表示，该字符串包含了该节点所在的车道信息、节点在当前轨迹中的位置以及节点在WGS84坐标系下的坐标信息。
   *
   * @return txString 返回一个包含当前轨迹节点信息的字符串表示
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << TX_VARS(mLaneInfo) << TX_VARS(mS) << TX_VARS(SamplingPt().StrWGS84());
    return oss.str();
  }

  /**
   * @brief 获取当前轨迹节点的车道信息
   *
   * @return 当前轨迹节点的车道信息
   */
  Base::Info_Lane_t LaneInfo() const TX_NOEXCEPT { return mLaneInfo; }
  Base::txFloat S() const TX_NOEXCEPT { return mS; }

  /**
   * @brief 检查第一个轨迹点是否是一个段开始点
   *
   * @return 如果第一个轨迹点是一个段开始点，返回true，否则返回false
   */
  Base::txBool FirstSegmentPt() const TX_NOEXCEPT { return mSegmentStartPt; }
  void ClearLaneOffset() TX_NOEXCEPT { mlOffset = 0.0; }

 protected:
  Base::Info_Lane_t mLaneInfo;
  Base::txFloat mS = 0.0;
  Base::txBool mIsValid = false;
  Coord::txENU mPt;
  Base::txFloat mlOffset = 0.0;
  Base::txBool mSegmentStartPt = false;
};
TX_NAMESPACE_CLOSE(Base)
