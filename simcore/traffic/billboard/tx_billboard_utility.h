// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_locate_info.h"

TX_NAMESPACE_OPEN(Utils)

inline Base::txBool UnInitialize(const Base::Info_Lane_t _uid) TX_NOEXCEPT {
  return (CallFail(_uid.isOnLaneLink)) && (0 == _uid.onLaneUid.laneId) && (0 == _uid.onLinkFromLaneUid.laneId) &&
         (0 == _uid.onLinkToLaneUid.laneId);
}

TX_NAMESPACE_CLOSE(Utils)
