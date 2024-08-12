// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_locate_info.h"

TX_NAMESPACE_OPEN(Base)

std::atomic<txSize> JointPointTreeNode::sCounter(100);

JointPointId_t JointPointId_t::MakeJointStartPointId(const Base::txLaneUId curLaneUid) TX_NOEXCEPT {
  JointPointId_t retV;
  retV.isStartPt = true;
  retV.locInfo.FromLane(curLaneUid);
  return retV;
}

JointPointId_t JointPointId_t::MakeJointEndPointId(const Base::txLaneUId curLaneUid) TX_NOEXCEPT {
  JointPointId_t retV;
  retV.isStartPt = false;
  retV.locInfo.FromLane(curLaneUid);
  return retV;
}

JointPointId_t JointPointId_t::MakeJointStartPointId(const Base::txLaneLinkID curLaneLinkId,
                                                     const Base::txLaneUId& fromLaneUid,
                                                     const Base::txLaneUId& toLaneUid) TX_NOEXCEPT {
  JointPointId_t retV;
  retV.isStartPt = true;
  retV.locInfo.FromLaneLink(curLaneLinkId, fromLaneUid, toLaneUid);
  return retV;
}

JointPointId_t JointPointId_t::MakeJointEndPointId(const Base::txLaneLinkID curLaneLinkId,
                                                   const Base::txLaneUId& fromLaneUid,
                                                   const Base::txLaneUId& toLaneUid) TX_NOEXCEPT {
  JointPointId_t retV;
  retV.isStartPt = false;
  retV.locInfo.FromLaneLink(curLaneLinkId, fromLaneUid, toLaneUid);
  return retV;
}

bool Info_Lane_t::operator<(const Info_Lane_t& o) const TX_NOEXCEPT {
  return Utils::Info_Lane_t_HashCompare::hash(*this) < Utils::Info_Lane_t_HashCompare::hash(o);
}
TX_NAMESPACE_CLOSE(Base)
