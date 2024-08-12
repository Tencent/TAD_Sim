// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_dead_line_area_manager.h"
#include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(Scene)
#if 0

class Cloud_DeadLineAreaManager : public Base::DeadLineAreaManager {
 public:
  virtual Base::txFloat DeadLineLength() const TX_NOEXCEPT TX_OVERRIDE { return FLAGS_tc_exit_length; }
  virtual Base::txBool ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid, const Base::txVec3& elemDir,
                                            const Coord::txENU& elementPt) const TX_NOEXCEPT TX_OVERRIDE;
};
#endif

TX_NAMESPACE_CLOSE(Scene)
