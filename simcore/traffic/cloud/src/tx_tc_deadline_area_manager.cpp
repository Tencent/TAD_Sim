// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_deadline_area_manager.h"
#if 0

#  include "HdMap/txHdMapCacheConcurrent.h"
#  include "tx_spatial_query.h"
#  include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(Scene)

Base::txFloat Cloud_DeadLineAreaManager::DeadLineLength() const TX_NOEXCEPT { return FLAGS_tc_exit_length; }

Base::txBool Cloud_DeadLineAreaManager::ArriveAtDeadlineArea(
    const Base::txLaneUId& elemLaneUid, const Base::txVec3& elemDir,
    const Coord::txENU& elementPt) const TX_NOEXCEPT {
    if (IsValid()) {
        if (elemLaneUid.roadId == _deadline_LaneUid.roadId &&
            elemLaneUid.sectionId == _deadline_LaneUid.sectionId &&
            elemLaneUid.laneId == _deadline_LaneUid.laneId) {
            return true;
        }

        if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(m_vLaneDir_Deadline), Utils::Vec3_Vec2(elemDir))) &&
            Coord::txENU::EuclideanDistanceLocal_2D(elementPt, _EndPos) < DeadLineLength()) {
            return true;
        }
    }
    return false;
}

TX_NAMESPACE_CLOSE(Scene)
#endif
