// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <sstream>
#include <string>
#include "tx_od_marco.h"

TX_NAMESPACE_OPEN(Base)

#if UseDouble
using txFloat = double;
#else
using txFloat = float;
#endif
using txString = std::string;
using txStringStream = std::stringstream;
using txInt = int;

using txSignedRoadID = int64_t;
using txDiff_t = long long;  // NOLINT
using txSize = std::size_t;
using txUInt = unsigned int;
using txUInt32 = uint32_t;
using txSysId = int64_t;
using txUShort = unsigned short;  // NOLINT
using txUByte = unsigned char;
using txUChar = unsigned char;
using txULong = unsigned long;  // NOLINT
using txLpsz = char const*;
using txBool = bool;
using txSessionId = int32_t;

using txLaneID = txInt;
using txLaneLinkID = int64_t;
using txSectionID = txUShort;
using txRoadID = int64_t;
using txBoundaryID = int64_t;
using txSectionUId = std::tuple<txRoadID, txSectionID>;
using txLaneUId = std::tuple<txRoadID, txSectionID, txLaneID>;
TX_NAMESPACE_CLOSE(Base)
